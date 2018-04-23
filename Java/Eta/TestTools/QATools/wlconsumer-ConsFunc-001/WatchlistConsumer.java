package com.thomsonreuters.upa.valueadd.examples.watchlistconsumer;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.shared.CommandLine;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;
//APIQA
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequestFlags;
//END APIQA
//APIQA:  Added DirectoryRequest
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
//END APIQA
import com.thomsonreuters.upa.valueadd.examples.common.ConnectionArg;
import com.thomsonreuters.upa.valueadd.examples.common.ItemArg;
import com.thomsonreuters.upa.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig.EventCounter;
import com.thomsonreuters.upa.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig.ItemInfo;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerCallback;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorRole;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

/**
 * <p>
 * This is a main class to run the UPA Value Add WatchlistConsumer application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * This is the main file for the WatchlistConsumer application.  It is a single-threaded
 * client application that utilizes the UPA Reactor's watchlist to provide recovery of data.
 * 
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the UPA Reactor, makes the desired connections, and
 * dispatches for events.
 * This application makes use of the RDM package for easier decoding of Login & Source Directory
 * messages.
 * </p>
 * <p>
 * This application supports consuming Level I Market Price, Level II Market By
 * Order, Level II Market By Price and Yield Curve. This application can optionally
 * perform on-stream and off-stream posting for Level I Market Price content. The
 * item name used for an off-stream post is "OFFPOST". For simplicity, the off-stream
 * post item name is not configurable, but users can modify the code if desired.
 * </p>
 * <p>
 * If multiple item requests are specified on the command line for the same domain and
 * the provider supports batch requests, this application will send the item requests
 * as a single Batch request.
 * </p>
 * <p>
 * If supported by the provider and the application requests view use, a dynamic
 * view will be requested with all Level I Market Price requests. For simplicity,
 * this view is not configurable but users can modify the code to change the
 * requested view.  
 * </p>
 * <p>
 * This application supports a symbol list request. The symbol list name is optional.
 * If the user does not provide a symbol list name, the name is taken from the source
 * directory response.
 * </p>
 * <p>
 * This application is intended as a basic usage example. Some of the design choices
 * were made to favor simplicity and readability over performance. This application 
 * Value Add and shows how using Value Add simplifies the writing of UPA
 * applications. Because Value Add is a layer on top of UPA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the UPA interfaces.
 * </p>
 * <H2>Setup Environment</H2>
 * <p>
 * The RDMFieldDictionary and enumtype.def files could be located in the
 * directory of execution or this application will request dictionary from
 * provider.
 * </p> 
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runWatchlistConsumer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runWatchlistConsumer -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-h Server host name. Default is <i>localhost</i>.
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-u Login user name. Default is system user name.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-mp Market Price domain item name. Default is <i>TRI.N</i>. The user can
 * specify multiple -mp instances, where each occurrence is associated with a
 * single item. For example, specifying -mp TRI -mp GOOG will provide content
 * for two MarketPrice items.
 * <li>-mbo Market By Order domain item name. No default. The user can specify
 * multiple -mbo instances, where each occurrence is associated with a single
 * item.
 * <li>-mbp market By Price domain item name. No default. The user can specify
 * multiple -mbp instances, where each occurrence is associated with a single
 * item.
 * <li>-yc Yield Curve domain item name. No default. The user can specify
 * multiple -yc instances, where each occurrence is associated with a
 * single item.
 * <li>-view viewFlag. Default is false. If true, each request will use a basic
 * dynamic view.
 * <li>-post postFlag. Default is false. If true, the application will attempt
 * to send post messages on the first requested Market Price item (i.e.,
 * on-stream).
 * <li>-offpost offPostFlag. Default is false. If true, the application will
 * attempt to send post messages on the login stream (i.e., off-stream).
 * <li>-publisherInfo publisherInfoFlag. Default is false. If true, the application will
 * attempt to send post messages with publisher ID and publisher address.
 * <li>-snapshot snapShotFlag. If true, each request will be non-streaming (i.e.
 * snapshot). Default is false (i.e. streaming requests).
 * <li>-sl symbolListName. symbolListName is optional without a default. If -sl
 * is specified without a ListName the itemList from the source directory
 * response will be used.
 * <li>-sld Requests item on the Symbol List domain and data streams for items on that list.
 * <li>-x Provides XML tracing of messages.
 * <li>-c Connection Type used (Socket, http, or encrypted).
 * Default is <i>Socket</i>.
 * <li>-runTime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * <li>-proxy proxyFlag. if provided, the application will attempt
 * to make an http or encrypted connection through a proxy server (if
 * connectionType is set to http or encrypted).
 * <li>-ph Proxy host name.
 * <li>-pp Proxy port number.
 * <li>-plogin User name on proxy server.
 * <li>-ppasswd Password on proxy server. 
 * <li>-pdomain Proxy Domain.
 * <li>-krbfile Proxy KRB file. 
 * <li>-keyfile keystore file for encryption.
 * <li>-keypasswd keystore password for encryption.
 * <li>-qSourceName (optional) specifies the source name for queue messages (if specified, configures consumer to receive queue messages)"
 * <li>-qDestName (optional) specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified)"
 * <li>-tunnel (optional) enables consumer to open tunnel stream and send basic text messages
 * <li>-tsServiceName (optional) specifies the service name for queue messages (if not specified, the service name specified in -c/-tcp is used)"
 * <li>-tsAuth (optional) specifies that consumer will request authentication when opening the tunnel stream. This applies to basic tunnel streams and those opened for queue messaging.
 * <li>-tsDomain (optional) specifies the domain that consumer will use when opening the tunnel stream. This applies to basic tunnel streams and those opened for queue messaging.
 * <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
 * <li>-ax Specifies the Authentication Extended information.
 * <li>-aid Specifies the Application ID.
 * </ul>
 * </p>
 */
public class WatchlistConsumer implements ConsumerCallback
{
    private final String FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private final String ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";
    private final String FIX_FIELD_DICTIONARY_FILE_NAME = "FDMFixFieldDictionary";
    private final String FIX_ENUM_TABLE_FILE_NAME = "FDMenumtypes.def";

    private final int MAX_QUEUE_DESTINATIONS = 10;

    private Reactor reactor;
    private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
    private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
    private WatchlistConsumerConfig watchlistConsumerConfig;
    private Selector selector;

    private long runtime;
    private Error error; // error information

    private DataDictionary fixdictionary;

    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

    ArrayList<ChannelInfo> chnlInfoList = new ArrayList<ChannelInfo>();

    private QueueMsgHandler queueMsgHandler;
    private TunnelStreamHandler tunnelStreamHandler;
    private String qServiceName;

    long cacheTime;
    long cacheInterval;
    StringBuilder cacheDisplayStr;
    Buffer cacheEntryBuffer;

    boolean _finalStatusEvent;
    private long closetime;
    private long closeRunTime;
    boolean closeHandled;

    private int FIELD_DICTIONARY_STREAM_ID = 3;
    private int ENUM_DICTIONARY_STREAM_ID = 4;

    ItemDecoder itemDecoder;
    boolean itemsRequested = false;

    // APIQA: adding newItemRequested which keeps track of number of "new" items
    // (requested after initla request) requested
    private int newItemRequested = 0;
    
    //APIQA:  adding a variable to count updates received
    private long updatesReceived;
    private int updateCount;

    // APIQA
    private int e1Counter = 0;
    private int e2Counter = 0;
    private int e3Counter = 0;
    private int e4Counter = 0;
    private int e5Counter = 0;
    private ArrayList<EventCounter> allEventCounters = new ArrayList<EventCounter>();

    // END APIQA

    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    public static final int MAX_MSG_SIZE = 1024;

    public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

    private final List<Integer> viewFieldList;
    // APIQA:
    private final List<String> viewElementNameList;
    private final List<Integer> reissueViewFieldList;
    
    boolean fieldDictionaryLoaded;
    boolean enumDictionaryLoaded;

    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
    private ItemRequest itemRequest;
    Buffer payload;

    public WatchlistConsumer()
    {
        fixdictionary = CodecFactory.createDataDictionary();

        error = TransportFactory.createError();

        dispatchOptions.maxMessages(1);
        _finalStatusEvent = true;

        closetime = 10; // 10 sec

        itemDecoder = new ItemDecoder();

        itemRequest = createItemRequest();
        itemsRequested = false;

        viewFieldList = new ArrayList<Integer>();
        viewElementNameList = new ArrayList<String>();
        
        // APIQA: reissue test
        reissueViewFieldList = new ArrayList<Integer>();

        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
            System.out.println("Selector.open() failed: " + e.getLocalizedMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }
    }

    /* Initializes the Value Add consumer application. */
    private void init(String[] args)
    {

        watchlistConsumerConfig = new WatchlistConsumerConfig();
        watchlistConsumerConfig.addCommandLineArgs();

        if (!watchlistConsumerConfig.init(args))
        {
            System.err.println("\nError loading command line arguments:\n");
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }

        itemDecoder.init();

        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("WatchlistConsumer initializing...");

        runtime = System.currentTimeMillis() + watchlistConsumerConfig.runtime() * 1000;
        closeRunTime = System.currentTimeMillis() + (watchlistConsumerConfig.runtime() + closetime) * 1000;

        // enable Reactor XML tracing if specified
        if (watchlistConsumerConfig.enableXmlTracing())
        {
            reactorOptions.enableXmlTracing();
        }

        // create reactor
        reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
        if (errorInfo.code() != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("createReactor() failed: " + errorInfo.toString());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        // register selector with reactor's reactorChannel.
        try
        {
            reactor.reactorChannel().selectableChannel().register(selector, SelectionKey.OP_READ, reactor.reactorChannel());
        }
        catch (ClosedChannelException e)
        {
            System.out.println("selector register failed: " + e.getLocalizedMessage());
            System.exit(ReactorReturnCodes.FAILURE);
        }

        /* create channel info, initialize channel info, and connect channels
         * for each connection specified */
        for (ConnectionArg connectionArg : watchlistConsumerConfig.connectionList())
        {
            // create channel info
            ChannelInfo chnlInfo = new ChannelInfo();
            chnlInfo.connectionArg = connectionArg;

            // initialize channel info
            initChannelInfo(chnlInfo);

            // connect channel
            int ret;
            if ((ret = reactor.connect(chnlInfo.connectOptions, (ReactorRole)chnlInfo.consumerRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
                System.exit(ReactorReturnCodes.FAILURE);
            }

            // add to ChannelInfo list
            chnlInfoList.add(chnlInfo);

            // APIQA
            allEventCounters = watchlistConsumerConfig.getEventCounters();
            // END APIQA

        }
    }

    protected ItemRequest createItemRequest()
    {
        return new ItemRequest();
    }

    /* Runs the Value Add consumer application. */
    private void run()
    {
        int selectRetVal, selectTime = 1000;
        while (true)
        {
            Set<SelectionKey> keySet = null;
            try
            {
                selectRetVal = selector.select(selectTime);
                if (selectRetVal > 0)
                {
                    keySet = selector.selectedKeys();
                }
            }
            catch (IOException e)
            {
                System.out.println("select failed: " + e.getLocalizedMessage());
                System.exit(ReactorReturnCodes.FAILURE);
            }

            // nothing to read
            if (keySet != null)
            {
                Iterator<SelectionKey> iter = keySet.iterator();
                int ret = ReactorReturnCodes.SUCCESS;
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    try
                    {
                        if (key.isReadable())
                        {
                            // retrieve associated reactor channel and dispatch
                            // on that channel
                            ReactorChannel reactorChnl = (ReactorChannel)key.attachment();

                            // dispatch until no more messages
                            while ((ret = reactorChnl.dispatch(dispatchOptions, errorInfo)) > 0)
                            {
                            }
                            if (ret == ReactorReturnCodes.FAILURE)
                            {
                                if (reactorChnl.state() != ReactorChannel.State.CLOSED && reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
                                {
                                    System.out.println("ReactorChannel dispatch failed: " + ret + "(" + errorInfo.error().text() + ")");
                                }
                            }
                        }
                    }
                    catch (CancelledKeyException e)
                    {
                    } // key can be canceled during shutdown
                }
            }

            if (System.currentTimeMillis() >= runtime && !closeHandled)
            {
                System.out.println("Consumer run-time expired, close now...");
                handleClose();
                closeHandled = true;
            }
            else if (System.currentTimeMillis() >= closeRunTime)
            {
                System.out.println("Consumer closetime expired, shutdown reactor.");
                break;
            }
	        if (!closeHandled)
	        {
	        	handlePosting();
	        	handleQueueMessaging();
	        	handleTunnelStream();
	        	
		        // send login reissue if login reissue time has passed
	        	for (ChannelInfo chnlInfo : chnlInfoList)
	        	{
	        		if (chnlInfo.canSendLoginReissue &&
	        			System.currentTimeMillis() >= chnlInfo.loginReissueTime)
	        		{
						LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
						submitOptions.clear();
						if (chnlInfo.reactorChannel.submit(loginRequest, submitOptions, errorInfo) !=  CodecReturnCodes.SUCCESS)
						{
							System.out.println("Login reissue failed. Error: " + errorInfo.error().text());
						}
						else
						{
							System.out.println("Login reissue sent");
						}
						chnlInfo.canSendLoginReissue = false;
	        		}
	        	}
	        }	        
	        
	        if(closeHandled && queueMsgHandler != null && queueMsgHandler._chnlInfo != null &&
	 	           !queueMsgHandler._chnlInfo.isQueueStreamUp) 
	        	break;
	 
	        if(closeHandled && tunnelStreamHandler != null && tunnelStreamHandler._chnlInfo != null &&
	           !tunnelStreamHandler._chnlInfo.isTunnelStreamUp) 
	        	break;
	        
	        if (closeHandled && queueMsgHandler == null && tunnelStreamHandler == null ) 
	        	break; 

		}		
	}


	//APIQA edited
	//APIQA: Requesting new item
	int requestNewItem(Reactor reactor, ReactorChannel channel, boolean existingItem, boolean isPrivate, boolean isSnap, boolean isView, int viewId, boolean isMsgKeyInUpdates, String name)
	{
		int ret = CodecReturnCodes.SUCCESS;
		itemRequest.clear();

		itemRequest.addItem(name);
		System.out.println("\n\n-----------------APIQA: Requesting new item " + name);

		itemRequest.domainType(6);//domaintype 6 is market price
		
		itemRequest.streamId(watchlistConsumerConfig.lastInitialStreamId()+ newItemRequested);
		
		if (! isSnap) {
			itemRequest.applyStreaming();
		}
		
		// Also populate itemInfo
		if (! existingItem)
		{
			ItemInfo itemInfo = watchlistConsumerConfig.createItemInfo();
			itemInfo.streamId(itemRequest.streamId());
			itemInfo.domain(itemRequest.domainType());
			itemInfo.name(name);
			//added specifying more attributes to itemInfo
			itemInfo.privateStream(isPrivate);
			itemInfo.viewStream(isView);
			itemInfo.snapshotStream(isSnap);
			itemInfo.msgKeyInUpdatesStream(isMsgKeyInUpdates);
			itemInfo.viewId(viewId);
			watchlistConsumerConfig.itemList().add(itemInfo);
		}
		
		// Encodeing VIEW
		payload = CodecFactory.createBuffer(); // move it to the top to share 
		payload.data(ByteBuffer.allocate(1024));

		if (payload == null)
		{
			return CodecReturnCodes.FAILURE;
		}

		encIter.clear();
		encIter.setBufferAndRWFVersion(payload, channel.majorVersion(), channel.minorVersion());

		//added enabling private stream for new item
		if(isPrivate)
		{
			itemRequest.applyPrivateStream();
		}

		if(isView)//added selectively making it a view item or not
		{
			itemRequest.applyHasView();
			System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW");

			if(viewId == 0)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 1 [FIDS: 2, 4, 38]");
				viewFieldList.add(2);
				viewFieldList.add(4);
				viewFieldList.add(38);

			}
			if (viewId == 1)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 1 [FIDS: 6, 22]");
				viewFieldList.add(6);
				viewFieldList.add(22);
			}
			if (viewId == 2)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 2 [FIDS 25,32]");
				viewFieldList.add(25);
				viewFieldList.add(32);
			}
			if (viewId == 3)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 3 [FIDS 25,32,267]");
				viewFieldList.add(22);
				viewFieldList.add(32);
				viewFieldList.add(267);
			}
			if (viewId == 4)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 4 [FIDS 11,38,267] on streamId");
				viewFieldList.add(11);
				viewFieldList.add(38);
				viewFieldList.add(267);
			}
			//APIQA: added more viewIds
			if (viewId == 5)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 5 [FIDS 2,32]");
				viewFieldList.add(2);
				viewFieldList.add(32);
			}
			if (viewId == 6)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 6 [FIDS 4,7,8]");
				viewFieldList.add(4);
				viewFieldList.add(7);
				viewFieldList.add(8);
			}
			if (viewId == 7)
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 7 [FIDS 0-99]");
				viewFieldList.add(1);
				viewFieldList.add(2);
				viewFieldList.add(3);
				viewFieldList.add(4);
				viewFieldList.add(5);
				viewFieldList.add(6);
				viewFieldList.add(7);
				viewFieldList.add(8);
				viewFieldList.add(9);
				viewFieldList.add(10);
				viewFieldList.add(11);
				viewFieldList.add(12);
				viewFieldList.add(13);
				viewFieldList.add(14);
				viewFieldList.add(15);
				viewFieldList.add(16);
				viewFieldList.add(17);
				viewFieldList.add(18);
				viewFieldList.add(19);
				viewFieldList.add(20);
				viewFieldList.add(21);
				viewFieldList.add(22);
				viewFieldList.add(23);
				viewFieldList.add(24);
				viewFieldList.add(25);
				viewFieldList.add(26);
				viewFieldList.add(27);
				viewFieldList.add(28);
				viewFieldList.add(29);
				viewFieldList.add(30);
				viewFieldList.add(31);
				viewFieldList.add(32);
				viewFieldList.add(33);
				viewFieldList.add(34);
				viewFieldList.add(35);
				viewFieldList.add(36);
				viewFieldList.add(37);
				viewFieldList.add(38);
				viewFieldList.add(39);
				viewFieldList.add(40);
				viewFieldList.add(41);
				viewFieldList.add(42);
				viewFieldList.add(43);
				viewFieldList.add(44);
				viewFieldList.add(45);
				viewFieldList.add(46);
				viewFieldList.add(47);
				viewFieldList.add(48);
				viewFieldList.add(49);
				viewFieldList.add(50);
				viewFieldList.add(51);
				viewFieldList.add(52);
				viewFieldList.add(53);
				viewFieldList.add(54);
				viewFieldList.add(55);
				viewFieldList.add(56);
				viewFieldList.add(57);
				viewFieldList.add(58);
				viewFieldList.add(59);
				viewFieldList.add(60);
				viewFieldList.add(61);
				viewFieldList.add(62);
				viewFieldList.add(63);
				viewFieldList.add(64);
				viewFieldList.add(65);
				viewFieldList.add(66);
				viewFieldList.add(67);
				viewFieldList.add(68);
				viewFieldList.add(69);
				viewFieldList.add(70);
				viewFieldList.add(71);
				viewFieldList.add(72);
				viewFieldList.add(73);
				viewFieldList.add(74);
				viewFieldList.add(75);
				viewFieldList.add(76);
				viewFieldList.add(77);
				viewFieldList.add(78);
				viewFieldList.add(79);
				viewFieldList.add(80);
				viewFieldList.add(81);
				viewFieldList.add(82);
				viewFieldList.add(83);
				viewFieldList.add(84);
				viewFieldList.add(85);
				viewFieldList.add(86);
				viewFieldList.add(87);
				viewFieldList.add(88);
				viewFieldList.add(89);
				viewFieldList.add(90);
				viewFieldList.add(91);
				viewFieldList.add(92);
				viewFieldList.add(93);
				viewFieldList.add(94);
				viewFieldList.add(95);
				viewFieldList.add(96);
				viewFieldList.add(97);
				viewFieldList.add(98);
				viewFieldList.add(99);
			}	
			itemRequest.viewFields(viewFieldList);
			ret = itemRequest.encode(encIter);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				errorInfo.error().text("RequestItem.encode() failed");
				errorInfo.error().errorId(ret);
				return ret;
			}
			itemRequest.requestMsg.encodedDataBody(payload);	
		
		}
		else
		{
			itemRequest.encode();
		}
		submitOptions.clear();
		if (watchlistConsumerConfig.serviceName() != null)
		{
			submitOptions.serviceName(watchlistConsumerConfig.serviceName());
		}
		//APIQA
        if (isMsgKeyInUpdates)
        {
            itemRequest.requestMsg.applyMsgKeyInUpdates();
            System.out.println("------------APIQA: Now MsgKeyInUpdates is applied");
        }
		
		ret = channel.submit(itemRequest.requestMsg, submitOptions, errorInfo);
		
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("\nReactorChannel.submit() failed: " + ret + "(" + errorInfo.error().text() + ")\n");
			System.exit(ReactorReturnCodes.FAILURE);
		}
		newItemRequested++;
		System.out.println("APIQA: New item requested: PrivateStream: "+isPrivate+", Snapshot: "+isSnap);
		return CodecReturnCodes.SUCCESS;
	}
	
	// APIQA: Reissue item
	int reissueItem(Reactor reactor, ReactorChannel channel,  int reissueType, int reissueViewId, int itemIndex)
	{
		int ret = CodecReturnCodes.SUCCESS;

        String nameOfItem = watchlistConsumerConfig.getNameOfItem(itemIndex);
        if(itemIndex >= watchlistConsumerConfig.itemList().size())
        {
    		System.out.println("\n\n-----------------APIQA: <Item Not found in itemlist> Invalid Reissue item " + nameOfItem + ", Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
    		return CodecReturnCodes.FAILURE;        	
       	
        }
        ItemInfo itInfo = watchlistConsumerConfig.itemList().get(itemIndex);
        if( itInfo == null || !(itInfo.name().equalsIgnoreCase(nameOfItem)))
        {
			System.out.println("\n\n-----------------APIQA: <Item Not found in itemlist> Invalid Reissue item " + nameOfItem + ", Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
			return CodecReturnCodes.FAILURE;        	
        }
        
		itemRequest.clear();

		itemRequest.addItem(nameOfItem);
		itemRequest.streamId(itInfo.streamId);
		itemRequest.domainType(itInfo.domain());        

		if(reissueType == 1)
		{
			if (!watchlistConsumerConfig.enableSnapshot())
	            itemRequest.applyStreaming();
			System.out.println("\n\n-----------------APIQA: Reissue item " + nameOfItem + " streamid " + itemRequest.streamId() + ", View Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
			// Encoding VIEW
			payload = CodecFactory.createBuffer(); // move it to the top to share 
			payload.data(ByteBuffer.allocate(1024));

			if (payload == null)
			{
				return CodecReturnCodes.FAILURE;
			}
			encIter.clear();
			encIter.setBufferAndRWFVersion(payload, channel.majorVersion(), channel.minorVersion());
			itemRequest.applyHasView();
			reissueViewFieldList.clear();
			if(reissueViewId == 0)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 0 [FIDS: 2, 4, 38]");
				reissueViewFieldList.add(2);
				reissueViewFieldList.add(4);
				reissueViewFieldList.add(38);
			}
			else if(reissueViewId == 1)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 1 [FIDS 6, 22]");
				reissueViewFieldList.add(6);
				reissueViewFieldList.add(22);
			}
			else if(reissueViewId == 2)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 2 [FIDS 25,32]");
				reissueViewFieldList.add(25);
				reissueViewFieldList.add(32);
			}
			else if(reissueViewId == 3)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 3 [FIDS 25,32,267]");
				reissueViewFieldList.add(22);
				reissueViewFieldList.add(32);
				reissueViewFieldList.add(267);
			}
			else if(reissueViewId == 4)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 4 [FIDS 11,38,267");
				reissueViewFieldList.add(11);
				reissueViewFieldList.add(38);
				reissueViewFieldList.add(267);
			}
			else if(reissueViewId == 5)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 5 [FIDS 2,32]");
				reissueViewFieldList.add(2);
				reissueViewFieldList.add(32);
			}
			else if(reissueViewId == 6)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 6 [FIDS 4,7,8");
				reissueViewFieldList.add(4);
				reissueViewFieldList.add(7);
				reissueViewFieldList.add(8);
			}
			else if(reissueViewId == 7)
			{
				System.out.println("------------------APIQA: reissue StreamID: " +  itemRequest.streamId() + " Setting VIEW 7 [FIDS 0-99");
				int vIdx =0;
				for(vIdx = 1; vIdx < 100; vIdx++ )
					reissueViewFieldList.add(vIdx);
			}
			itemRequest.viewFields(reissueViewFieldList);
			ret = itemRequest.encode(encIter);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("\n\n-----------------APIQA: Failed to encode View Reissue for item " + nameOfItem + ", Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
				errorInfo.error().text("RequestItem.encode() failed");
				errorInfo.error().errorId(ret);
				return ret;
			}
			itemRequest.requestMsg.encodedDataBody(payload);
		}
		else if(reissueType == 2)
		{ // Pause
            System.out.println("-----------------APIQA: reissue for item " + nameOfItem + " streamid " + itemRequest.streamId() + ", Reissue Type: " + reissueType + ", Setting PAUSE flags ");
			if (!watchlistConsumerConfig.enableSnapshot())
	            itemRequest.applyStreaming();
			itemRequest.applyPause();
  //          itemRequest.applyNoRefresh();			
            itemRequest.encode();
		}
		else if(reissueType == 3)
		{ // Resume
			if (!watchlistConsumerConfig.enableSnapshot())
	            itemRequest.applyStreaming();
           System.out.println("-----------------APIQA: reissue for item " + nameOfItem + " streamid " + itemRequest.streamId() + ", Reissue Type: " + reissueType + ", Setting Resume/Streamin ");
           System.out.println("-----------------APIQA: Setting RESUME/Streaming flags on item reissue for user streamid " + itemRequest.streamId());
            itemRequest.applyStreaming();
            itemRequest.encode();
		}
		else
		{
			System.out.println("\n\n-----------------APIQA: Item " + nameOfItem + "<UNSUPPORTED/INVALID> Reissue Type: " + reissueType + ", reissueViewId: " + reissueViewId);
			return CodecReturnCodes.FAILURE;        				
		}
	
		
		System.out.println(itemRequest.toString());

	    submitOptions.clear();
        if (watchlistConsumerConfig.serviceName() != null)
        {
            submitOptions.serviceName(watchlistConsumerConfig.serviceName());
        }
        ret = channel.submit(itemRequest.requestMsg, submitOptions, errorInfo);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nReactorChannel.submit() failed: " + ret + "(" + errorInfo.error().text() + ")\n");
        }

		return CodecReturnCodes.SUCCESS;
		
	}
	//END APIQA: Requesting new item


	/* Requests the desired items.  */
	int requestItems(Reactor reactor, ReactorChannel channel)
	{
		if (itemsRequested)
			return ReactorReturnCodes.SUCCESS;

		itemsRequested = true;
		System.out.println("\n\n Requesting first "+watchlistConsumerConfig.itemList().size()+" items.");

		for(int itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemList().size(); ++itemListIndex)
		{
		    int ret = CodecReturnCodes.SUCCESS;
		    
			itemRequest.clear();	
			
			itemRequest.addItem(watchlistConsumerConfig.itemList().get(itemListIndex).name);
			
			int domainType = watchlistConsumerConfig.itemList().get(itemListIndex).domain();
			
			itemRequest.domainType(domainType);
			itemRequest.streamId(watchlistConsumerConfig.itemList().get(itemListIndex).streamId);

			// APIQA:  Added a check on item itself - altered if
			if ((!watchlistConsumerConfig.enableSnapshot()) && (!watchlistConsumerConfig.itemList().get(itemListIndex).isSnapshot()))
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting STREAMING flag");
				itemRequest.applyStreaming();
			}

			if (watchlistConsumerConfig.itemList().get(itemListIndex).isPrivateStream())
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting PRIVATE flag");
				itemRequest.applyPrivateStream();
			}
			if (watchlistConsumerConfig.itemList().get(itemListIndex).isMsgKeyInUpdates())
			{
				System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting MSG_KEY_IN_UPDATES flag");
				itemRequest.requestMsg.applyMsgKeyInUpdates();
			}
			// END APIQA
            
			//APIQA
			if (watchlistConsumerConfig.hasReqQos()) {
	            itemRequest.applyHasQos();
	            itemRequest.applyHasWorstQos();
	            if (watchlistConsumerConfig.qosDynamic()) 
	            	itemRequest.qos().dynamic(watchlistConsumerConfig.qosDynamic());
	            else
	            	itemRequest.qos().dynamic(false);	            
	            itemRequest.qos().rate(watchlistConsumerConfig.qosRate());
	            itemRequest.qos().timeliness(watchlistConsumerConfig.qosTimeliness());
	            itemRequest.qos().timeInfo(watchlistConsumerConfig.qosTimeInfo());
	            itemRequest.qos().rateInfo(watchlistConsumerConfig.qosRateInfo());
	            itemRequest.worstQos().rate(watchlistConsumerConfig.worstQosRate());
	            itemRequest.worstQos().timeliness(watchlistConsumerConfig.worstQosTimeliness());
	            itemRequest.worstQos().timeInfo(watchlistConsumerConfig.worstQosTimeInfo());
	            itemRequest.worstQos().rateInfo(watchlistConsumerConfig.worstQosRateInfo());
			}
            //END APIQA
			if (domainType == DomainTypes.SYMBOL_LIST && watchlistConsumerConfig.itemList().get(itemListIndex).symbolListData)
			{
				itemRequest.symbolListData(true);
			    payload = CodecFactory.createBuffer(); 
			    payload.data(ByteBuffer.allocate(1024));

				if (payload == null)
				{
					return CodecReturnCodes.FAILURE;
				}

				encIter.clear();
				encIter.setBufferAndRWFVersion(payload, channel.majorVersion(), channel.minorVersion());
				
				
				ret = itemRequest.encode(encIter);
		         
				if (ret < CodecReturnCodes.SUCCESS)
				{
					errorInfo.error().text("RequestItem.encode() failed");
					errorInfo.error().errorId(ret);
					return ret;
				}

				itemRequest.requestMsg.encodedDataBody(payload);
			}
			else if (domainType == DomainTypes.MARKET_PRICE && (watchlistConsumerConfig.enableView()|| watchlistConsumerConfig.itemList().get(itemListIndex).isView))
			{			
			    payload = CodecFactory.createBuffer(); // move it to the top to share 
			    payload.data(ByteBuffer.allocate(1024));

				if (payload == null)
				{
					return CodecReturnCodes.FAILURE;
				}
				
				encIter.clear();
				encIter.setBufferAndRWFVersion(payload, channel.majorVersion(), channel.minorVersion());

				itemRequest.applyHasView();				
				
				// APIQA:  Added contents of view 
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 0)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 0 [FIDS 2,4,38]");
					viewFieldList.add(2);
					viewFieldList.add(4);
					viewFieldList.add(38);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 1)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 1 [FIDS 6,22]");
					viewFieldList.add(6);
					viewFieldList.add(22);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 2)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 2 [FIDS 25,32]");
					viewFieldList.add(25);
					viewFieldList.add(32);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 3)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 3 [FIDS 22,32,267]");
					viewFieldList.add(22);
					viewFieldList.add(32);
					viewFieldList.add(267);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 4)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 4 [FIDS 11,38,267] on streamId");
					viewFieldList.add(11);
					viewFieldList.add(38);
					viewFieldList.add(267);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 5)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 5 [FIDS 2,32]");
					viewFieldList.add(2);
					viewFieldList.add(32);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 6)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 6 [FIDS 4,7,8]");
					viewFieldList.add(4);
					viewFieldList.add(7);
					viewFieldList.add(8);
				}
				if (watchlistConsumerConfig.itemList().get(itemListIndex).viewId() == 7)
				{
					System.out.println("------------------APIQA: StreamID: " +  itemRequest.streamId() + " Setting VIEW 0 [FIDS 0-99]");
					viewFieldList.add(1);
					viewFieldList.add(2);
					viewFieldList.add(3);
					viewFieldList.add(4);
					viewFieldList.add(5);
					viewFieldList.add(6);
					viewFieldList.add(7);
					viewFieldList.add(8);
					viewFieldList.add(9);
					viewFieldList.add(10);
					viewFieldList.add(11);
					viewFieldList.add(12);
					viewFieldList.add(13);
					viewFieldList.add(14);
					viewFieldList.add(15);
					viewFieldList.add(16);
					viewFieldList.add(17);
					viewFieldList.add(18);
					viewFieldList.add(19);
					viewFieldList.add(20);
					viewFieldList.add(21);
					viewFieldList.add(22);
					viewFieldList.add(23);
					viewFieldList.add(24);
					viewFieldList.add(25);
					viewFieldList.add(26);
					viewFieldList.add(27);
					viewFieldList.add(28);
					viewFieldList.add(29);
					viewFieldList.add(30);
					viewFieldList.add(31);
					viewFieldList.add(32);
					viewFieldList.add(33);
					viewFieldList.add(34);
					viewFieldList.add(35);
					viewFieldList.add(36);
					viewFieldList.add(37);
					viewFieldList.add(38);
					viewFieldList.add(39);
					viewFieldList.add(40);
					viewFieldList.add(41);
					viewFieldList.add(42);
					viewFieldList.add(43);
					viewFieldList.add(44);
					viewFieldList.add(45);
					viewFieldList.add(46);
					viewFieldList.add(47);
					viewFieldList.add(48);
					viewFieldList.add(49);
					viewFieldList.add(50);
					viewFieldList.add(51);
					viewFieldList.add(52);
					viewFieldList.add(53);
					viewFieldList.add(54);
					viewFieldList.add(55);
					viewFieldList.add(56);
					viewFieldList.add(57);
					viewFieldList.add(58);
					viewFieldList.add(59);
					viewFieldList.add(60);
					viewFieldList.add(61);
					viewFieldList.add(62);
					viewFieldList.add(63);
					viewFieldList.add(64);
					viewFieldList.add(65);
					viewFieldList.add(66);
					viewFieldList.add(67);
					viewFieldList.add(68);
					viewFieldList.add(69);
					viewFieldList.add(70);
					viewFieldList.add(71);
					viewFieldList.add(72);
					viewFieldList.add(73);
					viewFieldList.add(74);
					viewFieldList.add(75);
					viewFieldList.add(76);
					viewFieldList.add(77);
					viewFieldList.add(78);
					viewFieldList.add(79);
					viewFieldList.add(80);
					viewFieldList.add(81);
					viewFieldList.add(82);
					viewFieldList.add(83);
					viewFieldList.add(84);
					viewFieldList.add(85);
					viewFieldList.add(86);
					viewFieldList.add(87);
					viewFieldList.add(88);
					viewFieldList.add(89);
					viewFieldList.add(90);
					viewFieldList.add(91);
					viewFieldList.add(92);
					viewFieldList.add(93);
					viewFieldList.add(94);
					viewFieldList.add(95);
					viewFieldList.add(96);
					viewFieldList.add(97);
					viewFieldList.add(98);
					viewFieldList.add(99);
				}
				// END APIQA:  Added contents of view

				itemRequest.viewFields(viewFieldList);

				ret = itemRequest.encode(encIter);
												         
				if (ret < CodecReturnCodes.SUCCESS)
				{
					errorInfo.error().text("RequestItem.encode() failed");
					errorInfo.error().errorId(ret);
					return ret;
				}							
				itemRequest.requestMsg.encodedDataBody(payload);				
			}
			else
			{
			    itemRequest.encode();
			}
					
			submitOptions.clear();
			if (watchlistConsumerConfig.serviceName() != null)
			{
			    submitOptions.serviceName(watchlistConsumerConfig.serviceName());
			}
			
			//APIQA
	        //if (isMsgKeyInUpdates)
	        // {
	        //    itemRequest.requestMsg.applyMsgKeyInUpdates();
	        //    System.out.println("------------APIQA:MsgKeyInUpdates applied");
	        // }
			
			ret = channel.submit(itemRequest.requestMsg, submitOptions, errorInfo);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("\nReactorChannel.submit() failed: " + ret + "(" + errorInfo.error().text() + ")\n");
				System.exit(ReactorReturnCodes.FAILURE);
			}
			
		}
		return CodecReturnCodes.SUCCESS;
	}	
		    
	
	private void requestDictionaries(ReactorChannel channel, ChannelInfo chnlInfo)
	{
		RequestMsg msg = (RequestMsg) CodecFactory.createMsg();

		/* set-up message */
		msg.msgClass(MsgClasses.REQUEST);
		
		msg.applyStreaming();
		msg.streamId(FIELD_DICTIONARY_STREAM_ID);
		chnlInfo.fieldDictionaryStreamId = FIELD_DICTIONARY_STREAM_ID;
		msg.domainType(DomainTypes.DICTIONARY);
		msg.containerType(DataTypes.NO_DATA);
		msg.msgKey().applyHasNameType();
		msg.msgKey().applyHasName();
		msg.msgKey().applyHasFilter();		
		msg.msgKey().filter(VerbosityValues.NORMAL);		
		msg.msgKey().name().data(FIELD_DICTIONARY_DOWNLOAD_NAME);

		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		submitOptions.serviceName(watchlistConsumerConfig.serviceName());
        
		channel.submit(msg, submitOptions, errorInfo);

        msg.streamId(ENUM_DICTIONARY_STREAM_ID);
        chnlInfo.enumDictionaryStreamId = ENUM_DICTIONARY_STREAM_ID;
        msg.msgKey().name().data(ENUM_TABLE_DOWNLOAD_NAME);

        channel.submit(msg, submitOptions, errorInfo);

    }

    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();

        switch (event.eventType())
        {
            case ReactorChannelEventTypes.CHANNEL_UP:
            {
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("Channel Up Event: " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("Channel Up Event");
                // register selector with channel event's reactorChannel
                try
                {
                    event.reactorChannel().selectableChannel().register(selector, SelectionKey.OP_READ, event.reactorChannel());
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }

                break;
            }
            case ReactorChannelEventTypes.FD_CHANGE:
            {
                System.out.println("Channel Change - Old Channel: " + event.reactorChannel().oldSelectableChannel() + " New Channel: " + event.reactorChannel().selectableChannel());

                // cancel old reactorChannel select
                SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();

                // register selector with channel event's new reactorChannel
                try
                {
                    event.reactorChannel().selectableChannel().register(selector, SelectionKey.OP_READ, event.reactorChannel());
                }
                catch (Exception e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_READY:
            {
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("Channel Ready Event: " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("Channel Ready Event");
                if (isRequestedServiceUp(chnlInfo))
                {
                    checkAndInitPostingSupport(chnlInfo);
                }

                break;

            }
            case ReactorChannelEventTypes.CHANNEL_OPENED:
            {
                // set ReactorChannel on ChannelInfo, again need this?
                chnlInfo.reactorChannel = event.reactorChannel();

                // APIQA: make possible for all intiial request to be done after
                // src dir refresh
                if (fieldDictionaryLoaded && enumDictionaryLoaded || itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile)
                {
                    if (!watchlistConsumerConfig.delayInitialRequest())
                        requestItems(reactor, event.reactorChannel());
                    if (watchlistConsumerConfig.reqItemBeforeLogin())
                    	break;
                }
                // END APIQA
                else
                    requestDictionaries(event.reactorChannel(), chnlInfo);

                break;
            }
            case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
            {
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down reconnecting: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down reconnecting");

                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("	Error text: " + event.errorInfo().error().text() + "\n");

                // unregister selectableChannel from Selector
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }

                // reset dictionary if not loaded from file
                if (itemDecoder.fieldDictionaryLoadedFromFile == false && itemDecoder.enumTypeDictionaryLoadedFromFile == false)
                {
                    if (chnlInfo.dictionary != null)
                    {
                        chnlInfo.dictionary.clear();
                    }
                }

                itemsRequested = false;
                chnlInfo.hasServiceInfo = false;
                chnlInfo.hasQServiceInfo = false;

                // APIQA
                e3Counter++;
                if (allEventCounters.size() > 0)
                {
                    for (int i = 0; i < allEventCounters.size(); ++i)
                    {
                        if (allEventCounters.get(i).delay() == e3Counter && allEventCounters.get(i).eventType() == 3)
                        {
                            int startIdx = allEventCounters.get(i).startIdx();
                            int endIdx = allEventCounters.get(i).endIdx();
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                boolean isSnap = watchlistConsumerConfig.getMPItemInfo(index, "isSnapshot");
                                boolean isView = watchlistConsumerConfig.getMPItemInfo(index, "isView");
                                boolean isPrivate = watchlistConsumerConfig.getMPItemInfo(index, "isPrivate");
                                boolean isMsgKeyInUpdates = watchlistConsumerConfig.getMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID = watchlistConsumerConfig.getViewId(index);
                                String nameOfItem = watchlistConsumerConfig.getNameOfItem(index);
                                requestNewItem(reactor, event.reactorChannel(), false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                            }
                            allEventCounters.remove(i);
                            break;
                        }
                    }
                }
                // END APIQA

                break;
            }
            case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down");

                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");

                // unregister selectableChannel from Selector
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }

                // close ReactorChannel
                if (chnlInfo.reactorChannel != null)
                {
                    chnlInfo.reactorChannel.close(errorInfo);
                }
                break;
            }
            case ReactorChannelEventTypes.WARNING:
                System.out.println("Received ReactorChannel WARNING event\n");
                break;
            default:
            {
                System.out.println("Unknown channel event!\n");
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int defaultMsgCallback(ReactorMsgEvent event)
    {
        String itemName = null;
        State itemState = null;
        ItemInfo item = null;

        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        Msg msg = event.msg();

        if (msg == null)
        {
            /* The message is not present because an error occurred while decoding it. Print 
             * the error and close the channel. If desired, the un-decoded message buffer
             * is available in event.transportBuffer(). */
            System.out.printf("defaultMsgCallback: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
            // unregister selectableChannel from Selector
            if (event.reactorChannel().selectableChannel() != null)
            {
                SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();
            }

            // close ReactorChannel
            if (chnlInfo.reactorChannel != null)
            {
                chnlInfo.reactorChannel.close(errorInfo);
            }
            return ReactorCallbackReturnCodes.SUCCESS;
        }

        item = watchlistConsumerConfig.getItemInfo(msg.streamId());

        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:

                RefreshMsg refreshMsg = (RefreshMsg)msg;
                if (refreshMsg.checkHasMsgKey())
                {
                    if (refreshMsg.msgKey().checkHasName())
                    {
                        itemName = refreshMsg.msgKey().name().toString(); // Buffer
                        if (item == null && refreshMsg.streamId() < 0)
                        {
                            watchlistConsumerConfig.addProvidedItemInfo(refreshMsg.streamId(), refreshMsg.msgKey(), refreshMsg.domainType());
                        }
                    }
                }
                else if (item != null)
                {
                    itemName = item.name();
                }

                System.out.println("DefaultMsgCallback Refresh ItemName: " + itemName + " Domain: " + DomainTypes.toString(refreshMsg.domainType()) + ", StreamId: " + refreshMsg.streamId());

                System.out.println("                      State: " + refreshMsg.state());

                itemState = refreshMsg.state();
                /* Decode data body according to its domain. */
                itemDecoder.decodeDataBody(event.reactorChannel(), refreshMsg);
                // APIQA
                if (refreshMsg.checkHasMsgKey())
                {
                        System.out.println("------------APIQA: DefaultMsgCallback RefreshMsg --  ItemName from MsgKey: " + refreshMsg.msgKey().name().toString() + "; ServiceId from MsgKey: " + refreshMsg.msgKey().serviceId()  + "; nameType from MsgKey: " + refreshMsg.msgKey().nameType() + "\n\n");
                }

                e5Counter++;               
                if (allEventCounters.size() > 0)
                {
                    for (int i = 0; i < allEventCounters.size(); ++i)
                    {
                        if (allEventCounters.get(i).delay() == e5Counter && allEventCounters.get(i).eventType() == 5)
                        {
                            int startIdx = allEventCounters.get(i).startIdx();
                            int endIdx = allEventCounters.get(i).endIdx();
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                boolean isSnap = watchlistConsumerConfig.getMPItemInfo(index, "isSnapshot");
                                boolean isView = watchlistConsumerConfig.getMPItemInfo(index, "isView");
                                boolean isPrivate;
                                isPrivate = watchlistConsumerConfig.getMPItemInfo(index, "isPrivate");
                                boolean isMsgKeyInUpdates = watchlistConsumerConfig.getMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID = watchlistConsumerConfig.getViewId(index);
                                String nameOfItem = watchlistConsumerConfig.getNameOfItem(index);                             
                                requestNewItem(reactor, event.reactorChannel(), false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);                                                
                            }
                            allEventCounters.remove(i);
                            break;
                        }
                    }
                }
                
                break;

            case MsgClasses.UPDATE:

                UpdateMsg updateMsg = (UpdateMsg)msg;
                if (updateMsg.checkHasMsgKey() && updateMsg.msgKey().checkHasName())
                {
                    itemName = updateMsg.msgKey().name().toString();
                }
                else if (item != null)
                {
                    itemName = item.name();
                }

                System.out.println("DefaultMsgCallback Update ItemName: " + itemName + " Domain: " + DomainTypes.toString(updateMsg.domainType()) + ", StreamId: " + updateMsg.streamId());

                /* Decode data body according to its domain. */
                // APIQA
                if (updateMsg.checkHasMsgKey())
                {
                        System.out.println("------------APIQA: DefaultMsgCallback UpdateMsg --  ItemName from MsgKey: " + updateMsg.msgKey().name().toString() + "; ServiceId from MsgKey: " + updateMsg.msgKey().serviceId()  + "; nameType from MsgKey: " + updateMsg.msgKey().nameType() + "\n\n");
                }
                itemDecoder.decodeDataBody(event.reactorChannel(), updateMsg);
                //API QA
                if(watchlistConsumerConfig.loginPauseAndResume()) {
                	 updatesReceived++;
                     // APIQA: Manufacture a login reissue:
                     if ( updatesReceived == 3 ) {
                         LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                         loginRequest.applyPause();
                         loginRequest.applyNoRefresh();
                         submitOptions.clear();
                         if ( (chnlInfo.reactorChannel.submit(loginRequest, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS) {
                             System.out.println("------------APIQA: attempted login reissue failed. Error: " + errorInfo.error().text());
                         } else {
                             System.out.println("------------APIQA: PAUSE-ALL login reissue done.");
                         }
                     }  	
                }
                if(watchlistConsumerConfig.reissueDirEvery5Updates()) {
                	updatesReceived++;
                	 if ((( updatesReceived % 5 ) == 0 ) && ( updatesReceived <= 60 )) {
                		 DirectoryRequest directoryRequest = (DirectoryRequest)chnlInfo.consumerRole.rdmDirectoryRequest();
                		 chnlInfo.consumerRole.rdmDirectoryRequest().filter(Directory.ServiceFilterFlags.INFO |
                			                     Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.LINK);
                         submitOptions.clear();
                         if ((chnlInfo.reactorChannel.submit(directoryRequest, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS) {
                             System.out.println("------------APIQA: attempted SRC DIR reissue failed. Error: " + errorInfo.error().text());
                         } else {
                             System.out.println("------------APIQA: SRC DIR reissue done.");
                         }
                	 }
                }
                if(watchlistConsumerConfig.reissueDirWithSID() != 0) {
                	 updatesReceived++;                	 
                	 if ( updatesReceived == 5 ) {
                		 DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
                		 submitOptions.clear();
                         directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
                         directoryRequest.streamId(2);
                         directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                                     Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
                         directoryRequest.applyHasServiceId();
                         directoryRequest.serviceId(watchlistConsumerConfig.reissueDirWithSID());
                         System.out.println("Try to reissue Directory with serviceId : " + watchlistConsumerConfig.reissueDirWithSID());
                         if ((chnlInfo.reactorChannel.submit(directoryRequest, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS) {
                             System.out.println("------------APIQA: attempted SRC DIR reissue failed. Error: " + errorInfo.error().text());
                         } else {
                             System.out.println("------------APIQA: SRC DIR reissue done.");
                         }
                	 }
                }
                
                //END APIQA
                e2Counter++;
                e4Counter++; // For reissue
                if (allEventCounters.size() > 0)
                {
                    for (int i = 0; i < allEventCounters.size(); ++i)
                    {
                        if (allEventCounters.get(i).delay() == e2Counter && allEventCounters.get(i).eventType() == 2)
                        {
                            int startIdx = allEventCounters.get(i).startIdx();
                            int endIdx = allEventCounters.get(i).endIdx();
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                boolean isSnap = watchlistConsumerConfig.getMPItemInfo(index, "isSnapshot");
                                boolean isView = watchlistConsumerConfig.getMPItemInfo(index, "isView");
                                boolean isPrivate;
                                isPrivate = watchlistConsumerConfig.getMPItemInfo(index, "isPrivate");
                                boolean isMsgKeyInUpdates = watchlistConsumerConfig.getMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID = watchlistConsumerConfig.getViewId(index);
                                String nameOfItem = watchlistConsumerConfig.getNameOfItem(index);
                                requestNewItem(reactor, event.reactorChannel(), false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                            }
                            allEventCounters.remove(i);
                            break;
                        }
                        
                        // Check for reissue
                        if (allEventCounters.get(i).delay() == e4Counter && allEventCounters.get(i).eventType() == 4)
                        {
                            int startIdx = allEventCounters.get(i).startIdx();
                            int endIdx = allEventCounters.get(i).endIdx();
                            int reissueType = allEventCounters.get(i).reissueType();
                            int reissueVid = allEventCounters.get(i).reissueViewId();
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                reissueItem(reactor, event.reactorChannel(), reissueType, reissueVid, index);
                            }
                            allEventCounters.remove(i);
                            break;
                        }
                    }
                }
                // END APIQA

                break;

            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;
                if (statusMsg.checkHasMsgKey())
                {
                    if (statusMsg.msgKey().checkHasName())
                    {
                        itemName = statusMsg.msgKey().name().toString();
                        if (item != null && statusMsg.streamId() < 0)
                        {
                            watchlistConsumerConfig.addProvidedItemInfo(statusMsg.streamId(), statusMsg.msgKey(), statusMsg.domainType());
                        }
                    }
                }
                else if (item != null)
                {
                    itemName = item.name();
                }

                System.out.println("DefaultMsgCallback Status -- ItemName: " + itemName + " Domain: " + DomainTypes.toString(statusMsg.domainType()) + ", StreamId: " + statusMsg.streamId());

                if (statusMsg.checkHasState())
                {
                    System.out.println(statusMsg.state());

                    itemState = statusMsg.state();
                }

                break;
            case MsgClasses.ACK:

                AckMsg ackMsg = (AckMsg)msg;
                if (ackMsg.checkHasMsgKey())
                {
                    if (ackMsg.msgKey().checkHasName())
                    {
                        itemName = ackMsg.msgKey().name().toString();
                    }
                }
                else if (item != null)
                {
                    itemName = item.name();
                }
                System.out.println("DefaultMsgCallback Ack --  ItemName: " + itemName + " Domain: " + DomainTypes.toString(ackMsg.domainType()) + ", StreamId: " + ackMsg.streamId());
                System.out.println(" ackId: " + ackMsg.ackId());
                if (ackMsg.checkHasSeqNum())
                {
                    System.out.println(" seqNum: " + ackMsg.seqNum());
                }
                if (ackMsg.checkHasNakCode())
                {
                    System.out.println(" nakCode: " + ackMsg.nakCode());
                }
                if (ackMsg.checkHasText())
                {
                    System.out.println(" text: " + ackMsg.text());
                }
                break;

            default:
                System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;

        }

        if (itemState != null && item != null)
        {
            /* Check state of any provider-driven streams.
             * If the state indicates the item was closed, remove it from our list. */
            if (msg.streamId() < 0 && itemState.streamState() != StreamStates.OPEN)
                watchlistConsumerConfig.removeProvidedItemInfo(item);

            /* Update item state. */
            else
                itemState.copy(item.state());
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        LoginMsgType msgType = event.rdmLoginMsg().rdmMsgType();

        switch (msgType)
        {
            case REFRESH:
                System.out.println("Received Login Refresh for Username: " + ((LoginRefresh)event.rdmLoginMsg()).userName());
                System.out.println(event.rdmLoginMsg().toString());

                // save loginRefresh
                ((LoginRefresh)event.rdmLoginMsg()).copy(chnlInfo.loginRefresh);

                System.out.println("Domain: " + DomainTypes.toString(DomainTypes.LOGIN) + ", StreamId: " + event.rdmLoginMsg().streamId());

                System.out.println(" State: " + chnlInfo.loginRefresh.state());
                if (chnlInfo.loginRefresh.checkHasUserName())
                    System.out.println(" UserName: " + chnlInfo.loginRefresh.userName().toString());

                // get login reissue time from authenticationTTReissue
                if (chnlInfo.loginRefresh.checkHasAuthenticationTTReissue())
                {
                    chnlInfo.loginReissueTime = chnlInfo.loginRefresh.authenticationTTReissue() * 1000;
                    chnlInfo.canSendLoginReissue = true;
                }

                break;

            case STATUS:
                LoginStatus loginStatus = (LoginStatus)event.rdmLoginMsg();
                System.out.println("Domain: " + DomainTypes.toString(DomainTypes.LOGIN) + ", StreamId: " + event.rdmLoginMsg().streamId());
                System.out.println("Received Login StatusMsg");
                if (loginStatus.checkHasState())
                {
                    System.out.println("	" + loginStatus.state());
                }
                if (loginStatus.checkHasUserName())
                    System.out.println(" UserName: " + loginStatus.userName().toString());
                
            	// APIQA:  Send a login close
                if(watchlistConsumerConfig.loginCloseAfterLoginStatus()) {
                    submitOptions.clear();
                    closeMsg.clear();
                    closeMsg.streamId(loginStatus.streamId());
                    closeMsg.msgClass(MsgClasses.CLOSE);
                    closeMsg.domainType(DomainTypes.LOGIN);
                    if ((chnlInfo.reactorChannel.submit(closeMsg, submitOptions, errorInfo)) != CodecReturnCodes.SUCCESS)
                        System.out.println("Close login Stream of " + loginStatus.streamId() + " Failed.");
                    else
                        System.out.println("----------------APIQA: Closed login stream of " + loginStatus.streamId() + " Failed.");
                }
                // END APIQA
                break;
            default:
                System.out.println("Received Unhandled Login Msg Type: " + msgType);
                break;
        }

        System.out.println("");

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        DirectoryMsgType msgType = event.rdmDirectoryMsg().rdmMsgType();
        List<Service> serviceList = null;

        switch (msgType)
        {
            case REFRESH:
                DirectoryRefresh directoryRefresh = (DirectoryRefresh)event.rdmDirectoryMsg();
                System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
                System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.REFRESH));
                // APIQA
                System.out.println("---------APIQA: ServiceList: " +  directoryRefresh.serviceList().toString() + "\n\n");
                // END APIQA
                System.out.println(directoryRefresh.state().toString());

                serviceList = directoryRefresh.serviceList();
                String serviceName = chnlInfo.connectionArg.service();

                for (Service service : serviceList)
                {
                    if (service.info().serviceName().toString() != null)
                    {
                        if (service.info().serviceName().toString().equals(serviceName))
                        {
                            // save serviceInfo associated with requested
                            // service name
                            if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
                            {
                                System.out.println("Service.copy() failure");
                                uninitialize();
                                System.exit(ReactorReturnCodes.FAILURE);
                            }
                            chnlInfo.hasServiceInfo = true;
                        }
                        if (service.info().serviceName().toString().equals(qServiceName))
                        {
                            // save serviceInfo associated with requested
                            // service name
                            if (service.copy(chnlInfo.qServiceInfo) < CodecReturnCodes.SUCCESS)
                            {
                                System.out.println("Service.copy() failure");
                                uninitialize();
                                System.exit(ReactorReturnCodes.FAILURE);
                            }

                            chnlInfo.hasQServiceInfo = true;
                        }
                    }
                }
                // APIQA
                if (!itemsRequested && watchlistConsumerConfig.delayInitialRequest())
                {
                    if (fieldDictionaryLoaded && enumDictionaryLoaded || itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile)
                    {
                        requestItems(reactor, event.reactorChannel());
                    }
                }
                if (watchlistConsumerConfig.reqItemBeforeLogin())
                {
                    if (fieldDictionaryLoaded && enumDictionaryLoaded || itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile)
                    {
                        requestItems(reactor, event.reactorChannel());
                    }
                }
                // END APIQA
                break;
            case UPDATE:
                DirectoryUpdate directoryUpdate = (DirectoryUpdate)event.rdmDirectoryMsg();

                serviceName = chnlInfo.connectionArg.service();
                String qServiceName = chnlInfo.connectionArg.qService();
                System.out.println("Received Source Directory Update");
                System.out.println(directoryUpdate.toString());

                System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
                System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.UPDATE));
                
                // APIQA:  Added this line
                System.out.println("---------APIQA: ServiceList: " +  directoryUpdate.serviceList().toString() + "\n\n");
                // END APIQA:  Added this line

                serviceList = directoryUpdate.serviceList();

                for (Service service : serviceList)
                {
                    if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId())
                    {
                        chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
                    }

                    if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.qServiceInfo.serviceId())
                    {
                        chnlInfo.qServiceInfo.action(MapEntryActions.DELETE);
                    }

                    boolean updateServiceInfo = false, updateQServiceInfo = false;
                    if (service.info().serviceName().toString() != null)
                    {
                        System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
                        // update service cache - assume cache is built with
                        // previous refresh message
                        if (service.info().serviceName().toString().equals(serviceName) || service.serviceId() == chnlInfo.serviceInfo.serviceId())
                        {
                            updateServiceInfo = true;
                        }
                        if (service.info().serviceName().toString().equals(qServiceName) || service.serviceId() == chnlInfo.qServiceInfo.serviceId())
                        {
                            updateQServiceInfo = true;
                        }
                    }
                    else
                    {
                        if (service.serviceId() == chnlInfo.serviceInfo.serviceId())
                        {
                            updateServiceInfo = true;
                        }
                        if (service.serviceId() == chnlInfo.qServiceInfo.serviceId())
                        {
                            updateQServiceInfo = true;
                        }
                    }

                    if (updateServiceInfo)
                    {
                        // update serviceInfo associated with requested service
                        // name
                        if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("Service.copy() failure");
                            uninitialize();
                            System.exit(ReactorReturnCodes.FAILURE);
                        }
                        chnlInfo.hasServiceInfo = true;
                    }
                    if (updateQServiceInfo)
                    {
                        // update serviceInfo associated with requested service
                        // name
                        if (service.copy(chnlInfo.qServiceInfo) < CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("Service.copy() failure");
                            uninitialize();
                            System.exit(ReactorReturnCodes.FAILURE);
                        }

                        chnlInfo.hasQServiceInfo = true;
                    }
                }
                // APIQA
                if(watchlistConsumerConfig.loginPauseAndResume())
                {
                	 updateCount++;
                     if (updateCount == 3) {
                         // Do a Login RESUME
                         LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                         loginRequest.flags(loginRequest.flags() &~ LoginRequestFlags.PAUSE_ALL);
                         loginRequest.flags(loginRequest.flags() | LoginRequestFlags.NO_REFRESH);
                         submitOptions.clear();
                         if ( (chnlInfo.reactorChannel.submit(loginRequest, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS) {
                             System.out.println("------------APIQA: attempted login reissue failed. Error: " + errorInfo.error().text());
                         } else {
                             System.out.println("------------APIQA: RESUME-ALL login reissue done.");
                         }
                     }
                }
                // End APIQA
                
                // APIQA
                e1Counter++;
                if (allEventCounters.size() > 0)
                {
                    for (int i = 0; i < allEventCounters.size(); ++i)
                    {
                        if (allEventCounters.get(i).delay() == e1Counter && allEventCounters.get(i).eventType() == 1)
                        {
                            int startIdx = allEventCounters.get(i).startIdx();
                            int endIdx = allEventCounters.get(i).endIdx();
                            for (int index = startIdx; index < endIdx; ++index)
                            {
                                boolean isSnap = watchlistConsumerConfig.getMPItemInfo(index, "isSnapshot");
                                boolean isView = watchlistConsumerConfig.getMPItemInfo(index, "isView");
                                boolean isPrivate = watchlistConsumerConfig.getMPItemInfo(index, "isPrivate");
                                boolean isMsgKeyInUpdates = watchlistConsumerConfig.getMPItemInfo(index, "isMsgKeyInUpdates");
                                int vID;
                                vID = watchlistConsumerConfig.getViewId(index);
                                String nameOfItem = watchlistConsumerConfig.getNameOfItem(index);
                                requestNewItem(reactor, event.reactorChannel(), false, isPrivate, isSnap, isView, vID, isMsgKeyInUpdates, nameOfItem);
                            }
                            allEventCounters.remove(i);
                            break;
                        }
                    }
                }
                // END APIQA

                break;
            case CLOSE:
                System.out.println("Received Source Directory Close");
                break;
            case STATUS:
                DirectoryStatus directoryStatus = (DirectoryStatus)event.rdmDirectoryMsg();
                System.out.println("Received Source Directory StatusMsg");
                System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
                System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.STATUS));
                System.out.println(directoryStatus.state().toString());
                if (directoryStatus.checkHasState())
                {
                    System.out.println("	" + directoryStatus.state());
                }
                break;
            default:
                System.out.println("Received Unhandled Source Directory Msg Type: " + msgType);
                break;
        }

        /* Refresh and update messages contain updates to service information. */
        if (serviceList != null)
        {
            for (Service service : serviceList)
            {
                System.out.println(" Service = " + service.serviceId() + " Action: " + MapEntryActions.toString(service.action()));

                if (chnlInfo.connectionArg.tunnel())
                {
                    tunnelStreamHandler.processServiceUpdate(chnlInfo.connectionArg.qService(), service);
                }

                if (queueMsgHandler != null)
                {
                    queueMsgHandler.processServiceUpdate(chnlInfo.connectionArg.qService(), service);
                }
            }
        }

        if (chnlInfo.connectionArg.tunnel())
        {
            if (!tunnelStreamHandler.isServiceFound())
            {
                System.out.println(" Directory response does not contain service name for tunnel streams: \n " + chnlInfo.connectionArg.qService());
            }
            else if (!tunnelStreamHandler.isServiceSupported())
            {
                System.out.println(" Service in use for tunnel streams does not support them: \n" + chnlInfo.connectionArg.qService());
            }
            else if (isRequestedQServiceUp(chnlInfo))
            {
                if (tunnelStreamHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED && chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                    {
                        uninitialize();
                        System.exit(ReactorReturnCodes.FAILURE);
                    }
                }
            }
        }

        if (queueMsgHandler != null)
        {
            if (!queueMsgHandler.isServiceFound())
            {
                System.out.println(" Directory response does not contain service name for queue messaging: \n " + chnlInfo.connectionArg.qService());
            }
            else if (!queueMsgHandler.isServiceSupported())
            {
                System.out.println(" Service in use for queue messaging does not support them: \n" + chnlInfo.connectionArg.qService());
            }
            else if (isRequestedQServiceUp(chnlInfo))
            {
                if (queueMsgHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED && chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                    {
                        uninitialize();
                        System.exit(ReactorReturnCodes.FAILURE);
                    }
                }
            }
        }

        System.out.println("");

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
        DictionaryMsgType msgType = event.rdmDictionaryMsg().rdmMsgType();

        // initialize dictionary
        if (chnlInfo.dictionary == null)
        {
            chnlInfo.dictionary = CodecFactory.createDataDictionary();
        }

        switch (msgType)
        {
            case REFRESH:
                DictionaryRefresh dictionaryRefresh = (DictionaryRefresh)event.rdmDictionaryMsg();

                if (dictionaryRefresh.checkHasInfo())
                {
                    /* The first part of a dictionary refresh should contain information about its type.
                     * Save this information and use it as subsequent parts arrive. */
                    switch (dictionaryRefresh.dictionaryType())
                    {
                        case Dictionary.Types.FIELD_DEFINITIONS:
                            fieldDictionaryLoaded = false;
                            chnlInfo.fieldDictionaryStreamId = dictionaryRefresh.streamId();
                            break;
                        case Dictionary.Types.ENUM_TABLES:
                            enumDictionaryLoaded = false;
                            chnlInfo.enumDictionaryStreamId = dictionaryRefresh.streamId();
                            break;
                        default:
                            System.out.println("Unknown dictionary type " + dictionaryRefresh.dictionaryType() + " from message on stream " + dictionaryRefresh.streamId());
                            chnlInfo.reactorChannel.close(errorInfo);
                            return ReactorCallbackReturnCodes.SUCCESS;
                    }
                }

                /* decode dictionary response */

                // clear decode iterator
                chnlInfo.dIter.clear();

                // set buffer and version info
                chnlInfo.dIter.setBufferAndRWFVersion(dictionaryRefresh.dataBody(), event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion());

                System.out.println("Received Dictionary Response: " + dictionaryRefresh.dictionaryName());

                if (dictionaryRefresh.streamId() == chnlInfo.fieldDictionaryStreamId)
                {
                    if (chnlInfo.dictionary.decodeFieldDictionary(chnlInfo.dIter, Dictionary.VerbosityValues.VERBOSE, error) == CodecReturnCodes.SUCCESS)
                    {
                        if (dictionaryRefresh.checkRefreshComplete())
                        {
                            fieldDictionaryLoaded = true;
                            itemDecoder.fieldDictionaryDownloadedFromNetwork = true;
                            itemDecoder.dictionary = chnlInfo.dictionary;
                            System.out.println("Field Dictionary complete.");
                        }
                    }
                    else
                    {
                        System.out.println("Decoding Field Dictionary failed: " + error.text());
                        chnlInfo.reactorChannel.close(errorInfo);
                    }
                }
                else if (dictionaryRefresh.streamId() == chnlInfo.enumDictionaryStreamId)
                {
                    if (chnlInfo.dictionary.decodeEnumTypeDictionary(chnlInfo.dIter, Dictionary.VerbosityValues.VERBOSE, error) == CodecReturnCodes.SUCCESS)
                    {
                        if (dictionaryRefresh.checkRefreshComplete())
                        {
                            enumDictionaryLoaded = true;
                            itemDecoder.enumTypeDictionaryDownloadedFromNetwork = true;
                            itemDecoder.dictionary = chnlInfo.dictionary;
                            System.out.println("EnumType Dictionary complete.");
                        }
                    }
                    else
                    {
                        System.out.println("Decoding EnumType Dictionary failed: " + error.text());
                        chnlInfo.reactorChannel.close(errorInfo);
                    }
                }
                else
                {
                    System.out.println("Received unexpected dictionary message on stream " + dictionaryRefresh.streamId());
                }

                if (fieldDictionaryLoaded && enumDictionaryLoaded)
                    requestItems(chnlInfo.reactorChannel.reactor(), chnlInfo.reactorChannel);

                break;
            case STATUS:
                DictionaryStatus dictionaryStatus = (DictionaryStatus)event.rdmDictionaryMsg();

                if (dictionaryStatus.streamId() == chnlInfo.fieldDictionaryStreamId)
                {
                    System.out.println("Received Dictionary StatusMsg for RWFFld, streamId: " + chnlInfo.fieldDictionaryStreamId);
                }
                else if (dictionaryStatus.streamId() == chnlInfo.enumDictionaryStreamId)
                {
                    System.out.println("Received Dictionary StatusMsg for RWFEnum, streamId: " + chnlInfo.enumDictionaryStreamId);
                }
                if (dictionaryStatus.checkHasState())
                {
                    System.out.println(dictionaryStatus.state());
                }
                break;
            default:
                System.out.println("Received Unhandled Dictionary Msg Type: " + msgType);
                break;
        }

        System.out.println("");

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    public boolean isRequestedServiceUp(ChannelInfo chnlInfo)
    {
        return chnlInfo.hasServiceInfo && chnlInfo.serviceInfo.checkHasState() && (!chnlInfo.serviceInfo.state().checkHasAcceptingRequests() || chnlInfo.serviceInfo.state().acceptingRequests() == 1)
                && chnlInfo.serviceInfo.state().serviceState() == 1;
    }

    public boolean isRequestedQServiceUp(ChannelInfo chnlInfo)
    {
        return chnlInfo.hasQServiceInfo && chnlInfo.qServiceInfo.checkHasState()
                && (!chnlInfo.qServiceInfo.state().checkHasAcceptingRequests() || chnlInfo.qServiceInfo.state().acceptingRequests() == 1) && chnlInfo.qServiceInfo.state().serviceState() == 1;
    }

    private void checkAndInitPostingSupport(ChannelInfo chnlInfo)
    {
        if (!(chnlInfo.shouldOnStreamPost || chnlInfo.shouldOffStreamPost))
            return;

        // set up posting if its enabled

        // ensure that provider supports posting - if not, disable posting
        if (!chnlInfo.loginRefresh.checkHasFeatures() || !chnlInfo.loginRefresh.features().checkHasSupportPost() || chnlInfo.loginRefresh.features().supportOMMPost() == 0)
        {
            // provider does not support posting, disable it
            chnlInfo.shouldOffStreamPost = false;
            chnlInfo.shouldOnStreamPost = false;
            chnlInfo.postHandler.enableOnstreamPost(false);
            chnlInfo.postHandler.enableOffstreamPost(false);
            System.out.println("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
            return;
        }

        if (watchlistConsumerConfig.publisherId() != null && watchlistConsumerConfig.publisherAddress() != null)
            chnlInfo.postHandler.setPublisherInfo(watchlistConsumerConfig.publisherId(), watchlistConsumerConfig.publisherAddress());
    }

    // on and off stream posting if enabled
    private void handlePosting()
    {
        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            if (chnlInfo.loginRefresh == null || chnlInfo.serviceInfo == null || chnlInfo.reactorChannel == null || chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
            {
                continue;
            }

            chnlInfo.postItemName.clear();

            if (chnlInfo.postHandler.enableOnstreamPost())
            {
                ItemInfo postingItem = null;

                // Find a first MarketPrice item
                // If found, send on-stream posts on it.
                for (int i = 0; i < watchlistConsumerConfig.itemCount(); i++)
                {
                    if (watchlistConsumerConfig.itemList().get(i).domain() == DomainTypes.MARKET_PRICE)
                    {
                        postingItem = watchlistConsumerConfig.itemList().get(i);
                        if (watchlistConsumerConfig.itemList().get(i).state().streamState() != StreamStates.OPEN || watchlistConsumerConfig.itemList().get(i).state().dataState() != DataStates.OK)
                        {
                            System.out.println("No currently available Market Price streams to on-stream post to.  Will retry shortly.");
                            return;
                        }
                        break;
                    }

                }

                if (postingItem == null)
                {
                    System.out.println("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");
                    return;
                }

                chnlInfo.postHandler.streamId(postingItem.streamId());
                chnlInfo.postHandler.postItemName().data(postingItem.name());
                chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
                chnlInfo.postHandler.dictionary(chnlInfo.dictionary);

                int ret = chnlInfo.postHandler.handlePosts(chnlInfo.reactorChannel, errorInfo);
                if (ret < CodecReturnCodes.SUCCESS)
                    System.out.println("Error posting onstream: " + errorInfo.error().text());
            }
            if (chnlInfo.postHandler.enableOffstreamPost())
            {
                chnlInfo.postHandler.streamId(chnlInfo.loginRefresh.streamId());
                chnlInfo.postHandler.postItemName().data("OFFPOST");
                chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
                chnlInfo.postHandler.dictionary(chnlInfo.dictionary);
                int ret = chnlInfo.postHandler.handlePosts(chnlInfo.reactorChannel, errorInfo);
                if (ret < CodecReturnCodes.SUCCESS)
                    System.out.println("Error posting offstream: " + errorInfo.error().text());
            }
        }
    }

    private void handleQueueMessaging()
    {
        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            if (chnlInfo.loginRefresh == null || chnlInfo.serviceInfo == null || chnlInfo.reactorChannel == null || chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
            {
                continue;
            }

            if (queueMsgHandler != null)
            {
                queueMsgHandler.sendQueueMsg(chnlInfo.reactorChannel);
            }
        }
    }

    private void handleTunnelStream()
    {
        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            if (chnlInfo.loginRefresh == null || chnlInfo.serviceInfo == null || chnlInfo.reactorChannel == null || chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
            {
                continue;
            }

            if (tunnelStreamHandler != null)
            {
                tunnelStreamHandler.sendMsg(chnlInfo.reactorChannel);
            }
        }
    }

    private void initChannelInfo(ChannelInfo chnlInfo)
    {
        // set up consumer role
        chnlInfo.consumerRole.defaultMsgCallback(this);
        chnlInfo.consumerRole.channelEventCallback(this);
        chnlInfo.consumerRole.loginMsgCallback(this);
        chnlInfo.consumerRole.directoryMsgCallback(this);
        chnlInfo.consumerRole.watchlistOptions().enableWatchlist(true);
        chnlInfo.consumerRole.watchlistOptions().itemCountHint(4);
        chnlInfo.consumerRole.watchlistOptions().maxOutstandingPosts(5);
        chnlInfo.consumerRole.watchlistOptions().obeyOpenWindow(true);
        chnlInfo.consumerRole.watchlistOptions().channelOpenCallback(this);
        
        if (itemDecoder.fieldDictionaryLoadedFromFile == false && itemDecoder.enumTypeDictionaryLoadedFromFile == false)
        {
            chnlInfo.consumerRole.dictionaryMsgCallback(this);
        }

        // initialize consumer role to default
        chnlInfo.consumerRole.initDefaultRDMLoginRequest();
        chnlInfo.consumerRole.initDefaultRDMDirectoryRequest();

        // use command line login user name if specified
        if (watchlistConsumerConfig.userName() != null && !watchlistConsumerConfig.userName().equals(""))
        {
            chnlInfo.consumerRole.rdmLoginRequest().userName().data(watchlistConsumerConfig.userName());
        }

        // use command line authentication token and extended authentication
        // information if specified
        if (watchlistConsumerConfig.authenticationToken() != null && !watchlistConsumerConfig.authenticationToken().equals(""))
        {
            chnlInfo.consumerRole.rdmLoginRequest().userNameType(Login.UserIdTypes.AUTHN_TOKEN);
            chnlInfo.consumerRole.rdmLoginRequest().userName().data(watchlistConsumerConfig.authenticationToken());

            if (watchlistConsumerConfig.authenticationExtended() != null && !watchlistConsumerConfig.authenticationExtended().equals(""))
            {
                chnlInfo.consumerRole.rdmLoginRequest().applyHasAuthenticationExtended();
                chnlInfo.consumerRole.rdmLoginRequest().authenticationExtended().data(watchlistConsumerConfig.authenticationExtended());
            }
        }

        // use command line application id if specified
        if (watchlistConsumerConfig.applicationId() != null && !watchlistConsumerConfig.applicationId().equals(""))
        {
            chnlInfo.consumerRole.rdmLoginRequest().attrib().applicationId().data(watchlistConsumerConfig.applicationId());
        }

        chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
        // APIQA
        int singleOpenArg = watchlistConsumerConfig.isSingleOpen();
        System.out.println("\nAPIQA: SingleOpen set to " + singleOpenArg);
        chnlInfo.consumerRole.rdmLoginRequest().attrib().singleOpen(singleOpenArg);
        int allowSuspectArg = watchlistConsumerConfig.allowSuspectData();
        chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
        System.out.println("APIQA: Allow Suspect Data set to " + allowSuspectArg);
        chnlInfo.consumerRole.rdmLoginRequest().attrib().allowSuspectData(allowSuspectArg);
        // END APIQA
        // APIQA
        if(watchlistConsumerConfig.reqDirWithSID() != 0)
        {
        	 chnlInfo.consumerRole.rdmDirectoryRequest().applyHasServiceId();
             chnlInfo.consumerRole.rdmDirectoryRequest().serviceId(watchlistConsumerConfig.reqDirWithSID());
        }
        // End APIQA
        if (itemDecoder.fieldDictionaryLoadedFromFile == true && itemDecoder.enumTypeDictionaryLoadedFromFile == true)
        {
            chnlInfo.dictionary = itemDecoder.getDictionary();
        }
        chnlInfo.shouldOffStreamPost = watchlistConsumerConfig.enableOffpost();
        chnlInfo.shouldOnStreamPost = watchlistConsumerConfig.enablePost();
        
        if (chnlInfo.shouldOnStreamPost)
        {
            boolean mpItemFound = false;
            if (chnlInfo.connectionArg.itemList() != null)
            {
                for (ItemArg itemArg : chnlInfo.connectionArg.itemList())
                {
                    if (itemArg.domain() == DomainTypes.MARKET_PRICE)
                    {
                        mpItemFound = true;
                        break;
                    }
                }
            }
            if (mpItemFound == false)
            {
                System.out.println("\nPosting will not be performed for this channel as no Market Price items were requested");
                chnlInfo.shouldOnStreamPost = false;
            }
        }

        chnlInfo.postHandler.enableOnstreamPost(chnlInfo.shouldOnStreamPost);
        chnlInfo.postHandler.enableOffstreamPost(chnlInfo.shouldOffStreamPost);
        chnlInfo.postHandler.enablePostMultipart(watchlistConsumerConfig.enablePostMultipart());

        // This sets up our basic timing so post messages will be sent
        // periodically
        chnlInfo.postHandler.initPostHandler();

        // set up reactor connect options
        chnlInfo.connectOptions.reconnectAttemptLimit(-1); // attempt to recover
                                                           // forever
        chnlInfo.connectOptions.reconnectMinDelay(500); // 0.5 second minimum
        chnlInfo.connectOptions.reconnectMaxDelay(3000); // 3 second maximum
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(chnlInfo.connectionArg.connectionType());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(chnlInfo.connectionArg.port());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(chnlInfo.connectionArg.hostname());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().interfaceName(chnlInfo.connectionArg.interfaceName());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().guaranteedOutputBuffers(1000);

        // handler encrypted or http connection
        chnlInfo.shouldEnableEncrypted = watchlistConsumerConfig.enableEncrypted();
        chnlInfo.shouldEnableHttp = watchlistConsumerConfig.enableHttp();

        if (chnlInfo.shouldEnableEncrypted)
        {
            ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
            cOpt.connectionType(ConnectionTypes.ENCRYPTED);
            cOpt.tunnelingInfo().tunnelingType("encrypted");
            setEncryptedConfiguration(cOpt);
        }
        else if (chnlInfo.shouldEnableHttp)
        {
            ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
            cOpt.connectionType(ConnectionTypes.HTTP);
            cOpt.tunnelingInfo().tunnelingType("http");
            setHTTPConfiguration(cOpt);
        }

        // exit program if both queue messaging and tunnel stream are configured
        if (chnlInfo.connectionArg.tunnel() && chnlInfo.connectionArg.qSource() != null && !chnlInfo.connectionArg.qSource().equals(""))
        {
            System.out.println("\nError: Cannot run with both tunnel stream messaging and queue messaging enabled." + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }

        // handle queue messaging configuration
        if (chnlInfo.connectionArg.qSource() != null && !chnlInfo.connectionArg.qSource().equals("") && queueMsgHandler == null)
        {
            if (chnlInfo.connectionArg.qDestList().size() <= MAX_QUEUE_DESTINATIONS)
            {
                loadFixDictionary();
                qServiceName = chnlInfo.connectionArg.qService();
                queueMsgHandler = new QueueMsgHandler(chnlInfo.connectionArg.qSource(), chnlInfo.connectionArg.qDestList(), fixdictionary, chnlInfo.connectionArg.tunnelAuth(),
                        chnlInfo.connectionArg.tunnelDomain());
            }
            else
            // exit if too many queue destinations entered
            {
                System.err.println("\nError: Example only supports " + MAX_QUEUE_DESTINATIONS + " queue destination names.\n");
                System.err.println(CommandLine.optionHelpString());
                System.exit(CodecReturnCodes.FAILURE);
            }
        }

        // handle basic tunnel stream configuration
        if (chnlInfo.connectionArg.tunnel() && tunnelStreamHandler == null)
        {
            qServiceName = chnlInfo.connectionArg.qService();
            tunnelStreamHandler = new TunnelStreamHandler(chnlInfo.connectionArg.tunnelAuth(), chnlInfo.connectionArg.tunnelDomain());
        }
    }

    // load FIX dictionary to support FIX Protocol
    public void loadFixDictionary()
    {
        fixdictionary.clear();
        if (fixdictionary.loadFieldDictionary(FIX_FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("\nUnable to load FIX field dictionary. \n\tText: " + error.text() + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }

        if (fixdictionary.loadEnumTypeDictionary(FIX_ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("\nUnable to load FIX enum dictionary. \n\tText: " + error.text() + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }
    }
    
     // APIQA:
    private void closeLoginStream(ChannelInfo chnlInfo) {
               /* encode item close */
               submitOptions.clear();
               closeMsg.clear();
               closeMsg.msgClass(MsgClasses.CLOSE);
               closeMsg.streamId(1);
               closeMsg.domainType(DomainTypes.LOGIN);
               closeMsg.containerType(DataTypes.NO_DATA);

               if ( (chnlInfo.reactorChannel.submit(closeMsg, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS)
               {
                       System.out.println("\n\n----------------APIQA: Closing login stream Failed.\n");
               } else
                       System.out.println("\n\n----------------APIQA: Closd LOGIN stream.\n");
     }
       // END APIQA:

    private void closeItemStreams(ChannelInfo chnlInfo)
    {
        // have offstream posting post close status
        if (chnlInfo.shouldOffStreamPost)
        {
            chnlInfo.postHandler.streamId(chnlInfo.loginRefresh.streamId());
            chnlInfo.postHandler.postItemName().data("OFFPOST");
            chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
            chnlInfo.postHandler.dictionary(chnlInfo.dictionary);
            chnlInfo.postHandler.closeOffStreamPost(chnlInfo.reactorChannel, errorInfo);
        }

        for (int itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemCount(); ++itemListIndex)
        {
            int domainType = watchlistConsumerConfig.itemList().get(itemListIndex).domain();

            int streamId = watchlistConsumerConfig.itemList().get(itemListIndex).streamId;

            /* encode item close */
            closeMsg.clear();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.streamId(streamId);
            closeMsg.domainType(domainType);
            closeMsg.containerType(DataTypes.NO_DATA);

            if ((chnlInfo.reactorChannel.submit(closeMsg, submitOptions, errorInfo)) != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Close itemStream of " + streamId + " Failed.");
            }
        }
    }

    /* Uninitializes the Value Add consumer application. */
    private void uninitialize()
    {
        System.out.println("Consumer unitializing and exiting...");

        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            // close items streams
            closeItemStreams(chnlInfo);

            // close queue messaging streams
            if (queueMsgHandler != null && chnlInfo.reactorChannel != null)
            {
                if (queueMsgHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("queueMsgHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }

            // close tunnel streams
            if (tunnelStreamHandler != null && chnlInfo.reactorChannel != null)
            {
                if (tunnelStreamHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("tunnelStreamHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }

            // close ReactorChannel
            if (chnlInfo.reactorChannel != null)
            {
                chnlInfo.reactorChannel.close(errorInfo);
            }
        }

        // shutdown reactor
        if (reactor != null)
        {
            reactor.shutdown(errorInfo);
        }
    }

    private void handleClose()
    {
        System.out.println("Consumer closes streams...");

        for (ChannelInfo chnlInfo : chnlInfoList)
        {
        	//APIQA
        	if (watchlistConsumerConfig.testOnlyLoginClose()) {
        		closeLoginStream(chnlInfo);
        		System.out.println("-----------APIQA: Closing only consumer login stream when run time expires\n\n");      		
        	}
        	// End APIQA
            closeItemStreams(chnlInfo);

            // close queue messaging streams
            if (queueMsgHandler != null && chnlInfo.reactorChannel != null)
            {
                if (queueMsgHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("queueMsgHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }

            // close tunnel streams
            if (tunnelStreamHandler != null && chnlInfo.reactorChannel != null)
            {
                if (tunnelStreamHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("tunnelStreamHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }
        }
    }

    private void setEncryptedConfiguration(ConnectOptions options)
    {
        setHTTPConfiguration(options);

        String keyFile = watchlistConsumerConfig.keyStoreFile();
        String keyPasswd = watchlistConsumerConfig.keystorePassword();
        if (keyFile == null)
        {
            System.err.println("Error: Keystore file not provided.");
            System.exit(CodecReturnCodes.FAILURE);
        }
        if (keyPasswd == null)
        {
            System.err.println("Error: Keystore password not provided.");
            System.exit(CodecReturnCodes.FAILURE);
        }

        options.tunnelingInfo().KeystoreFile(keyFile);
        options.tunnelingInfo().KeystorePasswd(keyPasswd);
    }

    private void setHTTPConfiguration(ConnectOptions options)
    {
        options.tunnelingInfo().objectName("");
        options.tunnelingInfo().KeystoreType("JKS");
        options.tunnelingInfo().SecurityProtocol("TLS");
        options.tunnelingInfo().SecurityProvider("SunJSSE");
        options.tunnelingInfo().KeyManagerAlgorithm("SunX509");
        options.tunnelingInfo().TrustManagerAlgorithm("PKIX");

        if (watchlistConsumerConfig.enableProxy())
        {
            String proxyHostName = watchlistConsumerConfig.proxyHostname();
            if (proxyHostName == null)
            {
                System.err.println("Error: Proxy hostname not provided.");
                System.exit(CodecReturnCodes.FAILURE);
            }
            String proxyPort = watchlistConsumerConfig.proxyPort();
            if (proxyPort == null)
            {
                System.err.println("Error: Proxy port number not provided.");
                System.exit(CodecReturnCodes.FAILURE);
            }

            options.tunnelingInfo().HTTPproxy(true);
            options.tunnelingInfo().HTTPproxyHostName(proxyHostName);
            try
            {
                options.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort));
            }
            catch (Exception e)
            {
                System.err.println("Error: Proxy port number not provided.");
                System.exit(CodecReturnCodes.FAILURE);
            }
        }

        // credentials
        if (options.tunnelingInfo().HTTPproxy())
        {
            setCredentials(options);
        }
    }

    /*
     * For BASIC authentication we need: HTTPproxyUsername, HTTPproxyPasswd For
     * NTLM authentication we need: HTTPproxyUsername, HTTPproxyPasswd,
     * HTTPproxyDomain, HTTPproxyLocalHostname For Negotiate/Kerberos we need:
     * HTTPproxyUsername, HTTPproxyPasswd, HTTPproxyDomain,
     * HTTPproxyKRB5configFile
     */
    private void setCredentials(ConnectOptions options)
    {
        String localIPaddress = null;
        String localHostName = null;

        String proxyUsername = watchlistConsumerConfig.proxyUsername();
        if (proxyUsername == null)
        {
            System.err.println("Error: Proxy username not provided.");
            System.exit(CodecReturnCodes.FAILURE);
        }

        String proxyPasswd = watchlistConsumerConfig.proxyPassword();
        if (proxyPasswd == null)
        {
            System.err.println("Error: Proxy password not provided.");
            System.exit(CodecReturnCodes.FAILURE);
        }
        String proxyDomain = watchlistConsumerConfig.proxyDomain();
        if (proxyDomain == null)
        {
            System.err.println("Error: Proxy domain not provided.");
            System.exit(CodecReturnCodes.FAILURE);
        }

        options.credentialsInfo().HTTPproxyUsername(proxyUsername);
        options.credentialsInfo().HTTPproxyPasswd(proxyPasswd);
        options.credentialsInfo().HTTPproxyDomain(proxyDomain);

        try
        {
            localIPaddress = InetAddress.getLocalHost().getHostAddress();
            localHostName = InetAddress.getLocalHost().getHostName();
        }
        catch (UnknownHostException e)
        {
            localHostName = localIPaddress;
        }
        options.credentialsInfo().HTTPproxyLocalHostname(localHostName);

        String proxyKrbfile = watchlistConsumerConfig.krbFile();
        if (proxyKrbfile == null)
        {
            System.err.println("Error: Proxy krbfile not provided.");
            System.exit(CodecReturnCodes.FAILURE);
        }
        options.credentialsInfo().HTTPproxyKRB5configFile(proxyKrbfile);

    }

    String mapServiceActionString(int action)
    {
        if (action == MapEntryActions.DELETE)
            return "DELETE";
        else if (action == MapEntryActions.ADD)
            return "ADD";
        else if (action == MapEntryActions.UPDATE)
            return "UPDATE";
        else
            return null;
    }

    public static void main(String[] args) throws Exception
    {
        WatchlistConsumer consumer = new WatchlistConsumer();
        consumer.init(args);
        consumer.run();
        consumer.uninitialize();
        System.exit(0);
    }
}
