//APIQA
package com.thomsonreuters.upa.valueadd.examples.watchlistconsumer;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.shared.CommandLine;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.valueadd.examples.common.ConnectionArg;
import com.thomsonreuters.upa.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig;
import com.thomsonreuters.upa.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig.ItemInfo;

/**
 * <p>
 * This is a main class to run the UPA Value Add WatchlistConsumer application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * This is the main file for the WatchlistConsumer application.  It is a multi-threaded
 * client application that utilizes the UPA Reactor's watchlist to provide recovery of data.
 * 
 * This application is designed to create per UPA Reactor/per UPA watchlist per thread.
 * Also allows client to send requests in different thread from UPA dispatch handing. 
 * 
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
 * <li>-tunnel (optional) enables consumer to open tunnel stream and send basic text messages
 * <li>-tsServiceName (optional) specifies the service name for tunnel stream messages (if not specified, the service name specified in -c/-tcp is used)"
 * <li>-tsAuth (optional) specifies that consumer will request authentication when opening the tunnel stream. This applies to basic tunnel streams.
 * <li>-tsDomain (optional) specifies the domain that consumer will use when opening the tunnel stream. This applies to basic tunnel streams.
 * <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
 * <li>-ax Specifies the Authentication Extended information.
 * <li>-aid Specifies the Application ID.
 * 
 * <p>Here is configuring per connection, per reactor, and per thread. "server"  configuration will override all previous connection configuration.
 *   Reading a list of items from the file which will be distributed evenly to all configured connections, and it will override the previous configured items. For example:
 * <li>-runtime 150 -server localhost:14002:DIRECT_FEED -server localhost:14003:ELEKTRON_FEED -itemFile c:/temp/20k.xml -itemCount 20000
 *  
 * </ul>
 */
public class WatchlistConsumer
{
	WatchlistConsumerConfig watchlistConsumerConfig;
    private long runtime;
    
	private List<ConsumerCallbackThread> consumerList = new ArrayList<ConsumerCallbackThread>();
	private List<ChannelInfo> chanInfoList = new ArrayList<ChannelInfo>();
	boolean shutDown = false;
	
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

 		// display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("WatchlistConsumer initializing...");
        
        runtime = System.currentTimeMillis() + watchlistConsumerConfig.runtime() * 1000;
                
        /* create per reactor/watchlist, per channel info per thread which initializes channel info, and connect channels
         * for each connection specified */
        for (ConnectionArg connectionArg : watchlistConsumerConfig.connectionList())
        {
        	// create channel info
        	ChannelInfo chnlInfo = new ChannelInfo();
        	chnlInfo.connectionArg = connectionArg;
        	ConnectOptions connectOptions = chnlInfo.connectInfo.connectOptions();
    		connectOptions.userSpecObject(chnlInfo);
    		
    		ConsumerCallbackThread consumerCallbackThread = new ConsumerCallbackThread(chnlInfo,this);
    		new Thread(consumerCallbackThread).start();
    	
    		consumerList.add(consumerCallbackThread);
    		chanInfoList.add(chnlInfo);
        }
        
        distributeItemRequest();
    }
    

    void distributeItemRequest()
    {
    	List<ItemInfo> itemList = watchlistConsumerConfig.itemList();
    	int numOfItems = itemList.size();
    	int numOfConsumer = consumerList.size();
		int maxItemPerConsumer = numOfItems/numOfConsumer;
		int consumerIndex = 0;
		List<ItemInfo> itemsPerConsumer = null;
		for (int i=0; i<numOfItems; i++)
		{
			if (i%maxItemPerConsumer == 0)
			{
			    if (consumerIndex < numOfConsumer)
			    {
			    	itemsPerConsumer = consumerList.get(consumerIndex).consumerReqeustThread.itemList;
			    }
			    else
			    	return;
			    
			    consumerIndex++;
			}
			
			itemsPerConsumer.add(itemList.get(i));
		}
		return;
    }

    /** Shutdown Consumer */
	public void shutdown()
	{
		shutDown = true;
	}
	
	public WatchlistConsumerConfig watchlistConsumerConfig()
	{
		return watchlistConsumerConfig;
	}
		
    /* Runs the Value Add consumer application. */
	private void run()		
	{		
		// main statistics polling thread here
		while(!shutDown)
		{
			try
			{
				Thread.sleep(1000);
			}
			catch (InterruptedException e)
			{
				System.out.printf("Thread.sleep(1000) failed\n");
				System.exit(-1);
			}
			
			// Handle run-time
	        if (System.currentTimeMillis() >= runtime)
	        {
	        	System.out.println("MultithreadConsumer run-time expired, close now...");
	        	break;        
	        }
		}
		
		for (ConsumerCallbackThread consumer : consumerList)
    	{
 			consumer.shutDown(true);
    	}
    	
		try
		{
			Thread.sleep(5000);
		}
		catch (InterruptedException e)
		{
			System.out.printf("Thread.sleep(1000) failed\n");
			System.exit(-1);
		}
	}
    
	public static void main(String[] args) throws Exception
    {		
		WatchlistConsumer consumer = new WatchlistConsumer();
        consumer.init(args);
        consumer.run(); 
        System.exit(0);
    }
}
