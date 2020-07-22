/*********************************************************************************
 * This is the UPA Consumer Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Consumer using the UPA Transport layer.
 *
 * Main Java source file for the UPA Consumer Training application. It is a 
 * single-threaded client application.
 *
 *********************************************************************************
 * UPA Consumer Training Module 1a: Establish network communication
 *********************************************************************************
 * Summary:
 * In this module, the application initializes the UPA Transport and 
 * connects the client. An OMM consumer application can establish a 
 * connection to other OMM Interactive Provider applications, including 
 * the Enterprise Platform, Data Feed Direct, and Elektron.
 *
 * Detailed Descriptions:
 * The first step of any UPA consumer application is to establish a 
 * network connection with its peer component (i.e., another application 
 * with which to interact). An OMM consumer typically creates an out-bound 
 * connection to the well-known hostname and port of a server (Interactive 
 * Provider or ADS). The consumer uses the Channel.connect() function to initiate 
 * the connection and then uses the Channel.init() function to complete 
 * channel initialization. 
 *
 * For this simple training app, only a single channel/connection is used for
 * the entire life of this app.
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod1a
 * (runs with a default set of parameters (-h localhost -p 14002 -i ""))
 *
 * or
 *
 * ./gradlew runconsumermod1a -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * UPA Consumer Training Module 1b: Ping (heartbeat) Management
 *********************************************************************************
 * Summary:
 * Ping or heartbeat messages indicate the continued presence of an application.
 * After the consumer connection is active, ping messages must be exchanged.
 * The negotiated ping timeout is retrieved using the Channel.pingTimeout() method.
 * The connection will be terminated if ping heartbeats are not sent or received
 * within the expected time frame.
 *
 * Detailed Descriptions:
 * Ping or heartbeat messages are used to indicate the continued presence of
 * an application. These are typically only required when no other information
 * is being exchanged. For example, there may be long periods of time that
 * elapse between requests made from an OMM consumer application. In this
 * situation, the consumer would send periodic heartbeat messages to inform
 * the providing application that it is still alive. Because the provider
 * application is likely sending more frequent information, providing updates
 * on any streams the consumer has requested, it may not need to send
 * heartbeats as the other data is sufficient to announce its continued
 * presence. It is the responsibility of each connection to manage the sending
 * and receiving of heartbeat messages.
 *
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod1b
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runconsumermod1b -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * UPA Consumer Training Module 1c: Reading and Writing Data
 *********************************************************************************
 * Summary:
 * When channel initialization is complete, the state of the channel
 * Channel.state() is TransportReturnCodes.ACTIVE, and applications can send
 * and receive data.
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
 * Command line usage:
 *
 * ./gradlew runconsumermod1c
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runconsumermod1c -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 *********************************************************************************
 * UPA Consumer Training Module 2: Log in
 *********************************************************************************
 * Summary:
 * Applications authenticate using the Login domain model. An OMM consumer must
 * authenticate with a provider using a Login request prior to issuing any other
 * requests or opening any other streams. After receiving a Login request, an
 * Interactive Provider determines whether a user is permissioned to access the
 * system. The Interactive Provider sends back a Login response, indicating to
 * the consumer whether access is granted.
 *
 * Detailed Descriptions:
 * After receiving a Login request, an Interactive Provider determines whether
 * a user is permissioned to access the system. The Interactive Provider sends
 * back a Login response, indicating to the consumer whether access is granted.
 *
 * a) If the application is denied, the Login stream is closed, and the
 * consumer application cannot send additional requests.
 * b) If the application is granted access, the Login response contains
 * information about available features, such as Posting, Pause and Resume,
 * and the use of Dynamic Views. The consumer application can use this
 * information to tailor its interaction with the provider.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod2
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300))
 *
 * or
 *
 * ./gradlew runconsumermod2 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 ************************************************************************
 * UPA Consumer Training Module 3: Obtain Source Directory
 ************************************************************************
 * Summary:
 * The Source Directory domain model conveys information about all available
 * services in the system. An OMM consumer typically requests a Source
 * Directory to retrieve information about available services and their
 * capabilities. This includes information about supported domain types, the
 * service's state, the quality of service (QoS), and any item group
 * information associated with the service.
 *
 * Detailed Descriptions:
 * The Source Directory Info filter contains service name and serviceId
 * information for all available services. When an appropriate service is
 * discovered by the OMM consumer, it uses the serviceId associated with the
 * service on all subsequent requests to that service.
 *
 * The Source Directory State filter contains status information for service,
 * which informs the consumer whether the service is Up and available, or Down
 * and unavailable.
 *
 * The Source Directory Group filter conveys item group status information,
 * including information about group states, as well as the merging of groups.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod3
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300 -s DIRECT_FEED))
 *
 * or
 *
 * ./gradlew runconsumermod3 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 ************************************************************************
 * UPA Consumer Training Module 4: Obtain Dictionary Information
 ************************************************************************
 * Summary:
 * Consumer applications often require a dictionary for encoding or decoding
 * specific pieces of information. This dictionary typically defines type and
 * formatting information. Content that uses the FieldList type requires
 * the use of a field dictionary (usually the Refinitiv RDMFieldDictionary,
 * although it could also be a user-defined or user-modified field dictionary).
 * A consumer application can choose whether to load necessary dictionary
 * information from a local file or download the information from an available
 * provider.
 *
 * Detailed Descriptions:
 * The Source Directory message should inform (from previous Module 3):
 * - DictionariesProvided: Which dictionaries are available for download.
 * - DictionariesUsed: The consumer of any dictionaries required to decode
 *   the content provided on a service. (Not used in previous Module 3)
 *
 * A consumer application can determine whether to load necessary dictionary
 * information from a local file or download the information from the
 * provider if available.
 *
 * - If loading from a file, UPA offers several utility functions to load and
 *   manage a properly-formatted field dictionary.
 * - If downloading information, the application issues a request using the
 *   Dictionary domain model. The provider application should respond with a
 *   dictionary response, typically broken into a multi-part message. UPA
 *   offers several utility functions for encoding and decoding of the
 *   Dictionary domain content.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod4
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300 -s DIRECT_FEED))
 *
 * or
 *
 * ./gradlew runconsumermod4 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 ************************************************************************
 * UPA Consumer Training Module 5: Issue Item Requests 
 ************************************************************************
 * Summary:
 * After the consumer application successfully logs in and obtains Source 
 * Directory and Dictionary information, it can request additional content. 
 * When issuing the request, the consuming application specifies the serviceId 
 * of the desired service along with a streamId. Requests can be sent for any 
 * domain using the formats defined in that domain model specification. In this 
 * simple example, we show how to make a Market Price level I data Item request 
 * to obtain the data from a provider.
 * 
 * Detailed Descriptions:
 * The Market Price domain provides access to Level I market information such as 
 * trades, indicative quotes, and top-of-book quotes. All information is sent as 
 * an FieldList. Field-value pairs contained in the field list include information 
 * related to that item (i.e., net change, bid, ask, volume, high, low, or last price).
 *
 * A Market Price request message is encoded and sent by OMM consumer applications. The 
 * request specifies the name and attributes of an item in which the consumer is 
 * interested. If a consumer wishes to receive updates, it can make a "streaming"
 * request by setting the RequestMsgFlags.STREAMING flag. If the flag is not set, the consumer 
 * is requesting a "snapshot," and the refresh should end the request.
 *
 * Market Price data is conveyed as an FieldList, where each FieldEntry 
 * corresponds to a piece of information and its current value. The field list should be 
 * decoded using its associated Field Dictionary, indicated by the dictionaryId present 
 * in the field list.
 * Similar to module_Dictionary, the main change is to add one more option in the second main loop.
 * Also added 4 more functions: sendMarketPriceItemRequest(), processMarketPriceItemResponse(), decodeMarketPricePayload()
 * and closeMarketPriceItemStream(). More details are in those functions.
 * 
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 * Command line usage:
 *
 * ./gradlew runconsumermod5
 * (runs with a default set of parameters (-h localhost -p 14002 -i "" -r 300 -s DIRECT_FEED -mp TRI))
 *
 * or
 *
 * ./gradlew runconsumermod5 -PcommandLineArgs="[-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-s <Service Name>] [-mp <Market Price Item Name>]"
 * (runs with specified set of parameters, all parameters are optional)
 *
 * Pressing the CTRL+C buttons terminates the program.
 *
 */

package com.thomsonreuters.upa.training.consumer;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.EnumType;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterEntryFlags;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapFlags;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceItem;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.rdm.UpdateEventTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WritePriorities;

/**
 * The Class Module_5_ItemRequest.
 */
public class Module_5_ItemRequest
{

    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String ENUM_TABLE_FILE_NAME = "enumtype.def";

    public static int FIELD_DICTIONARY_STREAM_ID = 3;
    public static int ENUM_TYPE_DICTIONARY_STREAM_ID = 4;
    public static int MARKETPRICE_ITEM_STREAM_ID = 5;

    static long serviceDiscoveryInfo_serviceId;

    static boolean serviceDiscoveryInfo_serviceNameFound = false;
    static boolean serviceDiscoveryInfo_upalDMTDictionarySupported = false;
    static boolean serviceDiscoveryInfo_upaDMTMarketPriceSupported = false;
    static boolean serviceDiscoveryInfo_RDMFieldDictionaryProvided = false;
    static boolean serviceDiscoveryInfo_enumtypeProvided = false;
    static int serviceDiscoveryInfo_ServiceState;
    static int serviceDiscoveryInfo_AcceptingRequests;
    static List<Qos> serviceDiscoveryInfo_qoS = new ArrayList<Qos>();

    static String serviceDiscoveryInfo_serviceName;

    public static boolean dictionariesLoadedInfo_fieldDictionaryLoaded = false;
    public static boolean dictionariesLoadedInfo_enumTypeDictionaryLoaded = false;
    public static boolean dictionariesLoadedInfo_fieldDictionaryFirstPart = true;
    public static boolean dictionariesLoadedInfo_enumTypeDictionaryFirstPart = true;
    public static boolean dictionariesLoadedInfo_isInitialized = false;

    /* dictionary file name  */
    final static String fieldDictionaryFileName = "RDMFieldDictionary";
    /* dictionary download name */
    final static String dictionaryDownloadName = "RWFFld";

    /* enum table file name */
    final static String enumTypeDictionaryFileName = "enumtype.def";
    /* enum table download name */
    final static String enumTableDownloadName = "RWFEnum";

    static int marketPriceItemInfo_streamId;
    static boolean marketPriceItemInfo_isRefreshComplete;
    static String marketPriceItemInfo_itemName;
    static MarketPriceItem marketPriceItemInfo_itemData;
    static State marketPriceItemInfo_itemState = CodecFactory.createState();
    static int MARKETPRICE_ITEM_STREAM_ID_START = -2;

    /**
     * The main method.
     *
     * @param args the arguments
     */
    public static void main(String[] args)
    {
        /**************************************************************************************************
         * DECLARING VARIABLES
         **************************************************************************************************/
        int retCode;

        /* Create error to keep track of any errors that may occur in Transport methods */
        Error error = TransportFactory.createError();

        /* Create and populate connect options to specify connection preferences */
        ConnectOptions cOpts = TransportFactory.createConnectOptions();

        /* InProgInfo Information for the In Progress Connection State */
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        /*  Channel Info returned by GetChannelInfo call */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        TransportBuffer msgBuf = null;

        long currentTime = 0;
        long upaRuntime = 0;
        long runTime = 0;

        /* Iterator used for decoding throughout the application - we can clear it and reuse it instead of recreating it */
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator(); /* the decode iterator is created (typically stack allocated)  */

        /* Create a channel to keep track of connection */
        Channel channel = null;

        /* Create an I/O notification mechanism (a Selector in our case) for our channel */
        Selector selector = null;

        /* Create initialization arguments (InitArgs) to specify any initialization options for Transport */
        InitArgs initArgs = TransportFactory.createInitArgs();

        MarketPriceItem marketPriceItem = new MarketPriceItem();

        /* the default option parameters */
        /* connect to server running on same machine */
        String srvrHostname = "localhost";
        /* server is running on port number 14003 */
        String srvrPortNo = "14002";
        /* use default NIC network interface card to bind to for all inbound and outbound data */
        String interfaceName = "";
        /* default service name is "DIRECT_FEED" used in source directory handler */
        String serviceName = "DIRECT_FEED";
        /* default item name is "TRI" used in market price item request handler */
        String itemName = "TRI";
        /* use default runTime of 300 seconds */
        runTime = 300;
        /* Create a bit mask to specify what I/O notification operations to keep track of (e.g. READ and CONNECT)*/
        short opMask = 0;

        /* User specifies options such as address, port, and interface from the command line.
         * User can have the flexibility of specifying any or all of the parameters in any order.
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
                else if ((args[i].equals("-mp")) == true)
                {
                    i += 2;
                    itemName = args[i - 1];
                }
                else
                {
                    System.out.printf("Error: Unrecognized option: %s\n\n", args[i]);
                    System.out.printf("Usage: %s or\n%s [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <runTime>] [-s <ServiceName>] [-mp <ItemName>]\n", args[0], args[0]);
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
        }

        DataDictionary dictionary;
        dictionary = CodecFactory.createDataDictionary();

        /*************************************
         * Step 1) Initialize Transport *
         *************************************/
        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s", error.errorId(), error.sysError(), error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        currentTime = System.currentTimeMillis();
        upaRuntime = currentTime + runTime * 1000;

        /* Set connect options */
        /* populate connect options, then pass to Connect function -
         * UPA Transport should already be initialized
         */
        /* use standard socket connection */
        cOpts.connectionType(ConnectionTypes.SOCKET); /* (0) Channel is a standard TCP socket connection type */
        cOpts.unifiedNetworkInfo().address(srvrHostname);
        cOpts.unifiedNetworkInfo().serviceName(srvrPortNo);
        cOpts.unifiedNetworkInfo().interfaceName(interfaceName);
        cOpts.blocking(false);
        cOpts.pingTimeout(60);
        cOpts.compressionType(CompressionTypes.NONE);

        cOpts.protocolType(Codec.protocolType());
        cOpts.majorVersion(Codec.majorVersion());
        cOpts.minorVersion(Codec.minorVersion());

        /* Initializes market price item fields. */
        marketPriceItem.initFields();
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

        /* Initialize item handler - initialize/reset market price item information */
        marketPriceItemInfo_itemName = itemName;
        marketPriceItemInfo_streamId = MARKETPRICE_ITEM_STREAM_ID_START;
        marketPriceItemInfo_isRefreshComplete = false;
        marketPriceItemInfo_itemData = marketPriceItem;

        /*********************************************************
         * For performance considerations, it is recommended to first load field
         * and enumerated dictionaries from local files, if they exist, at the
         * earlier stage of the consumer applications.
         *
         * When loading from local files, UPA offers several utility functions
         * to load and manage a properly-formatted field dictionary and enum
         * type dictionary.
         *
         * Only make Market Price item request after both dictionaries are
         * successfully loaded from files. If at least one of the dictionaries
         * fails to get loaded properly, it will continue the code path of
         * downloading the failed loaded dictionary or both dictionaries (if
         * neither exists in run-time path or loaded properly) from provider
         *********************************************************/

        dictionary.clear();

        /* load field dictionary from file - adds data from a Field Dictionary file to the DataDictionary */
        if (dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: " + error.text());

        }
        else
        {
            System.out.printf("Successfully loaded field dictionary from (local) file.\n\n");
            dictionariesLoadedInfo_fieldDictionaryLoaded = true;
        }

        if (dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: " + error.text());
        }
        else
        {
            System.out.printf("Successfully loaded enum type dictionary from (local) file.\n\n");
            dictionariesLoadedInfo_enumTypeDictionaryLoaded = true;
        }
        if ((dictionariesLoadedInfo_fieldDictionaryLoaded) && (dictionariesLoadedInfo_enumTypeDictionaryLoaded))
        {
            dictionariesLoadedInfo_isInitialized = true;
            System.out.printf("UPA Consumer application has successfully loaded both dictionaries from (local) files.\n\n");

        }
        /*****************************************************
         * Step 2) Connect to Server and receive a channel *
         *****************************************************/
        if ((channel = Transport.connect(cOpts, error)) == null)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            Transport.uninitialize();
            System.exit(TransportReturnCodes.FAILURE);
        }

        opMask |= SelectionKey.OP_READ;
        opMask |= SelectionKey.OP_CONNECT;

        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
            System.out.printf("Exception %s\n", e.getMessage());
            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
        }

        SelectionKey key = null;
        Set<SelectionKey> selectedKeys = null;
        Iterator<SelectionKey> keyIter = null;

        /*****************************************************************************************
         * Loop 1) Keep calling Channel.init() until it has properly connected
         * us to the server *
         *****************************************************************************************/
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
                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
            }

            /* Wait 1 second for any I/O notification updates in the channel */
            try
            {
                selector.select(1000);
                /* Create an iterator from the selector's updated channels*/
                selectedKeys = selector.selectedKeys();
                keyIter = selectedKeys.iterator();

                /* If our channel has not updated, we must have timed out */
                if (!keyIter.hasNext())
                {
                    System.out.printf("Channel initialization has timed out.\n");
                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                }
                else
                {
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
                            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                        }
                        System.out.printf("Channel successfully connected to server.\n");

                        /* Deduce an action from return code of Channel.init() */
                        switch (retCode)
                        {
                            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            {
                                /* Switch to a new channel if required */
                                if (inProgInfo.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                                {
                                    opMask = SelectionKey.OP_READ | SelectionKey.OP_CONNECT;
                                    System.out.printf("Channel switch, NEW: %d OLD %d\n", channel.selectableChannel(), channel.oldSelectableChannel());
                                    try
                                    {
                                        key = inProgInfo.oldSelectableChannel().keyFor(selector);
                                        key.cancel();
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                    }
                                    try
                                    {
                                        channel.selectableChannel().register(selector, opMask, channel);
                                    }
                                    catch (Exception e)
                                    {
                                        System.out.printf("Exception %s\n", e.getMessage());
                                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                    }
                                }
                                else
                                {
                                    System.out.printf("Channel init in progress...\n");
                                }
                            }
                                break;
                            case TransportReturnCodes.SUCCESS:
                            {
                                System.out.printf("Channel is now active!  Reading and writing can begin!\n");

                                /* Populate information from channel */
                                if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                }

                                /* Print out basic channel info */
                                System.out.printf("Channel Info:\n" + "Max Fragment Size:         %d\n" + "Max Output Buffers:        %d\n" + "Input Buffers:             %d\n" + "Send/Receive Buffer Sizes: %d/%d\n" + "Ping Timeout:              %d\n\n", channelInfo.maxFragmentSize(),
                                                  channelInfo.maxOutputBuffers(), channelInfo.numInputBuffers(), channelInfo.sysSendBufSize(), channelInfo.sysRecvBufSize(), channelInfo.pingTimeout());

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
                                System.out.printf("Unexpected return value\n");
                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
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

        /* Initialize ping management handler */
        initPingManagement(channel);
        /* Keep track of READ notifications */
        opMask |= SelectionKey.OP_READ;

        if ((retCode = sendLoginRequest(channel, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
        {
            opMask |= SelectionKey.OP_WRITE;
        }
        else if (retCode < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
        }

        /****************************************************
         * Loop 2) Check ping operations until runtime ends *
         ****************************************************/
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
                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
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
                        /* Initialize to a positive value for retCode in case we have more data that is available to read */
                        retCode = 1;

                        /******************************************************
                         * Loop 3) Read and decode for all buffers in channel *
                         ******************************************************/
                        while (retCode > TransportReturnCodes.SUCCESS)
                        {
                            /* Create read arguments (ReadArgs) to keep track of the statuses of any reads */
                            ReadArgs readArgs = TransportFactory.createReadArgs();

                            /**************************************************
                             * Step 4) Read message from channel into buffer *
                             **************************************************/
                            msgBuf = channel.read(readArgs, error);

                            if (msgBuf != null)
                            {
                                /* We have data to process */

                                /* Create message to represent buffer data */
                                Msg msg = CodecFactory.createMsg();

                                decodeIter.clear();
                                decodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

                                /******************************************
                                 * Step 5) Decode buffer message *
                                 ******************************************/
                                if ((retCode = msg.decode(decodeIter)) != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                }

                                /* Deduce an action based on the domain type of the message */
                                switch (msg.domainType())
                                {
                                    case DomainTypes.LOGIN:
                                    {
                                        if (processLoginResponse(msg, decodeIter) != TransportReturnCodes.SUCCESS)
                                        {
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }
                                        else
                                        {
                                            System.out.printf("UPA Consumer application is granted access and has logged in successfully.\n\n");
                                            serviceDiscoveryInfo_serviceName = serviceName;
                                            /* After it is granted access, those common daily activities and requesting could be done here.
                                             * But first we need the source directory.*/
                                            /* Send Source Directory request message */

                                            if ((retCode = sendSourceDirectoryRequest(channel, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
                                            {
                                                /* There is still data left to flush, leave our write notification enabled so we get called again.
                                                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                                 */
                                                /* set write fd if there's still other data queued */
                                                /* flush is done by application */
                                            }
                                            else if (retCode < TransportReturnCodes.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }
                                        }
                                    }
                                        break;
                                    /* (4) Source Message */
                                    case DomainTypes.SOURCE:
                                    {
                                        if (processSourceDirectoryResponse(channel, msg, error, decodeIter, selector) != TransportReturnCodes.SUCCESS)
                                        {
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }
                                        else
                                        {
                                            System.out.printf("UPA Consumer application has successfully received source directory information.\n\n");
                                        }

                                        /* exit app if service name entered by user cannot be found */
                                        if (!serviceDiscoveryInfo_serviceNameFound)
                                        {
                                            System.out.printf("\nSource directory response does not contain service name: %s. Exit app.\n", serviceName);
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }

                                        /* exit app if service we care about is NOT up and accepting requests */
                                        if ((serviceDiscoveryInfo_ServiceState != Directory.ServiceStates.UP) || (serviceDiscoveryInfo_AcceptingRequests == 0))
                                        {
                                            System.out.printf("\nService name: %s is NOT up and accepting requests. Exit app.\n", serviceName);
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }

                                        /* A consumer application can determine whether to load necessary dictionary information from a local file or
                                         * download the information from the provider if available.
                                         *
                                         * - If loading from a file, UPA offers several utility functions to load and manage a properly-formatted field dictionary.
                                         * - If downloading information, the application issues a request using the Dictionary domain model. The provider application
                                         * should respond with a dictionary response, typically broken into a multi-part message. UPA offers several utility functions
                                         * for encoding and decoding of the Dictionary domain content.
                                         *
                                         * In this simple app, we are trying to first loading necessary dictionary information from a local file, if it exists.
                                         * For performance considerations, we first load field and enumerated dictionaries from local files, if they exist,
                                         * at the earlier stage of the consumer applications.
                                         *
                                         * Otherwise, if loading dictionary information from a local file fails, we download the necessary dictionary information
                                         * from provider if available.
                                         */

                                        /*********************************************************
                                         * The Consumer app should already first
                                         * tried to load field and enumerated
                                         * dictionaries from local files at the
                                         * beginning of the app.
                                         *
                                         * Only make Market Price item request
                                         * after both dictionaries are
                                         * successfully loaded from files. If at
                                         * least one of the dictionaries fails
                                         * to get loaded properly, it will
                                         * continue the code path of downloading
                                         * the failed loaded dictionary or both
                                         * dictionaries (if neither exists in
                                         * run-time path or loaded properly)
                                         * from provider
                                         *********************************************************/

                                        /* Only make Market Price item request after both dictionaries are successfully loaded from files. */
                                        if ((dictionariesLoadedInfo_fieldDictionaryLoaded) && (dictionariesLoadedInfo_enumTypeDictionaryLoaded))
                                        {
                                            /* check to see if the provider supports the Market Price Domain Type (DomainTypes.MARKET_PRICE) */
                                            if (!serviceDiscoveryInfo_upaDMTMarketPriceSupported)
                                            {
                                                System.out.printf("\nMARKET_PRICE Domain Type is NOT supported by the indicated provider. Exit app.\n");
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }

                                            /* For this simple example, send just 1 Market Price item request message */

                                            {
                                                /* Send just 1 Market Price item request message */
                                                if ((retCode = sendMarketPriceItemRequest(channel, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
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
                                                    System.out.println("\nDictionary download not supported by the indicated provider");
                                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);

                                                }

                                            }

                                            break;
                                        }

                                        /*********************************************************
                                         * Need to continue the code path to
                                         * download at least 1 dictionary from
                                         * provider because at least 1 loading
                                         * of either field and enumerated
                                         * dictionaries from local files failed
                                         *
                                         * If downloading information, the
                                         * application issues a request using
                                         * the Dictionary domain model. The
                                         * provider application should respond
                                         * with a dictionary response, typically
                                         * broken into a multi-part message. UPA
                                         * offers several utility functions for
                                         * encoding and decoding of the
                                         * Dictionary domain content.
                                         *
                                         * Only make Market Price item request
                                         * after both dictionaries are
                                         * successfully downloaded from provider
                                         * or loaded from file already (in that
                                         * case, would not download for that
                                         * dictionary).
                                         *********************************************************/

                                        dictionary.clear();

                                        /* Will attempt to download the Refinitiv Field Dictionary (RDMFieldDictionary) from provider. */
                                        if (!dictionariesLoadedInfo_fieldDictionaryLoaded)
                                        {
                                            /* check if Dictionary Domain Type is supported */
                                            if (!serviceDiscoveryInfo_upalDMTDictionarySupported)
                                            {
                                                System.out.printf("\nDictionary Domain Type is NOT supported. Exit app.\n");
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }

                                            /* check if RDMFieldDictionary is available for downloading */
                                            if (!serviceDiscoveryInfo_RDMFieldDictionaryProvided)
                                            {
                                                System.out.printf("\nRDMFieldDictionary is NOT available for downloading. Exit app.\n");
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }

                                            /* Send RDMFieldDictionary Dictionary request message */
                                            if ((retCode = sendDictionaryRequest(channel, dictionary, channelInfo.maxFragmentSize(), selector, dictionaryDownloadName)) > CodecReturnCodes.SUCCESS)
                                            {
                                                /* There is still data left to flush, leave our write notification enabled so we get called again.
                                                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                                 */

                                                /* set write fd if there's still other data queued */
                                                /* flush is done by application */
                                                opMask |= SelectionKey.OP_WRITE;
                                            }
                                            else if (retCode < CodecReturnCodes.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }
                                        }

                                        /* Will attempt to download Enumerated Types Dictionaries (enumtype.def) from provider. */
                                        if (!dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                                        {
                                            /* check if Dictionary Domain Type is supported */
                                            if (!serviceDiscoveryInfo_upalDMTDictionarySupported)
                                            {
                                                System.out.printf("\nDictionary Domain Type is NOT supported. Exit app.\n");
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }

                                            /* check if enumtype.def is available for downloading */
                                            if (!serviceDiscoveryInfo_enumtypeProvided)
                                            {
                                                System.out.printf("\nenumtype.def is NOT available for downloading. Exit app.\n");
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }

                                            /* Send enumtype.def Dictionary request message */
                                            if ((retCode = sendDictionaryRequest(channel, dictionary, channelInfo.maxFragmentSize(), selector, enumTableDownloadName)) > CodecReturnCodes.SUCCESS)
                                            {
                                                /* There is still data left to flush, leave our write notification enabled so we get called again.
                                                 * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
                                                 */

                                                /* set write fd if there's still other data queued */
                                                /* flush is done by application */
                                                opMask |= SelectionKey.OP_WRITE;
                                            }
                                            else if (retCode < CodecReturnCodes.SUCCESS)
                                            {
                                                /* Closes channel, cleans up and exits the application. */
                                                closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                            }
                                        }

                                    }
                                        break;
                                    case DomainTypes.DICTIONARY:
                                    {
                                        if ((retCode = processDictionaryResponse(channel, msg, decodeIter, dictionary, selector)) != CodecReturnCodes.SUCCESS)
                                        {
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }
                                        else
                                        {
                                            /* Only make Market Price item request after both dictionaries are successfully downloaded from the provider. */
                                            if ((dictionariesLoadedInfo_fieldDictionaryLoaded) && (dictionariesLoadedInfo_enumTypeDictionaryLoaded))
                                            {
                                                dictionariesLoadedInfo_isInitialized = true;
                                                System.out.printf("UPA Consumer application has successfully downloaded both dictionaries or successfully downloaded 1 dictionary if the other dictionary has already been loaded from (local) file successfully.\n\n");

                                                /* check to see if the provider supports the Market Price Domain Type (MARKET_PRICE) */
                                                if (!serviceDiscoveryInfo_upaDMTMarketPriceSupported)
                                                {
                                                    System.out.printf("\nMARKET_PRICE Domain Type is NOT supported by the indicated provider. Exit app.\n");
                                                    /* Closes channel, cleans up and exits the application. */
                                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                                }

                                                /* For this simple example, send just 1 Market Price item request message */
                                                /* Send just 1 Market Price item request message */
                                                if ((retCode = sendMarketPriceItemRequest(channel, channelInfo.maxFragmentSize())) > TransportReturnCodes.SUCCESS)
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
                                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);

                                                }

                                            }
                                        }

                                    }
                                        break;
                                    case DomainTypes.MARKET_PRICE:
                                    {
                                        if (processMarketPriceItemResponse(channel, msg, decodeIter, dictionary, error) != TransportReturnCodes.SUCCESS)
                                        {
                                            /* Closes channel, cleans up and exits the application. */
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }
                                        else
                                        {
                                            System.out.printf("UPA Consumer application has successfully received Market Price item response.\n\n");
                                        }

                                    }
                                        break;
                                    default:
                                    {
                                        System.out.printf("Unhandled Domain Type: %d\n", msg.domainType());
                                    }
                                        break;
                                }

                                /* Acknowledge that a ping has been received */
                                receivedServerMsg = true;
                                System.out.printf("Ping message has been received successfully from the server due to data message ...\n\n");
                            }
                            else
                            {
                                /* Deduce an action from the return code of Channel.read() */
                                retCode = readArgs.readRetVal();
                                switch (retCode)
                                {
                                    /* Acknowledge that a ping has been received */
                                    case TransportReturnCodes.READ_PING:
                                        receivedServerMsg = true;
                                        System.out.printf("Ping message has been received successfully from the server due to ping message ...\n\n");
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
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        }
                                        try
                                        {
                                            channel.selectableChannel().register(selector, opMask, channel);
                                        }
                                        catch (Exception e)
                                        {
                                            System.out.printf("Exception %s\n", e.getMessage());
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
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
                                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                        break;
                                    default:
                                        if (retCode < 0)
                                        {
                                            System.out.printf("Unexpected return code\n");
                                            closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
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
                                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                                }
                            }
                        }
                    }
                }

                /* Processing ping management handler */
                if ((retCode = processPingManagementHandler(channel, opMask, selector)) > TransportReturnCodes.SUCCESS)
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
                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                }

                /* get current time */
                currentTime = System.currentTimeMillis();

                /* If the runtime has expired */
                if (System.currentTimeMillis() >= upaRuntime)
                {
                    if ((retCode = closeMarketPriceItemStream(channel, channelInfo.maxFragmentSize())) != TransportReturnCodes.SUCCESS) /* (retval > _RET_SUCCESS) or (retval < _RET_SUCCESS) */
                    {
                        /* When you close Market Price item, we want to make a best effort to get this across the network as it will gracefully
                         * close the open Market Price item stream. If this cannot be flushed or failed, this application will just close the
                         * connection for simplicity.
                         */

                        /* Closes channel, cleans up and exits the application. */
                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                    }

                    if ((retCode = closeLoginStream(channel, error)) < TransportReturnCodes.SUCCESS)
                    {
                        closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.FAILURE, dictionary);
                    }

                    else if (retCode > TransportReturnCodes.SUCCESS)
                    {
                        /* Flush before exiting */
                        key = channel.selectableChannel().keyFor(selector);
                        if (key.isWritable())
                        {
                            retCode = 1;

                            /******************************************************
                             * Loop 4) Flush all remaining buffers to channel *
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

                    System.out.printf("UPA Client run-time has expired...\n\n");
                    closeChannelCleanUpAndExit(channel, selector, TransportReturnCodes.SUCCESS, dictionary);
                }
            }
            catch (IOException e1)
            {
                e1.printStackTrace();
            }
        }
    }

    /*********************************************************
     * Closes channel and selector and exits application. *
     * 
     * @param channel - Channel to be closed *
     * @param selector - Selector to be closed *
     * @param code - if exit is due to errors/exceptions *
     * @param dictionary - data dictionary pointer *
     *********************************************************/
    public static void closeChannelCleanUpAndExit(Channel channel, Selector selector, int code, DataDictionary dictionary)
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

        if ((channel != null) && channel.close(error) < TransportReturnCodes.SUCCESS)
        {
            System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
        }

        /* when users are done, they should unload dictionaries to clean up memory */
        if (dictionary == null)
            System.out.printf("\nNULL Dictionary pointer.\n");
        else if (!dictionariesLoadedInfo_isInitialized)
            System.out.printf("\nNo need to delete dictionary - dictionary was not loaded yet.\n");
        else
            dictionary.clear();

        /******************************************
         * Step 6) Uninitialize Transport at end *
         ******************************************/
        Transport.uninitialize();

        if (code == TransportReturnCodes.SUCCESS)
        {
            System.out.printf("UPA Consumer Training Application successfully ended.\n");
        }

        System.exit(code);
    }

    /*
     * Initializes the ping times for upaChannel.
     * upaChannel - The channel for ping management info initialization
     * pingManagementInfo - The ping management information that is used
     */
    static int pingTimeoutServer; /* server ping timeout */
    static int pingTimeoutClient; /* client ping timeout */
    static long nextReceivePingTime; /* time client should receive next message/ping from server */
    static long nextSendPingTime; /* time to send next ping from client */
    static boolean receivedServerMsg; /* flag for server message received */

    /**
     * Inits the ping management.
     *
     * @param channel the channel
     */
    public static void initPingManagement(Channel channel)
    {
        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* set ping timeout for local and remote pings */
        pingTimeoutClient = channel.pingTimeout() / 3;
        pingTimeoutServer = channel.pingTimeout();

        /* set time to send next ping to remote connection */
        nextSendPingTime = currentTime + pingTimeoutClient * 1000;

        /* set time should receive next ping from remote connection */
        nextReceivePingTime = currentTime + pingTimeoutServer * 1000;
    }

    /**
     * Process ping management handler.
     *
     * @param channel the channel
     * @param opMask the op mask
     * @param selector the selector
     * @return the int
     */
    /*
     * Processing ping management handler
     * upaChannel - The channel for ping management processing
     * pingManagementInfo - The ping management information that is used
     */
    public static int processPingManagementHandler(Channel channel, Short opMask, Selector selector)
    {
        /* Handles the ping processing for upaChannel. Sends a ping to the server if the next send ping time has arrived and
         * checks if a ping has been received from the server within the next receive ping time.
         */
        int retval = TransportReturnCodes.SUCCESS;
        Error error = TransportFactory.createError();

        /* get current time */
        long currentTime = System.currentTimeMillis();

        /* handle client pings */
        if (currentTime >= nextSendPingTime)
        {
            /* send ping to server */
            /*********************************************************
             * Client/Consumer Application Life Cycle Major Step 4: Ping using
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
                        System.out.printf("Ping message has been sent successfully to the server ...\n\n");
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
                return TransportReturnCodes.FAILURE;

            }
        }
        return retval;
    }

    /**
     * ************************************************************
     * Sends a message buffer to the channel *.
     *
     * @param channel - the Channel to send the message buffer to *
     * @param msgBuf - the buffer to be sent *
     * @return status code *
     * ************************************************************
     */
    public static int sendMessage(Channel channel, TransportBuffer msgBuf)
    {
        Error error = TransportFactory.createError();
        int retCode = 0;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        writeArgs.priority(WritePriorities.HIGH);

        if ((retCode = channel.write(msgBuf, writeArgs, error)) == TransportReturnCodes.WRITE_CALL_AGAIN)
        {
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
                    System.out.printf("Error (%d) (errno: %d): %s\n", error.errorId(), error.sysError(), error.text());
                    channel.releaseBuffer(msgBuf, error);
                    return TransportReturnCodes.FAILURE;
                }
            }
        }

        return retCode;
    }

    final public static int LOGIN_STREAM_ID = 1;
    
    /**
     * ************************************************************
     * Sends login request message to a channel *.
     *
     * @param channel - the Channel of connection *
     * @param maxFragmentSize - the maximum fragment size before fragmentation *
     * @return status code *
     * ************************************************************
     */
    
    public static int sendLoginRequest(Channel channel, int maxFragmentSize)
    {
        int retCode;
        TransportBuffer msgBuf = null;
        Error error = TransportFactory.createError();

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        RequestMsg reqMsg = (RequestMsg)CodecFactory.createMsg();

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();

        Buffer applicationId = CodecFactory.createBuffer();
        Buffer applicationName = CodecFactory.createBuffer();
        UInt applicationRole = CodecFactory.createUInt();

        elementList.clear();
        elementEntry.clear();

        String userName = "put userName here";

        /* Get a buffer of the channel max fragment size */
        if ((msgBuf = upaGetBuffer(channel, maxFragmentSize, error)) == null)
        {
            return TransportReturnCodes.FAILURE;
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        /* Create and initialize request message */
        reqMsg.clear();
        reqMsg.msgClass(MsgClasses.REQUEST);
        reqMsg.streamId(LOGIN_STREAM_ID);
        reqMsg.domainType(DomainTypes.LOGIN);
        reqMsg.containerType(DataTypes.NO_DATA);
        reqMsg.flags(RequestMsgFlags.STREAMING);
        reqMsg.msgKey().flags(MsgKeyFlags.HAS_ATTRIB | MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME);

        if ((userName = System.getProperty("user.name")) != null)
        {
            reqMsg.msgKey().name().data(userName);
        }
        else
        {
            reqMsg.msgKey().name().data("Unknown");
        }

        reqMsg.msgKey().nameType(Login.UserIdTypes.NAME);
        reqMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

        if ((retCode = reqMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        elementList.clear();

        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);

        if ((retCode = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        applicationId.data("256");

        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPID);

        if ((retCode = elementEntry.encode(encIter, applicationId)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        applicationName.data("UPA Consumer Training");

        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.APPNAME);

        if ((retCode = elementEntry.encode(encIter, applicationName)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.ROLE);

        applicationRole.value(Login.RoleTypes.CONS);

        if ((retCode = elementEntry.encode(encIter, applicationRole)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        if ((retCode = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        if ((retCode = reqMsg.encodeKeyAttribComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        if ((retCode = reqMsg.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

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

    /**
     * ************************************************************
     * Processes a login response *.
     *
     * @param msg - the partially decoded message *
     * @param decIter - the decode iterator *
     * @return status code *
     * ************************************************************
     */
    public static int processLoginResponse(Msg msg, DecodeIterator decIter)
    {
        int retCode;
        State pState = CodecFactory.createState();

        String tempData = null;
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

                System.out.printf("Received Login Refresh Msg with Stream Id %d\n", msg.streamId());

                if ((msg.flags() & RefreshMsgFlags.SOLICITED) != 0)
                {
                    System.out.printf("The refresh msg is a solicited refresh (sent as a response to a request).\n");
                }
                else
                {
                    System.out.printf("A refresh sent to inform a consumer of an upstream change in information (i.e., an unsolicited refresh).\n");
                }

                key = msg.msgKey();

                if ((retCode = msg.decodeKeyAttrib(decIter, key)) != CodecReturnCodes.SUCCESS)
                {
                    return TransportReturnCodes.FAILURE;
                }

                if ((retCode = elementList.decode(decIter, null)) == CodecReturnCodes.SUCCESS)
                {
                    while ((retCode = elementEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (retCode == CodecReturnCodes.SUCCESS)
                        {
                            if (elementEntry.name().equals(ElementNames.APPID))
                            {
                                System.out.printf("\tReceived Login Response for ApplicationId: %s\n", elementEntry.encodedData().toString());
                            }
                            else if (elementEntry.name().equals(ElementNames.APPNAME))
                            {
                                System.out.printf("\tReceived Login Response for ApplicationName: %s\n", elementEntry.encodedData().toString());
                            }
                            else if (elementEntry.name().equals(ElementNames.POSITION))
                            {
                                System.out.printf("\tReceived Login Response for Position: %s\n", elementEntry.encodedData().toString());
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

                if (key != null)
                {
                    System.out.printf("Received Login Response for Username: %s\n", key.name().toString());
                }
                else
                {
                    System.out.printf("Received Login Response for Username: Unknown\n");
                }

                pState = ((RefreshMsg)msg).state();
                System.out.printf("%s\n\n", pState.toString());

                if (((msg.flags() & RefreshMsgFlags.SOLICITED) != 0) && (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.OK))
                {
                    System.out.printf("Login stream is OK and solicited\n");
                }
                else
                {
                    if (pState.streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        System.out.printf("Login stream is closed recover\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.CLOSED)
                    {
                        System.out.printf("Login attempt failed (stream closed)\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.SUSPECT)
                    {
                        System.out.printf("Login stream is suspect\n");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
                break;
            case MsgClasses.STATUS:
            {
                System.out.printf("Received Login StatusMsg\n");
                if ((msg.flags() & StatusMsgFlags.HAS_STATE) != 0)
                {
                    pState = ((StatusMsg)msg).state();
                    System.out.printf("%s\n\n", pState.toString());

                    if (pState.streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        System.out.printf("Login stream is closed recover\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.CLOSED)
                    {
                        System.out.printf("Login attempt failed (stream closed)\n");
                        return TransportReturnCodes.FAILURE;
                    }
                    else if (pState.streamState() == StreamStates.OPEN && pState.dataState() == DataStates.SUSPECT)
                    {
                        System.out.printf("Login stream is suspect\n");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
                break;
            case MsgClasses.UPDATE:
            {
                System.out.printf("Received Login Update\n");
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

    /**
     * ************************************************************
     * Close the login stream *.
     *
     * @param channel - the Channel of connection *
     * @param error - tracks error info *
     * @return status code *
     * ************************************************************
     */
    public static int closeLoginStream(Channel channel, Error error)
    {
        int retCode;
        TransportBuffer msgBuf = null;

        /* Create channel info as a holder */
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();

        /* Populate information from channel */
        if ((retCode = channel.info(channelInfo, error)) != TransportReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* Get a buffer of the channel max fragment size */
        if ((msgBuf = upaGetBuffer(channel, channelInfo.maxFragmentSize(), error)) == null)
        {
            return TransportReturnCodes.FAILURE;
        }

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        /* Create and initialize close message */
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(LOGIN_STREAM_ID);
        closeMsg.domainType(DomainTypes.LOGIN);
        closeMsg.containerType(DataTypes.NO_DATA);

        if ((retCode = closeMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("closeMsg.encode() failed with return code: %d\n", retCode);
            return TransportReturnCodes.FAILURE;
        }

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

    /**
     * ************************************************************
     * Performs two time pass to obtain buffer *.
     *
     * @param channel - the Channel of connection *
     * @param size - size of requested buffer *
     * @param error - tracks error info *
     * @return obtained buffer *
     * ************************************************************
     */
    public static TransportBuffer upaGetBuffer(Channel channel, int size, Error error)
    {
        int retCode;

        TransportBuffer msgBuf = null;

        /* First check error */
        if ((msgBuf = channel.getBuffer(size, false, error)) == null)
        {
            if (error.errorId() != TransportReturnCodes.NO_BUFFERS)
            {
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }

            /* Flush and obtain buffer again */
            retCode = channel.flush(error);
            if (retCode < TransportReturnCodes.SUCCESS)
            {
                System.out.printf("Channel.flush() failed with return code %d - <%s>\n", retCode, error.text());
                return null;
            }

            if ((msgBuf = channel.getBuffer(size, false, error)) == null)
            {
                System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                return null;
            }
        }
        return msgBuf;
    }

    final static int SRCDIR_STREAM_ID = 2;

    /**
     * Send source directory request.
     *
     * @param channel the channel
     * @param maxMsgSize the max msg size
     * @return the int
     */
    public static int sendSourceDirectoryRequest(Channel channel, int maxMsgSize)
    {
        int ret;
        Error error = TransportFactory.createError();
        TransportBuffer msgBuf = null;

        EncodeIterator encIter = CodecFactory.createEncodeIterator();

        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();

        /* get a buffer for the source directory request */
        if ((msgBuf = upaGetBuffer(channel, maxMsgSize, error)) == null)
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */

            return TransportReturnCodes.FAILURE;
        }

        requestMsg.clear();

        encIter.clear();
        if ((ret = encIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion())) < TransportReturnCodes.SUCCESS)
        {
            channel.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }

        /*encode source directory request*/
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(SRCDIR_STREAM_ID);
        requestMsg.domainType(DomainTypes.SOURCE);
        requestMsg.containerType(DataTypes.NO_DATA);
        /* RequestMsgFlags.STREAMING flag set. When the flag is set, the request is known as a "streaming" request, meaning
        * that the refresh will be followed by updates.
        */
        requestMsg.flags(RequestMsgFlags.STREAMING);

        /* A consumer can request information about all services by omitting serviceId information
         * If the consumer wishes to receive information about all services, the consumer should not specify a serviceId
         * (i.e., the consumer should not set MsgKeyFlags.HAS_SERVICE_ID).
         */

        /* set members in msgKey */
        /* (0x0008) This MsgKey has a filter, contained in \ref MsgKey::filter.  */
        requestMsg.msgKey().flags(MsgKeyFlags.HAS_FILTER);

        /* Because the Source Directory domain uses an FilterList, a consumer can indicate the specific source related
         * information in which it is interested via a msgKey.filter. Each bit-value represented in the filter corresponds
         * to an information set that can be provided in response messages.
         * Refinitiv recommends that a consumer application minimally request Info, State, and Group filters for the
         * Source Directory:
         * - The Info filter contains the service name and serviceId data for all available services. When an appropriate
         *   service is discovered by the OMM Consumer, the serviceId associated with the service is used on subsequent
         *   requests to that service.
         * - The State filter contains status data for the service. Such data informs the Consumer whether the service is
         *   up (and available) or down (and unavailable).
         * - The Group filter conveys any item group status information, including group states and as regards the merging
         *   of groups if applicable.
         */
        requestMsg.msgKey().filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);

        ret = requestMsg.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeDirectoryRequest(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        System.out.println("Send Directory Request");

        /* send source directory request */
        if ((ret = sendMessage(channel, msgBuf)) < TransportReturnCodes.SUCCESS)
        {
            /* Closes channel, cleans up and exits the application. */
            return TransportReturnCodes.FAILURE;
        }
        else if (ret > TransportReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush needs to be done by application */
        }
        return ret;

    }

    /**
     * Process source directory response.
     *
     * @param chnl the chnl
     * @param msg the msg
     * @param error the error
     * @param dIter the d iter
     * @param selector the selector
     * @return the int
     */
    public static int processSourceDirectoryResponse(Channel chnl, Msg msg, Error error, DecodeIterator dIter, Selector selector)
    {
        int retval;
        State pState = CodecFactory.createState();

        String tempData = null;
        Buffer tempBuffer = CodecFactory.createBuffer();

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        FilterList filterList = CodecFactory.createFilterList();

        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        ElementList elementList = CodecFactory.createElementList();
        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        int arrayCount = 0;

        int serviceCount = 0;
        /* this keeps track of which service we are actually interested in, that is,
         * when we run into the serviceName requested by the application
         */
        int foundServiceIndex = -1;

        String serviceName = "DIRECT_FEED";
        /* create primitive value to have key decoded into */
        UInt serviceId = CodecFactory.createUInt();

        UInt capabilities = CodecFactory.createUInt();

        String dictionariesProvided;

        Qos qoS = CodecFactory.createQos();
        List<Qos> QosBuf = new ArrayList<Qos>();
        boolean foundQoS = false;

        /* The ServiceState and AcceptingRequests elements in the State filter entry work together
         * to indicate the ability of a particular service to provide data:
         */
        UInt serviceState = CodecFactory.createUInt();
        UInt acceptingRequests = CodecFactory.createUInt();

        State serviceStatus = CodecFactory.createState();

        tempBuffer.data(tempData);

        RefreshMsg refreshMsg = (RefreshMsg)msg;
        StatusMsg statusMsg = (StatusMsg)msg;

        /* Switch cases depending on message class */
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            case MsgClasses.UPDATE:
            {

                /* decode source directory refresh or update */

                if (msg.msgClass() == MsgClasses.REFRESH)
                    System.out.printf("\nReceived Source Directory Refresh\n");
                else /* (4) Update Message */
                {
                    System.out.printf("\nReceived Source Directory Update\n");

                    /* When displaying update information, we should also display the updateType information. */
                    System.out.printf("UPDATE TYPE: %s\n", UpdateEventTypes.toString(((UpdateMsg)msg).updateType()));
                }

                /* decode contents into the map structure */
                if ((retval = map.decode(dIter)) < CodecReturnCodes.SUCCESS)
                {
                    /* decoding failure tends to be unrecoverable */
                    System.out.printf("Error (%d) (errno: %d)encountered with DecodeMap. Error Text: %s", error.errorId(), error.sysError(), error.text());
                    return retval;
                }

                /* @brief The primitive type associated with each MapEntry key. */
                /* source directory refresh or update key data type must be _DT_UINT */
                if (map.keyPrimitiveType() != DataTypes.UINT)
                {
                    System.out.printf("Map has incorrect keyPrimitiveType");
                    return CodecReturnCodes.FAILURE;
                }

                /* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
                 * indicates to UPA that user wants to decode summary data
                 */
                if ((map.flags() & MapFlags.HAS_SUMMARY_DATA) != 0)
                {
                    /* Map summary data is present. Its type should be that of Map.containerType */
                    System.out.printf("summary data is present. Its type should be that of Map.containerType\n");
                    /* Continue decoding ... */
                }
                else
                {
                    System.out.printf("\nMap summary data is NOT present.\n");
                }

                while ((retval = mapEntry.decode(dIter, serviceId)) != CodecReturnCodes.END_OF_CONTAINER)
                {
                    /* break out of decoding when predetermined max services (15) reached */
                    if (serviceCount == 15)
                    {
                        /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                         * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                         * the decoding process. This function will skip past remaining entries in the container and perform necessary
                         * synchronization between the content and iterator so that decoding can continue.
                         */
                        dIter.finishDecodeEntries();
                        System.out.printf("processSourceDirectoryResponse() maxServices limit reached - more services in message than memory can support\n");
                        break;
                    }

                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                    {
                        /* decoding failure tends to be unrecoverable */
                        System.out.printf("Error (%d) (errno: %d)encountered with DecodeMapEntry. Error Text: %s", error.errorId(), error.sysError(), error.text());
                        return retval;
                    }
                    else
                    {
                        if (msg.msgClass() == MsgClasses.REFRESH)
                            System.out.printf("\nReceived Source Directory Refresh for Decoded Service Id:%d", serviceId.toLong());
                        else /* (4) Update Message */
                            System.out.printf("\nReceived Source Directory Update for Decoded Service Id: %d", serviceId.toLong());

                        /* if this is the current serviceId we are interested in */
                        if ((serviceId.toLong() == serviceDiscoveryInfo_serviceId) && (serviceDiscoveryInfo_serviceNameFound == true))
                        {
                            /* this is the current serviceId we are interested in and requested by the UPA Consumer application */
                            System.out.printf(" (%s)\n", serviceDiscoveryInfo_serviceName);
                        }

                        /* decode contents into the filter list structure */
                        if ((retval = filterList.decode(dIter)) < CodecReturnCodes.SUCCESS)
                        {
                            /* decoding failure tends to be unrecoverable */
                            System.out.printf("Error (%d) (errno: %d)encountered with DecodeFilterList. Error Text: %s", error.errorId(), error.sysError(), error.text());
                            return retval;
                        }

                        /* decode each filter entry until there are no more left.
                         * Decodes an FilterEntry. This function expects the same DecodeIterator that was used with DecodeFilterList.
                         * This populates encData with an encoded entry.
                         */
                        while ((retval = filterEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (retval < CodecReturnCodes.SUCCESS)
                            {
                                /* decoding failure tends to be unrecoverable */
                                System.out.printf("Error (%d) (errno: %d)encountered with DecodeFilterEntry. Error Text: %s", error.errorId(), error.sysError(), error.text());
                                return retval;
                            }

                            else
                            {
                                /* Show how to use ContainerType.
                                 * After DecodeFilterEntry function returns, the FilterList.containerType (or FilterEntry.containerType if present)
                                 * can invoke the correct contained type's decode functions.
                                 * if filterEntry.containerType is present, switch on that; otherwise switch on filterList.containerType
                                 */
                                int cType;
                                if ((filterEntry.flags() & FilterEntryFlags.HAS_CONTAINER_TYPE) != 0)
                                    cType = filterEntry.containerType();
                                else
                                    cType = filterList.containerType();
                                switch (cType)
                                {
                                    /* (137) Map container type, used to represent primitive type key - container type paired entries.   <BR>*/
                                    case DataTypes.MAP:
                                    {
                                        /* Continue decoding map entries. call DecodeMap function
                                         * printf("DecodeFilterEntry: Continue decoding map entries.\n");
                                         */
                                    }
                                        break;
                                    /* (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
                                    case DataTypes.ELEMENT_LIST:
                                    {
                                        /* For Source Directory response, we actually know it is _DT_ELEMENT_LIST up-front
                                         * See code below in the next switch block: we are calling DecodeElementList to
                                         * continue decoding element entries
                                         */

                                        /* Continue decoding element entries. call DecodeElementList function
                                         * printf("DecodeFilterEntry: Continue decoding element entries.\n");
                                         */
                                    }
                                        break;
                                    default: /* Error handling */
                                    {
                                        System.out.printf("\nUnkonwn ContainerType: %d\n", cType);
                                        return CodecReturnCodes.FAILURE;
                                    }
                                }
                                /* decode source directory response information */
                                switch (filterEntry.id())
                                {
                                    case Directory.ServiceFilterIds.INFO: /* (1) Service Info Filter ID */
                                    {
                                        System.out.printf("\nDecoding Service Info Filter ID FilterListEntry\n");

                                        /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                        if ((retval = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
                                        {
                                            /* decoding failure tends to be unrecoverable */
                                            System.out.printf("Error (%d) (errno: %d)encountered with DecodeElementList. Error Text: %s", error.errorId(), error.sysError(), error.text());
                                            return retval;
                                        }
                                        /* decode element list elements */
                                        while ((retval = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                        {
                                            if (retval < CodecReturnCodes.SUCCESS)
                                            {
                                                /* decoding failure tends to be unrecoverable */
                                                System.out.printf("Error (%d) (errno: %d)encountered with DecodeElementEntry. Error Text: %s", error.errorId(), error.sysError(), error.text());
                                                return retval;
                                            }
                                            else
                                            {
                                                /* get service general information */

                                                /* Name */
                                                if (elementEntry.name().equals(ElementNames.NAME))
                                                {
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for ServiceName: %s\n", elementEntry.encodedData().toString());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for ServiceName: %s\n", elementEntry.encodedData().toString());

                                                    /* When an appropriate service is discovered by the OMM consumer, it uses the serviceId associated with the service
                                                     * on all subsequent requests to that service. Check if service name received in response matches that entered by user
                                                     * if it does, store the service id
                                                     */
                                                    serviceName = elementEntry.encodedData().toString();

                                                    if (serviceName.equals(serviceDiscoveryInfo_serviceName))
                                                    {
                                                        /* serviceName requested by the application is FOUND */
                                                        foundServiceIndex = serviceCount;

                                                        System.out.printf("\tService name: %s (%d) is discovered by the OMM consumer. \n", serviceName, serviceId.toLong());
                                                        serviceDiscoveryInfo_serviceId = serviceId.toLong();
                                                        serviceDiscoveryInfo_serviceNameFound = true;
                                                    }
                                                }
                                                /* Capabilities */
                                                else if (elementEntry.name().equals(ElementNames.CAPABILITIES))
                                                {
                                                    /* decode into the array structure header */
                                                    if ((retval = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        System.out.printf("Error %s (%d) encountered with DecodeArray. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    while ((retval = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                                    {
                                                        /* break out of decoding array items when predetermined max capabilities (10) reached */
                                                        if (arrayCount == 10)
                                                        {
                                                            /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                             * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                                                             * the decoding process. This function will skip past remaining entries in the container and perform necessary
                                                             * synchronization between the content and iterator so that decoding can continue.
                                                             */
                                                            dIter.finishDecodeEntries();
                                                            break;
                                                        }

                                                        if (retval == CodecReturnCodes.SUCCESS)
                                                        {
                                                            retval = capabilities.decode(dIter);
                                                            if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                            {
                                                                System.out.printf("Error %s (%d) encountered with DecodeUInt(). Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                                return retval;
                                                            }
                                                            if (msg.msgClass() == MsgClasses.REFRESH)
                                                                System.out.printf("\tReceived Source Directory Refresh for Decoded Capabilities[%d]: %d\n", arrayCount, capabilities.toLong());
                                                            else /* (4) Update Message */
                                                                System.out.printf("\tReceived Source Directory Update for Decoded Capabilities[%d]: %d\n", arrayCount, capabilities.toLong());

                                                            /* if advertising Dictionary domain type is supported */
                                                            if (capabilities.toLong() == DomainTypes.DICTIONARY)
                                                            {
                                                                System.out.printf("\tDomainTypes.DICTIONARY domain type is supported.\n");
                                                                serviceDiscoveryInfo_upalDMTDictionarySupported = true;
                                                            }

                                                            /* if advertising MarketPrice domain type is supported */
                                                            if (capabilities.toLong() == DomainTypes.MARKET_PRICE)
                                                            {
                                                                System.out.printf("\tDomainTypes.MARKET_PRICE domain type is supported.\n");
                                                                serviceDiscoveryInfo_upaDMTMarketPriceSupported = true;
                                                            }

                                                        }
                                                        else if (retval != CodecReturnCodes.BLANK_DATA)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            System.out.printf("Error %s (%d) encountered with DecodeArrayEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                            return retval;
                                                        }
                                                        arrayCount++;
                                                    }
                                                    arrayCount = 0;
                                                }
                                                /* DictionariesProvided */
                                                else if (elementEntry.name().equals(ElementNames.DICTIONARIES_PROVIDED))
                                                {
                                                    /* decode into the array structure header */
                                                    if ((retval = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        System.out.printf("Error %s (%d) encountered with DecodeArray. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    while ((retval = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                                    {
                                                        /* break out of decoding array items when predetermined max dictionaries (5) reached */
                                                        if (arrayCount == 5)
                                                        {
                                                            /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                             * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                                                             * the decoding process. This function will skip past remaining entries in the container and perform necessary
                                                             * synchronization between the content and iterator so that decoding can continue.
                                                             */
                                                            dIter.finishDecodeEntries();
                                                            break;
                                                        }

                                                        if (retval == CodecReturnCodes.SUCCESS)
                                                        {
                                                            if (msg.msgClass() == MsgClasses.REFRESH)
                                                                System.out.printf("\tReceived Source Directory Refresh for DictionariesProvided[%d]: %s\n", arrayCount, arrayEntry.encodedData().toString());
                                                            else /* (4) Update Message */
                                                                System.out.printf("\tReceived Source Directory Update for DictionariesProvided[%d]: %s\n", arrayCount, arrayEntry.encodedData().toString());

                                                            /* DictionariesProvided provide the dictionaries that are available for downloading */
                                                            /* Our training UPA Consumer app only cares about RDMFieldDictionary and enumtype.def */
                                                            dictionariesProvided = arrayEntry.encodedData().toString();

                                                            if (dictionariesProvided.equals(dictionaryDownloadName))
                                                            {
                                                                /* dictionary RDMFieldDictionary is available for downloading */
                                                                System.out.printf("\tDictionary Provided: %s with filename: %s \n", dictionariesProvided, fieldDictionaryFileName);
                                                                serviceDiscoveryInfo_RDMFieldDictionaryProvided = true;
                                                            }
                                                            else if (dictionariesProvided.equals(enumTableDownloadName))
                                                            {
                                                                /* dictionary enumtype.def is available for downloading */
                                                                System.out.printf("\tDictionary Provided: %s with filename: %s \n", enumTableDownloadName, enumTypeDictionaryFileName);
                                                                serviceDiscoveryInfo_enumtypeProvided = true;
                                                            }
                                                        }
                                                        else if (retval != CodecReturnCodes.BLANK_DATA)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            System.out.printf("Error %s (%d) encountered with DecodeArrayEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                            return retval;
                                                        }
                                                        arrayCount++;
                                                    }
                                                    arrayCount = 0;
                                                }

                                                /* QoS */
                                                else if (elementEntry.name().equals(ElementNames.QOS))
                                                {
                                                    foundQoS = true;
                                                    /* decode into the array structure header */
                                                    if ((retval = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
                                                    {
                                                        /* decoding failure tends to be unrecoverable */
                                                        System.out.printf("Error %s (%d) encountered with DecodeArray. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    while ((retval = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                                    {
                                                        /* break out of decoding array items when predetermined max QOS (5) reached */
                                                        if (arrayCount == 5)
                                                        {
                                                            /* The decoding process typically runs until the end of each container, indicated by CodecReturnCodes.END_OF_CONTAINER.
                                                             * This FinishDecodeEntries function sets the application to skip remaining entries in a container and continue
                                                             * the decoding process. This function will skip past remaining entries in the container and perform necessary
                                                             * synchronization between the content and iterator so that decoding can continue.
                                                             */
                                                            dIter.finishDecodeEntries();
                                                            break;
                                                        }

                                                        if (retval == CodecReturnCodes.SUCCESS)
                                                        {
                                                            /* Obtain QoS information such as data timeliness (e.g. real time) and rate (e.g. tick-by-tick). */
                                                            retval = qoS.decode(dIter);
                                                            if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                            {
                                                                System.out.printf("DecodeQos() failed with return code: %d\n", retval);
                                                                return retval;
                                                            }
                                                            else
                                                            {

                                                                QosBuf.add(qoS);
                                                                System.out.printf("\tReceived %s\n", QosBuf.get(0));
                                                            }
                                                        }
                                                        else if (retval != CodecReturnCodes.BLANK_DATA)
                                                        {
                                                            /* decoding failure tends to be unrecoverable */
                                                            System.out.printf("Error %s (%d) encountered with DecodeArrayEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                            return retval;
                                                        }
                                                        arrayCount++;
                                                    }
                                                    arrayCount = 0;

                                                    /* if this is the serviceName that is requested by the application */
                                                    if (serviceCount == foundServiceIndex)
                                                    {
                                                        /* Need to store the Source Directory QoS information */
                                                        Qos serviceQos = CodecFactory.createQos();
                                                        qoS.copy(serviceQos);
                                                        serviceDiscoveryInfo_qoS.add(serviceQos);
                                                    }
                                                }
                                            }
                                        }

                                        /* if QoS was not send in the directory refresh message set it to the default values */
                                        if (!foundQoS)
                                        {
                                            System.out.printf("\tNot Received Source Directory Refresh for QoS\n");
                                            System.out.printf("\tSet default QoS: Realtime/TickByTick/Static\n");

                                            for (arrayCount = 0; arrayCount < 5; arrayCount++)
                                            {
                                                serviceDiscoveryInfo_qoS.get(arrayCount).timeliness(QosTimeliness.REALTIME);
                                                serviceDiscoveryInfo_qoS.get(arrayCount).rate(QosRates.TICK_BY_TICK);
                                                serviceDiscoveryInfo_qoS.get(arrayCount).dynamic(false);
                                            }
                                        }
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.STATE: /* (2) Source State Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source State Filter ID FilterListEntry\n");

                                        /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                        if ((retval = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
                                        {
                                            /* decoding failure tends to be unrecoverable */
                                            System.out.printf("Error %s (%d) encountered with DecodeElementList. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            return retval;
                                        }

                                        /* decode element list elements */
                                        while ((retval = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                        {
                                            if (retval < CodecReturnCodes.SUCCESS)
                                            {
                                                /* decoding failure tends to be unrecoverable */
                                                System.out.printf("Error %s (%d) encountered with DecodeElementEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                return retval;
                                            }
                                            else
                                            {
                                                /* get service state information */

                                                /* The ServiceState and AcceptingRequests elements in the State filter entry work together to indicate the ability of
                                                 * a particular service to provide data:
                                                 * - ServiceState indicates whether the source of the data is accepting requests.
                                                 * - AcceptingRequests indicates whether the immediate upstream provider (the provider to which the consumer is directly connected)
                                                 *   can accept new requests and/or process reissue requests on already open streams.
                                                 */

                                                /* for our training app, for the service we are intersted in to be considered really up, both ServiceState has to be Up (1), and
                                                 * AcceptingRequests has to be Yes (1). That way, New requests and reissue requests can be successfully processed.
                                                 */

                                                /* ServiceState - Up(1), Down (0) */
                                                /* RDM_DIRECTORY_SERVICE_STATE_DOWN -  (0) Service state down */
                                                /* RDM_DIRECTORY_SERVICE_STATE_UP -  (1) Service state up */
                                                if (elementEntry.name().equals(ElementNames.SVC_STATE))
                                                {

                                                    retval = serviceState.decode(dIter);
                                                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                    {
                                                        System.out.printf("Error %s (%d) encountered with DecodeUInt(). Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Decoded ServiceState: %d\n", serviceState.toLong());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Decoded ServiceState: %d\n", serviceState.toLong());

                                                    /* if this is the serviceName that is requested by the application */
                                                    if (serviceCount == foundServiceIndex)
                                                    {
                                                        /* Need to track that service we care about is up */
                                                        serviceDiscoveryInfo_ServiceState = (int)serviceState.toLong();
                                                    }
                                                }

                                                /* AcceptingRequests - Yes (1), No (0) */
                                                else if (elementEntry.name().equals(ElementNames.ACCEPTING_REQS))
                                                {
                                                    retval = acceptingRequests.decode(dIter);
                                                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                    {
                                                        System.out.printf("Error %s (%d) encountered with DecodeUInt(). Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                        return retval;
                                                    }
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Decoded AcceptingRequests: %d\n", acceptingRequests.toLong());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Decoded AcceptingRequests: %d\n", acceptingRequests.toLong());

                                                    /* if this is the serviceName that is requested by the application */
                                                    if (serviceCount == foundServiceIndex)
                                                    {
                                                        /* Need to track that service we care about is accepting requests */
                                                        serviceDiscoveryInfo_AcceptingRequests = (int)acceptingRequests.toLong();
                                                    }
                                                }

                                                /* Status */
                                                else if (elementEntry.name().equals(ElementNames.STATUS))
                                                {
                                                    retval = serviceStatus.decode(dIter);
                                                    if (retval != CodecReturnCodes.SUCCESS && retval != CodecReturnCodes.BLANK_DATA)
                                                    {
                                                        System.out.printf("Error decoding State.\n");
                                                        return retval;
                                                    }
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Decoded State: %d %d %d %s\n", serviceStatus.streamState(), serviceStatus.dataState(), serviceStatus.code(), serviceStatus.text().toString());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Decoded State: %d %d %d %s\n", serviceStatus.streamState(), serviceStatus.dataState(), serviceStatus.code(), serviceStatus.text().toString());

                                                    tempBuffer.data(serviceStatus.toString());
                                                    System.out.printf("	%s\n\n", tempBuffer.toString());
                                                }
                                            }
                                        }

                                    }
                                        break;
                                    case Directory.ServiceFilterIds.GROUP: /* (3) Source Group Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source Group Filter ID FilterListEntry\n");

                                        /* decode element list - third parameter is 0 because we do not have set definitions in this example */
                                        if ((retval = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
                                        {
                                            /* decoding failure tends to be unrecoverable */
                                            System.out.printf("Error %s (%d) encountered with DecodeElementList. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                            return retval;
                                        }

                                        /* decode element list elements */
                                        while ((retval = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                                        {
                                            if (retval < CodecReturnCodes.SUCCESS)
                                            {
                                                /* decoding failure tends to be unrecoverable */
                                                System.out.printf("Error %s (%d) encountered with DecodeElementEntry. Error Text: %s\n", error.errorId(), error.sysError(), error.text());
                                                return retval;
                                            }
                                            else
                                            {
                                                /* get service group information */

                                                /* Group */
                                                if (elementEntry.name().equals(ElementNames.GROUP))
                                                {
                                                    if (msg.msgClass() == MsgClasses.REFRESH)
                                                        System.out.printf("\tReceived Source Directory Refresh for Group: %s\n", elementEntry.encodedData().data());
                                                    else /* (4) Update Message */
                                                        System.out.printf("\tReceived Source Directory Update for Group: %s\n", elementEntry.encodedData().data());
                                                }
                                            }
                                        }
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.LOAD: /* (4) Source Load Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source Load Filter ID FilterListEntry not supported in this app\n");
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.DATA: /* (5) Source Data Filter ID */
                                    {
                                        System.out.printf("\nDecoding Source Data Filter ID FilterListEntry not supported in this app\n");
                                    }
                                        break;
                                    case Directory.ServiceFilterIds.LINK: /* (6) Communication Link Filter ID */
                                    {
                                        System.out.printf("\nDecoding Communication Link Filter ID FilterListEntry not supported in this app\n");
                                    }
                                        break;
                                    default: /* Error handling */
                                    {
                                        System.out.printf("\nUnkonwn FilterListEntry filterID: %d\n", filterEntry.id());
                                        return CodecReturnCodes.FAILURE;
                                    }
                                }
                            }
                        }
                    }
                    serviceCount++;
                }

                if (msg.msgClass() == MsgClasses.REFRESH)
                {
                    pState = refreshMsg.state();
                    System.out.printf("	%s\n\n", pState.toString());
                }
            }
                break;

            case MsgClasses.STATUS:
                System.out.printf("\nReceived Source Directory StatusMsg\n");
                if ((statusMsg.flags() & StatusMsgFlags.HAS_STATE) != 0)
                {
                    pState = statusMsg.state();
                    System.out.printf("	%s\n\n", pState.toString());
                }
                break;

            case MsgClasses.CLOSE: /* (5) Close Message */
            {
                System.out.printf("\nReceived Source Directory Close\n");
            }
                break;
            default:
            {
                error.text("Received Unhandled Source Directory Msg Class: " + msg.msgClass());
                retval = CodecReturnCodes.FAILURE;
            }
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * ******************************************************************************************************************************
     * Processes a dictionary response. This consists of decoding the response.
     * * upaChannelInfo - The channel management information including the
     * dictionaries loaded information that is populated/updated * msg - The
     * partially decoded message * decodeIter - The decode iterator *
     * dataDictionary - the dictionary used for decoding the field entry data *
     * ******************************************************************************************************************************
     *
     * @param chnl the chnl
     * @param msg the msg
     * @param dIter the d iter
     * @param dictionary the dictionary
     * @param selector the selector
     * @return the int
     */
    public static int processDictionaryResponse(Channel chnl, Msg msg, DecodeIterator dIter, DataDictionary dictionary, Selector selector)
    {

        int ret = CodecReturnCodes.SUCCESS;
        Error error = TransportFactory.createError();

        MsgKey key = CodecFactory.createMsgKey();
        State pState = CodecFactory.createState();

        String tempData = null;
        Buffer tempBuffer = CodecFactory.createBuffer();
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        Int dictionaryType = CodecFactory.createInt();

        tempBuffer.data(tempData);

        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                /* decode dictionary response */

                /* get key */
                key = msg.msgKey();

                if (key != null)
                {
                    System.out.printf("Received Dictionary Response: %s\n", key.name());
                }
                else
                {
                    System.out.printf("Received Dictionary Response\n");
                }

                pState = refreshMsg.state();
                System.out.printf("%s\n\n", pState.toString());

                /* The dictionary response is typically broken into a multi-part message due to large size of the dictionary */
                if (((dictionariesLoadedInfo_fieldDictionaryFirstPart) && (msg.streamId() == FIELD_DICTIONARY_STREAM_ID)) || ((dictionariesLoadedInfo_enumTypeDictionaryFirstPart) && (msg.streamId() == ENUM_TYPE_DICTIONARY_STREAM_ID)))
                {
                    /* The first part of a dictionary refresh should contain information about its type.
                     * Save this information and use it as subsequent parts arrive. */

                    /* Extracts the RDM Dictionary Type (RDMDictionaryTypes) information from an encoded dictionary. This can determine the
                     * specific dictionary decode function that should be used (e.g. for dictionary type of RDM_DICTIONARY_FIELD_DEFINITIONS,
                     * the user would next invoke the DecodeFieldDictionary function).
                     * This is expected to be called after DecodeMsg (where the Msg.domainType is DICTIONARY), but before
                     * decoding the Msg.encDataBody payload.
                     */
                    if (dictionary.extractDictionaryType(dIter, dictionaryType, error) != CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("GetDictionaryType() failed: %s\n", error.text());
                        return CodecReturnCodes.FAILURE;
                    }
                    int dictionaryType_int = (int)dictionaryType.toLong();
                    switch (dictionaryType_int)
                    {
                        case Dictionary.Types.FIELD_DEFINITIONS:
                            /* (1) Field Dictionary type, typically referring to an RDMFieldDictionary */
                            dictionariesLoadedInfo_fieldDictionaryFirstPart = false;
                            break;
                        case Dictionary.Types.ENUM_TABLES:
                            /* (2) Enumeration Dictionary type, typically referring to an enumtype.def */
                            dictionariesLoadedInfo_enumTypeDictionaryFirstPart = false;
                            break;
                        default:
                            error.text("Received unexpected dictionary message on stream " + msg.streamId());
                            ret = CodecReturnCodes.FAILURE;
                    }
                }

                if (msg.streamId() == FIELD_DICTIONARY_STREAM_ID)
                {

                    ret = dictionary.decodeFieldDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        break;
                    }

                    /* (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages,
                     * as well as the final message in a multi-part response message sequence.
                     */
                    if ((refreshMsg.flags() & RefreshMsgFlags.REFRESH_COMPLETE) != 0)
                    {
                        dictionariesLoadedInfo_fieldDictionaryLoaded = true;
                        if (!dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                            System.out.printf("Field Dictionary complete, waiting for Enum Table...\n");
                        else
                            System.out.printf("Field Dictionary complete.\n");
                    }

                }
                else if (msg.streamId() == ENUM_TYPE_DICTIONARY_STREAM_ID)
                {

                    ret = dictionary.decodeEnumTypeDictionary(dIter, Dictionary.VerbosityValues.VERBOSE, error);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        break;
                    }

                    if ((refreshMsg.flags() & RefreshMsgFlags.REFRESH_COMPLETE) != 0)
                    {
                        dictionariesLoadedInfo_enumTypeDictionaryLoaded = true;
                        if (!dictionariesLoadedInfo_fieldDictionaryLoaded)
                            System.out.println("Enumerated Types Dictionary complete, waiting for Field Dictionary...");
                    }
                }
                else
                {
                    error.text("Received unexpected dictionary message on stream " + msg.streamId());
                    ret = CodecReturnCodes.FAILURE;
                    if (dictionariesLoadedInfo_fieldDictionaryLoaded && dictionariesLoadedInfo_enumTypeDictionaryLoaded)
                    {
                        System.out.printf("Dictionary ready, requesting Market Price item...\n\n");

                        /* Ok to send Market Price item request now */
                    }
                }
                break;
            case MsgClasses.STATUS:
                System.out.println("Received StatusMsg for dictionary");
                StatusMsg statusMsg = (StatusMsg)msg;
                if (statusMsg.checkHasState())
                {
                    State ppState = statusMsg.state();
                    System.out.printf("	%s\n\n", ppState.toString());

                }
                break;

            default:
                System.out.println("Received Unhandled Dictionary MsgClass: " + msg.msgClass());
                break;
        }
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Error processing dictionary response, exit.");
            closeChannelCleanUpAndExit(chnl, selector, TransportReturnCodes.FAILURE, dictionary);

        }
        return CodecReturnCodes.SUCCESS;

    }

    /*
     * Send a Dictionary request message to a channel. This consists of getting a message buffer, setting the dictionary
     * request information, encoding the dictionary request, and sending the dictionary request to the server. A Dictionary
     * request message is encoded and sent by OMM consumer applications. Some data requires the use of a dictionary for
     * encoding or decoding. This dictionary typically defines type and formatting information and directs the application
     * as to how to encode or decode specific pieces of information. Content that uses the FieldList type requires the
     * use of a field dictionary (usually the Refinitiv RDMFieldDictionary, though it could also be a user-defined or
     * modified field dictionary).
     * dictionaryName - The name of the dictionary to request
     */

    /**
     * Send dictionary request.
     *
     * @param chnl the chnl
     * @param dictionary the dictionary
     * @param maxFragmentSize the max fragment size
     * @param selector the selector
     * @param dictionaryName the dictionary name
     * @return the int
     */
    public static int sendDictionaryRequest(Channel chnl, DataDictionary dictionary, int maxFragmentSize, Selector selector, String dictionaryName)
    {

        Error error = TransportFactory.createError();

        int ret;
        TransportBuffer msgBuf = null;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter;
        encIter = CodecFactory.createEncodeIterator();

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(chnl, maxFragmentSize, error)) == null) /* first check Error */
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Dictionary request. */

        requestMsg.clear();
        encIter.clear();

        /* set the buffer on an EncodeIterator */
        if ((ret = encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion())) < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.DICTIONARY);
        requestMsg.containerType(DataTypes.NO_DATA);
        requestMsg.flags(RequestMsgFlags.NONE);

        if (dictionaryName.equals(dictionaryDownloadName))
        {
            requestMsg.streamId(FIELD_DICTIONARY_STREAM_ID);
        }
        else if (dictionaryName.equals(enumTableDownloadName))
        {
            requestMsg.streamId(ENUM_TYPE_DICTIONARY_STREAM_ID);
        }
        else
        {
            System.out.printf("\nSend Dictionary Request with Unknown Dictionary Name: %s\n", dictionaryName);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }
        requestMsg.msgKey().applyHasFilter();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().applyHasServiceId();

        requestMsg.msgKey().name().data(dictionaryName);
        requestMsg.msgKey().serviceId((int)serviceDiscoveryInfo_serviceId);
        requestMsg.msgKey().filter(Dictionary.VerbosityValues.VERBOSE);

        /* encode message - populate message and encode it */
        if ((ret = requestMsg.encode(encIter)) < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMsg() failed with return code: %d\n", ret);
            /* Closes channel, cleans up and exits the application. */
            return ret;
        }
        /* send dictionary request */
        if ((ret = sendMessage(chnl, msgBuf)) < CodecReturnCodes.SUCCESS)
        {
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }
        else if (ret > CodecReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush needs to be done by application */
        }

        return ret;

    }

    /**
     * Send market price item request.
     *
     * @param chnl the chnl
     * @param maxFragmentSize the max fragment size
     * @return the int
     */
    public static int sendMarketPriceItemRequest(Channel chnl, int maxFragmentSize)
    {

        Error error = TransportFactory.createError();

        int ret;
        TransportBuffer msgBuf = null;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter;
        encIter = CodecFactory.createEncodeIterator();

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(chnl, maxFragmentSize, error)) == null) /* first check Error */
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Dictionary request. */

        requestMsg.clear();

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */

        /* set the buffer on an EncodeIterator */
        if ((ret = encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion())) < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(MARKETPRICE_ITEM_STREAM_ID);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        /* No Batch request - since we are only showing one item in the itemList, it is a waste of bandwidth to send a batch request */

        /* No View */
        requestMsg.containerType(DataTypes.NO_DATA);
        /* RequestMsgFlags.STREAMING flag set. When the flag is set, the request is known as a "streaming" request, meaning
         * that the refresh will be followed by updates.
         */
        requestMsg.applyHasQos();
        requestMsg.applyHasPriority();
        requestMsg.applyStreaming();

        /* (0x002) This RequestMsg has priority information, contained in \ref RequestMsg::priorityClass and
         * \ref RequestMsg::priorityCount. This is used to indicate the importance of this stream.
         */
        requestMsg.priority().priorityClass(1);
        requestMsg.priority().count(1);

        /* (0x040) This RequestMsg contains quality of service information, contained in RequestMsg::qos.
         * If only \ref RequestMsg::qos is present, this is the QoS that will satisfy the request.
         * If RequestMsg::qos and RequestMsg::worstQos are both present, this indicates that any QoS
         * in that range will satisfy the request. '
         */
        /* copy the QoS information */
        serviceDiscoveryInfo_qoS.get(0).copy(requestMsg.qos());

        /* specify msgKey members */
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().applyHasNameType();
        requestMsg.msgKey().applyHasName();
        /* msgKey.nameType Optional. When consuming from Refinitiv sources, typically set to
         * InstrumentNameTypes.RIC (the "Reuters Instrument Code"). If this is not specified,
         * msgKey.nameType defaults to RDM_INSTRUMENT_NAME_TYPE_RIC.
         */
        requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
        /* msgKey.name Required in initial request, otherwise optional. The name of the requested item. */
        requestMsg.msgKey().name().data(marketPriceItemInfo_itemName);
        /* msgKey.serviceId is Required. This should be the Id associated with the service from which the consumer
         * wishes to request the item.
         */
        requestMsg.msgKey().serviceId((int)serviceDiscoveryInfo_serviceId);

        ret = requestMsg.encode(encIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("EncodeMsg() failed with return code: %d\n", ret);
            return ret;
        }

        /* send dictionary request */
        if ((ret = sendMessage(chnl, msgBuf)) < CodecReturnCodes.SUCCESS)
        {
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }
        else if (ret > CodecReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush needs to be done by application */
        }

        return ret;

    }

    /**
     * Process market price item response.
     *
     * @param chnl the chnl
     * @param msg the msg
     * @param dIter the d iter
     * @param dictionary the dictionary
     * @param error the error
     * @return the int
     */
    /*
     * Processes a market price response. This consists of extracting the key, printing out the item name contained in the key,
     * decoding the field list and field entry data.
     * msg - The partially decoded message
     * decodeIter - The decode iterator
     * dataDictionary - the dictionary used for decoding the field entry data
     */
    public static int processMarketPriceItemResponse(Channel chnl, Msg msg, DecodeIterator dIter, DataDictionary dictionary, Error error)
    {

        /* The MsgKey contains a variety of attributes used to identify the contents flowing within a particular stream.
         * This information, in conjunction with domainType and quality of service information, can be used to uniquely identify a
         * data stream.
         */
        MsgKey key = CodecFactory.createMsgKey();

        FieldList fieldList = CodecFactory.createFieldList();

        int ret;

        RefreshMsg refreshMsg = (RefreshMsg)msg;
        UpdateMsg updateMsg = (UpdateMsg)msg;

        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH: /* (2) Refresh Message */
            {

                System.out.println(" Received RefreshMsg for stream " + refreshMsg.streamId());
                /* update our item state list if its a refresh, then process just like update */

                marketPriceItemInfo_itemState.dataState(refreshMsg.state().dataState());
                marketPriceItemInfo_itemState.streamState(refreshMsg.state().streamState());
                /* refresh continued - process just like update */

                break;
            }

            case MsgClasses.UPDATE: /* (4) Update Message */
            {
                /* decode market price response for both Refresh Msg and Update Msg */

                if (msg.msgClass() == MsgClasses.REFRESH)
                {

                    System.out.printf("%s\n", refreshMsg.state().toString());
                }
                else if (msg.msgClass() == MsgClasses.UPDATE)
                {
                    System.out.printf("\nReceived Item Update Msg for stream %d \n", updateMsg.streamId());
                    /* When displaying update information, we should also display the updateType information. */
                    /* @brief Indicates domain-specific information about the type of content contained in this update.
                     * Domain Models.
                     */
                    System.out.printf("UPDATE TYPE: %s\n", ((UpdateMsg)msg).updateType());

                }

                /* get key */
                key = msg.msgKey();

                /* print out item name from key if it has it */

                if (key != null && key.checkHasName())
                {
                    System.out.printf("\n%s\nDOMAIN: %s\n", key.name().toString(), DomainTypes.toString(msg.domainType()));

                }
                else /* cached item name*/
                {
                    System.out.printf("\n%s\nDOMAIN: %s\n", marketPriceItemInfo_itemName, DomainTypes.toString(msg.domainType()));
                }

                /* decode into the MarketPrice Payload field list structure */
                ret = decodeMarketPricePayload(fieldList, dIter, dictionary);

                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("DecodeFieldList() failed with return code: " + ret);
                    return ret;
                }

            }
                break;
            case MsgClasses.STATUS: /* (3) Status Message */
            {
                StatusMsg statusMsg = (StatusMsg)msg;
                System.out.println("Received Item StatusMsg for stream " + msg.streamId());

                /* (0x020) Indicates that this StatusMsg has stream or group state information, contained in
                * StatusMsg::state.
                */

                if (statusMsg.checkHasState())
                {
                    System.out.printf("%s\n", statusMsg.state().toString());

                    /* Update our state table with the new state */
                    marketPriceItemInfo_itemState.dataState(statusMsg.state().dataState());
                    marketPriceItemInfo_itemState.streamState(statusMsg.state().streamState());
                }
            }
                break;
            case MsgClasses.ACK:
            {
                System.out.println("Received AckMsg for stream " + msg.streamId());

                /* get key */
                key = msg.msgKey();

                /* print out item name from key if it has it */

                if (key != null && key.checkHasName())
                {
                    System.out.printf("\n%.*s\nDOMAIN: %s\n", key.name().data(), DomainTypes.toString(msg.domainType()));

                }
                else
                /*cached item name*/
                {
                    System.out.printf("\n%.*s\nDOMAIN: %s\n", marketPriceItemInfo_itemName, DomainTypes.toString(msg.domainType()));
                }
                AckMsg ackMsg = (AckMsg)msg;
                System.out.printf("\tackId=%u\n", ackMsg.ackId());

                if (ackMsg.checkHasSeqNum())
                    System.out.printf("\tseqNum=%u\n", ackMsg.seqNum());

                if (ackMsg.checkHasNakCode())
                    System.out.printf("\tnakCode=%u\n", ackMsg.nakCode());

                if (ackMsg.checkHasText())
                    System.out.printf("\ttext=%s\n", ackMsg.text().data());

            }
                break;
            default:/* Error handling */
                System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    static UInt fidUIntValue = CodecFactory.createUInt();
    static Int fidIntValue = CodecFactory.createInt();
    static Real fidRealValue = CodecFactory.createReal();
    static com.thomsonreuters.upa.codec.Float fidFloatValue = CodecFactory.createFloat();
    static com.thomsonreuters.upa.codec.Double fidDoubleValue = CodecFactory.createDouble();
    static com.thomsonreuters.upa.codec.Enum fidEnumValue = CodecFactory.createEnum();
    static com.thomsonreuters.upa.codec.Date fidDateValue = CodecFactory.createDate();
    static Time fidTimeValue = CodecFactory.createTime();
    static DateTime fidDateTimeValue = CodecFactory.createDateTime();

    /**
     * Decode market price payload.
     *
     * @param fieldList the field list
     * @param dIter the d iter
     * @param dictionary the dictionary
     * @return the int
     */
    public static int decodeMarketPricePayload(FieldList fieldList, DecodeIterator dIter, DataDictionary dictionary)
    {

        int ret = 0;
        FieldEntry fEntry = CodecFactory.createFieldEntry();
        /* (0) Unknown Data Type. This is only valid when decoding an RsslFieldEntry type that requires a dictionary look-up.
         * If content is set defined, actual type enum will be present. <BR>
         */
        int dataType = DataTypes.UNKNOWN;
        Error error = TransportFactory.createError();

        fidUIntValue.clear();
        fidIntValue.clear();
        fidRealValue.clear();
        fidFloatValue.clear();
        fidDoubleValue.clear();
        fidEnumValue.clear();
        fidDateValue.clear();
        fidTimeValue.clear();
        fidDateTimeValue.clear();

        Qos fidQosValue = CodecFactory.createQos();
        State fidStateValue = CodecFactory.createState();
        /*get dictionary entry*/
        DictionaryEntry dictionaryEntry = dictionary.entry(fEntry.fieldId());

        /* decode into the field list structure */
        if ((ret = fieldList.decode(dIter, null)) == CodecReturnCodes.SUCCESS)
        {
            /* decode each field entry in list */
            while ((ret = fEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    /* decode each field entry info */
                    /* look up type in field dictionary and call correct primitive decode function */

                    /* get dictionary entry */
                    if (!dictionariesLoadedInfo_isInitialized)
                    {
                        return CodecReturnCodes.FAILURE;
                    }
                    else
                        dictionaryEntry = dictionary.entry(fEntry.fieldId());

                    // return if no entry found
                    if (dictionaryEntry == null)
                    {
                        System.out.printf("\tFid %d not found in dictionary\n", fEntry.fieldId());
                        return CodecReturnCodes.SUCCESS;
                    }

                    // print out fid name
                    System.out.printf("\t" + fEntry.fieldId() + "/" + dictionaryEntry.acronym().toString() + ": ");

                    // decode and print out fid value
                    dataType = dictionaryEntry.rwfType();

                    switch (dataType)
                    {
                        case DataTypes.UINT:
                            ret = fidUIntValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%d\n", fidUIntValue.toLong());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeUInt() failed: <" + CodecReturnCodes.toString(ret) + ">");
                                return ret;
                            }
                            break;
                        case DataTypes.INT:
                            ret = fidIntValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%d\n", fidIntValue.toLong());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeInt() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.FLOAT:
                            ret = fidFloatValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%f\n", fidFloatValue.toFloat());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeFloat() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.DOUBLE:
                            ret = fidDoubleValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%f\n", fidDoubleValue.toDouble());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeDouble() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.REAL:
                            ret = fidRealValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%s\n", fidRealValue.toString());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeReal() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.ENUM:
                            ret = fidEnumValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                EnumType enumType = dictionary.entryEnumType(dictionaryEntry, fidEnumValue);

                                if (enumType == null)
                                {
                                    System.out.printf("%f\n", fidEnumValue.toInt());
                                }
                                else
                                {
                                    System.out.printf(enumType.display().toString() + "(" + fidEnumValue.toInt() + ")");
                                }
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeEnum() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.DATE:
                            ret = fidDateValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%s\n", fidDateValue.toString());

                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeDate() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.TIME:
                            ret = fidTimeValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%s\n", fidTimeValue.toString());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeTime() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.DATETIME:
                            ret = fidDateTimeValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%s\n", fidDateTimeValue.toString());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeDateTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
                                return ret;
                            }
                            break;
                        case DataTypes.QOS:
                            ret = fidQosValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%s\n", fidQosValue.toString());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeQos() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        case DataTypes.STATE:
                            ret = fidStateValue.decode(dIter);
                            if (ret == CodecReturnCodes.SUCCESS)
                            {
                                System.out.printf("%s\n", fidStateValue.toString());
                            }
                            else if (ret != CodecReturnCodes.BLANK_DATA)
                            {
                                System.out.println("DecodeState() failed: <" + CodecReturnCodes.toString(ret) + ">");

                                return ret;
                            }
                            break;
                        // For an example of array decoding, see
                        // FieldListCodec.exampleDecode()
                        case DataTypes.ARRAY:
                            break;
                        case DataTypes.BUFFER:
                        case DataTypes.ASCII_STRING:
                        case DataTypes.UTF8_STRING:
                        case DataTypes.RMTES_STRING:
                            if (fEntry.encodedData().length() > 0)
                            {
                                System.out.printf("%s\n", fEntry.encodedData().toString());
                            }
                            else
                            {
                                ret = CodecReturnCodes.BLANK_DATA;
                            }
                            break;
                        default:
                            System.out.printf("Unsupported data type ( %s" + DataTypes.toString(dataType) + ")");
                            break;
                    }
                    if (ret == CodecReturnCodes.BLANK_DATA)
                    {
                        System.out.printf("<blank data>");
                    }
                }
                else
                {
                    /* decoding failure tends to be unrecoverable */
                    System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());

                    return ret;

                }
            }
        }
        else
        {
            /* decoding failure tends to be unrecoverable */
            System.out.printf("Error (%d) (errno: %d) encountered with Channel.getBuffer. Error Text: %s\n", error.errorId(), error.sysError(), error.text());

            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Close market price item stream.
     *
     * @param chnl the chnl
     * @param maxFragmentSize the max fragment size
     * @return the int
     */
    public static int closeMarketPriceItemStream(Channel chnl, int maxFragmentSize)
    {
        Error error = TransportFactory.createError();
        /* get a buffer for the item close */
        TransportBuffer msgBuf = null;
        CloseMsg closeMessage = (CloseMsg)CodecFactory.createMsg();
        int ret;

        /* Iterator used for encoding throughout the application - we can clear it and reuse it instead of recreating it */
        EncodeIterator encIter = CodecFactory.createEncodeIterator();

        /* Obtains a non-packable buffer of the requested size from the UPA Transport guaranteed buffer pool to write into for the Login request.
         * For an outbound TransportBuffer (specifically one gotten from Channel.getBuffer(), like this one) length() initially does indeed indicate the number of bytes available, 
         * but when the ByteBuffer's position is after the start 
         * (because the application has written something, i.e. encoded some or all of a Login request message), 
         * it actually returns the number of bytes between the start & current positions 
         * (basically, the number of bytes encoded thus far).
         */

        /* upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer. */
        if ((msgBuf = upaGetBuffer(chnl, maxFragmentSize, error)) == null) /* first check Error */
        {
            /* Connection should be closed, return failure */
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        /* if a buffer is returned, we can populate and write, encode an Msg into the buffer */

        /* Encodes the Dictionary request. */

        closeMessage.clear();

        /* clear encode iterator for reuse - this should be used to achieve the best performance while clearing the iterator. */
        encIter.clear();

        /* set version information of the connection on the encode iterator so proper versioning can be performed */

        /* set the buffer on an EncodeIterator */
        if ((ret = encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion())) < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.printf("\nSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }

        /* encode item close */
        closeMessage.msgClass(MsgClasses.CLOSE);
        closeMessage.streamId(MARKETPRICE_ITEM_STREAM_ID);
        closeMessage.domainType(DomainTypes.MARKET_PRICE);
        closeMessage.containerType(DataTypes.NO_DATA);

        /* encode message */

        /* Since there is no payload, no need for Init/Complete as everything is in the msg header */
        /* Functions without a suffix of Init or Complete (e.g. EncodeMsg) perform encoding within a single call,
         * typically used for encoding simple types like Integer or incorporating previously encoded data
         * (referred to as pre-encoded data).
         */
        ret = closeMessage.encode(encIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            chnl.releaseBuffer(msgBuf, error);
            System.out.println("encodeMarketPriceClose(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return TransportReturnCodes.FAILURE;
        }

        /* send Market Price item close */
        if ((ret = sendMessage(chnl, msgBuf)) < CodecReturnCodes.SUCCESS)
        {
            /* Closes channel, cleans up and exits the application. */
            return CodecReturnCodes.FAILURE;
        }
        else if (ret > CodecReturnCodes.SUCCESS)
        {
            /* There is still data left to flush, leave our write notification enabled so we get called again.
             * If everything wasn't flushed, it usually indicates that the TCP output buffer cannot accept more yet
             */

            /* set write fd if there's still other data queued */
            /* flush needs to be done by application */
        }

        return ret;
    }

}
