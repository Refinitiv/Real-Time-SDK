/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license
 *| AS IS with no warranty or guarantee of fit for purpose.
 *| See LICENSE.md for details.
 *| Copyright (C) 2019 LSEG. All rights reserved.     
 *|-------------------------------------------------------------------------------
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
 * connects to LSEG Real-Time Distribution System and sends a specific
 * set (non-interactive) of information (services, domains, and capabilities).
 * NIPs act like clients in a client-server relationship. Multiple NIPs can
 * connect to the same LSEG Real-Time Distribution System and publish
 * the same items and content.
 *
 * In this module, the OMM NIP application initializes the ETA Transport
 * and establish a connection to an ADH server. Once connected, an OMM NIP
 * can publish information into the ADH cache without needing to handle
 * requests for the information. The ADH can cache the information and
 * along with other Enterprise Platform components, provide the information
 * to any OMM NIProvider applications that indicate interest.
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
 * ./gradlew runniprovidermod1a
 * (runs with a default set of parameters (-h localhost -p 14003 -i ""))
 *
 * or
 *
 * ./gradlew runniprovidermod1a -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
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
 * the expected time frame, the connection can be terminated. LSEG
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
 * ./gradlew runniprovidermod1b
 * (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runniprovidermod1b -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *
 ************************************************************************
 * ETA NI Provider Training Module 1c: Reading and Writing Data
 ************************************************************************
 * Summary:
 * In this module, when a client or server Channel.state is
 * ACTIVE, it is possible for an application to receive
 * data from the connection. Similarly, when a client or server
 * Channel.state is ACTIVE, it is possible for an
 * application to write data to the connection. Writing involves a several
 * step process.
 *
 * Detailed Descriptions:
 * When a client or server Channel.state is ACTIVE, it is
 * possible for an application to receive data from the connection. The
 * arrival of this information is often announced by the I/O notification
 * mechanism that the Channel.socketId is registered with. The ETA
 * Transport reads information from the network as a byte stream, after
 * which it determines Buffer boundaries and returns each buffer one by
 * one.
 *
 * When a client or server Channel.state is ACTIVE, it is
 * possible for an application to write data to the connection. Writing
 * involves a several step process. Because the ETA Transport provides
 * efficient buffer management, the user is required to obtain a buffer
 * from the ETA Transport buffer pool. This can be the guaranteed output
 * buffer pool associated with an Channel. After a buffer is acquired,
 * the user can populate the Buffer.data and set the Buffer.length
 * to the number of bytes referred to by data. If queued information cannot
 * be passed to the network, a function is provided to allow the application
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able
 * to accept additional bytes for writing. The ETA Transport can continue to
 * queue data, even if the network is unable to write.
 *
 * Command line usage:
 *
 * ./gradlew runniprovidermod1c
 * (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runniprovidermod1a -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
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
 * After receiving a Login request, the ADH determines whether the NIP is
 * permissioned to access the system. The ADH sends a Login response, indicating
 * to the NIP whether the ADH grants it access.
 *
 * a) If the application is denied, the ADH closes the Login stream and the
 * NI provider application cannot perform any additional communication.
 * b) If the application gains access to the ADH, the Login response informs
 * the application of this. The NI provider must now provide a Source Directory.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runniprovidermod2
 * (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED))
 *
 * or
 *
 * ./gradlew runniprovidermod2 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 ************************************************************************
 * ETA NI Provider Training Module 3: Provide Source Directory Information
 ************************************************************************
 * Summary:
 * In this module, OMM NIP application provides Source Directory information.
 * The Source Directory domain model conveys information about all available
 * services in the system. After completing the Login process, an OMM NIP must
 * provide a Source Directory refresh.
 *
 * Detailed Descriptions:
 * The Source Directory domain model conveys information about all
 * available services in the system. After completing the Login process,
 * an OMM NIP must provide a Source Directory refresh indicating:
 *
 * a) Service, service state, QoS, and capability information associated
 * with the NIP
 * b) Supported domain types and any item group information associated
 * with the service.
 *
 * At a minimum, LSEG recommends that the NIP send the Info,
 * State, and Group filters for the Source Directory. Because this is provider
 * instantiated, the NIP should use a streamId with a negative value.
 *
 * a) The Source Directory Info filter contains service name and serviceId
 * information for all available services, though NIPs typically provide data
 * on only one service.
 * b) The Source Directory State filter contains status information for service.
 * This informs the ADH whether the service is Up and available or Down and
 * unavailable.
 * c) The Source Directory Group filter conveys item group status information,
 * including information about group states as well as the merging of groups.
 * For additional information about item groups, refer to ETAC Developer Guide.
 *
 * Content is encoded and decoded using the ETA Message Package and the ETA
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runniprovidermod3
 * (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1))
 *
 * or
 *
 * ./gradlew runniprovidermod3 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-id <Service ID>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
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
 * (usually the LSEG RDMFieldDictionary, though it could also be a 
 * user-defined or modified field dictionary).
 * 
 * Dictionaries may be available locally in a file for an OMM NIP appliation. In 
 * this Training example, the OMM NIP will use dictionaries that are available 
 * locally in a file.
 *
 * Command line usage:
 *
 * ./gradlew runniprovidermod4
 * (runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1))
 *
 * or
 *
 * ./gradlew runniprovidermod4 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-id <Service ID>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 */

package com.refinitiv.eta.training.niprovider;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.ElementListFlags;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FilterEntry;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.FilterList;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.training.common.TrainingModuleUtils;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteFlags;

public class Module_4_Dictionary
{

    /* dictionary file name  */
    public static String fieldDictionaryFileName = "RDMFieldDictionary";

    /* enum table file name */
    public static String enumTypeDictionaryFileName = "enumtype.def";

    public static void main(String[] args)
    {

        /******************************************************************************************************************
         * DECLARING VARIABLES
         ******************************************************************************************************************/
        /* For this simple training app, only a single channel/connection is used for the entire life of this app. */
        Channel channel = null;

        int serviceId = 1;
        /* Create an I/O notification mechanism (a Selector in our case) for our channel */
        Selector selector = null;
        /* Create a bit mask to specify what I/O notification operations to keep track of (e.g. READ and CONNECT)*/
        short opMask = 0;

        int retCode;
        /* Create error to keep track of any errors that may occur in Transport methods */
        Error error = TransportFactory.createError();

        /* Create and populate connect options to specify connection preferences */
        ConnectOptions cOpts = TransportFactory.createConnectOptions();

        /* InProgInfo Information for the In Progress Connection State */
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        /*  Channel Info*/
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        int maxMsgSize; /* the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool. */
        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long etaRuntime = 0;
        long runTime = 0;

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        /* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator(); /* the decode iterator is created (typically stack allocated)  */

        /* In this app, we are only interested in using 2 dictionaries:
         * - Field Dictionary (RDMFieldDictionary) and
         * - Enumerated Types Dictionaries (enumtype.def)
         *
         * Dictionaries may be available locally in a file for an OMM NIP appliation. In
         * this Training example, the OMM NIP will use dictionaries that are available
         * locally in a file.
        
         * Some data requires the use of a dictionary for encoding or decoding. This
         * dictionary typically defines type and formatting information and directs
         * the application as to how to encode or decode specific pieces of information.
         * Content that uses the FieldList type requires the use of a field dictionary
         * (usually the LSEG RDMFieldDictionary, though it could also be a
         * user-defined or modified field dictionary).
         */

        /* data dictionary */
        DataDictionary dataDictionary = CodecFactory.createDataDictionary();

        String errTxt = null;
        Buffer errorText = CodecFactory.createBuffer();
        errorText.data(errTxt);

        /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
        InitArgs initArgs = TransportFactory.createInitArgs();

        /* the default option parameters */
        /* connect to server running on same machine */
        String srvrHostname = "localhost";
        /* server is running on port number 14003 */
        String srvrPortNo = "14003";
        /* use default NIC network interface card to bind to for all inbound and outbound data */
        String interfaceName = "";
        /* default service name is "DIRECT_FEED" used in source directory handler */
        String serviceName = "DIRECT_FEED";
        /* use default runTime of 300 seconds */
        runTime = 300;
        /* User specifies options such as address, port, and interface from the command line.
         * User can have the flexibilty of specifying any or all of the parameters in any order.
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
                else if ((args[i].equals("-r")) == true)
                {
                    i += 2;
                    runTime = Integer.parseInt(args[i - 1]);
                }
                else if ((args[i].equals("-s")) == true)
                {
                    i += 2;
                    serviceName = args[i - 1];
                }
                else if ((args[i].equals("-id")) == true)
                {
                    i += 2;
                    serviceId = Integer.parseInt(args[i - 1]);
                }
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>]  [-s <ServiceName>] [-id <ServiceId>]\n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        /******************************************************************************************************************
         * INITIALIZATION - USING Initialize()
         ******************************************************************************************************************/
        /*********************************************************
         * Client/NIProv Application Life Cycle Major Step 1: Initialize ETA
         * Transport using Initialize The first ETA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any boot strapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the ETA Transport.
         *********************************************************/

        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        etaRuntime = currentTime + runTime * 1000;
        /* populate connect options, then pass to Connect function -
         * ETA Transport should already be initialized
         */
        /* use standard socket connection */
        cOpts.connectionType(ConnectionTypes.SOCKET); /* (0) Channel is a standard TCP socket connection type */
        cOpts.unifiedNetworkInfo().address(srvrHostname);
        cOpts.unifiedNetworkInfo().serviceName(srvrPortNo);
        cOpts.unifiedNetworkInfo().interfaceName(interfaceName);

        /* populate version and protocol with RWF information or protocol specific info */
        cOpts.protocolType(Codec.protocolType()); /* Protocol type definition for RWF */

        cOpts.majorVersion(Codec.majorVersion());
        cOpts.minorVersion(Codec.minorVersion());

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
        dataDictionary.clear();

        /* load field dictionary from file - adds data from a Field Dictionary file to the DataDictionary */

        if (dataDictionary.loadFieldDictionary(fieldDictionaryFileName, error) < 0)
        {
            System.out.printf("\nUnable to load field dictionary: %s.\n\tError Text: %s\n", fieldDictionaryFileName, error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }
        else
            System.out.printf("Successfully loaded field dictionary from local file.\n\n");

        /* load enumerated dictionary from file - adds data from an Enumerated Types Dictionary file to the DataDictionary */

        if (dataDictionary.loadEnumTypeDictionary(enumTypeDictionaryFileName, error) < 0)
        {
            System.out.printf("\nUnable to load enum type dictionary: %s.\n\tError Text: %s\n", enumTypeDictionaryFileName, error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }
        else
            System.out.printf("Successfully loaded enum type dictionary from local file.\n\n");

        System.out.printf("ETA NIP application has successfully loaded both dictionaries from local files.\n\n");

        /******************************************************************************************************************
         * CONNECTION SETUP - USING Connect()
         ******************************************************************************************************************/
        /*********************************************************
         * Client/NIProv Application Life Cycle Major Step 2: Connect using
         * Connect (OS connection establishment handshake) Connect call
         * Establishes an outbound connection, which can leverage standard
         * sockets, HTTP, or HTTPS. Returns an Channel that represents the
         * connection to the user. In the event of an error, null is returned
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

        int channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
        System.out.printf("Channel IPC descriptor = %d\n", channelFDValue);

        /* Connection was successful, add socketId to I/O notification mechanism and initialize connection */
        /* Typical FD_SET use, this may vary depending on the I/O notification mechanism the application is using */
        opMask |= SelectionKey.OP_READ;
        opMask |= SelectionKey.OP_CONNECT;
        opMask |= SelectionKey.OP_WRITE;
        if (!cOpts.blocking())
        {
            if (opMask == 0)
                opMask |= SelectionKey.OP_WRITE;
        }

        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
        }

        SelectionKey key = null;
        Set<SelectionKey> selectedKeys = null;
        Iterator<SelectionKey> keyIter = null;

        /******************************************************************************************************************
         * MAIN LOOP TO SEE IF RESPONSE RECEIVED FROM PROVIDER
         ******************************************************************************************************************/
        /* Main loop for getting connection active and successful completion of the initialization process
         * The loop calls select() to wait for notification
         * Currently, the main loop would exit if an error condition is triggered or
         * Channel.state transitions to CH_STATE_ACTIVE.
         */

        /*
         *If we want a non-blocking read call to the selector, we use select before read as read is a blocking call but select is not
         *If we want a blocking read call to the selector, such that we want to wait till we get a message, we should use read without select.
         *In the program below we will use select(), as it is non-blocking
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
                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
            }

            /* Set a timeout value if the ADH Infra server accepts the connection, but does not initialize it */

            /* Max waiting time */
            final int timeOutSeconds = 60;

            try
            {
                selector.select(timeOutSeconds * 1000);
                /* Create an iterator from the selector's updated channels*/
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has not updated, we must have timed out */
                if (!keyIter.hasNext())
                {
                    System.out.printf("Channel initialization has timed out.\n");
                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                }
                else
                {
                    /* Received a response from the provider. */

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
                        if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                        }

                        /* Handle return code appropriately */
                        switch (retCode)
                        {
                            /* (2)  Transport Success: Channel initialization is In progress, returned from InitChannel. */
                            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            {
                                /* Initialization is still in progress, check the InProgInfo for additional information */
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
                                    final int oldChannelFDValue = channelFDValue;
                                    channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                    System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", channelFDValue, oldChannelFDValue);

                                    /* File descriptor has changed, unregister old and register new */

                                    try
                                    {
                                        key = inProgInfo.oldSelectableChannel().keyFor(selector);
                                        key.cancel();
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                    }
                                }
                                else
                                {
                                    System.out.printf("Channel %d in progress...\n", channelFDValue);
                                }
                            }
                                break;

                            /* channel connection becomes active!
                             * Once a connection is established and transitions to the ACTIVE state,
                             * this Channel can be used for other transport operations.
                             */
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("Channel on fd %d is now active - reading and writing can begin.\n", channelFDValue);

                                /*********************************************************
                                 * Connection is now active. The Channel can be
                                 * used for all additional transport
                                 * functionality (e.g. reading, writing) now
                                 * that the state transitions to ACTIVE
                                 *********************************************************/

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with GetChannelInfo. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                    /* Connection should be closed, return failure */
                                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                }

                                /* Print out basic channel info */
                                System.out.printf("\nChannel %d active. Channel Info:\n" + "Max Fragment Size:           %d\n" + "Output Buffers:              %d Max, %d Guaranteed\n" + "Input Buffers:               %d\n" + "Send/Receive Buffer Sizes:   %d/%d\n" + "Ping Timeout:                %d\n",
                                        channelFDValue,
                                        channelInfo.maxFragmentSize(), /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                        channelInfo.maxOutputBuffers(), /* This is the maximum number of output buffers available to the channel. */
                                        channelInfo.guaranteedOutputBuffers(), /*  This is the guaranteed number of output buffers available to the channel. */
                                        channelInfo.numInputBuffers(), /*  This is the number of input buffers available to the channel. */
                                        channelInfo.sysSendBufSize(), /*  This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
                                        channelInfo.sysRecvBufSize(), /*  This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
                                        channelInfo.pingTimeout()); /* This is the value of the negotiated ping timeout */

                                System.out.printf("Connected component version: ");
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
                            default:
                            {
                                System.out.printf("Bad return value fd=%d <%d>\n", channelFDValue, retCode);
                                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                            }
                                break;
                        }
                    }
                }
            }
            catch (IOException e1)
            {

                e1.printStackTrace();
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
        maxMsgSize = channelInfo.maxFragmentSize(); /* @brief This is the max fragment size before fragmentation and reassembly is necessary. */

        /* Initialize ping management handler */
        initPingManagement(channel);

        /* clear encode iterator for initialization/use - this should be used to achieve the best performance while clearing the iterator. */
        encodeIter.clear();

        /* Send Login request message */
        if ((retCode = sendLoginRequest(channel, maxMsgSize, encodeIter)) > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush is done by application */
            opMask |= SelectionKey.OP_WRITE;
        }
        else if (retCode < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
        }

        /*****************************************************************************************************************
         * SECOND MAIN LOOP TO CONNECTION ACTIVE - KEEP LISTEINING FOR INCOMING
         * DATA
         ******************************************************************************************************************/
        /* Here were are using a new Main loop. An alternative design would be to combine this Main loop with
         * the Main loop for getting connection active. Some bookkeeping would be required for that approach.
         */

        /* Main loop for message processing (reading data, writing data, and ping management, etc.)
         * The loop calls select() to wait for notification
         * Currently, the only way to exit this Main loop is when an error condition is triggered or after
         * a predetermined run-time has elapsed.
         */
        while (true)
        {
            /* Register our channel to the selector and watch for I/O notifications specified by our operation mask */
            try
            {
                channel.selectableChannel().register(selector, opMask, channel);
            }
            catch (Exception e)
            {
                System.out.printf("Exception %s\n", e.getMessage());
                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
            }

            /* Wait for any I/O notification updates in the channel */
            try
            {
                selector.select(1000);
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has been updated */
                if (keyIter.hasNext())
                {
                    /* Check if channel is READ-able */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isReadable())
                    {
                        /* reading data from channel via Read/Exception FD */

                        /* When a client Channel.state is ACTIVE, it is possible for an application to receive data from the connection.
                         * The arrival of this information is often announced by the I/O notification mechanism that the Channel.socketId is registered with.
                         * The ETA Transport reads information from the network as a byte stream, after which it determines Buffer boundaries and returns
                         * each buffer one by one.
                         */

                        retCode = 1; /* initialize to a positive value for Read call in case we have more data that is available to read */

                        /******************************************************
                         * Loop 4) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = TransportFactory.createReadArgs();

                            /* There is more data to read and process and I/O notification may not trigger for it
                             * Either schedule another call to read or loop on read until retCode == CodecReturnCodes.SUCCESS
                             * and there is no data left in internal input buffer
                             */

                            /*********************************************************
                             * Client/NIProv Application Life Cycle Major Step
                             * 4: Read using channel.read() Read provides the
                             * user with data received from the connection. This
                             * function expects the Channel to be in the active
                             * state. When data is available, an Buffer
                             * referring to the information is returned, which
                             * is valid until the next call to Read. A return
                             * code parameter passed into the function is used
                             * to convey error information as well as
                             * communicate whether there is additional
                             * information to read. An I/O notification
                             * mechanism may not inform the user of this
                             * additional information as it has already been
                             * read from the socket and is contained in the Read
                             * input buffer.
                             *********************************************************/

                            if ((msgBuf = channel.read(readArgs, error)) != null)
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
                                Msg msg = CodecFactory.createMsg();

                                /* This decodeIter.clear() clear iterator function should be used to achieve the best performance while clearing the iterator. */
                                /* Clears members necessary for decoding and readies the iterator for reuse. You must clear DecodeIterator
                                 * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
                                 */
                                decodeIter.clear();

                                /* Set the RWF version to decode with this iterator */
                                retCode = decodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

                                /* Associates the DecodeIterator with the Buffer from which to decode. */
                                if (retCode != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("\nSetDecodeIteratorBuffer() failed with return code: %d\n", retCode);
                                    /* Closes channel, cleans up and exits the application. */
                                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                }

                                /* decode contents into the Msg structure */

                                if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("DecodeMsg(): Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                }

                                switch (msg.domainType())
                                {
                                    /* (1) Login Message */
                                    case DomainTypes.LOGIN:
                                    {
                                        if (processLoginResponse(msg, decodeIter) != TransportReturnCodes.SUCCESS)
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
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                        else
                                        {
                                            System.out.printf("ETA NI Provider application is granted access and has logged in successfully.\n\n");
                                            /* The Source Directory domain model conveys information about all available services in the system. After completing the
                                             * Login process, an OMM NIP must provide a Source Directory refresh message.
                                             */
                                            System.out.printf("ETA NI Provider application is providing a Source Directory refresh message.\n\n");
                                            sendSourceDirectoryResponse(channel, maxMsgSize, encodeIter, serviceName, serviceId);

                                        }
                                    }
                                        break;
                                    default:
                                    {
                                        System.out.printf("Unhandled Domain Type: %d\n", msg.domainType());
                                    }
                                        break;
                                }

                                /* Process data and update ping monitor since data was received */
                                /* set flag for server message received */
                                receivedServerMsg = true;
                                System.out.printf("Ping message has been received successfully from the server due to data message ...\n\n");

                            }
                            else
                            {

                                /* Deduce an action from the return code of Channel.read() */
                                retCode = readArgs.readRetVal();
                                switch (retCode)
                                {
                                    /* (-13) Transport Success: Read has received a ping message. There is no buffer in this case. */
                                    /* Acknowledge that a ping has been received */

                                    case TransportReturnCodes.READ_PING:
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
                                    case TransportReturnCodes.READ_FD_CHANGE:
                                    {
                                        /* File descriptor changed, typically due to tunneling keep-alive */
                                        /* Unregister old socketId and register new socketId */
                                        opMask = SelectionKey.OP_READ;
                                        final int oldChannelFDValue = channelFDValue;
                                        channelFDValue = TrainingModuleUtils.getFDValueOfSelectableChannel(channel.selectableChannel());
                                        System.out.printf("\nChannel In Progress - New FD: %d   Old FD: %d\n", channelFDValue, oldChannelFDValue);
                                        try
                                        {
                                            key = channel.selectableChannel().keyFor(selector);
                                            key.cancel();
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                        try
                                        {
                                            channel.selectableChannel().register(selector, opMask, channel);
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                    }
                                        break;
                                    /* (-11) Transport Success: Reading was blocked by the OS. Typically indicates that there are no bytes available to read,
                                     * returned from Read.
                                     */
                                    case TransportReturnCodes.READ_WOULD_BLOCK: /* Nothing to read */
                                        break;
                                    case TransportReturnCodes.READ_IN_PROGRESS:/* fall through to default. */
                                    case TransportReturnCodes.INIT_NOT_INITIALIZED:
                                    case TransportReturnCodes.FAILURE:
                                        System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        break;

                                    default: /* Error handling */
                                    {

                                        if (retCode < 0)
                                        {
                                            System.out.printf("Error (%d) (errno: %d) encountered with Read. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                        }
                                    }
                                        break;
                                }
                            }
                        }
                    }

                    /* An I/O notification mechanism can be used to indicate when the operating system can accept more data for output.
                     * Flush function is called because of a write file descriptor alert
                     */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isWritable())
                    {
                        /* Flush */
                        retCode = TransportReturnCodes.FAILURE;

                        /* this section of code was called because of a write file descriptor alert */
                        if ((retCode = channel.flush(error)) > TransportReturnCodes.SUCCESS)
                        {
                            /* There is still data left to flush, leave our write notification enabled so we get called again.
                             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
                             */
                        }
                        else
                        {
                            switch (retCode)
                            {
                                case TransportReturnCodes.SUCCESS:
                                {
                                    /* Everything has been flushed, no data is left to send - unset/clear write fd notification */
                                    key = channel.selectableChannel().keyFor(selector);
                                    try
                                    {
                                        channel.selectableChannel().register(selector, key.interestOps() - SelectionKey.OP_WRITE, channel);
                                    }
                                    catch (ClosedChannelException e)
                                    {
                                        e.printStackTrace();
                                    }
                                }
                                    break;
                                case TransportReturnCodes.FAILURE:
                                default:
                                {
                                    System.out.printf("Error (%d) (errno: %d) encountered with Init Channel fd=%d. Error Text: %s\n", error.errorId(), error.sysError(), channelFDValue, error.text());
                                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                                }
                            }
                        }
                    }
                }

                /* Processing ping management handler */
                if ((retCode = processPingManagementHandler(channel, error, opMask, selector)) > TransportReturnCodes.SUCCESS)
                {
                    /* There is still data left to flush, leave our write notification enabled so we get called again.
                     * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                     */

                    /* set write fd if there's still other data queued */
                    /* flush is done by application */
                    opMask |= SelectionKey.OP_WRITE;
                }
                else if (retCode < TransportReturnCodes.SUCCESS)
                {
                    /* Closes channel, cleans up and exits the application. */
                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                }

                /* get current time */
                currentTime = System.currentTimeMillis();

                /* Handles the run-time for the ETA NI Provider application. Here we exit the application after a predetermined time to run */
                /* If the runtime has expired */
                if (System.currentTimeMillis() >= etaRuntime)
                {
                    /* Closes all streams for the NI Provider after run-time has elapsed. */

                    /* Note that closing Login stream will automatically close all other streams at the provider */
                    if ((retCode = closeLoginStream(channel, maxMsgSize, encodeIter)) < TransportReturnCodes.SUCCESS) /* (retval > CodecReturnCodes.SUCCESS) or (retval < CodecReturnCodes.SUCCESS) */
                    {
                        /* When you close login, we want to make a best effort to get this across the network as it will gracefully
                         * close all open streams. If this cannot be flushed or failed, this application will just close the connection
                         * for simplicity.
                         */

                        /* Closes channel, cleans up and exits the application. */
                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                    }

                    /* flush before exiting */
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isWritable())
                    {
                        retCode = 1;
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {

                            retCode = channel.flush(error);

                        }
                        if (retCode < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Flush() failed with return code %d - <%s>\n", retCode, error.text());
                        }
                    }

                    System.out.printf("ETA Client run-time has expired...\n\n");
                    closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.SUCCESS);
                }
            }
            catch (IOException e1)
            {
                e1.printStackTrace();
            }
        }
    }

    /*
     * Closes channel, cleans up and exits the application.
     * etaChannel - The channel to be closed
     * code - if exit due to errors/exceptions
     */
    public static void closeChannelCleanUpAndExit(Channel channel, Selector selector, Error error, int code)
    {
        boolean isClosedAndClean = true;
        try
        {
            selector.close();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
        }

        /*********************************************************
         * Client/NIProv Application Life Cycle Major Step 5: Close connection
         * using CloseChannel (OS connection release handshake) CloseChannel
         * closes the client based Channel. This will release any pool based
         * resources back to their respective pools, close the connection, and
         * perform any additional necessary cleanup. When shutting down the
         * Transport, the application should release all unwritten pool buffers.
         * Calling CloseChannel terminates the connection to the ADH.
         *********************************************************/

        if ((channel != null)) {
            isClosedAndClean = channel.close(error) >= TransportReturnCodes.SUCCESS;
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
        Transport.uninitialize();

        if (isClosedAndClean) {
            System.out.println("NIProvider application has closed channel and has cleaned up successfully.");
        } else {
            System.out.printf("Error (%d) (errno: %d) encountered with CloseChannel. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
        }

        /* For applications that do not exit due to errors/exceptions such as:
         * Exits the application if the run-time has expired.
         */
        if (code == TransportReturnCodes.SUCCESS)
            System.out.printf("\nETA NI Provider Training application successfully ended.\n");

        /* End application */
        System.exit(0);

    }

    /*
     * Initializes the ping times for etaChannel.
     * etaChannel - The channel for ping management info initialization
     */
    static int pingTimeoutServer; /* server ping timeout */
    static int pingTimeoutClient; /* client ping timeout */
    static long nextReceivePingTime; /* time client should receive next message/ping from server */
    static long nextSendPingTime; /* time to send next ping from client */
    static boolean receivedServerMsg; /* flag for server message received */

    public static void initPingManagement(Channel channel)
    {
        /* set ping timeout for local and remote pings */
        pingTimeoutClient = channel.pingTimeout() / 3;
        pingTimeoutServer = channel.pingTimeout();

        /* set time to send next ping to remote connection */
        nextSendPingTime = System.currentTimeMillis() + pingTimeoutClient * 1000;

        /* set time should receive next ping from remote connection */
        nextReceivePingTime = System.currentTimeMillis() + pingTimeoutServer * 1000;
    }

    /*
     * Processing ping management handler
     * etaChannel - The channel for ping management processing
     */
    public static int processPingManagementHandler(Channel channel, Error error, Short opMask, Selector selector)
    {
        /* Handles the ping processing for etaChannel. Sends a ping to the server if the next send ping time has arrived and
         * checks if a ping has been received from the server within the next receive ping time.
         */
        int retval = TransportReturnCodes.SUCCESS;

        /* get current time */
        long currentTime = System.currentTimeMillis();

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
            if ((retval = channel.ping(error)) > TransportReturnCodes.SUCCESS)
            {
                /* Indicates that queued data was sent as a heartbeat and there is still information internally queued by the transport.
                 * The Flush function must be called to continue attempting to pass the queued bytes to the connection. This information may
                 * still be queued because there is not sufficient space in the connections output buffer.
                 * An I/O notification mechanism can be used to indicate when the socketId has write availability.
                 *
                 * There is still data left to flush, leave our write notification enabled so we get called again.
                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                 */

                /* flush needs to be done by application */
            }
            else
            {
                switch (retval)
                {
                    case TransportReturnCodes.SUCCESS:
                    {
                        /* Ping message has been sent successfully */
                        System.out.printf("Ping message has been sent successfully to the server ...\n\n");
                    }
                        break;
                    case TransportReturnCodes.FAILURE: /* fall through to default. */
                    default: /* Error handling */
                    {
                        System.out.printf("Error (%d) (errno: %d) encountered with Ping(). Error Text:%s\n", error.errorId(), error.sysError(), error.text());
                        closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);
                        /* Closes channel/connection, cleans up and exits the application. */
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            /* set time to send next ping from client */
            nextSendPingTime = currentTime + (pingTimeoutClient * 1000);
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
                nextReceivePingTime = currentTime + (pingTimeoutServer * 1000);
            }
            else /* lost contact with server */
            {
                /* Lost contact with remote (connection) */
                error.text("Lost contact with connection...\n");
                System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                closeChannelCleanUpAndExit(channel, selector, error, TransportReturnCodes.FAILURE);

            }
        }
        return retval;
    }

    /*
     * Sends a message buffer to a channel.
     * etaChannel - The channel to send the message buffer to
     * msgBuf - The msgBuf to be sent
     */
    public static int sendMessage(Channel channel, TransportBuffer msgBuf)
    {
        Error error = TransportFactory.createError();
        int retCode = 0;

        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        writeArgs.flags(WriteFlags.NO_FLAGS);
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

        /* Now write the data - keep track of ETA Transport return code -
         * Because positive values indicate bytes left to write, some negative transport layer return codes still indicate success
         */

        /* this example writes buffer as high priority and no write modification flags */
        if ((retCode = channel.write(msgBuf, writeArgs, error)) == TransportReturnCodes.WRITE_CALL_AGAIN)
        {
            /* (-10) Transport Success: Write is fragmenting the buffer and needs to be called again with the same buffer. This indicates that Write was
             * unable to send all fragments with the current call and must continue fragmenting
             */

            /* Large buffer is being split by transport, but out of output buffers. Schedule a call to Flush and then call the Write function again with
             * this same exact buffer to continue the fragmentation process. Only release the buffer if not passing it to Write again. */

            /* call flush and write again - breaking out if the return code is something other than TransportReturnCodes.WRITE_CALL_AGAIN (write call again) */
            while (retCode == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                /* Schedule a call to Flush */
                if ((retCode = channel.flush(error)) < TransportReturnCodes.SUCCESS)
                {
                    System.out.printf("Flush has failed with return code %d - <%s>\n", retCode, error.text());
                }
                /* call the Write function again with this same exact buffer to continue the fragmentation process. */
                retCode = channel.write(msgBuf, writeArgs, error);
            }
        }

        /* set write fd if there's still data queued */
        if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* The write was successful and there is more data queued in ETA Transport. The Flush function should be used to continue attempting to flush data
             * to the connection. ETA will release buffer.
             */

            /* flush needs to be done by application */
        }
        else
        {
            /* Handle return codes appropriately, not all return values are failure conditions */
            switch (retCode)
            {
                case TransportReturnCodes.SUCCESS:
                {
                    /* Successful write and all data has been passed to the connection */
                    /* Continue with next operations. ETA will release buffer.*/
                }
                    break;
                case TransportReturnCodes.NO_BUFFERS:
                {
                    channel.releaseBuffer(msgBuf, error);
                }
                    break;
                case TransportReturnCodes.WRITE_FLUSH_FAILED:
                {
                    if (channel.state() == ChannelState.CLOSED)
                    {
                        /* Channel is Closed - This is terminal. Treat as error, and buffer must be released - fall through to default. */
                    }
                    else
                    {
                        /* Channel.write() internally attempted to flush data to the connection but was blocked. This is not a failure and the user should not release their buffer."; 
                        /* Successful write call, data is queued. The Channel.flush() method should be used to continue attempting to flush data to the connection. */

                        /* Set write fd if flush failed */
                        /* Flush needs to be done by application */

                        /* Channel is still open, but Channel.write() tried to flush internally and failed. 
                         * Return positive value so the caller knows there's bytes to flush. 
                         */
                        return TransportReturnCodes.SUCCESS + 1;
                    }
                    break;
                }
                case TransportReturnCodes.FAILURE:
                default:
                {
                    System.out.printf("Error (%d) (errno: %d) encountered with Write : %s\n", error.errorId(), error.sysError(), error.text());
                    channel.releaseBuffer(msgBuf, error);
                    return TransportReturnCodes.FAILURE;
                }
            }
        }

        return retCode;
    }

    final public static int LOGIN_STREAM_ID = 1;

    /*
     * Send Login request message to a channel. This consists of getting a message buffer, setting the login request
     * information, encoding the login request, and sending the login request to the server. A Login request message is
     * encoded and sent by OMM consumer and OMM non-interactive provider applications. This message registers a user
     * with the system. After receiving a successful Login response, applications can then begin consuming or providing
     * additional content. An OMM provider can use the Login request information to authenticate users with DACS.
     * etaChannel - The channel to send the Login request message buffer to
     * maxMsgSize - the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.
     * encodeIter - The encode iterator
     */
    public static int sendLoginRequest(Channel channel, int maxFragmentSize, EncodeIterator encIter)
    {
        int retCode;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;

        /* Populate and encode a requestMsg */
        RequestMsg reqMsg = (RequestMsg)CodecFactory.createMsg();

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();

        Buffer applicationId = CodecFactory.createBuffer();
        Buffer applicationName = CodecFactory.createBuffer();
        UInt applicationRole = CodecFactory.createUInt();

        String userName = "put userName here";
        Buffer userNameBuf = CodecFactory.createBuffer();

        elementList.clear();
        elementEntry.clear();

        /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = etaGetBuffer(channel, maxFragmentSize, error)) == null)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Login request. */

        reqMsg.clear();
        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */

        /* set the buffer on an EncodeIterator */
        if ((retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* set-up message */
        reqMsg.msgClass(MsgClasses.REQUEST);/* (1) Request Message */
        /* StreamId */
        reqMsg.streamId(LOGIN_STREAM_ID);
        reqMsg.domainType(DomainTypes.LOGIN); /* (1) Login Message */
        reqMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/
        /* The initial Login request must be streaming (i.e., a RequestMsgFlags.STREAMING flag is required). */
        reqMsg.flags(RequestMsgFlags.STREAMING);

        /* set msgKey members */
        reqMsg.msgKey().flags(MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME);

        /* Username */
        userNameBuf.data(userName);

        if ((userName = System.getProperty("user.name")) != null)
        {
            reqMsg.msgKey().name().data(userName);
        }
        else
        {
            reqMsg.msgKey().name().data("Unknown");
        }

        reqMsg.msgKey().nameType(Login.UserIdTypes.NAME);/* (1) Name */
        /* (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
        reqMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

        /* encode message */

        /* since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit */
        /* EncodeMsgInit should return and inform us to encode our key opaque */
        if ((retCode = reqMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("reqMsg.encodeInit() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* encode our msgKey opaque */

        /* encode the element list */
        elementList.clear();

        elementList.flags(ElementListFlags.HAS_STANDARD_DATA); /* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */

        /* Begins encoding of an ElementList. */
        if ((retCode = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementList.encodeInit() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* ApplicationId */
        applicationId.data("256");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPID);

        if ((retCode = elementEntry.encode(encIter, applicationId)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementEntry.encode() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* ApplicationName */
        applicationName.data("ETA NI provider Training");

        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPNAME);
        if ((retCode = elementEntry.encode(encIter, applicationName)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementEntry.encode() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Role */
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.ROLE);

        /* (1) Application logs in as a provider */
        applicationRole.value(Login.RoleTypes.PROV);

        if ((retCode = elementEntry.encode(encIter, applicationRole)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementEntry.encode() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* complete encode element list */
        if ((retCode = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("elementList.encodeComplete() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* complete encode key */
        /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
         * for us to encode our container/msg payload
         */
        if ((retCode = reqMsg.encodeKeyAttribComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("reqMsg.encodeKeyAttribComplete() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode message */
        if ((retCode = reqMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("reqMsg.encodeComplete() failed for Login Request with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* send login request */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {

            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
             */

            /* set write flag if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retCode;
    }

    /*
     * Processes a login response. This consists of decoding the response.
     * msg - The partially decoded message
     * decodeIter - The decode iterator
     */
    public static int processLoginResponse(Msg msg, DecodeIterator decIter)
    {
        int retCode;
        State pState = CodecFactory.createState();

        String tempData = "guess";
        Buffer tempBuf = CodecFactory.createBuffer();

        tempBuf.data(tempData);

        /* Switch cases depending on message class */
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            {
                MsgKey key = CodecFactory.createMsgKey();

                ElementList elementList = CodecFactory.createElementList();
                ElementEntry elementEntry = CodecFactory.createElementEntry();

                /* check stream id */
                System.out.printf("\nReceived Login Refresh Msg with Stream Id %d\n", msg.streamId());

                /* check if it's a solicited refresh */
                if ((msg.flags() & RefreshMsgFlags.SOLICITED) != 0)
                {
                    System.out.printf("\nThe refresh msg is a solicited refresh (sent as a response to a request).\n");
                }
                else
                {
                    System.out.printf("A refresh sent to inform a NI provider of an upstream change in information (i.e., an unsolicited refresh).\n");
                }

                /* get key */
                key = msg.msgKey();

                /* decode key opaque data */
                if ((retCode = msg.decodeKeyAttrib(decIter, key)) != CodecReturnCodes.SUCCESS)
                {
                    System.out.printf("DecodeMsgKeyAttrib() failed with return code: %d\n", retCode);
                    return TransportReturnCodes.FAILURE;
                }

                /* decode element list */
                if ((retCode = elementList.decode(decIter, null)) == CodecReturnCodes.SUCCESS)
                {
                    /* decode each element entry in list */
                    while ((retCode = elementEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (retCode == CodecReturnCodes.SUCCESS)
                        {
                            /* get login response information */

                            /* Currently, ADH/Infra handling of login response was simpler than ADS handling -
                             * The Only Received Login Response from ADH is ApplicationId in the current implementation of ADH.
                             * In some cases, a lot of things don't apply (like SingleOpen, Support*, etc) as these are
                             * consumer based behaviors so ADH does not advertise them.
                             * Also, likely many defaults are being relied on from ADH, while ADS may be sending them even though default.
                             */

                            /* ApplicationId */
                            if (elementEntry.name().equals(ElementNames.APPID))
                            {
                                System.out.printf("\tReceived Login Response for ApplicationId: %s\n", elementEntry.encodedData().toString());
                            }
                            /* ApplicationName */
                            else if (elementEntry.name().equals(ElementNames.APPNAME))
                            {
                                System.out.printf("\tReceived Login Response for ApplicationName: %s\n", elementEntry.encodedData().toString());
                            }
                            /* Position */
                            else if (elementEntry.name().equals(ElementNames.POSITION))
                            {
                                System.out.printf("\tReceived Login Response for Position: %s\n", elementEntry.encodedData().toString());
                            }
                        }
                        else
                        {
                            System.out.printf("DecodeElementEntry() failed with return code: %d\n", retCode);
                            return TransportReturnCodes.FAILURE;
                        }
                    }
                }
                else
                {
                    System.out.printf("DecodeElementList() failed with return code: %d\n", retCode);
                    return TransportReturnCodes.FAILURE;
                }

                /* get Username */
                if (key != null)
                {
                    System.out.printf("\nReceived Login Response for ApplicationId: %s\n", key.name().toString());
                }
                else
                {
                    System.out.printf("\nReceived Login Response for ApplicationId: Unknown\n");
                }

                /* get state information */
                pState = ((RefreshMsg)msg).state();
                System.out.printf("%s\n\n", pState.toString());

                /* check if login okay and is solicited */
                if (((msg.flags() & RefreshMsgFlags.SOLICITED) != 0) && (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.OK))
                {
                    System.out.printf("Login stream is OK and solicited\n");
                }
                else /* handle error cases */
                {
                    if (pState.streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        /* Stream State is (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */

                        System.out.printf("Login stream is closed recover\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.CLOSED)
                    {
                        /* Stream State is (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */

                        System.out.printf("Login attempt failed (stream closed)\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.SUSPECT)
                    {
                        /* Stream State is (1) Open (typically implies that information will be streaming, as information changes updated
                         * information will be sent on the stream, after final RefreshMsg or StatusMsg)
                         *
                         * Data State is (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream
                         * is out of date or cannot be confirmed that it is current )
                         */
                        System.out.printf("Login stream is suspect\n");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
                break;
            case MsgClasses.STATUS:/* (3) Status Message */
            {
                System.out.printf("Received Login StatusMsg\n");
                if ((msg.flags() & StatusMsgFlags.HAS_STATE) != 0)/* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state. */
                {
                    /* get state information */
                    pState = ((StatusMsg)msg).state();
                    System.out.printf("%s\n\n", pState.toString());
                    /* handle error cases */
                    if (pState.streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        /* Stream State is (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */

                        System.out.printf("Login stream is closed recover\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.CLOSED)
                    {
                        /* Stream State is (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */

                        System.out.printf("Login attempt failed (stream closed)\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.SUSPECT)
                    {
                        /* Stream State is (1) Open (typically implies that information will be streaming, as information changes updated
                         * information will be sent on the stream, after final RefreshMsg or StatusMsg)
                         *
                         * Data State is (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream
                         * is out of date or cannot be confirmed that it is current )
                         */
                        System.out.printf("Login stream is suspect\n");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
                break;
            case MsgClasses.UPDATE: /* (4) Update Message */
            {
                System.out.printf("Received Login Update\n");
            }
                break;
            case MsgClasses.CLOSE: /* (5) Close Message */
            {
                System.out.printf("Received Login Close\n");
                return TransportReturnCodes.FAILURE;
            }
            default: /* Error handling */
            {
                System.out.printf("Received Unhandled Login Msg Class: %d\n", msg.msgClass());
                return TransportReturnCodes.FAILURE;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    /*
     * Close the Login stream. Note that closing Login stream will automatically close all other streams at the provider.
     * A Login close message is encoded and sent by OMM NIP applications. This message allows a NIP to log out
     * of the system. Closing a Login stream is equivalent to a 'Close All' type of message, where all open streams are
     * closed (thus all other streams associated with the user are closed).
     * etaChannel - The channel to send the Login close message buffer to
     * maxMsgSize - the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.
     * encodeIter - The encode iterator
     */
    public static int closeLoginStream(Channel channel, int maxMsgSize, EncodeIterator encIter)
    {
        int retCode;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();

        /* Get a buffer of the channel max fragment size */
        if ((msgBuf = etaGetBuffer(channel, maxMsgSize, error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */

            return TransportReturnCodes.FAILURE;
        }

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */

        if ((retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Create and initialize close message */
        closeMsg.msgClass(MsgClasses.CLOSE); /* (5) Close Message */
        closeMsg.streamId(LOGIN_STREAM_ID);
        closeMsg.domainType(DomainTypes.LOGIN); /* (1) Login Message */
        /* No payload associated with this close message */
        closeMsg.containerType(DataTypes.NO_DATA); /* (128) No Data <BR>*/
        /* encode message */

        /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
        /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
         * typically used for encoding simple types like Integer or incorporating previously encoded data
         * (referred to as pre-encoded data).
         */

        if ((retCode = closeMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("Msg.encode() failed with return code: %d\n", retCode);
            return TransportReturnCodes.FAILURE;
        }

        /* send login close */

        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* login close fails */
            /* Closes channel, cleans up and exits the application. */

            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
             */

            /* set write flag if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retCode;
    }

    /*
     * etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
     * Also, it simplifies the example codes and make the codes more readable.
     */
    public static TransportBuffer etaGetBuffer(Channel channel, int size, Error error)

    {
        int retCode;

        TransportBuffer msgBuf = null;

        /* Obtains a non-packable buffer of the requested size from the ETA Transport guaranteed buffer pool to write into for any request Msg.
         * When the Buffer is returned, the length member indicates the number of bytes available in the buffer (this should match the amount
         * the application requested). When populating, it is required that the application set length to the number of bytes actually used.
         * This ensures that only the required bytes are written to the network.
         */

        /**
         * @brief Retrieves a Buffer for use
         *
         *        Typical use: <BR>
         *        This is called when a buffer is needed to write data to.
         *        Generally, the user will populate the Buffer structure and
         *        then pass it to the Write function.
         *
         * @param chnl Channel who requests the buffer
         * @param size Size of the requested buffer
         * @param packedBuffer Set to TRUE if you plan on packing multiple
         *            messages into the same buffer
         * @param error Error, to be populated in event of an error
         * @return Buffer buffer to be filled in with valid memory
         * @see ReturnCodes
         */
        /* First check error */
        if ((msgBuf = channel.getBuffer(size, false, error)) == null)
        {
            if (error.errorId() != TransportReturnCodes.NO_BUFFERS)
            {
                /* Check to see if this is just out of buffers or if it's unrecoverable */

                /* it's unrecoverable Error */
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                /* Connection should be closed, return failure */
                /* Closes channel, cleans up and exits the application. */

                return null;
            }

            /* (-4) Transport Failure: There are no buffers available from the buffer pool, returned from GetBuffer.
             * Use Ioctl to increase pool size or use Flush to flush data and return buffers to pool.
             */

            /* The Flush function could be used to attempt to free buffers back to the pool */
            /* Flush and obtain buffer again */
            retCode = channel.flush(error);
            if (retCode < TransportReturnCodes.SUCCESS)
            {
                System.out.printf("Channel.flush() failed with return code %d - <%s>\n", retCode, error.text());
                /* Closes channel, cleans up and exits the application. */

                return null;
            }

            if ((msgBuf = channel.getBuffer(size, false, error)) == null)
            {
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                /* Closes channel, cleans up and exits the application. */

                return null;
            }
        }
        /* return  buffer to be filled in with valid memory */
        return msgBuf;
    }

    /*
     * Send Source Directory response to a channel. This consists of getting a message buffer, setting the source directory
     * response information, encoding the source directory response, and sending the source directory response to
     * the ADH server. OMM NIP application provides Source Directory information. The Source Directory domain model conveys
     * information about all available services in the system. After completing the Login process, an OMM NIP must
     * provide a Source Directory refresh.
     * etaChannel - The channel to send the Source Directory response message buffer to
     * maxMsgSize - the requested size of the buffer for GetBuffer function to obtain from the guaranteed/shared buffer pool.
     * encodeIter - The encode iterator
     * serviceName - The service name specified by the OMM NIP application (Optional to set)
     * serviceId - the serviceId specified by the OMM NIP application (Optional to set)
     */

    private static final int SRCDIR_STREAM_ID = -1;

    public static int sendSourceDirectoryResponse(Channel channel, int maxMsgSize, EncodeIterator encodeIter, String serviceName, int serviceId)
    {
        int retval;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;
        UInt tmpUInt = CodecFactory.createUInt();
        /* Populate and encode a refreshMsg */
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

        /* The refresh flags of the source directory response */
        int refreshFlags = 0;

        /* The refresh key with filter flags that indicate the filter entries to include */
        MsgKey refreshKey = CodecFactory.createMsgKey();
        int streamId;

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        FilterList sourceDirectoryFilterList = CodecFactory.createFilterList();
        String stateText;

        FilterEntry filterListItem = CodecFactory.createFilterEntry();
        ElementEntry element = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        /* etaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = etaGetBuffer(channel, maxMsgSize, error)) == null) /* first check Error */
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Source Directory refresh. */

        refreshMsg.clear();

        /* provide source directory response information */

        /* set refresh flags */
        /* The content of a Source Directory Refresh message is expected to be atomic and contained in a single part,
         * therefore RefreshMsgFlags.REFRESH_COMPLETE should be set.
         */

        /* (0x0008) The RefreshMsg has a message key, contained in \ref RefreshMsg.msgKey. */
        /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
        /* (0x0100) Indicates that any cached header or payload information associated with the RefreshMsg's item stream should be cleared. */
        refreshFlags |= RefreshMsgFlags.HAS_MSG_KEY;
        refreshFlags |= RefreshMsgFlags.REFRESH_COMPLETE;
        refreshFlags |= RefreshMsgFlags.CLEAR_CACHE;

        /* set filter flags */
        /* At a minimum, LSEG recommends that the NIP send the Info, State, and Group filters for the Source Directory. */
        refreshKey.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.LINK);

        /* StreamId */
        streamId = SRCDIR_STREAM_ID;

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encodeIter.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        if ((retval = encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        /* set-up message */
        refreshMsg.msgClass(MsgClasses.REFRESH); /* (2) Refresh Message */
        refreshMsg.domainType(DomainTypes.SOURCE); /* (4) Source Message */
        /* (137) Map container type, used to represent primitive type key - container type paired entries.   <BR>*/
        refreshMsg.containerType(DataTypes.MAP);

        refreshMsg.flags(refreshFlags);
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.state().code(StateCodes.NONE);
        stateText = "Source Directory Refresh Completed";
        refreshMsg.state().text().data(stateText);

        /* set members in msgKey */
        refreshMsg.msgKey().flags(MsgKeyFlags.HAS_FILTER);
        refreshMsg.msgKey().filter(refreshKey.filter());

        /* StreamId */
        refreshMsg.streamId(streamId);

        /* encode message - populate message and encode it */
        if ((retval = refreshMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMsgInit() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* encode map */
        map.keyPrimitiveType(DataTypes.UINT);
        map.containerType(DataTypes.FILTER_LIST);

        /**
         * @brief Begins encoding of an MapEntry, where any payload is encoded
         *        after this call using the appropriate container type encode
         *        functions, as specified by Map::containerType.
         *
         *        Begins encoding of an MapEntry<BR>
         *        Typical use:<BR>
         *        1. Call EncodeMapInit()<BR>
         *        2. If Map contains set definitions that are not pre-encoded,
         *        call appropriate set definition encode functions, followed by
         *        EncodeMapSetDefsComplete()<BR>
         *        3. If Map contains summary data that is not pre-encoded, call
         *        appropriate summary data container encoders, followed by
         *        EncodeMapSummaryDataComplete()<BR>
         *        4. To encode entries, call EncodeMapEntry() or
         *        EncodeMapEntryInit()..EncodeMapEntryComplete() for each
         *        MapEntry<BR>
         *        5. Call EncodeMapComplete() when all entries are completed<BR>
         */
        if ((retval = map.encodeInit(encodeIter, 0, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMapInit() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* encode map entry */
        mapEntry.action(MapEntryActions.ADD);
        tmpUInt.value(serviceId);
        if ((retval = mapEntry.encodeInit(encodeIter, tmpUInt, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMapEntry() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* encode filter list */
        sourceDirectoryFilterList.containerType(DataTypes.ELEMENT_LIST);

        /**
         * @brief Begin encoding process for FilterList container type.
         *
         *        Begins encoding of an FilterList<BR>
         *        Typical use:<BR>
         *        1. Call EncodeFilterListInit()<BR>
         *        2. To encode entries, call EncodeFilterEntry() or
         *        EncodeFilterEntryInit()..EncodeFilterEntryComplete() for each
         *        FilterEntry<BR>
         *        3. Call EncodeFilterListComplete() when all entries are
         *        completed<BR>
         */
        if ((retval = sourceDirectoryFilterList.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeFilterListInit() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* encode filter list items -
         * for each filter list, element name use default values if not set
         */

        /* (0x00000001) Source Info Filter Mask */
        if ((refreshKey.filter() & Directory.ServiceFilterFlags.INFO) != 0)
        {
            /* Encodes the service's general information. */

            List<Long> capabilitiesList = new ArrayList<Long>();
            List<Qos> qosList = new ArrayList<Qos>();
            Buffer tempBuffer = CodecFactory.createBuffer();

            filterListItem.clear();
            element.clear();
            elementList.clear();
            array.clear();

            /* encode filter list item */
            filterListItem.id(Directory.ServiceFilterIds.INFO); /* (1) Service Info Filter ID */
            filterListItem.action(FilterEntryActions.SET);
            if ((retval = filterListItem.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeFilterEntryInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* encode the element list */
            /* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */
            elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
            if ((retval = elementList.encodeInit(encodeIter, null, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementListInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* ServiceName */
            tempBuffer.data(serviceName);
            element.dataType(DataTypes.ASCII_STRING);
            element.name(ElementNames.NAME);
            if ((retval = element.encode(encodeIter, tempBuffer)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* Capabilities */
            element.dataType(DataTypes.ARRAY);
            element.name(ElementNames.CAPABILITIES);
            if ((retval = element.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            array.clear();
            /*  Set Array.itemLength to 1, as each domainType uses only one byte. */
            array.itemLength(1);
            array.primitiveType(DataTypes.UINT);
            if ((retval = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* Lists the domains which this service can provide.*/
            capabilitiesList.add((long)DomainTypes.DICTIONARY); /* (5) Dictionary Message */
            capabilitiesList.add((long)DomainTypes.MARKET_PRICE); /* (6) Market Price Message */
            capabilitiesList.add((long)DomainTypes.MARKET_BY_ORDER); /* (7) Market by Order/Order Book Message */
            capabilitiesList.add((long)DomainTypes.SYMBOL_LIST); /* (10) Symbol List Messages */
            capabilitiesList.add((long)DomainTypes.YIELD_CURVE); /* (22) Yield Curve */

            /* break out of decoding array items when predetermined max capabilities (10) reached */
            for (long capability : capabilitiesList)
            {
                tmpUInt.value(capability);
                retval = arrayEntry.encode(encodeIter, tmpUInt);
                if ((retval) < CodecReturnCodes.SUCCESS)
                {
                    channel.releaseBuffer(msgBuf, error);
                    System.out.printf("EncodeArrayEntry() failed with return code: %d\n", retval);
                    /* Closes channel, cleans up and exits the application. */
                    return retval;
                }
            }

            if ((retval = array.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            if ((retval = element.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* DictionariesProvided */
            element.dataType(DataTypes.ARRAY);
            element.name(ElementNames.DICTIONARIES_PROVIDED);
            if ((retval = element.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            array.clear();

            /* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
            array.primitiveType(DataTypes.ASCII_STRING);
            array.itemLength(0);
            if ((retval = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            tempBuffer.data("RWFFld");
            if ((retval = arrayEntry.encode(encodeIter, tempBuffer)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            tempBuffer.data("RWFEnum");
            if ((retval = arrayEntry.encode(encodeIter, tempBuffer)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            if ((retval = array.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            if ((retval = element.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* DictionariesUsed */
            element.name(ElementNames.DICTIONARIES_USED);
            element.dataType(DataTypes.ARRAY);
            if ((retval = element.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            array.clear();
            /* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
            array.primitiveType(DataTypes.ASCII_STRING);
            array.itemLength(0);

            if ((retval = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            tempBuffer.data("RWFFld");
            if ((retval = arrayEntry.encode(encodeIter, tempBuffer)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            tempBuffer.data("RWFEnum");
            if ((retval = arrayEntry.encode(encodeIter, tempBuffer)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            if ((retval = array.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            if ((retval = element.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* QoS */
            Qos qos = CodecFactory.createQos();
            qos.dynamic(false);
            /* Rate is Tick By Tick, indicates every change to information is conveyed */
            qos.rate(QosRates.TICK_BY_TICK);
            /* Timeliness is Realtime, indicates information is updated as soon as new information becomes available */
            qos.timeliness(QosTimeliness.REALTIME);
            qosList.add(qos);
            element.name(ElementNames.QOS);
            element.dataType(DataTypes.ARRAY);

            if ((retval = element.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
            array.clear();
            /* If itemLength is set to 0, entries are variable length and each encoded entry can have a different length. */
            array.itemLength(0);
            array.primitiveType(DataTypes.QOS);

            /**
             * @brief Perform array item encoding (item can only be simple
             *        primitive type such as \ref Int, Real, or Date and not
             *        another Array or container type)
             *
             *        Encodes entries in an Array.<BR>
             *        Typical use:<BR>
             *        1. Call EncodeArrayInit()<BR>
             *        2. Call EncodeArrayEntry() for each item in the array<BR>
             *        3. Call EncodeArrayComplete()<BR>
             *
             * @note Only one of pEncBuffer or pData should be supplied.
             */
            if ((retval = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            if ((retval = arrayEntry.encode(encodeIter, qosList.get(0))) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            if ((retval = array.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeArrayComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            if ((retval = element.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntryComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* complete encode element list */
            if ((retval = elementList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementListComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* complete encode filter list item */
            if ((retval = filterListItem.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeFilterEntryComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
        }

        /* (0x00000002) Source State Filter Mask */
        if ((refreshKey.filter() & Directory.ServiceFilterFlags.STATE) != 0)
        {
            /* Encodes the service's state information. */

            int serviceState;
            int acceptingRequests;

            /* Specifies a status change to apply to all items provided by this service.
            	* It is equivalent to sending an StatusMsg to each item.
            	*/
            State status = CodecFactory.createState();

            filterListItem.clear();
            element.clear();
            elementList.clear();

            /* encode filter list item */
            filterListItem.id(Directory.ServiceFilterIds.STATE); /* (2) Source State Filter ID */
            filterListItem.action(FilterEntryActions.SET);
            if ((retval = filterListItem.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeFilterEntryInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* encode the element list */
            /* (0x08) The ElementList contains standard encoded content (e.g. not set defined).  */
            elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
            if ((retval = elementList.encodeInit(encodeIter, null, 0)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementListInit() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* ServiceState */
            element.name(ElementNames.SVC_STATE);
            element.dataType(DataTypes.UINT);
            /* Indicates whether the original provider of the data is available to respond to new requests. */
            serviceState = 1; /* Service is Up */
            tmpUInt.value(serviceState);
            if ((retval = element.encode(encodeIter, tmpUInt)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* AcceptingRequests */
            element.dataType(DataTypes.UINT);
            element.name(ElementNames.ACCEPTING_REQS);
            /* Indicates whether the immediate provider can accept new requests and/or handle reissue requests on already open streams. */
            acceptingRequests = 1; /* 1: Yes */
            tmpUInt.value(acceptingRequests);

            if ((retval = element.encode(encodeIter, tmpUInt)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* Status */
            element.dataType(DataTypes.STATE);
            element.name(ElementNames.STATUS);

            /* The Status element can change the state of items provided by this service.
             * Prior to changing a service status, LSEG recommends that you issue item or group
             * status messages to update item states.
             */
            status.streamState(StreamStates.OPEN);
            status.dataState(DataStates.OK);
            status.code(StateCodes.NONE);
            status.text().data("OK");
            if ((retval = element.encode(encodeIter, status)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementEntry() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* complete encode element list */
            if ((retval = elementList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeElementListComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }

            /* complete encode filter list item */
            if ((retval = filterListItem.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
            {
                channel.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeFilterEntryComplete() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return retval;
            }
        }

        /* complete encode filter list */
        if ((retval = sourceDirectoryFilterList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeFilterListComplete() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* complete encode map entry */
        if ((retval = mapEntry.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMapEntryComplete() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* complete encode map */
        if ((retval = map.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMapComplete() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* complete encode message */
        if ((retval = refreshMsg.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMsgComplete() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return retval;
        }

        /* send source directory response */
        if ((retval = sendMessage(channel, msgBuf)) < CodecReturnCodes.SUCCESS)
        {
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }
        else if (retval > CodecReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retval;
    }

}
