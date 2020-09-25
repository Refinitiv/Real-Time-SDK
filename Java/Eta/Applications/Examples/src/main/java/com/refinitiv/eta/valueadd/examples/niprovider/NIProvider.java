package com.refinitiv.eta.valueadd.examples.niprovider;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.IoctlCodes;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.cache.CacheFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;
import com.refinitiv.eta.valueadd.examples.common.CacheInfo;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArg;
import com.refinitiv.eta.valueadd.examples.common.ItemArg;
import com.refinitiv.eta.valueadd.reactor.NIProviderCallback;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgEvent;
import com.refinitiv.eta.valueadd.reactor.Reactor;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorDispatchOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/**
 * <p>This is a main class to run UPA NIProvider application. The purpose of this
 * application is to non-interactively provide Level I Market Price and Level 2
 * Market By Order data to an Advanced Data Hub (ADH). It is a single-threaded
 * client application using ValueAdd components.
 * </p>
 * <H2>Summary</H2>
 * This class is responsible for the following:
 * <ul>
 * <li>Initialize and set command line options.
 * <li>Load Dictionary from file.
 * <li>Create a {@link Reactor UPA Channel}. It responds to messages through its Login,
 * Directory, Dictionary and default message callbacks.
 * <li>Connect to the ADH provider, send item refreshes, then send item updates.
 * </ul>
 * <p>
 * This class is also a call back for all events from Consumer/ADH. It
 * dispatches events to domain specific handlers.
 * </p>
 * <p>
 * Reliable multicast can be used to communicate between this application and any
 * ADH on the network. The non-interactive provider can then send one message to
 * all ADH's on the network instead of having to fan-out messages to each ADH
 * TCP/IP connection.
 * </p>
 * <p>
 * This application is intended as a basic usage example. Some of the design
 * choices were made to favor simplicity and readability over performance. It is
 * not intended to be used for measuring performance. This application uses
 * Value Add and shows how using Value Add simplifies the writing of UPA
 * applications. Because Value Add is a layer on top of UPA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the UPA interfaces.
 * </p>
 * <H2>Setup Environment</H2>
 * <p>
 * The RDMFieldDictionary and enumtype.def files must be located in the
 * directory of execution or this application will exit.
 * </p>
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runVANIProvider -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runVANIProvider -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below for Tcp primary and backup connection.
 * </p>
 * <ul>
 * <li>-c specifies a tcp connection to open and a list of items to provide:
 * <ul>
 *  <li>hostname:        Hostname of ADH to connect to
 *  <li>port:            Port of ADH to connect to
 *  <li>service:         Name of service to provide items on this connection
 *  <li>domain:itemName: Domain and name of an item to provide
 *       <br>A comma-separated list of these may be specified.
 *       <br>The domain may be any of: mp(MarketPrice), mbo(MarketByOrder)
 *       <br>Example Usage: -c localhost:14002 NI_PUB mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM -bc localhost:14005
 *  </li>
 *  <li>backup hostname: Backup hostname of ADH to connect to
 *  <li>backup port:     Backup port of ADH to connect to
 *  </ul>
 * </ul>
 * <p>
 * Arguments are listed below for reliable multicast primary and backup connection. 
 * </p>
 * <ul>
 * <li>-segmentedMulticast specifies a reliable multicast connection to open and a list of items to provide:
 * <ul>
 * <li>sa:     Sender address of ADH to connect to
 * <li>sp:     Sender port of ADH to connect to
 * <li>if:     Interface of ADH to connect to
 * <li>ra:     Receiver address of ADH to connect to
 * <li>rp:     Receiver port of ADH to connect to
 * <li>up:     Unicast port number
 * <li>service: Name of service to provide items on this connection
 * <li>domain:itemName: Domain and name of an item to provide
 *       <br>A comma-separated list of these may be specified.
 *       <br>The domain may be any of: mp(MarketPrice), mbo(MarketByOrder)
 *       <br>Example Usage: -segmentedMulticast 235.5.5.5:14002:localhost 235.5.5.4:14003 14007 NI_PUB mp:TRI.N,mp:IBM.N -mbc 235.5.5.6:14004:localhost 235.5.5.7:14005 14008        
 *  </li>
 *  <li>bsa:     Backup sender address of ADH to connect to
 *  <li>bsp:     Backup sender port of ADH to connect to
 *  <li>bif:     Backup interface of ADH to connect to
 *  <li>bra:     Backup receiver address of ADH to connect to
 *  <li>brp:     Backup receiver port of ADH to connect to
 *  <li>bup:     Backup unicast port of ADH to connect to  
 *  </ul>
 * </ul>
 * <p>
 * Additional arguments are listed below. 
 * </p>
 * <ul>
 * <li>-uname Login user name. Default is system user name.
 * <li>-x. Provides XML tracing of messages.
 * <li>-cache application supports apply/retrieve data to/from cache
 * <li>-runtime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
 * <li>-ax Specifies the Authentication Extended information.
 * <li>-aid Specifies the Application ID.
 * </ul>
 * 
 */
public class NIProvider implements NIProviderCallback 
{   
    private Reactor reactor;
    private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
    private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
    private NIProviderCmdLineParser commandLineParser = new NIProviderCmdLineParser();
    private Selector selector;

    private long runtime;
 
    private static final int SEND_INTERVAL = 1000; // send content every 1000 milliseconds    
    
    // default server host name
    private static final String defaultSrvrHostname = "localhost";
    
    // default server port number
    private static final String defaultSrvrPortNo = "14003";
    
    // default service name
    private static final String defaultServiceName = "NI_PUB1";
    
    // default service id
    private static final int defaultServiceId = 5;
    
    // default item name
    private static final String defaultItemName = "TRI.N";
    private static final String defaultItemName1 = "IBM.N";
        
    private Error error;    // error information
    
    private DataDictionary dictionary;
    
    private static final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private static final String ENUM_TABLE_FILE_NAME = "enumtype.def";    

	private boolean refreshesSent;
	
	ArrayList<ChannelInfo> chnlInfoList = new ArrayList<ChannelInfo>();
	
	DateFormat formatter = new SimpleDateFormat("MM:dd:yy:HH:mm:ss");
	
    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

    public NIProvider()
    {
        dictionary = CodecFactory.createDataDictionary();   
        error = TransportFactory.createError();
        dispatchOptions.maxMessages(1);
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

    /*
     * Initializes the Value Add NIProvider application.
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments. Initializing all domain
     * handlers. Loading dictionaries from file.
     */
    private void init(String[] args)
    {
        // parse command line
    	if (!commandLineParser.parseArgs(args))
        {
            System.err.println("\nError loading command line arguments:\n");
            commandLineParser.printUsage();
            System.exit(CodecReturnCodes.FAILURE);
        }

    	// add default connections to arguments if none specified
        if (commandLineParser.connectionList().size() == 0)
        {
        	// first connection - localhost:14003 NI_PUB mp:TRI.N
        	List<ItemArg> itemList = new ArrayList<ItemArg>();
        	ItemArg itemArg = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName, false);
        	itemList.add(itemArg);
          	ItemArg itemArg1 = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName1, false);
        	itemList.add(itemArg1);

           	ItemArg itemArg2 = new ItemArg(DomainTypes.MARKET_BY_ORDER, defaultItemName, false);
        	itemList.add(itemArg2);
          	ItemArg itemArg3 = new ItemArg(DomainTypes.MARKET_BY_ORDER, defaultItemName1, false);
        	itemList.add(itemArg3);        	
        	ConnectionArg connectionArg = new ConnectionArg(ConnectionTypes.SOCKET,
        													defaultServiceName,
        													defaultSrvrHostname,
        													defaultSrvrPortNo,
        													itemList);
        	commandLineParser.connectionList().add(connectionArg);

 
        }

        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("NIProvider initializing...");

        runtime = System.currentTimeMillis() + commandLineParser.runtime() * 1000;
              
        // load dictionary
        loadDictionary();
        
        // enable Reactor XML tracing if specified
        if (commandLineParser.enableXmlTracing())
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
        
      	// create channel info
       	ChannelInfo chnlInfo = new ChannelInfo();
      	chnlInfo.connectionArg = commandLineParser.connectionList().get(0);
        	
      	// initialize channel info
      	initChannelInfo(chnlInfo);

	    int ret;
	        	        
	    if ((ret = reactor.connect(chnlInfo.connectOptions, chnlInfo.niproviderRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
	    {
	    	System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
	      	System.exit(ReactorReturnCodes.FAILURE);
	    }
	        
	    // add to ChannelInfo list
	    chnlInfoList.add(chnlInfo);
       
    }

	private void run()
	{
		int selectRetVal, selectTime = 1000;
        long currentTime = System.currentTimeMillis();
        long nextSendTime = currentTime + SEND_INTERVAL;
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
	
	        // send login reissue if login reissue time has passed
        	for (ChannelInfo chnlInfo : chnlInfoList)
        	{
        		if (chnlInfo.reactorChannel == null ||
	        	    (chnlInfo.reactorChannel.state() != ReactorChannel.State.UP && 
	        	    chnlInfo.reactorChannel.state() != ReactorChannel.State.READY))
    	    	{
    	    		continue;
    	    	}

        		if (chnlInfo.canSendLoginReissue &&
        			System.currentTimeMillis() >= chnlInfo.loginReissueTime)
        		{
					LoginRequest loginRequest = chnlInfo.niproviderRole.rdmLoginRequest();
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

            // Handle run-time
            if (System.currentTimeMillis() >= runtime)
            {
                System.out.println("NIProvider run-time expired...");
                break;
            }

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
	        						System.out.println("ReactorChannel dispatch failed");
	        						uninitialize();
	        						System.exit(ReactorReturnCodes.FAILURE);
	        					}
	        				}
	        			}
	        		}
	        		catch (CancelledKeyException e)
	        		{
	        		} // key can be canceled during shutdown
	        	}		
	        }
	        
        	if (currentTime >= nextSendTime)
        	{
            	for (ChannelInfo chnlInfo : chnlInfoList)
            	{
            		if (chnlInfo.reactorChannel != null && chnlInfo.reactorChannel.state() == ReactorChannel.State.READY		
            				&& chnlInfo.loginRefresh.state().streamState() == StreamStates.OPEN &&
            				chnlInfo.loginRefresh.state().dataState() == DataStates.OK )
            		{
            			if (refreshesSent) // send updates since refreshes already sent
            			{
            				sendItemUpdates(chnlInfo);
            			}
            			else // send refreshes first
            			{
            				sendItemRefreshes(chnlInfo);
            				refreshesSent = true;
            			}            			
            		}
            		nextSendTime += SEND_INTERVAL;
            	}
        	}	         
    		currentTime = System.currentTimeMillis();
		}
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
    	        
    	        int rcvBufSize = 65535;
    	        int sendBufSize = 65535;
    			/* Change size of send/receive buffer since it's small by default on some platforms */
                if (event.reactorChannel().ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    System.out.println("channel.ioctl() failed: " + error.text());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
    
                if (event.reactorChannel().ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, rcvBufSize, errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    System.out.println("channel.ioctl() failed: " + error.text());
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
                SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();
    
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
                // set ReactorChannel on ChannelInfo
                chnlInfo.reactorChannel = event.reactorChannel();

    		    System.out.println("\nConnection is ready, starting publishing");
    			break;
    		}
            case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
            {
    			if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down reconnecting: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down reconnecting");
    
                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");
                            
                // allow Reactor to perform connection recovery
                
                // reset refreshesSent flag
                refreshesSent = false;
                
    	        // reset canSendLoginReissue flag
    	        chnlInfo.canSendLoginReissue = false;
                
                // unregister selectableChannel from Selector
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }

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
            default:
                break;
	    }

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int defaultMsgCallback(ReactorMsgEvent event)
	{
		Msg msg = event.msg();
		
		if (msg == null)
		{
			System.out.printf("defaultMsgCallback() received error: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		return ReactorCallbackReturnCodes.SUCCESS;
	}
        
	@Override
	public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		
		LoginMsg loginMsg = event.rdmLoginMsg();

		if (loginMsg == null)
		{
			System.out.printf("loginMsgCallback() received error: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
           
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		LoginMsgType msgType = event.rdmLoginMsg().rdmMsgType();

		switch (msgType)
		{
			case REFRESH:
				System.out.println("Received Login Refresh for Username: " + ((LoginRefresh)event.rdmLoginMsg()).userName());
				System.out.println(event.rdmLoginMsg().toString());
				
				// save loginRefresh
				((LoginRefresh)event.rdmLoginMsg()).copy(chnlInfo.loginRefresh);
	
				// get login reissue time from authenticationTTReissue
				if (chnlInfo.loginRefresh.checkHasAuthenticationTTReissue())
				{
					chnlInfo.loginReissueTime = chnlInfo.loginRefresh.authenticationTTReissue() * 1000;
					chnlInfo.canSendLoginReissue = true;
				}

				break;
			case STATUS:
				LoginStatus loginStatus = (LoginStatus)event.rdmLoginMsg();
				System.out.println("Received Login StatusMsg");
				if (loginStatus.checkHasState())
		    	{
					System.out.println("	" + loginStatus.state());
		    	}
				break;
			default:
				System.out.println("Received Unhandled Login Msg Type: " + msgType);
				break;
		}
        return ReactorCallbackReturnCodes.SUCCESS;
	}

    private void sendItemRefreshes(ChannelInfo chnlInfo)
    {
    	if (chnlInfo.marketPriceHandler.sendItemRefreshes(chnlInfo.reactorChannel, chnlInfo.mpItemList, 	
   			 defaultServiceId, chnlInfo.cacheInfo, errorInfo)!= CodecReturnCodes.SUCCESS)
       {
            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
            {
                System.out.println(errorInfo.error().text());
                uninitialize();
                System.exit(ReactorReturnCodes.FAILURE);
            }
       }
   	
       	if (chnlInfo.marketByOrderHandler.sendItemRefreshes(chnlInfo.reactorChannel, chnlInfo.mboItemList, 	
   			 defaultServiceId, chnlInfo.cacheInfo, errorInfo)!= CodecReturnCodes.SUCCESS)
       {
            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
            {
                System.out.println(errorInfo.error().text());
                uninitialize();
                System.exit(ReactorReturnCodes.FAILURE);
            }
       }        	
    }  
        
    private void sendItemUpdates(ChannelInfo chnlInfo)
    {
    	if (chnlInfo.marketPriceHandler.sendItemUpdates(chnlInfo.reactorChannel, chnlInfo.cacheInfo, errorInfo)!= CodecReturnCodes.SUCCESS)
        {
            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
            {
                System.out.println(errorInfo.error().text());
                uninitialize();
                System.exit(ReactorReturnCodes.FAILURE);
            }
        }
   	
       	if (chnlInfo.marketByOrderHandler.sendItemUpdates(chnlInfo.reactorChannel, chnlInfo.cacheInfo, errorInfo)!= CodecReturnCodes.SUCCESS)
        {
            if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
            {
                System.out.println(errorInfo.error().text());
                uninitialize();
                System.exit(ReactorReturnCodes.FAILURE);
            }
        }  	
    }      
    
    /* Load dictionary from file. */
	void loadDictionary()
    {
        dictionary.clear();
        if (dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
                    + error.text());
            System.exit(ReactorReturnCodes.FAILURE);
        }
 
        if (dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
                        + error.text());
            System.exit(ReactorReturnCodes.FAILURE);
        }
    }

	private void initChannelInfo(ChannelInfo chnlInfo)
	{				
        // set up niprovider role   
		chnlInfo.niproviderRole.channelEventCallback(this);
		chnlInfo.niproviderRole.loginMsgCallback(this);
		chnlInfo.niproviderRole.defaultMsgCallback(this);
		      	
        chnlInfo.marketPriceHandler = new MarketPriceHandler(chnlInfo.itemWatchList, dictionary);    
        chnlInfo.marketByOrderHandler = new MarketByOrderHandler(chnlInfo.itemWatchList, dictionary);
     
        // initialize niprovider role to default
        String serviceName = chnlInfo.connectionArg.service();
        chnlInfo.niproviderRole.initDefaultRDMLoginRequest();
        chnlInfo.niproviderRole.initDefaultRDMDirectoryRefresh(serviceName, defaultServiceId);       

		// use command line login user name if specified
        if (commandLineParser.userName() != null && !commandLineParser.userName().equals(""))
        {
            LoginRequest loginRequest = chnlInfo.niproviderRole.rdmLoginRequest();
            loginRequest.userName().data(commandLineParser.userName());
        }
        
        // use command line authentication token and extended authentication information if specified
        if (commandLineParser.authenticationToken() != null && !commandLineParser.authenticationToken().equals(""))
        {
            LoginRequest loginRequest = chnlInfo.niproviderRole.rdmLoginRequest();
            loginRequest.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
            loginRequest.userName().data(commandLineParser.authenticationToken());

            if (commandLineParser.authenticationExtended() != null && !commandLineParser.authenticationExtended().equals(""))
            {
            	loginRequest.applyHasAuthenticationExtended();
                loginRequest.authenticationExtended().data(commandLineParser.authenticationExtended());
            }
        }
        
        // use command line application id if specified
        if (commandLineParser.applicationId() != null && !commandLineParser.applicationId().equals(""))
        {
            LoginRequest loginRequest = chnlInfo.niproviderRole.rdmLoginRequest();
            loginRequest.attrib().applicationId().data(commandLineParser.applicationId());
        }
 
        createItemLists(chnlInfo);
        
        // set up reactor connect options
        chnlInfo.connectOptions.reconnectAttemptLimit(-1); // attempt to recover forever
        chnlInfo.connectOptions.reconnectMinDelay(1000); // 1 second minimum
        chnlInfo.connectOptions.reconnectMaxDelay(60000); // 60 second maximum
        
    	int connectionType = chnlInfo.connectionArg.connectionType();       
        
        if (connectionType == ConnectionTypes.SOCKET)
        {        
        	chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
        	chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
        	chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(connectionType);
        	chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(chnlInfo.connectionArg.port());
        	chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(chnlInfo.connectionArg.hostname());
        	chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);
        }
        else if (connectionType == ConnectionTypes.RELIABLE_MCAST)
        {
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(connectionType);
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().segmentedNetworkInfo().sendAddress(chnlInfo.connectionArg.sendAddress());
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().segmentedNetworkInfo().sendServiceName(chnlInfo.connectionArg.sendPort());
     		chnlInfo.connectOptions.connectionList().get(0).connectOptions().segmentedNetworkInfo().interfaceName(chnlInfo.connectionArg.interfaceName());
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().segmentedNetworkInfo().recvAddress(chnlInfo.connectionArg.recvAddress());
       		chnlInfo.connectOptions.connectionList().get(0).connectOptions().segmentedNetworkInfo().recvServiceName(chnlInfo.connectionArg.recvPort());
       		chnlInfo.connectOptions.connectionList().get(0).connectOptions().segmentedNetworkInfo().unicastServiceName(chnlInfo.connectionArg.unicastPort());       	        		     		
    		chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);       	
        }
        // add backup connection if specified
        if (commandLineParser.hasBackupConfig())
        {
        	 /**
             * {@link ConnectionTypes#SOCKET} - {@link ConnectionTypes#RELIABLE_MCAST}.
             * 
             * @param connectionType the connectionType to set
             * 
             * @see ConnectionTypes
             */     	
        
        	connectionType = commandLineParser.backupConnectionType();
        
        	if (connectionType == ConnectionTypes.SOCKET && commandLineParser.backupHostname() != null && commandLineParser.backupPort() != null)
        	{        	
       
        		ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
        		chnlInfo.connectOptions.connectionList().add(connectInfo);
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().majorVersion(Codec.majorVersion());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().minorVersion(Codec.minorVersion());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().connectionType(connectionType);
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName(commandLineParser.backupPort());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().unifiedNetworkInfo().address(commandLineParser.backupHostname());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().userSpecObject(chnlInfo);
        	}
        	else if (connectionType == ConnectionTypes.RELIABLE_MCAST )
        	{
        		ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
        		chnlInfo.connectOptions.connectionList().add(connectInfo);
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().majorVersion(Codec.majorVersion());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().minorVersion(Codec.minorVersion());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().connectionType(connectionType);
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().segmentedNetworkInfo().sendAddress(commandLineParser.backupSendAddress());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().segmentedNetworkInfo().sendServiceName(commandLineParser.backupSendPort());
         		chnlInfo.connectOptions.connectionList().get(1).connectOptions().segmentedNetworkInfo().interfaceName(commandLineParser.backupInterfaceName());
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().segmentedNetworkInfo().recvAddress(commandLineParser.backupRecvAddress());
           		chnlInfo.connectOptions.connectionList().get(1).connectOptions().segmentedNetworkInfo().recvServiceName(commandLineParser.backupRecvPort());
           		chnlInfo.connectOptions.connectionList().get(1).connectOptions().segmentedNetworkInfo().unicastServiceName(commandLineParser.backupUnicastPort());
           	        		     		
        		chnlInfo.connectOptions.connectionList().get(1).connectOptions().userSpecObject(chnlInfo);
        	}
        		
        }
        
        if (commandLineParser.cacheOption())
        {
        	initializeCache(chnlInfo.cacheInfo);
        	initializeCacheDictionary(chnlInfo.cacheInfo, dictionary);
        }
	}
	
	/*
     * initializeCache
     */
    private void initializeCache(CacheInfo cacheInfo)
    {
    	cacheInfo.useCache = true;
    	cacheInfo.cacheOptions.maxItems(10000);
    	cacheInfo.cacheDictionaryKey.data("cacheDictionary1");
    	
    	cacheInfo.cache = CacheFactory.createPayloadCache(cacheInfo.cacheOptions, cacheInfo.cacheError);
    	if (cacheInfo.cache == null)
    	{
			System.out.println("Error: Failed to create cache. Error (" + cacheInfo.cacheError.errorId() + 
						") : " + cacheInfo.cacheError.text());
			cacheInfo.useCache = false;
		}
    	
    	cacheInfo.cursor = CacheFactory.createPayloadCursor();
    	if (cacheInfo.cursor == null)
    	{
			System.out.println("Error: Failed to create cache entry cursor.");
			cacheInfo.useCache = false;
		}
    }
    
    /*
     * unintializeCache
     */
    private void uninitializeCache(CacheInfo cacheInfo)
    {
    	if (cacheInfo.cache != null)
    		cacheInfo.cache.destroy();
    	cacheInfo.cache = null;

    	if (cacheInfo.cursor != null)
    		cacheInfo.cursor.destroy();
    	cacheInfo.cursor = null;
    }
    
    /*
     * initalizeCacheDictionary
     */
    private void initializeCacheDictionary(CacheInfo cacheInfo, DataDictionary dictionary)
    {
    	if (dictionary != null)
		{
    		if ( cacheInfo.cache.setDictionary(dictionary,	cacheInfo.cacheDictionaryKey.toString(),
    										cacheInfo.cacheError) != CodecReturnCodes.SUCCESS )
    		{
    			System.out.println("Error: Failed to bind RDM Field dictionary to cache. Error (" + cacheInfo.cacheError.errorId() + 
						") : " + cacheInfo.cacheError.text());
				cacheInfo.useCache = false;
    		}
		}
		else
		{
			System.out.println("Error: No RDM Field dictionary for cache.\n");
			cacheInfo.useCache = false;
		}
    }
	
	private void createItemLists(ChannelInfo chnlInfo)
	{
        // add specified items to item watch list
        for (ItemArg itemArg : chnlInfo.connectionArg.itemList())
        {
        	switch (itemArg.domain())
        	{
        		case DomainTypes.MARKET_PRICE:
	        		if (!itemArg.enablePrivateStream())
	        		{
	        			chnlInfo.mpItemList.add(itemArg.itemName());
	        		}
	        		else
	        		{
	        			chnlInfo.mppsItemList.add(itemArg.itemName());	        			
	        		}
	        		break;
        		case DomainTypes.MARKET_BY_ORDER:
	        		if (!itemArg.enablePrivateStream())
	        		{
	        			chnlInfo.mboItemList.add(itemArg.itemName());
	        		}
	        		else
	        		{
	        			chnlInfo.mbopsItemList.add(itemArg.itemName());	        			
	        		}
	        		break;
        		case DomainTypes.MARKET_BY_PRICE:
        			System.out.println("Does not support publishing MarketByPrice items.");
        			System.exit(ReactorReturnCodes.FAILURE);        			
        			break;
        		default:
	        		break;
        	}
        }
	}


    private void closeItemStreams(ChannelInfo chnlInfo)
    {
        // close item streams if opened
    	chnlInfo.marketPriceHandler.closeStreams(chnlInfo.reactorChannel, errorInfo);
    	chnlInfo.marketByOrderHandler.closeStreams(chnlInfo.reactorChannel, errorInfo);
	}

    
	private void uninitialize()
	{
        System.out.println("NIProvider unitializing and exiting...");
        
    	for (ChannelInfo chnlInfo : chnlInfoList)
    	{
	        // close items streams
	        closeItemStreams(chnlInfo);
	
            // close ReactorChannel
            if (chnlInfo.reactorChannel != null)
            {
                chnlInfo.reactorChannel.close(errorInfo);
            }

            uninitializeCache(chnlInfo.cacheInfo);
    	}
    	
        // shutdown reactor
        reactor.shutdown(errorInfo);
	}

	public static void main(String[] args) throws Exception
    {
        NIProvider niprovider = new NIProvider();
        niprovider.init(args);
        niprovider.run();
        niprovider.uninitialize();
        System.exit(0);
    }
}
