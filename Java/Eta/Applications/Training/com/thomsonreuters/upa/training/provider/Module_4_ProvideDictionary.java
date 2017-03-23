/*
 *|-------------------------------------------------------------------------------
 *| This source code is provided under the Apache 2.0 license and is provided	--
 *| AS IS with no warranty or guarantee of fit for purpose.  See the project's 	--
 *| LICENSE.md for details.														--
 *| Copyright Thomson Reuters 2015. All rights reserved.						--
 *|-------------------------------------------------------------------------------
 */

/*****************************************************************************************
 * This is the UPA Interactive Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step
 * training how to build a UPA OMM Interactive Provider using the UPA Transport layer.
 *
 * Main Java source file for the UPA Interactive Provider Training application. It is a
 * single-threaded client application.
 *
 *****************************************************************************************
 * UPA Interactive Provider Training Module 1a: Establish network communication
 *****************************************************************************************
 * Summary:
 * An OMM Interactive Provider application opens a listening socket on a well-known
 * port allowing OMM consumer applications to connect. Once connected, consumers
 * can request data from the Interactive Provider.
 *
 * In this module, the OMM Interactive Provider application opens a listening socket
 * on a well-known port allowing OMM consumer applications to connect.
 *
 * Detailed Descriptions:
 * The first step of any UPA Interactive Provider application is to establish
 * a listening socket, usually on a well-known port so that consumer applications
 * can easily connect. The provider uses the Transport.bind() method to open the port
 * and listen for incoming connection attempts.
 * Whenever an OMM consumer application attempts to connect, the provider uses
 * the Server.accept() method to begin the connection initialization process.
 *
 * For this simple training app, the interactive provider only supports a single client.
 *
 *****************************************************************************************
 * UPA Interactive Provider Training Module 1b: Ping (heartbeat) Management
 *****************************************************************************************
 * Summary:
 * In this module, after establishing a connection, ping messages might
 * need to be exchanged. The negotiated ping timeout is available via
 * the Channel. If ping heartbeats are not sent or received within
 * the expected time frame, the connection can be terminated. Thomson
 * Reuters recommends sending ping messages at intervals one-third the
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 * Once the connection is active, the consumer and provider applications
 * might need to exchange ping messages. A negotiated ping timeout is available
 * via Channel corresponding to each connection (this value might differ on
 * a per-connection basis). A connection can be terminated if ping heartbeats
 * are not sent or received within the expected time frame. Thomson Reuters
 * recommends sending ping messages at intervals one-third the size of the ping timeout.
 * Ping or heartbeat messages are used to indicate the continued presence of
 * an application. These are typically only required when no other information is
 * being exchanged. Because the provider application is likely sending more frequent
 * information, providing updates on any streams the consumer has requested,
 * it may not need to send heartbeats as the other data is sufficient to announce
 * its continued presence. It is the responsibility of each connection to manage
 * the sending and receiving of heartbeat messages.
 *
 *****************************************************************************************
 * UPA Interactive Provider Training Module 1c: Reading and Writing Data
 *****************************************************************************************
 * Summary:
 * In this module, when a client or server Channel.state() is
 * ChannelState.ACTIVE, it is possible for an application to receive
 * data from the connection. Similarly, when a client or server
 * Channel.state() is ChannelState.ACTIVE, it is possible for an
 * application to write data to the connection. Writing involves a several
 * step process.
 *
 * Detailed Descriptions:
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is
 * possible for an application to receive data from the connection. The
 * arrival of this information is often announced by the I/O notification
 * mechanism that the Channel.scktChannel() is registered with. The UPA
 * Transport reads information from the network as a byte stream, after
 * which it determines buffer boundaries and returns each buffer one by
 * one.
 *
 * When a client or server Channel.state() is ChannelState.ACTIVE, it is
 * possible for an application to write data to the connection. Writing
 * involves a several step process. Because the UPA Transport provides
 * efficient buffer management, the user is required to obtain a buffer
 * from the UPA Transport buffer pool. This can be the guaranteed output
 * buffer pool associated with a Channel. After a buffer is acquired,
 * the user can populate the Buffer.data and set the Buffer.length
 * to the number of bytes referred to by data. If queued information cannot
 * be passed to the network, a function is provided to allow the application
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able
 * to accept additional bytes for writing. The UPA Transport can continue to
 * queue data, even if the network is unable to write.
 *
 *****************************************************************************************
 * UPA Interactive Provider Training Module 2: Perform/Handle Login Process
 *****************************************************************************************
 * Summary:
 * Applications authenticate with one another using the Login domain model.
 * An OMM Interactive Provider must handle the consumer's Login request messages
 * and supply appropriate responses.
 *
 * In this module, after receiving a Login request, the Interactive Provider
 * can perform any necessary authentication and permissioning.
 *
 * Detailed Descriptions:
 * After receiving a Login request, the Interactive Provider can perform any
 * necessary authentication and permissioning.
 *
 * a) If the Interactive Provider grants access, it should send an RefreshMsg
 * to convey that the user successfully connected. This message should indicate
 * the feature set supported by the provider application.
 * b) If the Interactive Provider denies access, it should send an StatusMsg,
 * closing the connection and informing the user of the reason for denial.
 *
 * The login handler for this simple Interactive Provider application only allows
 * one login stream per channel. It provides functions for processing login requests
 * from consumers and sending back the responses. Functions for sending login request
 * reject/close status messages, initializing the login handler, and closing login streams
 * are also provided.
 *
 * Also please note for simple training app, the interactive provider only supports
 * one client session from the consumer, that is, only supports one channel/client connection.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA
 * Data Package.
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 3: Provide Source Directory Information
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application provides Source Directory
 * information. The Source Directory domain model conveys information about all
 * available services in the system. An OMM consumer typically requests a Source Directory
 * to retrieve information about available services and their capabilities.
 *
 * Detailed Descriptions:
 * The Source Directory domain model conveys information about all available services
 * in the system. An OMM consumer typically requests a Source Directory to retrieve
 * information about available services and their capabilities. This includes information
 * about supported domain types, the service's state, the QoS, and any item group
 * information associated with the service. Thomson Reuters recommends that at a minimum,
 * an Interactive Provider supply the Info, State, and Group filters for the Source Directory.
 *
 * a) The Source Directory Info filter contains the name and serviceId for each
 * available service. The Interactive Provider should populate the filter with information
 * specific to the services it provides.
 *
 * b) The Source Directory State filter contains status information for the service informing
 * the consumer whether the service is Up (available), or Down (unavailable).
 *
 * c) The Source Directory Group filter conveys item group status information, including
 * information about group states, as well as the merging of groups. If a provider determines
 * that a group of items is no longer available, it can convey this information by sending
 * either individual item status messages (for each affected stream) or a Directory message
 * containing the item group status information.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA
 * Data Package.
 * 
 ************************************************************************
 * UPA Interactive Provider Training Module 4: Provide Necessary Dictionaries
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application provides Necessary Dictionaries.
 * Some data requires the use of a dictionary for encoding or decoding. The dictionary 
 * typically defines type and formatting information, and tells the application how to 
 * encode or decode information.
 *
 * Detailed Descriptions:
 * Some data requires the use of a dictionary for encoding or decoding. The dictionary 
 * typically defines type and formatting information, and tells the application how to 
 * encode or decode information. Content that uses the FieldList type requires the 
 * use of a field dictionary (usually the Thomson Reuters RDMFieldDictionary, though it 
 * can instead be a user-defined or modified field dictionary).
 * 
 * The Source Directory message should notify the consumer about dictionaries needed to 
 * decode content sent by the provider. If the consumer needs a dictionary to decode 
 * content, it is ideal that the Interactive Provider application also make this dictionary
 * available to consumers for download. The provider can inform the consumer whether the
 * dictionary is available via the Source Directory.
 * 
 * If loading from a file, UPA offers several utility functions for loading and managing 
 * a properly-formatted field dictionary. There are also utility functions provided to 
 * help the provider encode into an appropriate format for downloading. 
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *****************************************************************************************/

package com.thomsonreuters.upa.training.provider;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.shared.DirectoryRejectReason;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WritePriorities;

public class Module_4_ProvideDictionary
{
    public static boolean loginRequestInfo_IsInUse;
    public static int loginRequestInfo_StreamId;
    public static String loginRequestInfo_Username;
    public static String loginRequestInfo_ApplicationId;
    public static String loginRequestInfo_ApplicationName;
    public static String loginRequestInfo_Position;
    public static String loginRequestInfo_Password;
    public static String loginRequestInfo_InstanceId;
    public static String loginRequestInfo_Role;

    public static int sourceDirectoryRequestInfo_StreamId;
    public static String sourceDirectoryRequestInfo_ServiceName;
    public static int sourceDirectoryRequestInfo_ServiceId;
    public static boolean sourceDirectoryRequestInfo_IsInUse;

    public static enum LoginRejectReason
    {
        MAX_LOGIN_REQUESTS_REACHED, NO_USER_NAME_IN_REQUEST
    }

    public static int fieldDictionaryRequestInfo_StreamId;
    public static String fieldDictionaryRequestInfo_DictionaryName;
    public static MsgKey fieldDictionaryRequestInfo_MsgKey = CodecFactory.createMsgKey();
    public static boolean fieldDictionaryRequestInfo_IsInUse;

    public static int enumTypeDictionaryRequestInfo_StreamId;
    public static String enumTypeDictionaryRequestInfo_DictionaryName;
    public static MsgKey enumTypeDictionaryRequestInfo_MsgKey = CodecFactory.createMsgKey();
    public static boolean enumTypeDictionaryRequestInfo_IsInUse;

    /* dictionary file name  */
    final static String fieldDictionaryFileName = "RDMFieldDictionary";
    /* dictionary download name */
    final static String fieldDictionaryDownloadName = "RWFFld";
    /* enum table file name */
    final static String enumTypeDictionaryFileName = "enumtype.def";
    /* enum table download name */
    final static String enumTypeDictionaryDownloadName = "RWFEnum";

    private static final int MAX_DICTIONARY_STATUS_MSG_SIZE = 1024;

    public enum DictionaryRejectReason
    {
        UNKNOWN_DICTIONARY_NAME, MAX_DICTIONARY_REQUESTS_REACHED, DICTIONARY_RDM_DECODER_FAILED;
    }

    public static void main(String[] args) throws IOException
    {
        /**************************************************************************************************
         * DECLARING VARIABLES
         **************************************************************************************************/
        /* Create a server to eventually accept connection requests */
        Server upaSrvr = null;

        boolean clientAccepted = false;

        /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
        InitArgs initArgs = TransportFactory.createInitArgs();

        /* Create error to keep track of any errors that may occur in Transport methods */
        Error error = TransportFactory.createError();

        /* For this simple training app, the interactive provider only supports a single client. If the consumer disconnects,
         * the interactive provider would simply exit.
         *
         * If you want the provider to support multiple client sessions at the same time, you need to implement support
         * for multiple client sessions feature similar to what Provider example is doing.
         */
        /* Create a channel to keep track of connection */
        Channel channel = null;

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long upaRuntime = 0;
        long runTime = 0;

        /* Create decode iterator to decode the contents of the buffer */
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();

        /* Create an I/O notification mechanism (a Selector in our case) for our channel */
        Selector selector = null;

        /* Create and populate binding options to specify listening socket preferences */
        BindOptions bindOpts = TransportFactory.createBindOptions();

        /* Create accept options to specify any options for accepting */
        AcceptOptions acceptOpts = TransportFactory.createAcceptOptions();

        /* Create initialization progress info (InProgInfo) to keep track of channel initialization with Channel.init() */
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        /* Create a bit mask to specify what I/O notification operations to keep track of (e.g. ACCEPT)*/
        short opMask = 0;

        /* Max waiting time */
        final int timeOutSeconds = 60;

        int retCode;

        /* the default option parameters */
        /* server is running on port number 14002 */
        String srvrPortNo = "14002";
        /* default service name is "DIRECT_FEED" used in source directory handler */
        String serviceName = "DIRECT_FEED";

        /* use default runTime of 300 seconds */
        runTime = 300;

        /* User specifies options such as address, port, and interface from the command line.
         * User can have the flexibility of specifying any or all of the parameters in any order.
         */
        if (args.length > 1)
        {
            int i = 0;

            while (i < args.length)
            {

                if ((args[i].equals("-p")) == true)
                {
                    i += 2;
                    srvrPortNo = args[i - 1];
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
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-p <SrvrPortNo>] [-r <runTime>] [-s <ServiceName>]\n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        /* In this app, we are only interested in using 2 dictionaries:
         * - Thomson Reuters Field Dictionary (RDMFieldDictionary) and
         * - Enumerated Types Dictionaries (enumtype.def)
         *
         * We will just use dictionaries that are available locally in a file.
         */

        /* data dictionary */
        DataDictionary _dictionary;
        _dictionary = CodecFactory.createDataDictionary();

        /**************************************************************************************************
         * INITIALIZATION
         **************************************************************************************************/
        /*********************************************************
         * Server/Provider Application Life Cycle Major Step 1: Initialize UPA
         * Transport using Initialize The first UPA Transport function that an
         * application should call. This creates and initializes internal memory
         * and structures, as well as performing any bootstrapping for
         * underlying dependencies. The Initialize function also allows the user
         * to specify the locking model they want applied to the UPA Transport.
         *********************************************************/

        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        upaRuntime = currentTime + runTime * 1000;

        /* populate bind options, then pass to Bind function -
         * UPA Transport should already be initialized
         */
        /* Set bind options */
        bindOpts.serviceName(srvrPortNo); /* server is running on default port number 14002 */
        bindOpts.pingTimeout(60); /* servers desired ping timeout is 60 seconds, pings should be sent every 20 */
        bindOpts.minPingTimeout(30); /* min acceptable ping timeout is 30 seconds, pings should be sent every 10 */

        /* set up buffering, configure for shared and guaranteed pools */
        bindOpts.guaranteedOutputBuffers(1000);
        bindOpts.maxOutputBuffers(2000);
        bindOpts.sharedPoolSize(50000);
        bindOpts.sharedPoolLock(true);

        bindOpts.serverBlocking(false); /* perform non-blocking I/O */
        bindOpts.channelsBlocking(false); /* perform non-blocking I/O */
        bindOpts.compressionType(CompressionTypes.NONE); /* server does not desire compression for this connection */

        /* populate version and protocol with RWF information (found in Iterators.h) or protocol specific info */
        bindOpts.protocolType(Codec.protocolType()); /* Protocol type definition for RWF */
        bindOpts.majorVersion(Codec.majorVersion());
        bindOpts.minorVersion(Codec.minorVersion());
        /*********************************************************
         * We will just use dictionaries that are available locally in a file.
         * We will exit the interactive provider application if any dictionary
         * cannot be loaded properly or does not exist in the current runtime
         * path.
         *
         * For performance considerations, it is recommended to first load field
         * and enumerated dictionaries from local files, if they exist, at the
         * earlier stage of the interactive provider applications.
         *
         * When loading from local files, UPA offers several utility functions
         * to load and manage a properly-formatted field dictionary and enum
         * type dictionary.
         *********************************************************/

        /* clear the DataDictionary dictionary before first use/load
         * This should be done prior to the first call of a dictionary loading function, if the initializer is not used.
         */
        _dictionary.clear();
        /* load field dictionary from file - adds data from a Field Dictionary file to the DataDictionary */
        if (_dictionary.loadFieldDictionary(fieldDictionaryFileName, error) < 0)
        {
            System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: " + error.text());
            System.exit(TransportReturnCodes.FAILURE);

        }
        else
        {
            System.out.printf("Successfully loaded field dictionary from (local) file.\n\n");

        }

        if (_dictionary.loadEnumTypeDictionary(enumTypeDictionaryFileName, error) < 0)
        {
            System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: " + error.text());
            System.exit(TransportReturnCodes.FAILURE);

        }
        else
        {
            System.out.printf("Successfully loaded enum type dictionary from (local) file.\n\n");

        }

        System.out.printf("UPA provider app has successfully loaded both dictionaries from local files.\n\n");

        /**************************************************************************************************
         * Bind and receive a server
         **************************************************************************************************/
        /* Bind UPA server */
        if ((upaSrvr = Transport.bind(bindOpts, error)) == null)
        {
            System.out.printf("Error (%d) (errno: %d) encountered with Bind. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
            /* End application, uninitialize to clean up first */
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }

        System.out.printf("Server bound on port %d\n", upaSrvr.portNumber());

        opMask |= SelectionKey.OP_ACCEPT;

        /* Register our channel to the selector and watch for I/O notifications specified by our operation mask */
        try
        {
            selector = Selector.open();
            upaSrvr.selectableChannel().register(selector, opMask, upaSrvr);
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
        }

        SelectionKey key = null;
        Set<SelectionKey> selectedKeys = null;
        Iterator<SelectionKey> keyIter = null;

        /**************************************************************************************************
         * MAIN LOOP - Listen for connection requests until we accept a client
         **************************************************************************************************/
        /* Main Loop #1 for detecting incoming client connections. */
        while (!clientAccepted)
        {
            /* Wait for any I/O notification updates in the channel for our specified amt of seconds (e.g. 60 sec.)*/
            selector.select(timeOutSeconds * 1000);

            /* Create an iterator from the selector's updated channels*/
            selectedKeys = selector.selectedKeys();
            keyIter = selectedKeys.iterator();

            /* If our channel has been updated */
            if (keyIter.hasNext())
            {
                /* Check if channel is ACCEPT-able */
                key = upaSrvr.selectableChannel().keyFor(selector);
                if (key.isAcceptable())
                {
                    acceptOpts.nakMount(false);

                    /*****************************************
                     * Step 3) Accept the connection request *
                     *****************************************/
                    /*********************************************************
                     * Server/Provider Application Life Cycle Major Step 3:
                     * Accept connection using Accept This step is performed per
                     * connected client/connection/channel Uses the Server that
                     * represents the listening socket connection and begins the
                     * accepting process for an incoming connection request.
                     * Returns an Channel that represents the client connection.
                     * In the event of an error, NULL is returned and additional
                     * information can be found in the Error structure. The
                     * Accept function can also begin the rejection process for
                     * a connection through the use of the AcceptOptions
                     * structure. Once a connection is established and
                     * transitions to the _CH_STATE_ACTIVE state, this Channel
                     * can be used for other transport operations.
                     *********************************************************/
                    /* An OMM Provider application can begin the connection accepting or rejecting process by using the Accept function */
                    if ((channel = upaSrvr.accept(acceptOpts, error)) == null)
                    {
                        System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                        Transport.uninitialize();
                    }
                    else
                    {
                        /* For this simple training app, the interactive provider only supports one client session from the consumer. */
                        System.out.printf("New client on channel!\n");
                        /*set clientAccepted to be TRUE and exit the while Main Loop #1*/
                        clientAccepted = true;
                    }
                }
            }
        }

        /* Keep track of READ notifications */
        opMask = SelectionKey.OP_READ;

        /*****************************************************************************************
         * Loop 2) Keep calling Channel.init() until it has properly connected
         * us to the server *
         *****************************************************************************************/
        /* Main Loop #2 for getting connection active and successful completion of the initialization process
         * Currently, the main loop would exit if an error condition is triggered 
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
                System.out.printf("Exception: %s\n", e.getMessage());
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
            }

            /* Wait for any I/O notification updates in the channel for our specified amt of seconds (e.g. 60 sec.)*/
            selector.select(timeOutSeconds * 1000);

            /* Create an iterator from the selector's updated channels*/
            selectedKeys = selector.selectedKeys();
            keyIter = selectedKeys.iterator();

            /* If our channel has not updated, we must have timed out */
            if (!keyIter.hasNext())
            {
                System.out.printf("Channel initialization has timed out.\n");
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
            }
            else
            {
                /* Check if channel is READ-able */
                key = channel.selectableChannel().keyFor(selector);
                if (key.isReadable())
                {
                    /****************************************************************************
                     * Step 4) Call Channel.init() to progress channel
                     * initialization further. * * This method is called
                     * multiple times throughout the Loop 2, as it makes * more
                     * progress towards channel initialization. *
                     ***************************************************************************/
                    /* Internally, the UPA initialization process includes several actions. The initialization includes
                     * any necessary UPA connection handshake exchanges, including any HTTP or HTTPS negotiation.
                     * Compression, ping timeout, and versioning related negotiations also take place during the
                     * initialization process. This process involves exchanging several messages across the connection,
                     * and once all message exchanges have completed the Channel.state will transition. If the connection
                     * is accepted and all types of negotiations completed properly, the Channel state will become
                     * ACTIVE. If the connection is rejected, either due to some kind of negotiation failure
                     * or because an Server rejected the connection by setting nakMount to TRUE, the Channel state
                     * will become CLOSED.
                     *
                     * Note:
                     * For both client and server channels, more than one call to InitChannel can be required to complete
                     * the channel initialization process.
                     */
                    if ((retCode = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                    }

                    /* Deduce an action from return code of Channel.init() */
                    switch (retCode)
                    {
                        case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                        {
                            /* Switch to a new channel if required */
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
                                opMask = SelectionKey.OP_READ;
                                System.out.printf("Channel switch, NEW: %d OLD %d\n", channel.selectableChannel(), channel.oldSelectableChannel());
                                try
                                {
                                    key = inProgInfo.oldSelectableChannel().keyFor(selector);
                                    key.cancel();
                                }
                                catch (Exception e)
                                {
                                    System.out.printf("Exception: %s\n", e.getMessage());
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                }
                                try
                                {
                                    channel.selectableChannel().register(selector, opMask, channel);
                                }
                                catch (Exception e)
                                {
                                    System.out.printf("Exception: %s\n", e.getMessage());
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                }
                            }
                            else
                            {
                                System.out.printf("Channel init in progress...\n");
                            }
                        }
                            break;
                        /* channel connection becomes active!
                         * Once a connection is established and transitions to the _CH_STATE_ACTIVE state,
                         * this Channel can be used for other transport operations.
                         */
                        case TransportReturnCodes.SUCCESS:
                        {
                            System.out.printf("Channel is now active!  Reading and writing can begin!\n");

                            /*********************************************************
                             * Connection is now active. The Channel can be used
                             * for all additional transport functionality (e.g.
                             * reading, writing) now that the state transitions
                             * to ACTIVE
                             *********************************************************/

                            /* After channel is active, use UPA Transport utility function GetChannelInfo to query Channel negotiated
                             * parameters and settings and retrieve all current settings. This includes maxFragmentSize and negotiated
                             * compression information as well as many other values.
                             */

                            /* Populate information from channel */
                            if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                            {
                                System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                            }

                            /* Print out basic channel info */
                            System.out.printf("\nChannel Info:\n" + "Max Fragment Size:         %d\n" + "Max Output Buffers:        %d, %d  Guaranteed\n" + "Input Buffers:             %d\n" + "Send/Receive Buffer Sizes: %d/%d\n" + "Ping Timeout:              %d\n",
                                              channelInfo.maxFragmentSize(), /*  This is the max fragment size before fragmentation and reassembly is necessary. */
                                              channelInfo.maxOutputBuffers(), /* This is the maximum number of output buffers available to the channel. */
                                              channelInfo.guaranteedOutputBuffers(), /*  This is the guaranteed number of output buffers available to the channel. */
                                              channelInfo.numInputBuffers(), /*  This is the number of input buffers available to the channel. */
                                              channelInfo.sysSendBufSize(), /*  This is the systems Send Buffer size. This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
                                              channelInfo.sysRecvBufSize(), /*  This is the systems Receive Buffer size. This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
                                              channelInfo.pingTimeout()); /* This is the value of the negotiated ping timeout */
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

                            System.out.printf("\n\n");

                            /*********************************************************
                             * Server/Provider Application Life Cycle Major Step
                             * 7: Closes a listening socket associated with an
                             * Server. This will release any pool based
                             * resources back to their respective pools, close
                             * the listening socket, and perform any additional
                             * necessary cleanup. Any established connections
                             * will remain open, allowing for continued
                             * information exchange.
                             *********************************************************/

                            /* clean up server using CloseServer call.
                             * If a server is being shut down, the CloseServer function should be used to close the listening socket and perform
                             * any necessary cleanup. All currently connected Channels will remain open. This allows applications to continue
                             * to send and receive data, while preventing new applications from connecting. The server has the option of calling
                             * CloseChannel to shut down any currently connected applications.
                             * When shutting down the UPA Transport, the application should release any unwritten pool buffers.
                             * The listening socket can be closed by calling CloseServer. This prevents any new connection attempts.
                             * If shutting down connections for all connected clients, the provider should call CloseChannel for each connection client.
                            */
                            if ((upaSrvr == null) & (upaSrvr.close(error) < TransportReturnCodes.SUCCESS))
                            {
                                System.out.printf("Error (%d) (errno: %d) encountered with CloseServer.  Error Text : %s\n", error.errorId(), error.sysError(), error.text());

                                /* End application, uninitialize to clean up first */
                                Transport.uninitialize();
                                System.exit(TransportReturnCodes.FAILURE);
                            }

                            /*set upaSrvr to be null*/
                            upaSrvr = null;
                        }
                            break;
                        default: /* Error handling */
                        {
                            System.out.printf("Unexpected init return code: %d\n", retCode);
                            closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                        }
                            break;
                    }
                }
            }
        }

        /* Initialize ping management handler */
        initPingManagement(channel);

        clearLoginRequestInfo();

        /* Clears the source directory request information */
        clearSourceDirectoryReqInfo();

        /* Clears the dictionary request information */
        clearDictionaryReqInfo();

        /* set the ServiceName and Service Id */
        sourceDirectoryRequestInfo_ServiceName = serviceName;
        /* service id associated with the service name of provider */
        sourceDirectoryRequestInfo_ServiceId = 1234;

        /****************************************************
         * Loop 3) Check ping operations until runtime ends *
         ****************************************************/
        /* Here were are using a new Main loop #3. An alternative design would be to combine this Main loop #3 (message processing)
         * with the other 2 earlier Main loops, namely, Main Loop #1 (detecting incoming client connections), and
         * Main Loop #2 (getting connection active and successful completion of the initialization process) as a single provider Main Loop.
         * Some bookkeeping would be required for that approach.
         */

        /* Main Loop #3 for message processing (reading data, writing data, and ping management, etc.)
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
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
            }

            /* Wait 1 second for any I/O notification updates in the channel */
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
                    /* Initialize to a positive value for retCode in case we have more data that is available to read */
                    retCode = 1;

                    /******************************************************
                     * Loop 4) Read and decode for all buffers in channel *
                     ******************************************************/
                    while (retCode > TransportReturnCodes.SUCCESS)
                    {
                        /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                        ReadArgs readArgs = TransportFactory.createReadArgs();

                        /**************************************************
                         * Step 5) Read message from channel into buffer *
                         **************************************************/
                        msgBuf = channel.read(readArgs, error);

                        if (msgBuf != null)
                        {
                            /* if a buffer is returned, we have data to process and code is success */

                            /* Processes a response from the channel/connection. This consists of performing a high level decode of the message and then
                             * calling the applicable specific function for further processing.
                             */

                            /* No need to clear the message before we decode into it. UPA Decoding populates all message members (and that is true for any
                             * decoding with UPA, you never need to clear anything but the iterator)
                             */
                            /* We have data to process */

                            /* Create message to represent buffer data */
                            Msg msg = CodecFactory.createMsg();

                            /* This ClearDecodeIterator clear iterator function should be used to achieve the best performance while clearing the iterator. */
                            /* Clears members necessary for decoding and readies the iterator for reuse. You must clear DecodeIterator
                             * before decoding content. For performance purposes, only those members required for proper functionality are cleared.
                             */
                            decodeIter.clear();

                            /* Set the RWF version to decode with this iterator */
                            int ret = decodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
                            /* Associates the DecodeIterator with the Buffer from which to decode. */
                            if ((ret) != CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("\nSetDecodeIteratorBuffer() failed with return code: %d\n", ret);
                                /* Closes channel, closes server, cleans up and exits the application. */
                                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                            }
                            /******************************************
                             * Step 6) Decode buffer message *
                             ******************************************/
                            if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                            }

                            /* Deduce an action based on the domain type of the message */
                            switch (msg.domainType())
                            {
                                case DomainTypes.LOGIN:
                                {
                                    if ((retCode = processLoginRequest(channel, msg, decodeIter, error)) > TransportReturnCodes.SUCCESS)
                                    {
                                        /* There is still data left to flush, leave our write notification enabled so we get called again.
                                         * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                         */

                                        /* set write flag if there's still other data queued */
                                        /* flush is done by application */
                                        opMask |= SelectionKey.OP_WRITE;
                                    }
                                    else if (retCode < TransportReturnCodes.SUCCESS)
                                    {
                                        clearLoginRequestInfo();
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                    }
                                }
                                    break;

                                case DomainTypes.SOURCE:
                                {
                                    if ((retCode = processSourceDirectoryRequest(channel, decodeIter, msg, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
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
                                        /* Closes channel, closes server, cleans up and exits the application. */
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                    }
                                }
                                    break;
                                /* (5) Dictionary Message */
                                case DomainTypes.DICTIONARY:
                                {
                                    retCode = processDictionaryRequest(channel, msg, decodeIter, _dictionary, channelInfo.maxFragmentSize());
                                    if (retCode > TransportReturnCodes.SUCCESS)
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
                                        /* Closes channel, closes server, cleans up and exits the application. */
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                    }
                                }
                                    break;
                                default: /* Error handling */
                                {
                                    System.out.printf("Unhandled Domain Type: %d\n", msg.domainType());
                                }
                                    break;
                            }

                            /* Acknowledge that a ping has been received */
                            receivedClientMsg = true;
                            System.out.printf("Ping message has been received successfully from the client due to data message ...\n\n");
                        }
                        else
                        {
                            /* Deduce an action from the return code of Channel.read() */
                            retCode = readArgs.readRetVal();
                            /* keep track of the return values from read so data is not stranded in the input buffer.
                             * Handle return codes appropriately, not all return values are failure conditions
                             */
                            switch (retCode)
                            {
                                /* Acknowledge that a ping has been received */
                                case TransportReturnCodes.READ_PING:
                                    /* Update ping monitor */
                                    /* set flag for server message received */
                                    receivedClientMsg = true;
                                    System.out.printf("Ping message has been received successfully from the client due to ping message ...\n\n");
                                    break;

                                /* Switch to a new channel if required */
                                case TransportReturnCodes.READ_FD_CHANGE:
                                    opMask = SelectionKey.OP_READ;
                                    System.out.printf("Channel switch, NEW: %d OLD %d\n", channel.selectableChannel(), channel.oldSelectableChannel());
                                    try
                                    {
                                        key = channel.selectableChannel().keyFor(selector);
                                        key.cancel();
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                    }
                                    break;

                                case TransportReturnCodes.READ_WOULD_BLOCK:
                                    /* Nothing to read */
                                    break;
                                case TransportReturnCodes.READ_IN_PROGRESS:
                                    /* Reading from multiple threads */
                                    break;
                                case TransportReturnCodes.INIT_NOT_INITIALIZED:
                                case TransportReturnCodes.FAILURE:
                                    System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                default:
                                {
                                    if (retCode < 0)
                                    {
                                        System.out.printf("Unexpected return code\n");
                                        closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                                    }
                                }
                            }
                        }
                    }
                }

                key = channel.selectableChannel().keyFor(selector);
                if (key.isWritable())
                {
                    retCode = TransportReturnCodes.FAILURE;

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
                                channel.selectableChannel().register(selector, key.interestOps() - SelectionKey.OP_WRITE, channel);
                            }
                                break;
                            case TransportReturnCodes.FAILURE: /* fall through to default. */
                            default: /* Error handling */
                            {
                                System.out.printf("Error (%d) (errno: %d): %s", error.errorId(), error.sysError(), error.text());
                                /* Connection should be closed, return failure */
                                /* Closes channel/connection, cleans up and exits the application. */
                                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                            }
                        }
                    }
                }
            }

            /* Processing ping management handler */
            if ((retCode = processPingManagementHandler(channel)) > TransportReturnCodes.SUCCESS)
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
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
            }

            /* get current time */
            currentTime = System.currentTimeMillis();

            /* If the runtime has expired */
            if (System.currentTimeMillis() >= upaRuntime)
            {
                /* Closes all streams for the Interactive Provider after run-time has elapsed in our simple Interactive Provider example.
                 * If the provider application must shut down, it can either leave consumer connections intact or shut them down. If the provider
                 * decides to close consumer connections, the provider should send an StatusMsg on each connection's Login stream closing the stream.
                 * At this point, the consumer should assume that its other open streams are also closed.
                 */

                /* send close status messages to all streams on the connected client channel */

                if ((retCode = sendDictionaryCloseStatusMessage(channel, error, channelInfo.maxFragmentSize())) != TransportReturnCodes.SUCCESS)

                {
                    /* When you send close status message to dictionary stream, we want to make a best effort to get this across the network as it will gracefully
                     * close all open dictionary consumer streams. If this cannot be flushed or failed, this application will just close the connection
                     * for simplicity.
                     */

                    /* Closes channel, closes server, cleans up and exits the application. */
                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                }
                /* send close status message to login stream */

                if ((retCode = sendLoginCloseStatusMsg(channel, error)) != TransportReturnCodes.SUCCESS)
                {
                    /* When you send close status message to login stream, we want to make a best effort to get this across the network as it will gracefully
                     * close all open consumer streams. If this cannot be flushed or failed, this application will just close the connection
                     * for simplicity.
                     */

                    /* Closes channel, closes server, cleans up and exits the application. */
                    closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.FAILURE, _dictionary);
                }
                /* flush before exiting */
                else if (retCode > TransportReturnCodes.SUCCESS)
                {
                    key = channel.selectableChannel().keyFor(selector);
                    if (key.isWritable())
                    {
                        retCode = 1;

                        /******************************************************
                         * Loop 5) Flush all remaining buffers to channel *
                         ******************************************************/
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {
                            retCode = channel.flush(error);
                        }
                        if (retCode < TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Flush has failed with return code %d - <%s>\n", retCode, error.text());
                        }
                    }
                }

                System.out.printf("UPA Server run-time has expired...\n\n");
                closeChannelServerCleanUpAndExit(channel, upaSrvr, selector, TransportReturnCodes.SUCCESS, _dictionary);
            }
        }
    }

    /*********************************************************
     * Closes channel and selector and exits application. *
     * 
     * @param channel - Channel to be closed *
     * @param server - server to be closed *
     * @param code - if exit is due to errors/exceptions *
     * @param selector - Selector to be closed *
     * @param dictionary - Dictionary *
     *********************************************************/
    public static void closeChannelServerCleanUpAndExit(Channel channel, Server server, Selector selector, int code, DataDictionary dictionary)
    {
        Error error = TransportFactory.createError();

        try
        {
            selector.close();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
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
        /* clean up server using CloseServer call.
         * If a server is being shut down, the CloseServer function should be used to close the listening socket and perform
         * any necessary cleanup. All currently connected Channels will remain open. This allows applications to continue
         * to send and receive data, while preventing new applications from connecting. The server has the option of calling
         * CloseChannel to shut down any currently connected applications.
         * When shutting down the UPA Transport, the application should release any unwritten pool buffers.
         * The listening socket can be closed by calling CloseServer. This prevents any new connection attempts.
         * If shutting down connections for all connected clients, the provider should call CloseChannel for each connection client.
        */
        if ((server != null) && server.close(error) < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
        }

        /* when users are done, they should unload dictionaries to clean up memory */
        if (dictionary == null)
            System.out.printf("\nNULL Dictionary pointer.\n");
        else
            dictionary.clear();
        /*********************************************************
         * Server/Provider Application Life Cycle Major Step 8: Uninitialize UPA
         * Transport using Uninitialize The last UPA Transport function that an
         * application should call. This uninitialized internal data structures
         * and deletes any allocated memory.
         *********************************************************/
        /* All UPA Transport use is complete, must uninitialize.
         * The uninitialization process allows for any heap allocated memory to be cleaned up properly.
         */
        Transport.uninitialize();
        /* For applications that do not exit due to errors/exceptions such as:
         * Exits the application if the run-time has expired.
         */
        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("\nUPA Interactive Provider Training Application successfully ended.\n");
        }

        /* End application */
        System.exit(code);
    }

    /**************************************************************************************************
     * Closes a dictionary stream. streamId - The stream id to close the
     * dictionary for
     **************************************************************************************************/
    public static void closeDictionary(Channel chnl, int streamId, Error error, DataDictionary _dictionary)
    {
        /* find the original request information associated with streamId */
        if (fieldDictionaryRequestInfo_StreamId == streamId)
        {
            System.out.printf("Closing dictionary stream id %d with dictionary name: %s\n", fieldDictionaryRequestInfo_StreamId, fieldDictionaryRequestInfo_DictionaryName);

            /* Clears the original field dictionary request information */
            clearDictionaryReqInfo();
        }
        else if (enumTypeDictionaryRequestInfo_StreamId == streamId)
        {
            System.out.printf("Closing dictionary stream id %d with dictionary name: %s\n", enumTypeDictionaryRequestInfo_StreamId, enumTypeDictionaryRequestInfo_DictionaryName);

            /* Clears the original enumType dictionary request information */
            clearDictionaryReqInfo();
        }

    }

    public static void clearDictionaryReqInfo()
    {
        fieldDictionaryRequestInfo_StreamId = 0;

        fieldDictionaryRequestInfo_MsgKey.clear(); /* Clears an  message key */
        fieldDictionaryRequestInfo_MsgKey.name().data(fieldDictionaryRequestInfo_DictionaryName);

        /* (0x07) "Normal" Verbosity, e.g. all but description */
        fieldDictionaryRequestInfo_MsgKey.filter(Dictionary.VerbosityValues.NORMAL);
        fieldDictionaryRequestInfo_IsInUse = false;

        enumTypeDictionaryRequestInfo_StreamId = 0;

        enumTypeDictionaryRequestInfo_MsgKey.clear(); /* Clears an  message key */
        enumTypeDictionaryRequestInfo_MsgKey.name().data(enumTypeDictionaryRequestInfo_DictionaryName);
        /* (0x07) "Normal" Verbosity, e.g. all but description */
        enumTypeDictionaryRequestInfo_MsgKey.filter(Dictionary.VerbosityValues.NORMAL);
        enumTypeDictionaryRequestInfo_IsInUse = false;
    }

    /*********************************************************
     * Initializes the ping times for upaChannel.
     * 
     * @param channel - The channel for ping management info initialization
     *********************************************************/

    static int pingTimeoutServer; /* server ping timeout */
    static int pingTimeoutClient; /* client ping timeout */
    static long nextReceivePingTime; /* time client should receive next message/ping from server */
    static long nextSendPingTime; /* time to send next ping from client */
    static boolean receivedClientMsg; /* flag for server message received */

    public static void initPingManagement(Channel channel)
    {
        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* set ping timeout for server and client */
        /* Applications are able to configure their desired pingTimeout values, where the ping timeout is the point at which a connection
         * can be terminated due to inactivity. Heartbeat messages are typically sent every one-third of the pingTimeout, ensuring that
         * heartbeats are exchanged prior to a timeout occurring. This can be useful for detecting loss of connection prior to any kind of
         * network or operating system notification that may occur.
         */
        pingTimeoutServer = channel.pingTimeout() / 3;
        pingTimeoutClient = channel.pingTimeout();

        /* set time to send next ping to remote connection */
        nextSendPingTime = currentTime + pingTimeoutServer * 1000;

        /* set time should receive next ping from remote connection */
        nextReceivePingTime = currentTime + pingTimeoutClient * 1000;

        receivedClientMsg = false;

    }

    /*********************************************************
     * Processing ping management handler
     * 
     * @param channel - The channel for ping management processing
     *********************************************************/

    public static int processPingManagementHandler(Channel channel)
    {
        /* Handles the ping processing for upaChannel. Sends a ping to the client if the next send ping time has arrived and
         * checks if a ping has been received from the client within the next receive ping time.
         */
        int retval = TransportReturnCodes.SUCCESS;
        Error error = TransportFactory.createError();

        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* handle server pings */
        if (currentTime >= nextSendPingTime)
        {
            /* send ping to client */
            /*********************************************************
             * Server/Provider Application Life Cycle Major Step 5: Ping using
             * Ping Attempts to write a heartbeat message on the connection.
             * This function expects the Channel to be in the active state. If
             * an application calls the Ping function while there are other
             * bytes queued for output, the UPA Transport layer will suppress
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
                        System.out.printf("Ping message has been sent successfully to the client ...\n\n");
                    }
                        break;
                    case TransportReturnCodes.FAILURE: /* fall through to default. */
                    default: /* Error handling */
                    {
                        System.out.printf("Error (%d) (errno: %d) encountered with Ping(). Error Text:%s\n", error.errorId(), error.sysError(), error.text());
                        /* Closes channel/connection, cleans up and exits the application. */
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            /* set time to send next ping from server */
            nextSendPingTime = currentTime + (pingTimeoutServer * 1000);
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
                nextReceivePingTime = currentTime + (pingTimeoutClient * 1000);
            }
            else /* lost contact with server */
            {
                /* Lost contact with remote (connection) */
                error.text("Lost contact with client...\n");
                System.out.printf("Error (%d) (errno: %d) %s\n", error.errorId(), error.sysError(), error.text());
                return TransportReturnCodes.FAILURE;

            }
        }
        return retval;
    }

    /**************************************************************
     * Sends a message buffer to the channel *
     * 
     * @param channel - the Channel to send the message buffer to *
     * @param msgBuf - the buffer to be sent *
     * @return status code *
     **************************************************************/
    public static int sendMessage(Channel channel, TransportBuffer msgBuf)
    {
        int retCode = 0;
        Error error = TransportFactory.createError();
        /* send the request */

        /*********************************************************
         * Server/Provider Application Life Cycle Major Step 5: Write using
         * Writer Writer performs any writing or queuing of data. This function
         * expects the Channel to be in the active state and the buffer to be
         * properly populated, where length reflects the actual number of bytes
         * used. This function allows for several modifications to be specified
         * for this call. Here we use _WRITE_NO_FLAGS. For more information on
         * other flag enumeration such as _WRITE_DO_NOT_COMPRESS or
         * _WRITE_DIRECT_SOCKET_WRITE, see the UPA C developers guide for Write
         * Flag Enumeration Values supported by UPA Transport.
         *
         * The UPA Transport also supports writing data at different priority
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

        /* Now write the data - keep track of UPA Transport return code -
         * Because positive values indicate bytes left to write, some negative transport layer return codes still indicate success
         */
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        writeArgs.priority(WritePriorities.HIGH);

        if ((retCode = channel.write(msgBuf, writeArgs, error)) == TransportReturnCodes.WRITE_CALL_AGAIN)
        {
            /* (-10) Transport Success: Write is fragmenting the buffer and needs to be called again with the same buffer. This indicates that Write was
             * unable to send all fragments with the current call and must continue fragmenting
             */

            /* Large buffer is being split by transport, but out of output buffers. Schedule a call to Flush and then call the Write function again with
             * this same exact buffer to continue the fragmentation process. Only release the buffer if not passing it to Write again. */

            /* call flush and write again - breaking out if the return code is something other than _RET_WRITE_CALL_AGAIN (write call again) */

            while (retCode == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                if ((retCode = channel.flush(error)) < TransportReturnCodes.SUCCESS)
                {
                    System.out.printf("Flush has failed with return code %d - <%s>\n", retCode, error.text());
                }
                retCode = channel.write(msgBuf, writeArgs, error);
            }

        }

        if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* The write was successful and there is more data queued in UPA Transport. The Channel.flush() method should be used to continue attempting to flush data
             * to the connection. UPA will release buffer.
             */

            /* Flush needs to be done by application */
        }
        else
        {
            switch (retCode)
            {
                case TransportReturnCodes.SUCCESS:
                {
                    /* Successful write and all data has been passed to the connection */
                    /* Continue with next operations. UPA will release buffer.*/
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

                        /* Set write flag if flush failed */
                        /* Flush needs to be done by application */

                        /* Channel is still open, but Channel.write() tried to flush internally and failed.
                         * Return positive value so the caller knows there's bytes to flush.
                         */
                        return TransportReturnCodes.SUCCESS + 1;
                    }
                }
                case TransportReturnCodes.NO_BUFFERS:
                {
                    channel.releaseBuffer(msgBuf, error);
                }
                    break;
                case TransportReturnCodes.FAILURE:
                default:
                {
                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                    channel.releaseBuffer(msgBuf, error);
                    return TransportReturnCodes.FAILURE;
                }
            }
        }

        return retCode;
    }

    /**************************************************************
     * Processes a login request *
     * 
     * @param channel - the Channel of connection *
     * @param msg - the partially decoded message *
     * @param decIter - the decode iterator *
     * @param error - tracks error info *
     * @return status code *
     **************************************************************/
    public static int processLoginRequest(Channel channel, Msg msg, DecodeIterator decIter, Error error) throws UnknownHostException
    {
        MsgKey requestKey = null;

        int retCode;

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();

        /* Switch cases depending on message class */
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
            {
                /* get request message key - retrieve the MsgKey structure from the provided decoded message structure */
                requestKey = msg.msgKey();

                /* check if key has user name */
                /* user name is only login user type accepted by this application (user name is the default type) */
                if (((requestKey.flags() & MsgKeyFlags.HAS_NAME) == 0) || (((requestKey.flags() & MsgKeyFlags.HAS_NAME_TYPE) != 0) && (requestKey.nameType() != Login.UserIdTypes.NAME)))
                {
                    if ((retCode = sendLoginRequestRejectStatusMsg(channel, msg.streamId(), LoginRejectReason.NO_USER_NAME_IN_REQUEST)) != TransportReturnCodes.SUCCESS)
                    {
                        return TransportReturnCodes.FAILURE;
                    }
                    break;
                }

                if (loginRequestInfo_IsInUse && (loginRequestInfo_StreamId != msg.streamId()))
                {
                    if ((retCode = sendLoginRequestRejectStatusMsg(channel, msg.streamId(), LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED)) != TransportReturnCodes.SUCCESS)
                    {
                        return TransportReturnCodes.FAILURE;
                    }
                    break;
                }

                /* decode login request */

                /* get StreamId */
                loginRequestInfo_StreamId = msg.streamId();
                loginRequestInfo_IsInUse = true;

                /* get Username */
                if (requestKey.name().data() == null)
                {
                    loginRequestInfo_Username = null;
                }
                else
                {
                    loginRequestInfo_Username = requestKey.name().toString();
                }

                /* decode key opaque data */

                /**
                 * @brief Allows the user to continue decoding of any message
                 *        key attributes with the same \ref DecodeIterator used
                 *        when calling DecodeMsg
                 *
                 *        Typical use:<BR>
                 *        1. Call DecodeMsg()<BR>
                 *        2. If there are any message key attributes and the
                 *        application wishes to decode them using the same \ref
                 *        DecodeIterator, call decodeKeyAttrib() and continue
                 *        decoding using the appropriate container type decode
                 *        functions, as indicated by
                 *        MsgKey::attribContainerType<BR>
                 *        3. If payload is present and the application wishes to
                 *        decode it, use the appropriate decode functions, as
                 *        specified in \ref MsgBase::containerType<BR>
                 */
                if ((retCode = msg.decodeKeyAttrib(decIter, requestKey)) != CodecReturnCodes.SUCCESS)
                {
                    return TransportReturnCodes.FAILURE;
                }

                /* decode element list */

                /**
                 * @brief Decodes an ElementList container
                 *
                 *        Typical use:<BR>
                 *        1. Call elementList.decode()<BR>
                 *        2. Call ElementEntry.decode until error or
                 *        ::END_OF_CONTAINER is returned.<BR>
                 */
                if ((retCode = elementList.decode(decIter, null)) == CodecReturnCodes.SUCCESS)
                {
                    /* decode each element entry in list */
                    while ((retCode = element.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        /* get login request information */

                        if (retCode == CodecReturnCodes.SUCCESS)
                        {
                            /* ApplicationId */
                            if (element.name().toString().equals(ElementNames.APPID))
                            {
                                loginRequestInfo_ApplicationId = element.encodedData().toString();
                            }
                            /* ApplicationName */
                            else if (element.name().equals(ElementNames.APPNAME))
                            {
                                loginRequestInfo_ApplicationName = element.encodedData().toString();
                            }
                            /* Position */
                            else if (element.name().equals(ElementNames.POSITION))
                            {
                                loginRequestInfo_Position = element.encodedData().toString();
                            }
                            /* Password */
                            else if (element.name().equals(ElementNames.PASSWORD))
                            {
                                loginRequestInfo_Password = element.encodedData().toString();
                            }
                            /* InstanceId */
                            else if (element.name().equals(ElementNames.INST_ID))
                            {
                                loginRequestInfo_InstanceId = element.encodedData().toString();
                            }
                            /* Role */
                            else if (element.name().equals(ElementNames.ROLE))
                            {
                                loginRequestInfo_Role = element.encodedData().toString();
                            }
                        }
                        else
                        {
                            return TransportReturnCodes.FAILURE;
                        }
                    }
                }
                else
                {
                    return TransportReturnCodes.FAILURE;
                }

                System.out.printf("Received Login Request for Username: %s\n", loginRequestInfo_Username);
                /* send login response */
                if ((retCode = sendLoginResponse(channel)) != TransportReturnCodes.SUCCESS)
                {
                    return retCode;
                }
            }
                break;

            case MsgClasses.CLOSE:
            {
                System.out.printf("Received Login Close for StreamId %d\n", msg.streamId());
                /* close login stream */
                closeLoginStream(msg.streamId());
            }
                break;

            default:
            {
                System.out.printf("Received Unhandled Login Msg Class: %d\n", msg.msgClass());
                return TransportReturnCodes.FAILURE;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    /**************************************************************
     * Sends the login refresh response to channel *
     * 
     * @param channel - the Channel of connection *
     **************************************************************/
    public static int sendLoginResponse(Channel channel) throws UnknownHostException
    {
        int retCode;
        TransportBuffer msgBuf = null;
        Error error = TransportFactory.createError();
        /* Populate and encode a refreshMsg */
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();

        Buffer applicationId = CodecFactory.createBuffer();
        Buffer applicationName = CodecFactory.createBuffer();
        Buffer applicationPosition = CodecFactory.createBuffer();

        UInt supportBatchRequests = CodecFactory.createUInt();

        elementList.clear();
        elementEntry.clear();

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        /* Populate information from channel */
        if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* Get a buffer of the channel max fragment size */

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(channel, channelInfo.maxFragmentSize(), error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if ((retCode) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* initialize refresh message */

        refreshMsg.clear();
        /* provide login refresh response information */

        /* set refresh flags */

        /* set-up message */
        refreshMsg.msgClass(MsgClasses.REFRESH);/* (2) Refresh Message */
        refreshMsg.domainType(DomainTypes.LOGIN);/* (1) Login Message */
        refreshMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/
        /* (0x0008) The RefreshMsg has a message key, contained in \ref RefreshMsg::msgBase::msgKey. */
        /* (0x0020) Indicates that this RefreshMsg is a solicited response to a consumer's request. */
        /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
        /* (0x0100) Indicates that any cached header or payload information associated with the RefreshMsg's item stream should be cleared. */
        refreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.CLEAR_CACHE);
        /* (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RefreshMsg or StatusMsg) */
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.state().code(StateCodes.NONE);

        String stateText = "Login accepted by host ";
        String hostName = null;

        if ((hostName = InetAddress.getLocalHost().getHostName()) != null)
        {
            hostName = "localhost";
        }

        stateText = hostName;

        refreshMsg.state().text().data(stateText);
        /* provide login response information */

        /* StreamId - just set the Login response stream id info to be the same as the Login request stream id info */
        refreshMsg.streamId(loginRequestInfo_StreamId);
        /* set msgKey members */
        /* (0x0020) This MsgKey has additional attribute information, contained in \ref MsgKey::encAttrib. The container type of the attribute information is contained in \ref MsgKey::attribContainerType. */
        /* (0x0004) This MsgKey has a nameType enumeration, contained in \ref MsgKey::nameType. */
        /* (0x0002) This MsgKey has a name buffer, contained in \ref MsgKey::name.  */
        refreshMsg.msgKey().flags(MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME);

        /* Username */
        refreshMsg.msgKey().name().data(loginRequestInfo_Username);
        refreshMsg.msgKey().nameType(Login.UserIdTypes.NAME);
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

        /* encode message */

        /* since our msgKey has opaque that we want to encode, we need to use refreshMsg.encode */

        /* refreshMsg.encode should return and inform us to encode our key opaque */

        /**
         * @brief Begin encoding process for an Msg.
         *
         *        Begins encoding of an Msg<BR>
         *        Typical use:<BR>
         *        1. Populate desired members on the Msg<BR>
         *        2. Call refreshMsg.encodeInit() to begin message encoding<BR>
         *        3. If the Msg requires any message key attributes, but they
         *        are not pre-encoded and populated on the MsgKey::encAttrib,
         *        the refreshMsg.encodeInit() function will return
         *        ::ENCODE_MSG_KEY_OPAQUE. Call appropriate encode functions, as
         *        indicated by MsgKey::attribContainerType. When attribute
         *        encoding is completed, followed with
         *        refreshMsg.encodeMsgKeyAttribComplete() to continue with
         *        message encoding<BR>
         *        4. If the Msg requires any extended header information, but it
         *        is not pre-encoded and populated in the extendedHeader \ref
         *        Buffer, the refreshMsg.encodeInit() (or when also encoding
         *        attributes, the EncodeMsgKeyAttribComplete()) function will
         *        return ::ENCODE_EXTENDED_HEADER. Call any necessary extended
         *        header encoding functions; when completed call
         *        EncodeExtendedHeaderComplete() to continue with message
         *        encoding<BR>
         *        5. If the Msg requires any payload, but it is not pre-encoded
         *        and populated in the \ref encDataBody, the
         *        refreshMsg.encodeInit() (or when encoding message key
         *        attributes or extended header,
         *        refreshMsg.encodeKeyAttribComplete() or
         *        refreshMsg.encodeExtendedHeaderComplete() ) function will
         *        return ::ENCODE_CONTAINER. Call appropriate payload encode
         *        functions, as indicated by \ref ::containerType. If no payload
         *        is required or it is provided as pre-encoded, this function
         *        will return ::SUCCESS<BR>
         *        6. Call EncodeMsgComplete() when all content is completed<BR>
         */
        if ((retCode = refreshMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeMsgInit failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* encode our msgKey opaque */

        /* encode the element list */
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);/* (0x08) The ElementList contains standard encoded content (e.g. not set defined). */

        /**
         * @brief Begin encoding process for ElementList container type.
         *
         *        Begins encoding of an ElementList<BR>
         *        Typical use:<BR>
         *        1. Call elementList.encodeInit()<BR>
         *        2. To encode entries, call elementEntry.encode()
         *        orelementEntry.encodeInit()..elementEntry.encodeComplete() for
         *        each elementEntry<BR>
         *        3. Call elementList.encodeComplete() when all entries are
         *        completed<BR>
         */
        if ((retCode = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementListInit failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* ApplicationId */
        applicationId.data("256");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPID);
        if ((retCode = elementEntry.encode(encIter, applicationId)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* ApplicationName */
        applicationName.data("UPA Provider Training");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPNAME);

        if ((retCode = elementEntry.encode(encIter, applicationName)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Position - just set the Login response position info to be the same as the Login request position info */
        applicationPosition.data(loginRequestInfo_Position);
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.POSITION);

        if ((retCode = elementEntry.encode(encIter, applicationPosition)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* SupportBatchRequests */
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.SUPPORT_BATCH);

        if ((retCode = elementEntry.encode(encIter, supportBatchRequests)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementEntry failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode element list - Completes encoding of an ElementList */
        if ((retCode = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeElementListComplete failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* complete encode key */

        /* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
           for us to encode our container/msg payload */
        if ((retCode = refreshMsg.encodeKeyAttribComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nencodeKeyAttribComplete failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* complete encode message */
        if ((retCode = refreshMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nencodeComplete failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* send login response */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet 
             */

            /* set write flags if there's still other data queued */
            /* flush needs to be done by application */
        }

        return retCode;
    }

    /**********************************************************************
     * Sends the login request reject status message for a channel *
     * 
     * @param streamId - the stream id to close the login for *
     **********************************************************************/
    public static void closeLoginStream(int streamId)
    {
        /* find original request information associated with streamId */
        if (loginRequestInfo_StreamId == streamId)
        {
            System.out.printf("Closing login stream id %d with user name: %s\n", loginRequestInfo_StreamId, loginRequestInfo_Username);
            /* Clears the original login request information */
            clearLoginRequestInfo();
        }
    }

    /**************************************************************
     * Performs two time pass to obtain buffer *
     * 
     * @param channel - the Channel of connection *
     * @param size - size of requested buffer *
     * @param error - tracks error info *
     * @return obtained buffer *
     **************************************************************/
    public static TransportBuffer upaGetBuffer(Channel channel, int size, Error error)
    {
        int retCode;
        TransportBuffer msgBuf = null;

        /* First check error */
        if ((msgBuf = channel.getBuffer(size, false, error)) == null)
        {
            /* Check to see if this is just out of buffers or if it's unrecoverable */
            if (error.errorId() != TransportReturnCodes.NO_BUFFERS)
            {
                /* Connection should be closed, return failure */
                /* Closes channel, closes server, cleans up and exits the application. */
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }

            /* (-4) Transport Failure: There are no buffers available from the buffer pool, returned from GetBuffer.
             * This can happen if the reader isn't keeping up and/or we have a lot of write threads in multithreaded apps.
             * Use Ioctl to increase pool size or use Flush to flush data and return buffers to pool.
             */

            /* Flush and obtain buffer again */
            retCode = channel.flush(error);
            if (retCode < TransportReturnCodes.SUCCESS)
            {
                System.out.printf("Channel.flush() failed with return code %d - <%s>\n", retCode, error.text());
                return null;
            }
            /* call GetBuffer again to see if it works now after Flush */
            if ((msgBuf = channel.getBuffer(size, false, error)) == null)
            {
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }

        }
        /* return  buffer to be filled in with valid memory */
        return msgBuf;
    }

    /**********************************************************************
     * Sends the login request reject status message for a channel *
     * 
     * @param channel - the Channel of connection *
     * @param streamId - the stream id of the login request reject status *
     * @param reason - the reason for the reject *
     * @return status code *
     **********************************************************************/
    public static int sendLoginRequestRejectStatusMsg(Channel channel, int streamId, LoginRejectReason reason) throws UnknownHostException
    {
        int retCode;
        TransportBuffer msgBuf = null;
        Error error = TransportFactory.createError();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        /* Populate information from channel */
        if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(channel, channelInfo.maxFragmentSize(), error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        encIter.clear();
        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if ((retCode) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* Create and initialize status message */
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the login request reject status. */

        statusMsg.clear();
        /* set-up message */
        statusMsg.msgClass(MsgClasses.STATUS);/* (3) Status Message */
        statusMsg.streamId(streamId);
        statusMsg.domainType(DomainTypes.LOGIN);/* (1) Login Message */
        /* No payload associated with this close status message */
        statusMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/
        /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state.  */
        statusMsg.flags(StatusMsgFlags.HAS_STATE);
        /* (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */
        statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().code(StateCodes.NONE);

        /* Switch cases depending on login reject reason */
        switch (reason)
        {
            case MAX_LOGIN_REQUESTS_REACHED:
            {
                /* (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
                statusMsg.state().code(StateCodes.TOO_MANY_ITEMS);

                String stateText = "Login request rejected for stream id " + streamId + " - max request count reached";
                String hostName = null;

                if ((hostName = InetAddress.getLocalHost().getHostName()) != null)
                {
                    hostName = "localhost";
                }

                stateText = hostName;

                statusMsg.state().text().data(stateText);
            }
                break;
            case NO_USER_NAME_IN_REQUEST:
            {
                /* (5) Usage Error (indicates an invalid usage within the system) */
                statusMsg.state().code(StateCodes.USAGE_ERROR);

                String stateText = "Login request rejected for stream id " + streamId + " - request does not contain user name";
                String hostName = null;

                if ((hostName = InetAddress.getLocalHost().getHostName()) != null)
                {
                    hostName = "localhost";
                }

                stateText = hostName;

                statusMsg.state().text().data(stateText);
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
        if ((retCode = statusMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeMsg() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* send login request reject status */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* send login request reject status fails */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush */

            /* If the login request reject status doesn't flush, just close channel and exit the app. When you send login request reject status msg, 
             * we want to make a best effort to get this across the network as it will gracefully close the open login 
             * stream. If this cannot be flushed, this application will just close the connection for simplicity.
             */

            /* Closes channel, closes server, cleans up and exits the application. */
        }

        return retCode;
    }

    /**************************************************************
     * Sends the login close status message for a channel *
     * 
     * @param channel - the Channel of connection *
     * @param error - tracks error info *
     * @return status code *
     **************************************************************/
    public static int sendLoginCloseStatusMsg(Channel channel, Error error) throws UnknownHostException
    {
        int retCode;
        TransportBuffer msgBuf = null;

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */
        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();

        /* Provider uses StatusMsg to send the login close status and to close the stream. */
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        /* Populate information from channel */
        if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* Get a buffer of the channel max fragment size */

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(channel, channelInfo.maxFragmentSize(), error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the login close status. */

        statusMsg.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        retCode = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if ((retCode) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* initialize status message */
        /* set-up message */
        statusMsg.msgClass(MsgClasses.STATUS);/* (3) Status Message */
        statusMsg.streamId(loginRequestInfo_StreamId);
        statusMsg.domainType(DomainTypes.LOGIN);/* (1) Login Message */
        /* No payload associated with this close status message */
        statusMsg.containerType(DataTypes.NO_DATA); /* (128) No Data <BR>*/

        /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state.  */
        statusMsg.flags(StatusMsgFlags.HAS_STATE);
        /* (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
        statusMsg.state().streamState(StreamStates.CLOSED);
        /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().code(StateCodes.NONE);

        String stateText = "Login stream closed";
        String hostName = null;

        if ((hostName = InetAddress.getLocalHost().getHostName()) != null)
        {
            hostName = "localhost";
        }

        stateText = hostName;

        statusMsg.state().text().data(stateText);
        /* encode message */

        /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
        /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
         * typically used for encoding simple types like Integer or incorporating previously encoded data
         * (referred to as pre-encoded data).
         */
        if ((retCode = statusMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nencode() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        /* send login close status */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* login close status fails */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush */

            /* If the login close status doesn't flush, just close channel and exit the app. When you send login close status msg, 
             * we want to make a best effort to get this across the network as it will gracefully close the open login 
             * stream. If this cannot be flushed, this application will just close the connection for simplicity.
             */

            /* Closes channel, closes server, cleans up and exits the application. */
        }

        return retCode;
    }

    /****************************************
     * Clears the login request info values *
     ****************************************/
    public static void clearLoginRequestInfo()
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

    /************************************************************************************************************
     * Processes a source directory request. This consists of decoding the
     * source directory request and calling * sendSourceDirectoryResponse() to
     * send the source directory response. *
     * 
     * @param chnl - the Channel of connection *
     * @param dIter - The decode iterator *
     * @param msg - The partially decoded message *
     * @param maxMsgSize - The channel max message size *
     ************************************************************************************************************/
    public static int processSourceDirectoryRequest(Channel chnl, DecodeIterator dIter, Msg msg, int maxMsgSize)
    {

        int retCode = 0;

        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
            {
                /* get request message key - retrieve the MsgKey structure from the provided decoded message structure */
                MsgKey msgKey = msg.msgKey();
                /* check if key has minimal filter flags -
                 * Does key have minimal filter flags.  Request key must minimally have
                 * RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER,
                 * and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags.
                 */
                if (!(msgKey.checkHasFilter() && (msgKey.filter() & Directory.ServiceFilterFlags.INFO) != 0 && (msgKey.filter() & Directory.ServiceFilterFlags.STATE) != 0 && (msgKey.filter() & Directory.ServiceFilterFlags.GROUP) != 0))
                {

                    /* if stream id is different from last request, this is an invalid request */
                    if (sendSrcDirectoryRequestRejectStatusMsg(chnl, msg.streamId(), DirectoryRejectReason.INCORRECT_FILTER_FLAGS, maxMsgSize) != TransportReturnCodes.SUCCESS)
                        return TransportReturnCodes.FAILURE;
                    break;
                }

                /* decode source directory request */

                /* get StreamId */
                sourceDirectoryRequestInfo_StreamId = msg.streamId();
                sourceDirectoryRequestInfo_IsInUse = true;

                System.out.printf("\nReceived Source Directory Request\n");

                /* send source directory response */

                retCode = sendSourceDirectoryResponse(chnl, maxMsgSize, sourceDirectoryRequestInfo_ServiceName, sourceDirectoryRequestInfo_ServiceId);
                if (retCode != TransportReturnCodes.SUCCESS)
                    return retCode;

            }
                break;
            case MsgClasses.CLOSE:
            {
                System.out.printf("\nReceived Source Directory Close for StreamId %d\n", msg.streamId());

                /* close source directory stream */
                closeSourceDirectoryStream(msg.streamId(), chnl);
            }
                break;
            default:
            {
                System.out.printf("\nReceived Unhandled Source Directory Msg Class: %d\n", msg.msgClass());
                return TransportReturnCodes.FAILURE;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    /************************************************************************************************************************************
     * Send Source Directory response to a channel. This consists of getting a
     * message buffer, setting the source directory * response information,
     * encoding the source directory response, and sending the source directory
     * response to * the consumer. The Source Directory domain model conveys
     * information about all available services in the system. * An OMM consumer
     * typically requests a Source Directory to retrieve information about
     * available services and their capabilities. *
     * 
     * @param channel - the Channel of connection *
     * @param maxMsgSize - The channel max message size *
     * @param serviceName - The service name specified by the OMM interactive
     *            provider application (Optional to set) *
     * @param serviceId - the serviceId specified by the OMM interactive
     *            provider application (Optional to set) *
     ************************************************************************************************************************************/
    public static int sendSourceDirectoryResponse(Channel channel, int maxMsgSize, String serviceName, int serviceId)
    {
        int retval;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;
        UInt tmpUInt = CodecFactory.createUInt();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        /* Populate and encode a refreshMsg */
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

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

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encodeIter.clear();

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(channel, maxMsgSize, error)) == null)/* first check Error */
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Source Directory refresh response. */

        refreshMsg.clear();

        /* provide source directory response information */

        /* set refresh flags */
        /* The content of a Source Directory Refresh message is expected to be atomic and contained in a single part,
         * therefore _RFMF_REFRESH_COMPLETE should be set.
         */

        /* (0x0008) The RefreshMsg has a message key, contained in \ref RefreshMsg::msgBase::msgKey. */
        /* (0x0020) Indicates that this RefreshMsg is a solicited response to a consumer's request. */
        /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
        /* (0x0100) Indicates that any cached header or payload information associated with the RefreshMsg's item stream should be cleared. */
        refreshFlags |= RefreshMsgFlags.HAS_MSG_KEY;
        refreshFlags |= RefreshMsgFlags.REFRESH_COMPLETE;
        refreshFlags |= RefreshMsgFlags.CLEAR_CACHE;
        /* set filter flags */
        /* At a minimum, Thomson Reuters recommends that the NIP send the Info, State, and Group filters for the Source Directory. */
        refreshKey.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.LINK);

        /* StreamId */
        streamId = sourceDirectoryRequestInfo_StreamId;

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
            for (Long capability : capabilitiesList)
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
            serviceState = Directory.ServiceStates.UP; /* Service is Up */
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
             * Prior to changing a service status, Thomson Reuters recommends that you issue item or group
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

    /***************************************************************************************************************
     * Sends the source directory request reject status message for a channel.
     * 
     * @param channel - the Channel of connection
     * @param streamId - The stream id of the source directory request reject
     *            status
     * @param reason - The reason for the reject
     * @param maxFragmentSize - max fragment size before fragmentation
     ***************************************************************************************************************/
    public static int sendSrcDirectoryRequestRejectStatusMsg(Channel channel, int streamId, DirectoryRejectReason reason, int maxFragmentSize)
    {

        int retCode = 0;
        Error error = TransportFactory.createError();

        TransportBuffer msgBuf = null;

        /* Provider uses StatusMsg to send the source directory request reject status message. */
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encodeIter.clear();

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        /* Create channel info as a holder */

        if ((msgBuf = upaGetBuffer(channel, maxFragmentSize, error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the source directory request reject status. */

        statusMsg.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        /* set the buffer on an EncodeIterator */
        if ((retCode = encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retCode);
            /* Closes channel, closes server, cleans up and exits the application. */
            return retCode;
        }
        /* set-up message */
        statusMsg.streamId(streamId);
        statusMsg.msgClass(MsgClasses.STATUS);/* (3) Status Message */
        statusMsg.domainType(DomainTypes.SOURCE); /* (4) Source Message */
        /* No payload associated with this close status message */
        statusMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/
        /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state.  */
        statusMsg.flags(StatusMsgFlags.HAS_STATE);
        /* (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RefreshMsg or an StatusMsg) */
        statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().code(StateCodes.NONE);

        switch (reason)
        {
            case MAX_SRCDIR_REQUESTS_REACHED:
            {
                /* (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
                statusMsg.state().code(StateCodes.TOO_MANY_ITEMS);

                String stateText = "Source directory request rejected for stream id " + streamId + " - max request count reached";

                statusMsg.state().text().data(stateText);
            }
                break;
            case INCORRECT_FILTER_FLAGS:
            {
                /* (5) Usage Error (indicates an invalid usage within the system) */
                statusMsg.state().code(StateCodes.USAGE_ERROR);

                String stateText = "Source directory request rejected for stream id " + streamId + " - request must minimally have RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER, and RDM_DIRECTORY_SERVICE_GROUP_FILTER filter flags";

                statusMsg.state().text().data(stateText);
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
        if ((retCode = statusMsg.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* send source directory request reject status */
        if ((retCode = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* send source directory request reject status fails */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retCode > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush */

            /* If the source directory request reject status doesn't flush, just close channel and exit the app. When you send source directory request reject status msg,
             * we want to make a best effort to get this across the network as it will gracefully close the open source directory
             * stream. If this cannot be flushed, this application will just close the connection for simplicity.
             */

            /* Closes channel, closes server, cleans up and exits the application. */
        }
        return retCode;
    }

    /************************************************************************************************************
     * Closes a source directory stream. *
     * 
     * @param streamId - The stream id to close the source directory for *
     * @param channel - Channel of connection *
     ************************************************************************************************************/
    public static void closeSourceDirectoryStream(int streamId, Channel channel)
    {
        /* find original request information associated with streamId */
        if (sourceDirectoryRequestInfo_StreamId == streamId)
        {
            System.out.printf("Closing source directory stream id %d with service name: %s \n", loginRequestInfo_Username);

            /* Clears the original source directory request information */
            clearSourceDirectoryReqInfo();
        }
    }

    public static void clearSourceDirectoryReqInfo()
    {
        sourceDirectoryRequestInfo_StreamId = 0;
        sourceDirectoryRequestInfo_ServiceName = null;
        sourceDirectoryRequestInfo_ServiceId = 0;
        sourceDirectoryRequestInfo_IsInUse = false;
    }

    /****************************************************************************************************************************
     * Processes a dictionary request. This consists of decoding the dictionary
     * request and calling the corresponding flavors * of the
     * sendDictionaryResponse() functions to send the dictionary response. *
     * 
     * @param chnl - Channel of connection *
     * @param msg - The partially decoded message *
     * @param dIter - The decode iterator *
     * @param dictionary - The dictionary to encode field information or
     *            enumerated type information from *
     * @param maxSize - The channel max message size *
     ****************************************************************************************************************************/
    public static int processDictionaryRequest(Channel chnl, Msg msg, DecodeIterator dIter, DataDictionary dictionary, int maxSize)
    {
        MsgKey requestKey = CodecFactory.createMsgKey();

        int retval = 0;

        Error error = TransportFactory.createError();

        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
            {
                /* get request message key - retrieve the MsgKey structure from the provided decoded message structure */
                requestKey = (MsgKey)(msg.msgKey());

                /* decode dictionary request */

                /* first check if this is fieldDictionary or enumTypeDictionary request */
                if (fieldDictionaryDownloadName.equals(requestKey.name().toString()))
                {
                    fieldDictionaryRequestInfo_IsInUse = true;

                    /* get StreamId */
                    fieldDictionaryRequestInfo_StreamId = msg.streamId();

                    if (requestKey.copy(fieldDictionaryRequestInfo_MsgKey) == CodecReturnCodes.FAILURE)
                    {
                        if (sendDictionaryRequestRejectStatusMsg(chnl, msg.streamId(), DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED, maxSize) != CodecReturnCodes.SUCCESS)
                            return CodecReturnCodes.FAILURE;
                        break;
                    }
                    fieldDictionaryRequestInfo_DictionaryName = fieldDictionaryRequestInfo_MsgKey.name().toString();

                    System.out.printf("\nReceived Dictionary Request for DictionaryName: %s\n", fieldDictionaryRequestInfo_DictionaryName);

                    /* send field dictionary response */
                    if ((retval = sendDictionaryResponse(chnl, dictionary, Dictionary.Types.FIELD_DEFINITIONS, maxSize)) != CodecReturnCodes.SUCCESS)
                    {
                        return retval;
                    }
                }
                else if (enumTypeDictionaryDownloadName.equals(requestKey.name().toString()))
                {
                    enumTypeDictionaryRequestInfo_IsInUse = true;

                    /* get StreamId */
                    enumTypeDictionaryRequestInfo_StreamId = msg.streamId();

                    if (requestKey.copy(enumTypeDictionaryRequestInfo_MsgKey) == CodecReturnCodes.FAILURE)
                    {
                        if (sendDictionaryRequestRejectStatusMsg(chnl, msg.streamId(), DictionaryRejectReason.MAX_DICTIONARY_REQUESTS_REACHED, maxSize) != CodecReturnCodes.SUCCESS)
                            return CodecReturnCodes.FAILURE;
                        break;
                    }
                    enumTypeDictionaryRequestInfo_DictionaryName = enumTypeDictionaryRequestInfo_MsgKey.name().toString();

                    System.out.printf("\nReceived Dictionary Request for DictionaryName: %s\n", enumTypeDictionaryRequestInfo_DictionaryName);

                    /* send enum type dictionary response */
                    if ((retval = sendDictionaryResponse(chnl, dictionary, Dictionary.Types.ENUM_TABLES, maxSize)) != CodecReturnCodes.SUCCESS)
                    {
                        return retval;
                    }
                }
                else
                {
                    if (sendDictionaryRequestRejectStatusMsg(chnl, msg.streamId(), DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, maxSize) != CodecReturnCodes.SUCCESS)
                        return CodecReturnCodes.FAILURE;
                    break;
                }
            }
                break;
            case MsgClasses.CLOSE:
                System.out.println("Received Dictionary Close for StreamId " + msg.streamId());

                /*close dictionary stream*/
                closeDictionary(chnl, msg.streamId(), error, dictionary);

                break;
            default:
                System.out.println("Received Unhandled Dictionary MsgClass: " + msg.msgClass());
                return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    /*****************************************************************************************************************************************************
     * Sends the field dictionary or enumType dictionary response to a channel.
     * This consists of getting a message buffer, encoding the field dictionary
     * or enumType dictionary response, and sending the field dictionary or
     * enumType dictionary response to the server.
     * 
     * @param chnl - Channel of connection
     * @param dictionary - The dictionary to encode field information or
     *            enumerated type information from
     * @param dictionaryType - the type of the dictionary
     * @param maxSize - The channel max message size
     *****************************************************************************************************************************************************/

    public static int sendDictionaryResponse(Channel chnl, DataDictionary dictionary, int dictionaryType, int maxSize)
    {
        int retval;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;
        int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;
        int dictionaryFid = -32768; // MIN_FID
        Int tmpInt = CodecFactory.createInt();
        String stateText = null;

        /* flag indicating first part of potential multi part refresh message */
        boolean firstPartMultiPartRefresh = true;
        boolean dictionaryComplete = false;

        /* Provider uses RefreshMsg to send the field dictionary or enum type dictionary refresh response to a channel. */
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator(); /* the encode iterator is created (typically stack allocated)  */

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encodeIter.clear();

        switch (dictionaryType)
        {
            case Dictionary.Types.FIELD_DEFINITIONS:
                /* set starting fid to loaded field dictionary's minimum fid */
                dictionaryFid = dictionary.minFid();
                break;
            case Dictionary.Types.ENUM_TABLES:
                /* set starting fid to loaded enumType dictionary's minimum fid, which is 0 in this case */
                /* for EncodeEnumTypeDictionaryAsMultiPart() API all, must be initialized to 0 on the first call and is updated with each successfully encoded part. */
                dictionaryFid = 0;
                break;
            default:
                break;
        }

        /* field Dictionary takes multiple parts to respond */
        while (true)
        {

            /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            switch (dictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:

                    if ((msgBuf = upaGetBuffer(chnl, maxSize, error)) == null) /* first check Error */
                    {
                        /* Connection should be closed, return failure */
                        /* Closes channel, closes server, cleans up and exits the application. */
                        return CodecReturnCodes.FAILURE;
                    }
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    /* EnumType Dictionary now supports fragmenting at a message level - However, some EnumType Dictionary message can be still very large, up to 10K */
                    if ((msgBuf = upaGetBuffer(chnl, MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE, error)) == null) /* first check Error */
                    {
                        /* Connection should be closed, return failure */
                        /* Closes channel, closes server, cleans up and exits the application. */
                        return CodecReturnCodes.FAILURE;
                    }
                    break;
                default:
                    break;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the field dictionary or enumType dictionary refresh response. */

            refreshMsg.clear();

            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */

            if ((retval = encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion())) < CodecReturnCodes.SUCCESS)
            {
                chnl.releaseBuffer(msgBuf, error);
                System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
                /* Closes channel, closes server, cleans up and exits the application. */
                return CodecReturnCodes.FAILURE;
            }

            /* set-up message */
            refreshMsg.msgClass(MsgClasses.REFRESH);/* (2) Refresh Message */
            refreshMsg.domainType(DomainTypes.DICTIONARY); /* (5) Dictionary Message */

            /* StreamId */
            refreshMsg.streamId(fieldDictionaryRequestInfo_StreamId);

            switch (dictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    refreshMsg.streamId(fieldDictionaryRequestInfo_StreamId);
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    refreshMsg.streamId(enumTypeDictionaryRequestInfo_StreamId);
                    break;
                default:
                    break;
            }

            /* (138) Series container type, represents row based tabular information where no specific indexing is required.   <BR>*/
            refreshMsg.containerType(DataTypes.SERIES);
            /* (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RefreshMsg or StatusMsg) */
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().code(StateCodes.NONE);
            refreshMsg.state().dataState(DataStates.OK);

            /* (0x0008) The RefreshMsg has a message key, contained in \ref RefreshMsg::msgBase::msgKey. */
            /* (0x0020) Indicates that this RefreshMsg is a solicited response to a consumer's request. */
            refreshMsg.applySolicited();
            refreshMsg.applyHasMsgKey();

            /* (0x0002) This MsgKey has a name buffer, contained in \ref MsgKey::name.  */
            /* (0x0008) This MsgKey has a filter, contained in \ref MsgKey::filter.  */
            /* (0x0001) This MsgKey has a service id, contained in \ref MsgKey::serviceId.  */

            refreshMsg.msgKey().applyHasFilter();
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().applyHasServiceId();

            switch (dictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    refreshMsg.msgKey().filter(fieldDictionaryRequestInfo_MsgKey.filter());
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    refreshMsg.msgKey().filter(enumTypeDictionaryRequestInfo_MsgKey.filter());
                    break;
                default:
                    break;
            }

            refreshMsg.msgKey().serviceId(sourceDirectoryRequestInfo_ServiceId);

            /* when doing a multi part refresh only the first part has the RFMF_CLEAR_CACHE flag set */
            if (firstPartMultiPartRefresh)
            {
                /* (0x0100) Indicates that any cached header or payload information associated with the RefreshMsg's item stream should be cleared. */
                refreshMsg.applyClearCache();
            }

            switch (dictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    stateText = "Field Dictionary Refresh (starting fid " + dictionaryFid + ")";
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    stateText = "Enum Type Dictionary Refresh (starting fid " + dictionaryFid + ")";
                    break;
                default:
                    break;
            }
            refreshMsg.state().text().data(stateText);

            /* DictionaryName */
            switch (dictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    refreshMsg.msgKey().name().data(fieldDictionaryRequestInfo_DictionaryName);
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    refreshMsg.msgKey().name().data(enumTypeDictionaryRequestInfo_DictionaryName);
                    break;
                default:
                    break;
            }

            /* encode message */

            /**
             * @brief Begin encoding process for an Msg.
             *
             *        Begins encoding of an Msg<BR>
             *        Typical use:<BR>
             *        1. Populate desired members on the Msg<BR>
             *        2. Call EncodeMsgInit() to begin message encoding<BR>
             *        3. If the Msg requires any message key attributes, but
             *        they are not pre-encoded and populated on the
             *        MsgKey::encAttrib, the EncodeMsgInit() function will
             *        return ::CodecReturnCodes.ENCODE_MSG_KEY_OPAQUE. Call
             *        appropriate encode functions, as indicated by
             *        MsgKey::attribContainerType. When attribute encoding is
             *        completed, followed with EncodeMsgKeyAttribComplete() to
             *        continue with message encoding<BR>
             *        4. If the Msg requires any extended header information,
             *        but it is not pre-encoded and populated in the
             *        extendedHeader \ref Buffer, the EncodeMsgInit() (or when
             *        also encoding attributes, the
             *        EncodeMsgKeyAttribComplete()) function will return
             *        ::CodecReturnCodes.ENCODE_EXTENDED_HEADER. Call any
             *        necessary extended header encoding functions; when
             *        completed call EncodeExtendedHeaderComplete() to continue
             *        with message encoding<BR>
             *        5. If the Msg requires any payload, but it is not
             *        pre-encoded and populated in the \ref
             *        MsgBase::encDataBody, the EncodeMsgInit() (or when
             *        encoding message key attributes or extended header,
             *        EncodeMsgKeyAttribComplete() or
             *        EncodeExtendedHeaderComplete() ) function will return
             *        ::CodecReturnCodes.ENCODE_CONTAINER. Call appropriate
             *        payload encode functions, as indicated by \ref
             *        MsgBase::containerType. If no payload is required or it is
             *        provided as pre-encoded, this function will return
             *        ::CodecReturnCodes.SUCCESS<BR>
             *        6. Call EncodeMsgComplete() when all content is
             *        completed<BR>
             */
            if ((retval = refreshMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
            {
                chnl.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeMsgInit() failed with return code: %d\n", retval);
                /* Closes channel, closes server, cleans up and exits the application. */
                return CodecReturnCodes.FAILURE;
            }

            /* encode dictionary into message */

            switch (dictionaryType)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    /**
                     * @brief Encode the field definitions dictionary
                     *        information into a data payload according the
                     *        domain model, using the field information from the
                     *        entries present in this dictionary. This function
                     *        supports building the encoded data in multiple
                     *        parts -- if there is not enough available buffer
                     *        space to encode the entire dictionary, subsequent
                     *        calls can be made to this function, each producing
                     *        the next segment of fields.
                     * @param eIter Iterator to be used for encoding. Prior to
                     *            each call, the iterator must be cleared and
                     *            initialized to the buffer to be used for
                     *            encoding.
                     * @param dictionary The dictionary to encode field
                     *            information from.
                     * @param currentFid Tracks which fields have been encoded
                     *            in case of multi-part encoding. Must be
                     *            initialized to dictionary->minFid on the first
                     *            call and is updated with each successfully
                     *            encoded part.
                     * @param verbosity The desired verbosity to encode. See
                     *            DictionaryVerbosity.
                     * @param errorText Buffer to hold error text if the
                     *            encoding fails.
                     * @return CodecReturnCodes.DICT_PART_ENCODED when encoding
                     *         parts, CodecReturnCodes.SUCCESS for final part or
                     *         single complete payload.
                     * @see DataDictionary, DictionaryVerbosity,
                     *      DecodeFieldDictionary
                     */

                    tmpInt.value(dictionaryFid);
                    retval = dictionary.encodeFieldDictionary(encodeIter, tmpInt, (int)fieldDictionaryRequestInfo_MsgKey.filter(), error);
                    dictionaryFid = (int)tmpInt.toLong();
                    if ((retval) != CodecReturnCodes.SUCCESS)
                    {
                        /* dictionary encode failed */
                        if (retval != CodecReturnCodes.DICT_PART_ENCODED) /* (10) Dictionary Success: Successfully encoded part of a dictionary message, returned from the  dictionary processing functions. */
                        {
                            chnl.releaseBuffer(msgBuf, error);
                            System.out.printf("EncodeFieldDictionary() failed '%s'\n", error.text());
                            return retval;
                        }
                    }
                    else /* dictionary encode complete */
                    {
                        dictionaryComplete = true;

                        /* set refresh complete flag */
                        /* Set the RFMF_REFRESH_COMPLETE flag on an encoded RefreshMsg buffer */
                        encodeIter.setRefreshCompleteFlag();
                    }
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    /**
                     * @brief Encode the enumerated types dictionary according
                     *        the domain model, using the information from the
                     *        tables and referencing fields present in this
                     *        dictionary. This function supports building the
                     *        encoded data in multiple parts -- if there is not
                     *        enough available buffer space to encode the entire
                     *        dictionary, subsequent calls can be made to this
                     *        function, each producing the next segment of
                     *        fields. Note: This function will use the type
                     *        DT_ASCII_STRING for the DISPLAY array.
                     * @param eIter Iterator to be used for encoding.
                     * @param dictionary The dictionary to encode enumerated
                     *            type information from.
                     * @param currentFid Tracks which fields have been encoded
                     *            in case of multi-part encoding. Must be
                     *            initialized to 0 on the first call and is
                     *            updated with each successfully encoded part.
                     * @param verbosity The desired verbosity to encode. See
                     *            RDMDictionaryVerbosityValues.
                     * @param errorText Buffer to hold error text if the
                     *            encoding fails.
                     * @see DataDictionary, RDMDictionaryVerbosityValues,
                     *      EncodeEnumTypeDictionary, DecodeEnumTypeDictionary
                     */
                    tmpInt.value(dictionaryFid);                	
                    retval = dictionary.encodeEnumTypeDictionaryAsMultiPart(encodeIter, tmpInt, (int)enumTypeDictionaryRequestInfo_MsgKey.filter(), error);
                    dictionaryFid = (int)tmpInt.toLong();

                    if ((retval) != CodecReturnCodes.SUCCESS)
                    {
                        /* dictionary encode failed */
                        if (retval != CodecReturnCodes.DICT_PART_ENCODED) /* (10) Dictionary Success: Successfully encoded part of a dictionary message, returned from the  dictionary processing functions. */
                        {
                            chnl.releaseBuffer(msgBuf, error);
                            System.out.printf("EncodeEnumTypeDictionaryAsMultiPart() failed '%s'\n", error.text());
                            return retval;
                        }
                    }
                    else /* dictionary encode complete */
                    {
                        dictionaryComplete = true;

                        /* set refresh complete flag */
                        /* Set the RFMF_REFRESH_COMPLETE flag on an encoded RefreshMsg buffer */
                        encodeIter.setRefreshCompleteFlag();
                    }
                    break;
                default:
                    break;
            }

            /* complete encode message */
            retval = refreshMsg.encodeComplete(encodeIter, true);
            if ((retval) < CodecReturnCodes.SUCCESS)
            {
                chnl.releaseBuffer(msgBuf, error);
                System.out.printf("EncodeMsgComplete() failed with return code: %d\n", retval);
                return retval;
            }

            firstPartMultiPartRefresh = false;

            /* send dictionary refresh response */
            if ((retval = sendMessage(chnl, msgBuf)) < CodecReturnCodes.SUCCESS)
            {
                /* dictionary refresh response fails */
                /* Closes channel, closes server, cleans up and exits the application. */
                return CodecReturnCodes.FAILURE;
            }
            else if (retval > CodecReturnCodes.SUCCESS)
            {
                /* There is still data left to flush */

                /* If the dictionary close status doesn't flush, just close channel and exit the app. When you send dictionary close status msg,
                 * we want to make a best effort to get this across the network as it will gracefully close the open dictionary
                 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
                 */

                /* Closes channel, closes server, cleans up and exits the application. */
            }

            /* break out of loop when all dictionary responses sent */
            if (dictionaryComplete)
            {
                break;
            }

            /* sleep between dataDictionary responses */
        }

        return retval;
    }

    /***********************************************************************************************************
     * Sends the dictionary request reject status message for a channel.
     * 
     * @param chnl - Channel of connection
     * @param streamId - The stream id of the dictionary request reject status
     * @param reason - The reason for the reject
     * @param maxSize - The channel max message size
     ************************************************************************************************************/
    public static int sendDictionaryRequestRejectStatusMsg(Channel chnl, int streamId, DictionaryRejectReason reason, int maxSize)
    {
        int retval;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;
        /* Provider uses StatusMsg to send the dictionary close status and to close the stream. */
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();/* the encode iterator is created (typically stack allocated)  */

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        _encodeIter.clear();

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(chnl, maxSize, error)) == null)/* first check Error */
        {
            /* Connection should be closed, return failure */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the dictionary close status. */

        statusMsg.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */
        /* set the buffer on an EncodeIterator */
        if ((retval = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion())) < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        /* set-up message */
        statusMsg.msgClass(MsgClasses.STATUS); /* (3) Status Message */

        statusMsg.streamId(streamId);

        statusMsg.domainType(DomainTypes.DICTIONARY); /* (5) Dictionary Message */
        /* No payload associated with this close status message */
        statusMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/

        /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state.  */
        statusMsg.applyHasState();
        /* (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
        statusMsg.state().streamState(StreamStates.CLOSED);
        /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().code(StateCodes.NONE);

        switch (reason)
        {
            case UNKNOWN_DICTIONARY_NAME:
                statusMsg.state().code(StateCodes.NOT_FOUND);
                statusMsg.state().streamState(StreamStates.CLOSED);
                statusMsg.state().text().data("Dictionary request rejected for stream id " + streamId + " - dictionary name unknown");
                break;
            case MAX_DICTIONARY_REQUESTS_REACHED:
                statusMsg.state().code(StateCodes.TOO_MANY_ITEMS);
                statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
                statusMsg.state().text().data("Dictionary request rejected for stream id " + streamId + " -  max request count reached");
                break;
            default:
                break;
        }

        /* encode message*/
        retval = statusMsg.encode(_encodeIter);
        if ((retval) < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("\nEncodeMsg() failed with return code: %d\n", retval);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }
        if ((retval = sendMessage(chnl, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* dictionary close status fails */
            /* Closes channel, closes server, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (retval > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush */

            /* If the dictionary close status doesn't flush, just close channel and exit the app. When you send dictionary close status msg,
             * we want to make a best effort to get this across the network as it will gracefully close the open dictionary
             * stream. If this cannot be flushed, this application will just close the connection for simplicity.
             */

            /* Closes channel, closes server, cleans up and exits the application. */
        }

        return retval;

    }

    public static int sendDictionaryCloseStatusMessage(Channel chnl, Error error, int maxSize)
    {

        int retval = 0;
        TransportBuffer msgBuf = chnl.getBuffer(MAX_DICTIONARY_STATUS_MSG_SIZE, false, error);

        /* Provider uses StatusMsg to send the dictionary close status and to close the stream. */
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
        int i;

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();/* the encode iterator is created (typically stack allocated)  */

        String stateText = null;

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        _encodeIter.clear();

        for (i = 0; i < 2; i++)
        {

            /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
            if ((msgBuf = upaGetBuffer(chnl, maxSize, error)) == null)/* first check Error */
            {
                /* Connection should be closed, return failure */
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCodes.FAILURE;
            }

            /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

            /* Encodes the dictionary close status. */

            statusMsg.clear();

            /* set version information of the connection on the encode iterator so proper versioning can be performed */
            /* set the buffer on an EncodeIterator */
            if ((retval = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion())) < CodecReturnCodes.SUCCESS)
            {
                chnl.releaseBuffer(msgBuf, error);
                System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return CodecReturnCodes.FAILURE;
            }

            /* set-up message */
            statusMsg.msgClass(MsgClasses.STATUS); /* (3) Status Message */

            if (i == 0)
                statusMsg.streamId(fieldDictionaryRequestInfo_StreamId);
            else
                statusMsg.streamId(enumTypeDictionaryRequestInfo_StreamId);

            statusMsg.domainType(DomainTypes.DICTIONARY); /* (5) Dictionary Message */
            /* No payload associated with this close status message */
            statusMsg.containerType(DataTypes.NO_DATA);/* (128) No Data <BR>*/

            /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in StatusMsg::state.  */
            statusMsg.applyHasState();
            /* (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
            statusMsg.state().streamState(StreamStates.CLOSED);
            /* (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
            statusMsg.state().dataState(DataStates.SUSPECT);
            statusMsg.state().code(StateCodes.NONE);
            stateText = "Dictionary stream closed";
            statusMsg.state().text().data(stateText);

            /* encode message */

            /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
            /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
             * typically used for encoding simple types like Integer or incorporating previously encoded data
             * (referred to as pre-encoded data).
             */
            if ((retval = statusMsg.encode(_encodeIter)) < CodecReturnCodes.SUCCESS)
            {
                chnl.releaseBuffer(msgBuf, error);
                System.out.printf("\nEncodeMsg() failed with return code: %d\n", retval);
                /* Closes channel, cleans up and exits the application. */
                return CodecReturnCodes.FAILURE;

            }

            /* send dictionary close status */

            if ((retval = sendMessage(chnl, msgBuf)) < TransportReturnCodes.SUCCESS)
            {
                /* dictionary close status fails */
                /* Closes channel, closes server, cleans up and exits the application. */
                return TransportReturnCodes.FAILURE;
            }
            else if (retval > TransportReturnCodes.SUCCESS)
            {
                /* There is still data left to flush */

                /* If the dictionary close status doesn't flush, just close channel and exit the app. When you send dictionary close status msg,
                 * we want to make a best effort to get this across the network as it will gracefully close the open dictionary
                 * stream. If this cannot be flushed, this application will just close the connection for simplicity.
                 */

                /* Closes channel, closes server, cleans up and exits the application. */
            }
        }

        return retval;
    }

}