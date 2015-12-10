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
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.CommandLine;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues;
import com.thomsonreuters.upa.rdm.DomainTypes;
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
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;
import com.thomsonreuters.upa.valueadd.examples.common.ConnectionArg;
import com.thomsonreuters.upa.valueadd.examples.common.ItemArg;
import com.thomsonreuters.upa.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig.ItemInfo;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerCallback;
import com.thomsonreuters.upa.valueadd.reactor.DictionaryDownloadModes;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectInfo;
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
 * is not intended to be used for measuring performance. This application uses
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
 * Change directory to the <i>ValueAdd/Examples</i> directory and run <i>ant</i> to
 * build.
 * </p>
 * <p>
 *  java -cp ./bin;../../Libs/upa.jar;../Libs/upaValueAdd.jar;../Libs/upaValueAddCache.jar
 * com.thomsonreuters.upa.valueadd.examples.watchlistconsumer.WatchlistConsumer [-h srvrHostName] [-p
 * srvrPortNo] [-uname loginUserName] [-s serviceName] [-mp|-mpps
 * marketPriceItemName] [-mbo|-mbops marketByOrderItemName] [-mbp|-mbpps
 * marketByPriceItemName] [-yc|-ycps yieldCurveItemName] [-view viewFlag]
 * [-post postFlag] [-offpost offPostFlag] [-publisherInfo id, ipaddress] [-snapshot snapShotFlag]
 * [-sl symbolListName] [-x] [-connectionType connectionType]
 * [-runtime runTime]  [-proxy true or false] [-ph proxy server hostName]
 * [-pp proxy port number] [-plogin proxy userName] [-ppasswd proxy password]
 * [-pdomain proxy domain] [-krbfile proxyKRBFile] [-keyfile keystoreFile] [-keypasswd keystore password]
 *   
 * <p>
 * <ul>
 * <li>-h Server host name. Default is <i>localhost</i>.
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-uname Login user name. Default is system user name.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-mp Market Price domain item name. Default is <i>TRI.N</i>. The user can
 * specify multiple -mp instances, where each occurrence is associated with a
 * single item. For example, specifying -mp TRI -mp GOOG will provide content
 * for two MarketPrice items.
 * <li>-mpps Market Price domain item name on private stream. No default. The
 * user can specify multiple -mpps instances, where each occurrence is
 * associated with a single item.
 * <li>-mbo Market By Order domain item name. No default. The user can specify
 * multiple -mbo instances, where each occurrence is associated with a single
 * item.
 * <li>-mbops Market By Order domain item name on private stream. No default.
 * The user can specify multiple -mbops instances, where each occurrence is
 * associated with a single item.
 * <li>-mbp market By Price domain item name. No default. The user can specify
 * multiple -mbp instances, where each occurrence is associated with a single
 * item.
 * <li>-mbpps market By Price domain item name on private stream. No default.
 * The user can specify multiple -mbpps instances, where each occurrence is
 * associated with a single item.
 * <li>-yc Yield Curve domain item name. No default. The user can specify
 * multiple -yc instances, where each occurrence is associated with a
 * single item.
 * <li>-ycps Yield Curve domain item name on private stream. No default. The
 * user can specify multiple -ycps instances, where each occurrence is
 * associated with a single item.
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
 * is specified without a symbolListName the itemList from the source directory
 * response will be used.
 * <li>-x Provides XML tracing of messages.
 * <li>-connectionType. Connection Type used (Socket, http, or encrypted).
 * Default is <i>Socket</i>.
 * <li>-runtime run time. Default is 600 seconds. Controls the time the
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
 * <li>-uname changes the username used when logging into the provider
 *
 * <li>-post specifies that the application should attempt to send post messages on the first requested Market Price item
 *
 * <li>-offpost specifies that the application should attempt to send post messages on the login stream (i.e., off-stream)
 *
 * <li>-publisherInfo specifies that the application provides its own user id and address
 * 
 * <li>-snapshot specifies each request using non-streaming
 *
 * <li>-x provides an XML trace of messages
 * 
 * <li>-runtime adjusts the running time of the application
 * 
 * <li>-cache will store all open items in cache and periodically dump contents
 *
 * <li>-cacheInterval number of seconds between displaying cache contents, must greater than 0
 *
 * <li>-proxy proxyFlag. if provided, the application will attempt
 * to make an http or encrypted connection through a proxy server (if
 * connectionType is set to http or encrypted).
 *
 * <li>-ph Proxy host name.
 * 
 * <li>-pp Proxy port number.
 * 
 * <li>-plogin User name on proxy server.
 * 
 * <li>-ppasswd Password on proxy server. 
 * 
 * <li>-pdomain Proxy Domain.
 * 
 * <li>-krbfile Proxy KRB file. 
 * 
 * <li>-keyfile keystore file for encryption.
 * 
 * <li>-keypasswd keystore password for encryption.
 * 
 * </ul>
 * </p>
 */
public class WatchlistConsumer implements ConsumerCallback
{
    private final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private final String ENUM_TABLE_FILE_NAME = "enumtype.def";
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
    private Error error;    // error information
    
    private DataDictionary fixdictionary;
    
    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
    
	ArrayList<ChannelInfo> chnlInfoList = new ArrayList<ChannelInfo>();

    private QueueMsgHandler queueMsgHandler;
    private TunnelStreamHandler tunnelStreamHandler;
   
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
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    
	public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    private final List<Integer> viewFieldList;
    private List<Service> serviceList;
    boolean fieldDictionaryLoaded;
    boolean enumDictionaryLoaded;
    ArrayList<Integer> postItemsIndex = new ArrayList<Integer>();
    int currentPostIndex = 0;
    State postingItemState; 
    
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
        postItemsIndex.add(0);
 
        viewFieldList = new ArrayList<Integer>();
                
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

    
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.out.println("Consumer exits...");
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
			reactor.reactorChannel().selectableChannel().register(selector,
																SelectionKey.OP_READ,
																reactor.reactorChannel());
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
							// retrieve associated reactor channel and dispatch on that channel 
							ReactorChannel reactorChnl = (ReactorChannel)key.attachment();

							// dispatch until no more messages
							while ((ret = reactorChnl.dispatch(dispatchOptions, errorInfo)) > 0) {}
							if (ret == ReactorReturnCodes.FAILURE)
							{
								if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
										reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
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
            else if (System.currentTimeMillis() >= closeRunTime )
            {
            	 System.out.println("Consumer closetime expired, shutdown reactor.");
            	 break;
            }
	        if (!closeHandled)
	        {
	        	handlePosting();
	        	handleQueueMessaging();
	        	handleTunnelStream();
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

	
	/* Requests the desired items.  */
	int requestItems(Reactor reactor, ReactorChannel channel)
	{
		if (itemsRequested)
			return ReactorReturnCodes.SUCCESS;

		itemsRequested = true;

		for(int itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemCount(); ++itemListIndex)
		{
		    int ret = CodecReturnCodes.SUCCESS;
		    
			itemRequest.clear();	       
			itemRequest.applyStreaming();

			itemRequest.addItem(watchlistConsumerConfig.itemList().get(itemListIndex).name);  
		        
			int domainType = watchlistConsumerConfig.itemList().get(itemListIndex).domain();
		        
			itemRequest.domainType(domainType);
			itemRequest.streamId(watchlistConsumerConfig.itemList().get(itemListIndex).streamId);
				
			if (domainType == DomainTypes.SYMBOL_LIST && watchlistConsumerConfig.itemList().get(itemListIndex).symbolListData)
			{
				itemRequest.requestMsg.containerType(DataTypes.ELEMENT_LIST);
			    payload = CodecFactory.createBuffer(); 
			    payload.data(ByteBuffer.allocate(1024));

				if (payload == null)
				{
					return CodecReturnCodes.FAILURE;
				}
				
				encIter.clear();
				encIter.setBufferAndRWFVersion(payload, channel.majorVersion(), channel.minorVersion());
				ret = itemRequest.encodeSymbollistDataStreamPayload(encIter);    
				
				if (ret < CodecReturnCodes.SUCCESS)
				{
					errorInfo.error().text("RequestItem.encode() failed");
					errorInfo.error().errorId(ret);
					return ret;
				}			
				
				itemRequest.requestMsg.encodedDataBody(payload);
			}
			else if (domainType == DomainTypes.MARKET_PRICE && watchlistConsumerConfig.enableView())
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
				viewFieldList.add(22);
				viewFieldList.add(25);
				viewFieldList.add(30);
				viewFieldList.add(31);
				viewFieldList.add(1025);
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
		
			System.out.println(itemRequest.toString());
			
			submitOptions.clear();
			if (watchlistConsumerConfig.serviceName() != null)
			{
			    submitOptions.serviceName(watchlistConsumerConfig.serviceName());
			}
			else
			{
			    itemRequest.requestMsg.msgKey().applyHasServiceId();
			    itemRequest.requestMsg.msgKey().serviceId(itemRequest.serviceId());			    
			}

			ret = channel.submit(itemRequest.requestMsg, submitOptions, errorInfo);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("\nReactorChannel.submit() failed: " + ret + "(" + errorInfo.error().text() + ")\n");
			}
		}
		return CodecReturnCodes.SUCCESS;
	}	
		    
	
	private void requestDictionaries(ReactorChannel channel)
	{
		RequestMsg msg = (RequestMsg) CodecFactory.createMsg();

		/* set-up message */
		msg.msgClass(MsgClasses.REQUEST);
		
		msg.applyStreaming();
		msg.streamId(FIELD_DICTIONARY_STREAM_ID);  
		msg.domainType(DomainTypes.DICTIONARY);
		msg.containerType(DataTypes.NO_DATA);
		msg.msgKey().applyHasNameType();
		msg.msgKey().applyHasName();
		msg.msgKey().applyHasFilter();		
		msg.msgKey().filter(VerbosityValues.NORMAL);		
		msg.msgKey().name().data(FIELD_DICTIONARY_FILE_NAME);

		ReactorSubmitOptions submitOptions = ReactorFactory
				.createReactorSubmitOptions();

		submitOptions.serviceName(watchlistConsumerConfig.serviceName());
		
        channel.submit(msg, submitOptions, errorInfo);
         
 		msg.streamId(ENUM_DICTIONARY_STREAM_ID); 
 		msg.msgKey().name().data(ENUM_TABLE_FILE_NAME);
 		
        channel.submit(msg, submitOptions, errorInfo);
		
	}

    @Override
	public int reactorChannelEventCallback(ReactorChannelEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		
		switch(event.eventType())
		{				
    		case ReactorChannelEventTypes.CHANNEL_UP:
    		{
    	        // register selector with channel event's reactorChannel
    	        try
    	        {
    				event.reactorChannel().selectableChannel().register(selector,
    																	SelectionKey.OP_READ,
    																	event.reactorChannel());
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
    	        System.out.println("Channel Change - Old Channel: "
    	                + event.reactorChannel().oldSelectableChannel() + " New Channel: "
    	                + event.reactorChannel().selectableChannel());
    	        
    	        // cancel old reactorChannel select
    	        try
    	        {
    	            SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
    	            key.cancel();
    	        }
    	        catch (Exception e)
    	        {
    	        } // old channel may be null so ignore
    
    	        // register selector with channel event's new reactorChannel
    	        try
    	        {
    	        	event.reactorChannel().selectableChannel().register(selector,
    	        													SelectionKey.OP_READ,
    	        													event.reactorChannel());
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
       			if (isRequestedServiceUp(chnlInfo))
    			{
        			checkAndInitPostingSupport(chnlInfo); 
    			}
    			
    			if (isRequestedQServiceUp(chnlInfo))
    			{
                    if (queueMsgHandler != null)
                    {
                        if (queueMsgHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                        {
                            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                            {
                                uninitialize();
                                System.exit(ReactorReturnCodes.FAILURE);
                            }
                        }
                    }
                    
                    if (tunnelStreamHandler != null)
                    {
                        if (tunnelStreamHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                        {
                            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                            {
                                uninitialize();
                                System.exit(ReactorReturnCodes.FAILURE);
                            }
                        }
                    }
    			}
                break;    			
    		
    		}
    		case ReactorChannelEventTypes.CHANNEL_OPENED:
    		{
    			// set ReactorChannel on ChannelInfo, again need this?
    			chnlInfo.reactorChannel = event.reactorChannel();

				if (fieldDictionaryLoaded && enumDictionaryLoaded
						 || itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile )
					requestItems(reactor, event.reactorChannel());
				else
					requestDictionaries(event.reactorChannel());
    			
                break;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
    		{
    			if (chnlInfo.reactorChannel != null)
    				System.out.println("\nConnection down reconnecting: Channel " + chnlInfo.reactorChannel.selectableChannel());
    			else
    				System.out.println("\nConnection down reconnecting");
    
    			if (event.errorInfo() != null && event.errorInfo().error().text() != null)
    				System.out.println("	Error text: " + event.errorInfo().error().text() + "\n");
    						    			
    			// unregister selectableChannel from Selector
                try
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    key.cancel();
                }
                catch (Exception e) { } // channel may be null so ignore
    			
    			// reset dictionary if not loaded from file
    	        if (itemDecoder.fieldDictionaryLoadedFromFile == false &&
    	            itemDecoder.enumTypeDictionaryLoadedFromFile == false)
    	        {
    	        	if (chnlInfo.dictionary != null)
    	        	{
    	        		chnlInfo.dictionary.clear();
    	        	}
    	        }
    	            	        
    	        itemsRequested = false;
    	        postingItemState = null;
    	        chnlInfo.hasServiceInfo = false;
    	        chnlInfo.hasQServiceInfo = false;
    	        
    	        // private stream not recovered anyway 
    	        // close queue messaging streams
                if (queueMsgHandler != null &&
                    chnlInfo.reactorChannel != null)
                {
                    if (queueMsgHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                    {
                        System.out.println("queueMsgHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                    }
                }

                // close tunnel streams
                if (tunnelStreamHandler != null &&
                    chnlInfo.reactorChannel != null)
                {
                    if (tunnelStreamHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                    {
                        System.out.println("tunnelStreamHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                    }
                }
                break;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
                if (chnlInfo.reactorChannel != null)
                    System.out.println("\nConnection down: Channel " + chnlInfo.reactorChannel.selectableChannel());
                else
                    System.out.println("\nConnection down");
    
                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");
                
                // unregister selectableChannel from Selector
                try
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    key.cancel();
                }
                catch (Exception e) { } // channel may be null so ignore

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
            try
            {
                SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                key.cancel();
            }
            catch (Exception e) { } // channel may be null so ignore

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
                if ( refreshMsg.checkHasMsgKey())
                {
                	if (refreshMsg.msgKey().checkHasName())
                	{
                		itemName = refreshMsg.msgKey().name().toString(); // Buffer
                	    if ( item != null && refreshMsg.streamId() < 0)
                	    {
                	    	watchlistConsumerConfig.addProvidedItemInfo(refreshMsg.streamId(), refreshMsg.msgKey(), 
                	    			refreshMsg.domainType());
                	    }
                	}
                }
                else if (item != null)
                {
                	itemName = item.name();
                }
                   				
    		    System.out.println("DefaultMsgCallback Refresh ItemName: " + itemName + " Domain: " + refreshMsg.domainType() + ", StreamId: " + refreshMsg.streamId());

				System.out.println("                      State: "  + refreshMsg.state());
 
				itemState = refreshMsg.state();

				if (refreshMsg.streamId() == watchlistConsumerConfig.itemList().get(postItemsIndex.get(currentPostIndex)).streamId)
				{
					postingItemState = itemState;
				}
    			/* Decode data body according to its domain. */
    			itemDecoder.decodeDataBody(event.reactorChannel(), refreshMsg);            	
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
                
    		    System.out.println("DefaultMsgCallback Update ItemName: " + itemName + " Domain: " + updateMsg.domainType() + ", StreamId: " + updateMsg.streamId());

    			/* Decode data body according to its domain. */
    			itemDecoder.decodeDataBody(event.reactorChannel(), updateMsg);            	
            	break;

            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg; 
                if (statusMsg.checkHasMsgKey())
                {
                	if (statusMsg.msgKey().checkHasName())
                	{
                		itemName = statusMsg.msgKey().name().toString();
                   	    if ( item != null && statusMsg.streamId() < 0)
                	    {
                	    	watchlistConsumerConfig.addProvidedItemInfo(statusMsg.streamId(), statusMsg.msgKey(), 
                	    			statusMsg.domainType());
                	    }
                	}
                }
                else if (item != null)
                {
                	itemName = item.name();
                }
                 				
    		    System.out.println("DefaultMsgCallback Status -- ItemName: " + itemName + " Domain: " + statusMsg.domainType() + ", StreamId: " + statusMsg.streamId());

    		    if ( statusMsg.checkHasState())
    		    {
    				System.out.println("              State: "  + statusMsg.state());
    				 
    				itemState = statusMsg.state();
    				if (statusMsg.streamId() == watchlistConsumerConfig.itemList().get(postItemsIndex.get(currentPostIndex)).streamId)
    				{
    					postingItemState = itemState;
    				}
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
    		    System.out.println("DefaultMsgCallback Ack --  ItemName: " + itemName + " Domain: " + ackMsg.domainType() + ", StreamId: " + ackMsg.streamId());
    		    System.out.println(" ackId: " + ackMsg.ackId());
    		    if ( ackMsg.checkHasSeqNum())
    		    {
    		    	 System.out.println(" seqNum: " + ackMsg.seqNum()); 
    		    }
    		    if ( ackMsg.checkHasNakCode())
    		    {
    		    	 System.out.println(" nakCode: " + ackMsg.nakCode()); 
    		    }
    		    if ( ackMsg.checkHasText())
    		    {
    		    	 System.out.println(" text: " + ackMsg.text()); 
    		    }
    		    break;
 
            default:
            	System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;
                
        }
        /* Check state of any provider-driven streams.
         * If the state indicates the item was closed, remove it from our list. */
        if (itemState != null && item != null && msg.streamId() < 0 && itemState.streamState() != StreamStates.OPEN)
        	watchlistConsumerConfig.removeProvidedItemInfo(item);
    
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
				System.out.println("\nReceived Login Refresh for Username: " + ((LoginRefresh)event.rdmLoginMsg()).userName());
				System.out.println(event.rdmLoginMsg().toString());
				
				// save loginRefresh
				((LoginRefresh)event.rdmLoginMsg()).copy(chnlInfo.loginRefresh);
					
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.LOGIN) + ", StreamId: " + event.rdmLoginMsg().streamId());
				
				System.out.println(" State: "  + chnlInfo.loginRefresh.state());
				if ( chnlInfo.loginRefresh.checkHasUserName()) 
					System.out.println(" UserName: " + chnlInfo.loginRefresh.userName().toString());
				
				
				break;
				
			case STATUS:
				LoginStatus loginStatus = (LoginStatus)event.rdmLoginMsg();
				System.out.println("\nDomain: " + DomainTypes.toString(DomainTypes.LOGIN) + ", StreamId: " + event.rdmLoginMsg().streamId());
				System.out.println("Received Login StatusMsg");
				if (loginStatus.checkHasState())
		    	{
					System.out.println("	" + loginStatus.state());
		    	}
				if (loginStatus.checkHasUserName()) 
					System.out.println(" UserName: " + loginStatus.userName().toString());

				break;
			default:
				System.out.println("Received Unhandled Login Msg Type: " + msgType);
				break;
		}
		
        return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		DirectoryMsgType msgType = event.rdmDirectoryMsg().rdmMsgType();

		switch (msgType)
		{
			case REFRESH:
				DirectoryRefresh directoryRefresh = (DirectoryRefresh)event.rdmDirectoryMsg();
				System.out.println("\nDomain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.REFRESH));
				System.out.println(directoryRefresh.state().toString());
		    					
				serviceList = directoryRefresh.serviceList();	
				String serviceName = chnlInfo.connectionArg.service();

				for (Service service : serviceList)
				{
					if(service.info().serviceName().toString() != null)
			        {
						if (service.info().serviceName().toString().equals(serviceName))
						{
							// save serviceInfo associated with requested service name
							if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
							{
								System.out.println("Service.copy() failure");
								uninitialize();
								System.exit(ReactorReturnCodes.FAILURE);                    
			                	}
								chnlInfo.hasServiceInfo = true;
						}
			        }
				}

				break;
			case UPDATE:
				DirectoryUpdate directoryUpdate = (DirectoryUpdate)event.rdmDirectoryMsg();
				System.out.println("\nDomain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.UPDATE));
				serviceList = directoryUpdate.serviceList();								
				serviceName = chnlInfo.connectionArg.service();

				for (Service service : serviceList)										
				{
					if(service.info().serviceName().toString() != null)
			        {
						if (service.info().serviceName().toString().equals(serviceName))
						{
							// save serviceInfo associated with requested service name
							if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
							{
								System.out.println("Service.copy() failure");
								uninitialize();
								System.exit(ReactorReturnCodes.FAILURE);                    
			                	}
								chnlInfo.hasServiceInfo = true;
						}
			        }
				}
				break;
			case CLOSE:
				System.out.println("\nReceived Source Directory Close");
				break;
			case STATUS:
				DirectoryStatus directoryStatus = (DirectoryStatus)event.rdmDirectoryMsg();
				System.out.println("\nReceived Source Directory StatusMsg");
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.STATUS));
				System.out.println(directoryStatus.state().toString());
				if (directoryStatus.checkHasState())
		    	{
					System.out.println("	" + directoryStatus.state());
		    	}
				break;
			default:
				System.out.println("\nReceived Unhandled Source Directory Msg Type: " + msgType);
		    	break;
		}
		
		/* Refresh and update messages contain updates to service information. */		
		if ( serviceList != null ) 
		{
			for (Service service : serviceList)
			{
				System.out.println(" Service = " + service.serviceId() + " Action: " + MapEntryActions.toString(service.action()));
				
				if ( service.serviceId() == chnlInfo.serviceInfo.serviceId())
				{
					if (postingItemState!= null &&  postingItemState.streamState() == StreamStates.OPEN && 
							postingItemState.dataState() == DataStates.OK)
						// let default msg callback update of postingItemSatte take precedence
					{
						postingItemState = service.state().status();
					}
				}

				if (chnlInfo.connectionArg.tunnel()) 
				{
					tunnelStreamHandler.processServiceUpdate(chnlInfo.connectionArg.qService(), service);        	
				}
        
				if (chnlInfo.connectionArg.qService() != null ) 
				{
					queueMsgHandler.processServiceUpdate(chnlInfo.connectionArg.qService(), service);        	
				}
			}
		}
        
		if (chnlInfo.connectionArg.tunnel()) 
		{
			if (!tunnelStreamHandler.isServiceFound())
			{
				System.out.println(" Directory response does not contain service name for tunnel streams: \n " 
						+ chnlInfo.connectionArg.qService());
			}
			else if (!tunnelStreamHandler.isServiceSupported())
			{
				System.out.println(" Service in use for tunnel streams does not support them: \n"						 
						+ chnlInfo.connectionArg.qService());
			}
		}
		
		if (chnlInfo.connectionArg.qService() != null) 
		{
			if (!queueMsgHandler.isServiceFound())
			{
				System.out.println(" Directory response does not contain service name for queue messaging: \n " 
						+ chnlInfo.connectionArg.qService());
			}
			else if (!queueMsgHandler.isServiceSupported())
			{
				System.out.println(" Service in use for queue messaging does not support them: \n"						 
						+ chnlInfo.connectionArg.qService());
			}
		}
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
					switch(dictionaryRefresh.dictionaryType())
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
				chnlInfo.dIter.setBufferAndRWFVersion(dictionaryRefresh.dataBody(),
													event.reactorChannel().majorVersion(),
													event.reactorChannel().minorVersion());

				System.out.println("Received Dictionary Response: " + dictionaryRefresh.dictionaryName());

				if (dictionaryRefresh.streamId() == chnlInfo.fieldDictionaryStreamId)
				{
					if (chnlInfo.dictionary.decodeFieldDictionary(chnlInfo.dIter, Dictionary.VerbosityValues.VERBOSE, error) == CodecReturnCodes.SUCCESS)
					{
						if (dictionaryRefresh.checkRefreshComplete())
						{
							fieldDictionaryLoaded = true;
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
				
				if (fieldDictionaryLoaded  && enumDictionaryLoaded)
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
				break;
			default:
				System.out.println("Received Unhandled Dictionary Msg Type: " + msgType);
				break;		
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	

    public boolean isRequestedServiceUp(ChannelInfo chnlInfo)
    {
        return  chnlInfo.hasServiceInfo &&
			chnlInfo.serviceInfo.checkHasState() && (!chnlInfo.serviceInfo.state().checkHasAcceptingRequests() ||
                chnlInfo.serviceInfo.state().acceptingRequests() == 1) && chnlInfo.serviceInfo.state().serviceState() == 1;
    }

    public boolean isRequestedQServiceUp(ChannelInfo chnlInfo)
    {
        return  chnlInfo.hasQServiceInfo &&
            chnlInfo.qServiceInfo.checkHasState() && (!chnlInfo.qServiceInfo.state().checkHasAcceptingRequests() ||
                chnlInfo.qServiceInfo.state().acceptingRequests() == 1) && chnlInfo.qServiceInfo.state().serviceState() == 1;
    }

    private void checkAndInitPostingSupport(ChannelInfo chnlInfo)
    {
        if (!(chnlInfo.shouldOnStreamPost || chnlInfo.shouldOffStreamPost))
            return;

        // set up posting if its enabled 
        
        // ensure that provider supports posting - if not, disable posting
        if (!chnlInfo.loginRefresh.checkHasFeatures() ||
                !chnlInfo.loginRefresh.features().checkHasSupportPost() ||
                chnlInfo.loginRefresh.features().supportOMMPost() == 0)
        {
            // provider does not support posting, disable it
        	chnlInfo.shouldOffStreamPost = false;
        	chnlInfo.shouldOnStreamPost = false;
        	chnlInfo.postHandler.enableOnstreamPost(false);
        	chnlInfo.postHandler.enableOffstreamPost(false);
            System.out.println("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
            return;
        }
        
        if ( watchlistConsumerConfig.publisherId() != null && watchlistConsumerConfig.publisherAddress() != null)
        	chnlInfo.postHandler.setPublisherInfo(watchlistConsumerConfig.publisherId(), watchlistConsumerConfig.publisherAddress());
    }
	
    // on and off stream posting if enabled
    private void handlePosting()
    {    	
    	postItemsIndex.clear();
       	for (int i = 0; i < watchlistConsumerConfig.itemCount(); i++)
    	{
    		if (watchlistConsumerConfig.itemList().get(i).domain() == DomainTypes.MARKET_PRICE)
    		{
    			int postStreamId = watchlistConsumerConfig.itemList().get(i).streamId;	        		
	            if (postStreamId > 0 && watchlistConsumerConfig.itemList().get(i).name().length() > 0)
	            {
	            	postItemsIndex.add(i);
	            }
    		}
    	}	        	
    	
    	for (ChannelInfo chnlInfo : chnlInfoList)
    	{
	    	if (chnlInfo.loginRefresh == null ||
	    		chnlInfo.serviceInfo == null ||
	    		chnlInfo.reactorChannel == null ||
	    		chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
	    	{	    		
	    		continue;
	    	}
	    	
        	chnlInfo.postItemName.clear();
	    	
	        if (chnlInfo.postHandler.enableOnstreamPost())
	        {	        	
	            if (postItemsIndex.size() == 0 )
	            {
	        		System.out.println("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");	            
	                return;
	            }
	            else
	            {
	            	chnlInfo.postHandler.streamId(watchlistConsumerConfig.itemList().get(postItemsIndex.get(currentPostIndex)).streamId);
	            	chnlInfo.postHandler.postItemName().data(watchlistConsumerConfig.itemList().get(postItemsIndex.get(currentPostIndex)).name());
	            }

	            if ( postingItemState!= null && 
	            		(postingItemState.streamState() != StreamStates.OPEN 
	            		|| postingItemState.dataState() != DataStates.OK)) 
	            {
                    if ( currentPostIndex < (postItemsIndex.size()-1) ) 
                    {
                    	postingItemState = null;
                    	currentPostIndex ++;
                    }
	            	return;
	            }
	            
	           
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
	                System.out.println("Error posting offstream: " + error.text());
	        }
    	}
    }

    private void handleQueueMessaging()
    {
        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            if (chnlInfo.loginRefresh == null ||
                chnlInfo.serviceInfo == null ||
                chnlInfo.reactorChannel == null ||
                chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
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
            if (chnlInfo.loginRefresh == null ||
                chnlInfo.serviceInfo == null ||
                chnlInfo.reactorChannel == null ||
                chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
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
		
        if (itemDecoder.fieldDictionaryLoadedFromFile == false &&
        	itemDecoder.enumTypeDictionaryLoadedFromFile == false)
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
        
        chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
        chnlInfo.consumerRole.rdmLoginRequest().attrib().singleOpen(1);
        chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
        chnlInfo.consumerRole.rdmLoginRequest().attrib().allowSuspectData(0);
                
        // if unable to load from file and no queue messaging, enable consumer to download dictionary
        if (itemDecoder.fieldDictionaryLoadedFromFile == false &&
            itemDecoder.enumTypeDictionaryLoadedFromFile == false &&
            chnlInfo.connectionArg.qSource() == null)
        {
        	chnlInfo.consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
        }

        if (itemDecoder.fieldDictionaryLoadedFromFile == true &&
        	itemDecoder.enumTypeDictionaryLoadedFromFile == true)
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
	        	for (ItemArg itemArg  : chnlInfo.connectionArg.itemList())
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

        // This sets up our basic timing so post messages will be sent
        // periodically
        chnlInfo.postHandler.initPostHandler();
        
        
        // set up reactor connect options
        chnlInfo.connectOptions.reconnectAttemptLimit(-1); // attempt to recover forever
        chnlInfo.connectOptions.reconnectMinDelay(1000); // 1 second minimum
        chnlInfo.connectOptions.reconnectMaxDelay(60000); // 60 second maximum
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(chnlInfo.connectionArg.connectionType());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(chnlInfo.connectionArg.port());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(chnlInfo.connectionArg.hostname());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().guaranteedOutputBuffers(1000);
        // add backup connection if specified
        if (watchlistConsumerConfig.backupHostname() != null && watchlistConsumerConfig.backupPort() != null)
        {
        	ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
        	chnlInfo.connectOptions.connectionList().add(connectInfo);
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().majorVersion(Codec.majorVersion());
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().minorVersion(Codec.minorVersion());
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().connectionType(chnlInfo.connectionArg.connectionType());
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName(watchlistConsumerConfig.backupPort());
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().unifiedNetworkInfo().address(watchlistConsumerConfig.backupHostname());
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().userSpecObject(chnlInfo);
            chnlInfo.connectOptions.connectionList().get(1).connectOptions().guaranteedOutputBuffers(1000);
        }
        
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
                queueMsgHandler = new QueueMsgHandler(chnlInfo.connectionArg.qSource(),
                                                      chnlInfo.connectionArg.qDestList(),
                                                      fixdictionary, chnlInfo.connectionArg.tunnelAuth(), chnlInfo.connectionArg.tunnelDomain() );                               
            }
            else // exit if too many queue destinations entered
            {
                System.err.println("\nError: Example only supports " + MAX_QUEUE_DESTINATIONS + " queue destination names.\n");
                System.err.println(CommandLine.optionHelpString());
                System.exit(CodecReturnCodes.FAILURE);
            }
        }

        // handle basic tunnel stream configuration
        if (chnlInfo.connectionArg.tunnel() && tunnelStreamHandler == null)
        {
            tunnelStreamHandler = new TunnelStreamHandler(chnlInfo.connectionArg.tunnelAuth(), chnlInfo.connectionArg.tunnelDomain());
        }
	}
	
	// load FIX dictionary to support FIX Protocol
	public void loadFixDictionary()
	{      
        fixdictionary.clear();
        if (fixdictionary.loadFieldDictionary(FIX_FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("\nUnable to load FIX field dictionary. \n\tText: "
                        + error.text() + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }

        if (fixdictionary.loadEnumTypeDictionary(FIX_ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("\nUnable to load FIX enum dictionary. \n\tText: "
                        + error.text() + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }	    
	}    

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
    	
		for(int itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemCount(); ++itemListIndex)
		{			        
			int domainType = watchlistConsumerConfig.itemList().get(itemListIndex).domain();
		        
			int streamId = watchlistConsumerConfig.itemList().get(itemListIndex).streamId;
	           
			/* encode item close */
			closeMsg.clear();
			closeMsg.msgClass(MsgClasses.CLOSE);
			closeMsg.streamId(streamId);
			closeMsg.domainType(domainType);
			closeMsg.containerType(DataTypes.NO_DATA); 

		    if ( (chnlInfo.reactorChannel.submit(closeMsg, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS)
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
            if (queueMsgHandler != null &&
                chnlInfo.reactorChannel != null)
            {
                if (queueMsgHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("queueMsgHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }
	
            // close tunnel streams
            if (tunnelStreamHandler != null &&
                chnlInfo.reactorChannel != null)
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
            if ( proxyHostName == null)
            {
            	System.err.println("Error: Proxy hostname not provided.");  
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }           
            String proxyPort = watchlistConsumerConfig.proxyPort();
            if ( proxyPort == null)
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
            catch(Exception e)
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
        if ( proxyUsername == null)
        {
        	System.err.println("Error: Proxy username not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }    
              
        String proxyPasswd = watchlistConsumerConfig.proxyPassword();
        if ( proxyPasswd == null)
        {
        	System.err.println("Error: Proxy password not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }     
        String proxyDomain = watchlistConsumerConfig.proxyDomain();
        if ( proxyDomain == null)
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

	public static void main(String[] args) throws Exception
    {		
        WatchlistConsumer consumer = new WatchlistConsumer();
        consumer.init(args);
        consumer.run(); 
        consumer.uninitialize();
        System.exit(0);
    }
}
