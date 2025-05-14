/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.consumer;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.*;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.shared.network.ChannelHelper;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.cache.CacheFactory;
import com.refinitiv.eta.valueadd.cache.PayloadEntry;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;
import com.refinitiv.eta.valueadd.examples.common.CacheInfo;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArg;
import com.refinitiv.eta.valueadd.examples.common.ItemArg;
import com.refinitiv.eta.valueadd.examples.consumer.StreamIdWatchList.StreamIdKey;
import com.refinitiv.eta.valueadd.examples.consumer.StreamIdWatchList.WatchListEntry;
import com.refinitiv.eta.valueadd.reactor.*;

import static java.util.concurrent.TimeUnit.NANOSECONDS;

/**
 * <p>
 * This is a main class to run the ETA Value Add Consumer application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * The purpose of this application is to demonstrate consuming data from
 * an OMM Provider using Value Add components. It is a single-threaded
 * client application.
 * </p>
 * <p>
 * The consumer application implements callbacks that process information
 * received by the provider. It creates the Reactor, creates the desired
 * connections, then dispatches from the Reactor for events and messages.
 * Once it has received the event indicating that the channel is ready,
 * it will make the desired item requests (snapshot or streaming) to a
 * provider and appropriately processes the responses. The resulting decoded 
 * responses from the provided are displayed on the console.
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
 * Value Add and shows how using Value Add simplifies the writing of ETA
 * applications. Because Value Add is a layer on top of ETA, you may see a
 * slight decrease in performance compared to writing applications directly to
 * the ETA interfaces.
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
 * Linux: ./gradlew runVAConsumer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runVAConsumer -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-c specifies a connection to open and a list of items to request:
 * <ul>
 *  <li>hostname:        Hostname of provider to connect to
 *  <li>port:            Port of provider to connect to
 *  <li>service:         Name of service to request items from on this connection
 *  <li>domain:itemName: Domain and name of an item to request
 *       <br>A comma-separated list of these may be specified.
 *       <br>The domain may be any of: mp(MarketPrice), mbo(MarketByOrder), mbp(MarketByPrice), yc(YieldCurve), sl(SymbolList)
 *       <br>The domain may also be any of the private stream domains: mpps(MarketPrice PS), mbops(MarketByOrder PS), mbpps(MarketByPrice PS), ycps(YieldCurve PS)
 *       <br>Example Usage: -c localhost:14002 DIRECT_FEED mp:TRI,mp:GOOG,mpps:FB,mbo:MSFT,mbpps:IBM,sl
 *       <br>&nbsp;&nbsp;(for SymbolList requests, a name can be optionally specified)
 *  <li>-tunnel (optional) enables consumer to open tunnel stream and send basic text messages
 *  <li>-tsServiceName (optional) specifies the service name for tunnel stream messages (if not specified, the service name specified in -c/-tcp is used)"
 *  <li>-tsAuth (optional) specifies that consumer will request authentication when opening the tunnel stream. This applies to basic tunnel streams.
 *  <li>-tsDomain (optional) specifies the domain that consumer will use when opening the tunnel stream. This applies to basic tunnel streams.
 *  </li>
 * </ul>
 * </li>
 * <li>-uname changes the username used when logging into the provider
 *
 * <li>-passwd changes the password used when logging into the provider
 *
 * <li>-clientId specifies a unique ID for application making the request to LDP token service, also known as AppKey generated using an AppGenerator
 *
 * <li>-sessionMgnt enables the session management in the Reactor
 *
 * <li>-view specifies each request using a basic dynamic view
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
 * <li>-statisticInterval number of seconds between displaying reactor channel statistics, must greater than 0
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
 * <li>-at Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.
 *
 * <li>-ax Specifies the Authentication Extended information.
 *
 * <li>-aid Specifies the Application ID.
 *
 * <li>-rtt enables rtt support by a consumer. If provider make distribution of RTT messages, consumer will return back them. In another case, consumer will ignore them.
 *
 * </ul>
 */
public class Consumer implements ConsumerCallback, ReactorAuthTokenEventCallback, ReactorOAuthCredentialEventCallback,
		ReactorJsonConversionEventCallback, ReactorServiceNameToIdCallback
{
	private final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
	private final String ENUM_TABLE_FILE_NAME = "enumtype.def";

	private Reactor reactor;
	private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
	private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
	private ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
	private ReactorJsonConverterOptions jsonConverterOptions = ReactorFactory.createReactorJsonConverterOptions();
	private ConsumerCmdLineParser consumerCmdLineParser = new ConsumerCmdLineParser();
	private Selector selector;

	private long runtime;

	ReactorChannelStats reactorChannelStats = ReactorFactory.createReactorChannelStats();

	// default server host name
	private static final String defaultSrvrHostname = "localhost";

	// default server port number
	private static final String defaultSrvrPortNo = "14002";

	// default service name
	private static final String defaultServiceName = "DIRECT_FEED";

	// default item name
	private static final String defaultItemName = "TRI.N";

	// default item name 2
	private static final String defaultItemName2 = ".DJI";

	private Error error;    // error information

	private DataDictionary dictionary;

	private boolean fieldDictionaryLoadedFromFile;
	private boolean enumTypeDictionaryLoadedFromFile;

	ArrayList<ChannelInfo> chnlInfoList = new ArrayList<ChannelInfo>();

	private TunnelStreamHandler tunnelStreamHandler;
	private String tsServiceName;

	long cacheTime;
	long cacheInterval;
	long statisticTime;
	long statisticInterval;
	StringBuilder cacheDisplayStr;
	Buffer cacheEntryBuffer;

	boolean _finalStatusEvent;
	private long closetime;
	private long closeRunTime;
	boolean closeHandled;

	private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	private Map<ReactorChannel, Integer> socketFdValueMap = new HashMap<>();

	public Consumer()
	{
		dictionary = CodecFactory.createDataDictionary();

		error = TransportFactory.createError();
		dispatchOptions.maxMessages(1);
		_finalStatusEvent = true;
		closetime = 10; // 10 sec
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
		// parse command line
		if (!consumerCmdLineParser.parseArgs(args))
		{
			System.err.println("\nError loading command line arguments:\n");
			consumerCmdLineParser.printUsage();
			System.exit(CodecReturnCodes.FAILURE);
		}

		// add default connections to arguments if none specified
		if (consumerCmdLineParser.connectionList().size() == 0 &&
			!consumerCmdLineParser.enableSessionMgnt())
		{
			// first connection - localhost:14002 DIRECT_FEED mp:TRI.N
			List<ItemArg> itemList = new ArrayList<ItemArg>();
			ItemArg itemArg = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName, false);
			itemList.add(itemArg);
			ConnectionArg connectionArg = new ConnectionArg(ConnectionTypes.SOCKET,
					defaultServiceName,
					defaultSrvrHostname,
					defaultSrvrPortNo,
					itemList);
			consumerCmdLineParser.connectionList().add(connectionArg);

			// second connection - localhost:14002 DIRECT_FEED mp:TRI.N mp:.DJI
			List<ItemArg> itemList2 = new ArrayList<ItemArg>();
			ItemArg itemArg2 = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName2, false);
			itemList2.add(itemArg);
			itemList2.add(itemArg2);
			ConnectionArg connectionArg2 = new ConnectionArg(ConnectionTypes.SOCKET,
					defaultServiceName,
					defaultSrvrHostname,
					defaultSrvrPortNo,
					itemList2);
			consumerCmdLineParser.connectionList().add(connectionArg2);
		}

		// display product version information
		System.out.println(Codec.queryVersion().toString());
		System.out.println("Consumer initializing...");

		runtime = System.currentTimeMillis() + consumerCmdLineParser.runtime() * 1000;
		closeRunTime = System.currentTimeMillis() + (consumerCmdLineParser.runtime() + closetime) * 1000;

		// load dictionary
		loadDictionary();

		// enable Reactor XML tracing if specified
		if (consumerCmdLineParser.enableXmlTracing())
		{
			reactorOptions.enableXmlTracing();
		}

		cacheInterval = consumerCmdLineParser.cacheInterval();
		cacheTime = System.currentTimeMillis() + cacheInterval*1000;

		statisticInterval = consumerCmdLineParser.statisticInterval();
		statisticTime = System.currentTimeMillis() + statisticInterval*1000;

		// Set reactor statistics to keep track of
		if(statisticInterval > 0)
		{
			reactorOptions.statistics(ReactorOptions.StatisticFlags.READ | ReactorOptions.StatisticFlags.WRITE | ReactorOptions.StatisticFlags.PING);
		}
		
		// Set Token Generator URLs
		if(consumerCmdLineParser.tokenURLV1() != null && !consumerCmdLineParser.tokenURLV1().equals(""))
		{
			reactorOptions.tokenServiceURL_V1().data(consumerCmdLineParser.tokenURLV1());
		}
		
		if(consumerCmdLineParser.tokenURLV2() != null && !consumerCmdLineParser.tokenURLV2().equals(""))
		{
			reactorOptions.tokenServiceURL_V2().data(consumerCmdLineParser.tokenURLV2());
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
		
		int index = 0;
		for (ConnectionArg connectionArg : consumerCmdLineParser.connectionList())
		{
			// create channel info
			ChannelInfo chnlInfo = new ChannelInfo();
			chnlInfo.connectionArg = connectionArg;
			
			//APIQA
        	chnlInfo.consumerRole.initDefaultRDMLoginRequest();
        	
        	if (index==0)
        	{
        		// connection1 is for STS
        		oAuthCredential.clear();
                if (consumerCmdLineParser.userName1() != null && !consumerCmdLineParser.userName1().equals(""))
                {
                	LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                    loginRequest.userName().data(consumerCmdLineParser.userName1());
                }
                if (consumerCmdLineParser.passwd1() != null)
                {
                	LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                    loginRequest.password().data(consumerCmdLineParser.passwd1());
                    loginRequest.applyHasPassword();
                    oAuthCredential.password().data(consumerCmdLineParser.passwd1());
                    /* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
        			oAuthCredential.reactorOAuthCredentialEventCallback(this);
                }
                if (consumerCmdLineParser.clientId1_1() != null && !consumerCmdLineParser.clientId1_1().equals(""))
                {
                	oAuthCredential.clientId().data(consumerCmdLineParser.clientId1_1());
                	oAuthCredential.takeExclusiveSignOnControl(consumerCmdLineParser.takeExclusiveSignOnControl());
                }
        	}
        	else if (index==1)
        	{
        		// connection2 is for STS
        		oAuthCredential.clear();
                if (consumerCmdLineParser.userName2() != null && !consumerCmdLineParser.userName2().equals(""))
                {
                	LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                    loginRequest.userName().data(consumerCmdLineParser.userName2());
                }
                if (consumerCmdLineParser.passwd2() != null)
                {
                	LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                    loginRequest.password().data(consumerCmdLineParser.passwd2());
                    loginRequest.applyHasPassword();
                    oAuthCredential.password().data(consumerCmdLineParser.passwd2());
                    /* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
        			oAuthCredential.reactorOAuthCredentialEventCallback(this);
                }
                if (consumerCmdLineParser.clientId1_2() != null && !consumerCmdLineParser.clientId1_2().equals(""))
                {
                	oAuthCredential.clientId().data(consumerCmdLineParser.clientId1_2());
                	oAuthCredential.takeExclusiveSignOnControl(consumerCmdLineParser.takeExclusiveSignOnControl());
                }
        	}
        	else if (index==2)
        	{
        		// connection3 is for STS
        		oAuthCredential.clear();
                if (consumerCmdLineParser.userName3() != null && !consumerCmdLineParser.userName3().equals(""))
                {
                	LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                    loginRequest.userName().data(consumerCmdLineParser.userName3());
                }
                if (consumerCmdLineParser.passwd3() != null)
                {
                	LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
                    loginRequest.password().data(consumerCmdLineParser.passwd3());
                    loginRequest.applyHasPassword();
                    oAuthCredential.password().data(consumerCmdLineParser.passwd3());
                    /* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
        			oAuthCredential.reactorOAuthCredentialEventCallback(this);
                }
                if (consumerCmdLineParser.clientId1_3() != null && !consumerCmdLineParser.clientId1_3().equals(""))
                {
                	oAuthCredential.clientId().data(consumerCmdLineParser.clientId1_3());
                	oAuthCredential.takeExclusiveSignOnControl(consumerCmdLineParser.takeExclusiveSignOnControl());
                }
        	}
        	else if (index==3)
        	{
        		 // connection4 is for OAuthV2 
        		 oAuthCredential.clear();
        		 if (consumerCmdLineParser.clientId2_1() != null && !consumerCmdLineParser.clientId2_1().equals(""))
                 {
        			 oAuthCredential.clientSecret().data(consumerCmdLineParser.clientSecret1());
        			 oAuthCredential.clientId().data(consumerCmdLineParser.clientId2_1());
                     /* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
        			 oAuthCredential.reactorOAuthCredentialEventCallback(this);
                 }
        	}
        	else if (index==4)
        	{
        		 // connection5 is for OAuthV2 
        		 oAuthCredential.clear();
        		 if (consumerCmdLineParser.clientId2_2() != null && !consumerCmdLineParser.clientId2_2().equals(""))
                 {
        			 oAuthCredential.clientSecret().data(consumerCmdLineParser.clientSecret2());
        			 oAuthCredential.clientId().data(consumerCmdLineParser.clientId2_2());
                     /* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
        			 oAuthCredential.reactorOAuthCredentialEventCallback(this);
                 }
        	}
        	else
        	{
        		 // connection6 is for OAuthV2 
        		 oAuthCredential.clear();
        		 if (consumerCmdLineParser.clientId2_3() != null && !consumerCmdLineParser.clientId2_3().equals(""))
                 {
        			 oAuthCredential.clientSecret().data(consumerCmdLineParser.clientSecret3());
        			 oAuthCredential.clientId().data(consumerCmdLineParser.clientId2_3());
                     /* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. */
        			 oAuthCredential.reactorOAuthCredentialEventCallback(this);
                 }
        	}
			// initialize channel info        	
			initChannelInfo(chnlInfo);

			// connect channel
			int ret;
			if ((ret = reactor.connect(chnlInfo.connectOptions, chnlInfo.consumerRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
			{
				System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
				//API QA commented out to let app continue for success channel
				//System.exit(ReactorReturnCodes.FAILURE);
			} else {
	        	 System.out.println("QA reactor.connect SUCCESS for index:" + index);
	        }
			//APIQA
	        chnlInfo.consumerRole.watchlistOptions().enableWatchlist(true);
	        //END APIQA
			// add to ChannelInfo list
			chnlInfoList.add(chnlInfo);
			index ++;
		}

		jsonConverterOptions.dataDictionary(dictionary);
		jsonConverterOptions.serviceNameToIdCallback(this);
		jsonConverterOptions.jsonConversionEventCallback(this);

		// Initialize the JSON converter
		if ( reactor.initJsonConverter(jsonConverterOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
		{
			System.out.println("Reactor.initJsonConverter() failed: " + errorInfo.toString());
			System.exit(ReactorReturnCodes.FAILURE);
		}
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

			long currentTime = System.currentTimeMillis();
			if (currentTime >= cacheTime && cacheInterval > 0)
			{
				cacheTime = System.currentTimeMillis() + cacheInterval*1000;

				for (ChannelInfo chnlInfo : chnlInfoList)
				{
					if (chnlInfo.cacheInfo.useCache)
						displayCache(chnlInfo);
				}

				cacheTime = currentTime + cacheInterval*1000;
			}

			if (currentTime >= statisticTime && statisticInterval > 0)
			{
				statisticTime = System.currentTimeMillis() + statisticInterval*1000;

				ChannelInfo chnlInfo = chnlInfoList.get(0);

				if(reactorOptions.statistics() != 0 && chnlInfo.reactorChannel != null)
					displayReactorChannelStats(chnlInfo);

				statisticTime = currentTime + statisticInterval*1000;
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

			// Handle run-time
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
				handleTunnelStream();

				// send login reissue if login reissue time has passed
				for (ChannelInfo chnlInfo : chnlInfoList)
				{
					if (chnlInfo.reactorChannel == null ||
						(chnlInfo.reactorChannel.state() != ReactorChannel.State.UP &&
						 chnlInfo.reactorChannel.state() != ReactorChannel.State.READY))
					{
						continue;
					}

					if (chnlInfo.canSendLoginReissue && (consumerCmdLineParser.enableSessionMgnt() == false) &&
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

			if(closeHandled && tunnelStreamHandler != null && tunnelStreamHandler._chnlInfo != null &&
			   !tunnelStreamHandler._chnlInfo.isTunnelStreamUp)
				break;
		}
	}

	@Override
	public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event)
	{
		if (event.errorInfo().code() != ReactorReturnCodes.SUCCESS)
		{
			System.out.println("Retrive an access token failed. Text: " + event.errorInfo().toString());
		}
		else
		{
			ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();

			if (chnlInfo.reactorChannel != null &&
				(chnlInfo.reactorChannel.state() == ReactorChannel.State.UP ||
				 chnlInfo.reactorChannel.state() == ReactorChannel.State.READY))
			{
				LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
				loginRequest.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
				loginRequest.userName().data(event.reactorAuthTokenInfo().accessToken());
				// Do not send the password
				loginRequest.flags(loginRequest.flags() & ~LoginRequestFlags.HAS_PASSWORD);
				loginRequest.applyNoRefresh();

				submitOptions.clear();

				if (chnlInfo.reactorChannel.submit(loginRequest, submitOptions, errorInfo) != CodecReturnCodes.SUCCESS)
				{
					System.out.println("Login reissue failed. Error: " + errorInfo.error().text());
				}
				else
				{
					System.out.println("Login reissue sent");
				}
			}
		}
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int reactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent)
	{
		ReactorOAuthCredentialRenewalOptions renewalOptions = ReactorFactory.createReactorOAuthCredentialRenewalOptions();
		ReactorOAuthCredentialRenewal oAuthCredentialRenewal = ReactorFactory.createReactorOAuthCredentialRenewal();
		ReactorOAuthCredential reactorOAuthCredential = (ReactorOAuthCredential)reactorOAuthCredentialEvent.userSpecObj();

		renewalOptions.renewalModes(ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD);
		oAuthCredentialRenewal.password().data(reactorOAuthCredential.password().toString());

		reactorOAuthCredentialEvent.reactor().submitOAuthCredentialRenewal(renewalOptions, oAuthCredentialRenewal, errorInfo);

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int reactorChannelEventCallback(ReactorChannelEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();

		switch(event.eventType())
		{
			case ReactorChannelEventTypes.CHANNEL_UP:
			{
				if (event.reactorChannel().selectableChannel() != null)
					System.out.println("Channel Up Event: " + event.reactorChannel().selectableChannel());
				else
					System.out.println("Channel Up Event");

				//define socket id of consumer's channel
				//define new socket fd value
				final int fdSocketId =
						ChannelHelper.defineFdValueOfSelectableChannel(event.reactorChannel().channel().selectableChannel());
				socketFdValueMap.put(event.reactorChannel(), fdSocketId);

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

				//define new socket id of consumer's channel
				final int fdSocketId =
						ChannelHelper.defineFdValueOfSelectableChannel(event.reactorChannel().channel().selectableChannel());
				socketFdValueMap.put(event.reactorChannel(), fdSocketId);

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
				if (event.reactorChannel().selectableChannel() != null)
					System.out.println("Channel Ready Event: " + event.reactorChannel().selectableChannel());
				else
					System.out.println("Channel Ready Event");

				if (isRequestedServiceUp(chnlInfo))
				{
					checkAndInitPostingSupport(chnlInfo);

					if ( !chnlInfo.itemWatchList.isEmpty() )
					{
						chnlInfo.itemWatchList.clear();
						if (chnlInfo.cacheInfo.cache != null)
							chnlInfo.cacheInfo.cache.clear();
					}

					sendMPRequests(chnlInfo);
					sendMBORequests(chnlInfo);
					sendMBPRequests(chnlInfo);
					sendSymbolListRequests(chnlInfo);
					sendYieldCurveRequests(chnlInfo);
					chnlInfo.requestsSent = true;
				}

				if (isRequestedTunnelStreamServiceUp(chnlInfo))
				{
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
			case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			{
				if (event.reactorChannel().selectableChannel() != null)
					System.out.println("\nConnection down reconnecting: Channel " + event.reactorChannel().selectableChannel());
				else
					System.out.println("\nConnection down reconnecting");

				if (event.errorInfo() != null && event.errorInfo().error().text() != null)
					System.out.println("	Error text: " + event.errorInfo().error().text() + "\n");

				// allow Reactor to perform connection recovery

				// unregister selectableChannel from Selector
				if (event.reactorChannel().selectableChannel() != null)
				{
					SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
					if (key != null)
						key.cancel();
				}

				// reset dictionary if not loaded from file
				if (fieldDictionaryLoadedFromFile == false &&
					enumTypeDictionaryLoadedFromFile == false)
				{
					if (chnlInfo.dictionary != null)
					{
						chnlInfo.dictionary.clear();
					}
				}

				// reset item request(s) sent flag
				chnlInfo.requestsSent = false;

				// reset hasServiceInfo flag
				chnlInfo.hasServiceInfo = false;
				chnlInfo.hasTunnelStreamServiceInfo = false;

				// reset canSendLoginReissue flag
				chnlInfo.canSendLoginReissue = false;

				setItemState(chnlInfo, StreamStates.CLOSED_RECOVER, DataStates.SUSPECT, StateCodes.NONE );

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
				System.out.println("Received ReactorChannel WARNING event.");
				if (event.errorInfo() != null && event.errorInfo().error().text() != null)
					System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");

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

		// set response message
		chnlInfo.responseMsg = msg;

		// set-up decode iterator if message has message body
		if (msg.encodedDataBody() != null && msg.encodedDataBody().data() != null)
		{
			// clear decode iterator
			chnlInfo.dIter.clear();

			// set buffer and version info
			chnlInfo.dIter.setBufferAndRWFVersion(msg.encodedDataBody(),
					event.reactorChannel().majorVersion(),
					event.reactorChannel().minorVersion());
		}

		processResponse(chnlInfo);

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

				// set login stream id in MarketPriceHandler and YieldCurveHandler
				chnlInfo.marketPriceHandler.loginStreamId(event.rdmLoginMsg().streamId());
				chnlInfo.yieldCurveHandler.loginStreamId(event.rdmLoginMsg().streamId());

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
			case RTT:
				LoginRTT loginRTT = (LoginRTT) event.rdmLoginMsg();
				System.out.printf("\nReceived login RTT message from Provider %d.\n", socketFdValueMap.get(event.reactorChannel()));
				System.out.printf("\tTicks: %du\n", NANOSECONDS.toMicros(loginRTT.ticks()));
				if (loginRTT.checkHasRTLatency()) {
					System.out.printf("\tLast Latency: %du\n", NANOSECONDS.toMicros(loginRTT.rtLatency()));
				}
				if (loginRTT.checkHasTCPRetrans()) {
					System.out.printf("\tProvider side TCP Retransmissions: %du\n", loginRTT.tcpRetrans());
				}
				System.out.println("RTT Response sent to provider by reactor.\n");
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
				processServiceRefresh(directoryRefresh, chnlInfo);
				if (chnlInfo.serviceInfo.action() == MapEntryActions.DELETE)
				{
					error.text("rdmDirectoryMsgCallback(): DirectoryRefresh Failed: directory service is deleted");
					return ReactorCallbackReturnCodes.SUCCESS;
				}
				break;
			case UPDATE:
				DirectoryUpdate directoryUpdate = (DirectoryUpdate)event.rdmDirectoryMsg();
				processServiceUpdate(directoryUpdate, chnlInfo);
				if (chnlInfo.serviceInfo.action() == MapEntryActions.DELETE)
				{
					error.text("rdmDirectoryMsgCallback(): DirectoryUpdate Failed: directory service is deleted");
					return ReactorCallbackReturnCodes.SUCCESS;
				}
				if (isRequestedServiceUp(chnlInfo) && !chnlInfo.requestsSent)
				{
					checkAndInitPostingSupport(chnlInfo);

					if ( !chnlInfo.itemWatchList.isEmpty() )
					{
						chnlInfo.itemWatchList.clear();
						if (chnlInfo.cacheInfo.cache != null)
							chnlInfo.cacheInfo.cache.clear();
					}

					sendMPRequests(chnlInfo);
					sendMBORequests(chnlInfo);
					sendMBPRequests(chnlInfo);
					sendSymbolListRequests(chnlInfo);
					sendYieldCurveRequests(chnlInfo);
					chnlInfo.requestsSent = true;
				}
				if (isRequestedTunnelStreamServiceUp(chnlInfo))
				{
					if ((tunnelStreamHandler != null && tunnelStreamHandler._chnlInfo != null && !tunnelStreamHandler._chnlInfo.isTunnelStreamUp) ||
						(tunnelStreamHandler != null && tunnelStreamHandler._chnlInfo == null))
					{
						if (tunnelStreamHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
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
				}

				break;
			case CLOSE:
				System.out.println("Received Source Directory Close");
				break;
			case STATUS:
				DirectoryStatus directoryStatus = (DirectoryStatus)event.rdmDirectoryMsg();
				System.out.println("\nReceived Source Directory StatusMsg");
				if (directoryStatus.checkHasState())
				{
					System.out.println("	" + directoryStatus.state());
				}
				break;
			default:
				System.out.println("Received Unhandled Source Directory Msg Type: " + msgType);
				break;
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
			chnlInfo.dictionary = dictionary;
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
							chnlInfo.fieldDictionaryStreamId = dictionaryRefresh.streamId();
							break;
						case Dictionary.Types.ENUM_TABLES:
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
							if (chnlInfo.cacheInfo.useCache)
								initializeCacheDictionary(chnlInfo.cacheInfo,chnlInfo.dictionary);

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
				break;
			case STATUS:
				System.out.println("Received Dictionary StatusMsg");
				break;
			default:
				System.out.println("Received Unhandled Dictionary Msg Type: " + msgType);
				break;
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId,
											  ReactorServiceNameToIdEvent serviceNameToIdEvent)
	{
		ChannelInfo chnlInfo = (ChannelInfo)serviceNameToIdEvent.reactorChannel().userSpecObj();

		/* Checks whether the service name is used by the channel. */
		if(chnlInfo.serviceInfo.checkHasInfo() &&
		   serviceNameToId.serviceName().equals(chnlInfo.serviceInfo.info().serviceName().toString()))
		{
			serviceNameToId.serviceId(chnlInfo.serviceInfo.serviceId());
			return CodecReturnCodes.SUCCESS;
		}
		else
		{
			return CodecReturnCodes.FAILURE;
		}
	}

	@Override
	public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent)
	{
		System.out.println("JSON Conversion error: " + jsonConversionEvent.error().text());

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	private void processServiceRefresh(DirectoryRefresh directoryRefresh, ChannelInfo chnlInfo)
	{
		String serviceName = chnlInfo.connectionArg.service();
		System.out.println("Received Source Directory Refresh");
		System.out.println(directoryRefresh.toString());
		for (Service service : directoryRefresh.serviceList())
		{
			if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() )
			{
				chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
			}

			if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.tsServiceInfo.serviceId() )
			{
				chnlInfo.tsServiceInfo.action(MapEntryActions.DELETE);
			}

			if(service.info().serviceName().toString() != null)
			{
				System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
				// cache service requested by the application
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
					setItemState(chnlInfo, service.state().status().streamState(), service.state().status().dataState(),
							service.state().status().code() );
				}
				if (service.info().serviceName().toString().equals(tsServiceName))
				{
					// save serviceInfo associated with requested service name
					if (service.copy(chnlInfo.tsServiceInfo) < CodecReturnCodes.SUCCESS)
					{
						System.out.println("Service.copy() failure");
						uninitialize();
						System.exit(ReactorReturnCodes.FAILURE);
					}

					chnlInfo.hasTunnelStreamServiceInfo = true;
				}
			}
		}
	}

	private void processServiceUpdate(DirectoryUpdate directoryUpdate, ChannelInfo chnlInfo)
	{
		String serviceName = chnlInfo.connectionArg.service();
		String tsServiceName = chnlInfo.connectionArg.tsService();
		System.out.println("Received Source Directory Update");
		System.out.println(directoryUpdate.toString());
		for (Service service : directoryUpdate.serviceList())
		{
			if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() )
			{
				chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
			}

			if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.tsServiceInfo.serviceId() )
			{
				chnlInfo.tsServiceInfo.action(MapEntryActions.DELETE);
			}

			boolean updateServiceInfo = false, updateTSServiceInfo = false;
			if(service.info().serviceName().toString() != null)
			{
				System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
				// update service cache - assume cache is built with previous refresh message
				if (service.info().serviceName().toString().equals(serviceName) ||
					service.serviceId() == chnlInfo.serviceInfo.serviceId())
				{
					updateServiceInfo = true;
				}
				if (service.info().serviceName().toString().equals(tsServiceName) ||
					service.serviceId() == chnlInfo.tsServiceInfo.serviceId())
				{
					updateTSServiceInfo = true;
				}
			}
			else
			{
				if (service.serviceId() == chnlInfo.serviceInfo.serviceId())
				{
					updateServiceInfo = true;
				}
				if (service.serviceId() == chnlInfo.tsServiceInfo.serviceId())
				{
					updateTSServiceInfo = true;
				}
			}

			if (updateServiceInfo)
			{
				// update serviceInfo associated with requested service name
				if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("Service.copy() failure");
					uninitialize();
					System.exit(ReactorReturnCodes.FAILURE);
				}
				chnlInfo.hasServiceInfo = true;
				setItemState(chnlInfo, service.state().status().streamState(), service.state().status().dataState(),
						service.state().status().code() );
			}
			if (updateTSServiceInfo)
			{
				// update serviceInfo associated with requested service name
				if (service.copy(chnlInfo.tsServiceInfo) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("Service.copy() failure");
					uninitialize();
					System.exit(ReactorReturnCodes.FAILURE);
				}

				chnlInfo.hasTunnelStreamServiceInfo = true;
			}
		}
	}

	public boolean isRequestedServiceUp(ChannelInfo chnlInfo)
	{
		return  chnlInfo.hasServiceInfo &&
				chnlInfo.serviceInfo.checkHasState() && (!chnlInfo.serviceInfo.state().checkHasAcceptingRequests() ||
														 chnlInfo.serviceInfo.state().acceptingRequests() == 1) && chnlInfo.serviceInfo.state().serviceState() == 1;
	}

	public boolean isRequestedTunnelStreamServiceUp(ChannelInfo chnlInfo)
	{
		return  chnlInfo.hasTunnelStreamServiceInfo &&
				chnlInfo.tsServiceInfo.checkHasState() && (!chnlInfo.tsServiceInfo.state().checkHasAcceptingRequests() ||
														   chnlInfo.tsServiceInfo.state().acceptingRequests() == 1) && chnlInfo.tsServiceInfo.state().serviceState() == 1;
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

		if ( consumerCmdLineParser.publisherId() != null && consumerCmdLineParser.publisherAddress() != null)
			chnlInfo.postHandler.setPublisherInfo(consumerCmdLineParser.publisherId(), consumerCmdLineParser.publisherAddress());

		// This sets up our basic timing so post messages will be sent
		// periodically
		chnlInfo.postHandler.initPostHandler();
	}

	// on and off stream posting if enabled
	private void handlePosting()
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

			if (chnlInfo.postHandler.enableOnstreamPost())
			{
				chnlInfo.postItemName.clear();
				int postStreamId = chnlInfo.marketPriceHandler.getFirstItem(chnlInfo.postItemName);
				if (postStreamId == 0 || chnlInfo.postItemName.length() == 0)
				{
					return;
				}
				chnlInfo.postHandler.streamId(postStreamId);
				chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
				chnlInfo.postHandler.dictionary(chnlInfo.dictionary);
				chnlInfo.postHandler.postItemName().data(chnlInfo.postItemName.data(), chnlInfo.postItemName.position(), chnlInfo.postItemName.length());

				int ret = chnlInfo.postHandler.handlePosts(chnlInfo.reactorChannel, errorInfo);
				if (ret < CodecReturnCodes.SUCCESS)
					System.out.println("Error posting onstream: " + error.text());
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

	private void processResponse(ChannelInfo chnlInfo)
	{
		switch (chnlInfo.responseMsg.domainType())
		{
			case DomainTypes.MARKET_PRICE:
				System.out.println("(Channel " + chnlInfo.reactorChannel.selectableChannel() + "):");
				processMarketPriceResp(chnlInfo);
				break;
			case DomainTypes.MARKET_BY_ORDER:
				System.out.print("(Channel " + chnlInfo.reactorChannel.selectableChannel() + "):");
				processMarketByOrderResp(chnlInfo);
				break;
			case DomainTypes.MARKET_BY_PRICE:
				System.out.print("(Channel " + chnlInfo.reactorChannel.selectableChannel() + "):");
				processMarketByPriceResp(chnlInfo);
				break;
			case DomainTypes.SYMBOL_LIST:
				System.out.println("(Channel " + chnlInfo.reactorChannel.selectableChannel() + "):");
				processSymbolListResp(chnlInfo);
				break;
			case DomainTypes.YIELD_CURVE:
				System.out.println("(Channel " + chnlInfo.reactorChannel.selectableChannel() + "):");
				processYieldCurveResp(chnlInfo);
				break;
			default:
				System.out.println("Unhandled Domain Type: " + chnlInfo.responseMsg.domainType());
				break;
		}
	}

	private void processSymbolListResp(ChannelInfo chnlInfo)
	{
		if (chnlInfo.symbolListHandler.processResponse(chnlInfo.responseMsg,
				chnlInfo.dIter,
				chnlInfo.dictionary) != CodecReturnCodes.SUCCESS)
		{
			System.out.println(errorInfo.error().text());
			uninitialize();
			System.exit(ReactorReturnCodes.FAILURE);
		}
	}

	private void processMarketByPriceResp(ChannelInfo chnlInfo)
	{
		if (chnlInfo.marketByPriceHandler.processResponse(chnlInfo.responseMsg,
				chnlInfo.dIter,
				chnlInfo.dictionary,
				chnlInfo.cacheInfo,
				errorInfo) != CodecReturnCodes.SUCCESS)
		{
			System.out.println(errorInfo.error().text());
			uninitialize();
			System.exit(ReactorReturnCodes.FAILURE);
		}
	}

	private void processMarketByOrderResp(ChannelInfo chnlInfo)
	{
		if (chnlInfo.marketByOrderHandler.processResponse(chnlInfo.responseMsg,
				chnlInfo.dIter,
				chnlInfo.dictionary,
				chnlInfo.cacheInfo,
				errorInfo) != CodecReturnCodes.SUCCESS)
		{
			System.out.println(errorInfo.error().text());
			uninitialize();
			System.exit(ReactorReturnCodes.FAILURE);
		}
	}

	private void processMarketPriceResp(ChannelInfo chnlInfo)
	{
		if (chnlInfo.marketPriceHandler.processResponse(chnlInfo.responseMsg,
				chnlInfo.dIter,
				chnlInfo.dictionary,
				chnlInfo.cacheInfo,
				errorInfo) != CodecReturnCodes.SUCCESS)
		{
			System.out.println(errorInfo.error().text());
			uninitialize();
			System.exit(ReactorReturnCodes.FAILURE);
		}
	}

	private void processYieldCurveResp(ChannelInfo chnlInfo)
	{
		if (chnlInfo.yieldCurveHandler.processResponse(chnlInfo.responseMsg,
				chnlInfo.dIter,
				chnlInfo.dictionary,
				chnlInfo.cacheInfo,
				errorInfo) != CodecReturnCodes.SUCCESS)
		{
			System.out.println(errorInfo.error().text());
			uninitialize();
			System.exit(ReactorReturnCodes.FAILURE);
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
		}
		else
		{
			fieldDictionaryLoadedFromFile = true;
		}

		if (dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
		{
			System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
							   + error.text());
		}
		else
		{
			enumTypeDictionaryLoadedFromFile = true;
		}
	}

	private void initChannelInfo(ChannelInfo chnlInfo)
	{
		// set up consumer role
		chnlInfo.consumerRole.defaultMsgCallback(this);
		chnlInfo.consumerRole.channelEventCallback(this);
		chnlInfo.consumerRole.loginMsgCallback(this);
		chnlInfo.consumerRole.directoryMsgCallback(this);
		if (fieldDictionaryLoadedFromFile == false ||
			enumTypeDictionaryLoadedFromFile == false)
		{
			chnlInfo.consumerRole.dictionaryMsgCallback(this);
		}

		// initialize consumer role to default
		//API QA commented it out
		//chnlInfo.consumerRole.initDefaultRDMLoginRequest();
		chnlInfo.consumerRole.initDefaultRDMDirectoryRequest();

		/* API QA commented out
		// use command line login user name if specified
		if (consumerCmdLineParser.userName() != null && !consumerCmdLineParser.userName().equals(""))
		{
			LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
			loginRequest.userName().data(consumerCmdLineParser.userName());
		}
		if (consumerCmdLineParser.passwd() != null)
		{
			LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
			loginRequest.password().data(consumerCmdLineParser.passwd());
			loginRequest.applyHasPassword();

			oAuthCredential.password().data(consumerCmdLineParser.passwd());

			/* Specified the ReactorOAuthCredentialEventCallback to get sensitive information as needed to authorize with the token service. 
			oAuthCredential.reactorOAuthCredentialEventCallback(this);
		}
	
		if (consumerCmdLineParser.clientId() != null && !consumerCmdLineParser.clientId().equals(""))
		{
			oAuthCredential.clientId().data(consumerCmdLineParser.clientId());
			
			if(consumerCmdLineParser.clientSecret() != null && !consumerCmdLineParser.clientSecret().equals(""))
			{
				oAuthCredential.clientSecret().data(consumerCmdLineParser.clientSecret());
			}
			else
			{
				oAuthCredential.takeExclusiveSignOnControl(consumerCmdLineParser.takeExclusiveSignOnControl());
			}
		}
		// END API QA */
				
		oAuthCredential.userSpecObj(oAuthCredential);
		chnlInfo.consumerRole.reactorOAuthCredential(oAuthCredential);

		// use command line authentication token and extended authentication information if specified
		if (consumerCmdLineParser.authenticationToken() != null && !consumerCmdLineParser.authenticationToken().equals(""))
		{
			LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
			loginRequest.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
			loginRequest.userName().data(consumerCmdLineParser.authenticationToken());

			if (consumerCmdLineParser.authenticationExtended() != null && !consumerCmdLineParser.authenticationExtended().equals(""))
			{
				loginRequest.applyHasAuthenticationExtended();
				loginRequest.authenticationExtended().data(consumerCmdLineParser.authenticationExtended());
			}
		}

		// use command line application id if specified
		if (consumerCmdLineParser.applicationId() != null && !consumerCmdLineParser.applicationId().equals(""))
		{
			LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
			loginRequest.attrib().applicationId().data(consumerCmdLineParser.applicationId());
		}

		if (consumerCmdLineParser.enableRtt()) {
			chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasSupportRoundTripLatencyMonitoring();
		}

		// if unable to load from file, enable consumer to download dictionary
		if (fieldDictionaryLoadedFromFile == false ||
			enumTypeDictionaryLoadedFromFile == false)
		{
			chnlInfo.consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
			dictionary = CodecFactory.createDataDictionary(); //drop the old dictionary
		}

		chnlInfo.dictionary = dictionary;

		chnlInfo.shouldOffStreamPost = consumerCmdLineParser.enableOffpost();
		// this application requires at least one market price item to be
		// requested for on-stream posting to be performed
		chnlInfo.shouldOnStreamPost = consumerCmdLineParser.enablePost();
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
		chnlInfo.marketPriceHandler.snapshotRequest(consumerCmdLineParser.enableSnapshot());
		chnlInfo.marketByOrderHandler.snapshotRequest(consumerCmdLineParser.enableSnapshot());
		chnlInfo.marketByPriceHandler.snapshotRequest(consumerCmdLineParser.enableSnapshot());
		chnlInfo.yieldCurveHandler.snapshotRequest(consumerCmdLineParser.enableSnapshot());
		chnlInfo.symbolListHandler.snapshotRequest(consumerCmdLineParser.enableSnapshot());
		chnlInfo.marketPriceHandler.viewRequest(consumerCmdLineParser.enableView());
		// create item lists from those specified on command line
		createItemLists(chnlInfo);

		// set up reactor connect options
		chnlInfo.connectOptions.reconnectAttemptLimit(-1); // attempt to recover forever
		chnlInfo.connectOptions.reconnectMinDelay(1000); // 1 second minimum
		chnlInfo.connectOptions.reconnectMaxDelay(60000); // 60 second maximum
		chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
		chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
		chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(chnlInfo.connectionArg.connectionType());

		if (consumerCmdLineParser.enableSessionMgnt())
		{
			chnlInfo.connectOptions.connectionList().get(0).enableSessionManagement(true);
			// register for authentication callback
			chnlInfo.connectOptions.connectionList().get(0).reactorAuthTokenEventCallback(this);
		}

		chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(chnlInfo.connectionArg.port());
		chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(chnlInfo.connectionArg.hostname());

		chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);
		chnlInfo.connectOptions.connectionList().get(0).connectOptions().guaranteedOutputBuffers(1000);
		// add backup connection if specified
		if (consumerCmdLineParser.backupHostname() != null && consumerCmdLineParser.backupPort() != null)
		{
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			chnlInfo.connectOptions.connectionList().add(connectInfo);
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().majorVersion(Codec.majorVersion());
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().minorVersion(Codec.minorVersion());
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().connectionType(chnlInfo.connectionArg.connectionType());
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName(consumerCmdLineParser.backupPort());
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().unifiedNetworkInfo().address(consumerCmdLineParser.backupHostname());
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().userSpecObject(chnlInfo);
			chnlInfo.connectOptions.connectionList().get(1).connectOptions().guaranteedOutputBuffers(1000);

			if (consumerCmdLineParser.enableSessionMgnt())
			{
				chnlInfo.connectOptions.connectionList().get(1).enableSessionManagement(true);
				// register for authentication callback
				chnlInfo.connectOptions.connectionList().get(1).reactorAuthTokenEventCallback(this);

				ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(1).connectOptions();
				cOpt.connectionType(ConnectionTypes.ENCRYPTED);
				cOpt.tunnelingInfo().tunnelingType("encrypted");
				setEncryptedConfiguration(cOpt);
			}
		}

		// handler encrypted or http connection
		chnlInfo.shouldEnableEncrypted = consumerCmdLineParser.enableEncrypted();
		chnlInfo.shouldEnableHttp = consumerCmdLineParser.enableHttp();
		chnlInfo.shouldEnableWebsocket = consumerCmdLineParser.enableWebsocket();

		if (chnlInfo.shouldEnableEncrypted)
		{
			ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
			cOpt.connectionType(ConnectionTypes.ENCRYPTED);
			cOpt.encryptionOptions().connectionType(consumerCmdLineParser.encryptedConnectionType());

			if(consumerCmdLineParser.encryptedConnectionType() == ConnectionTypes.WEBSOCKET)
			{
				cOpt.wSocketOpts().protocols(consumerCmdLineParser.protocolList());
			}

			setEncryptedConfiguration(cOpt);
		}
		else if (chnlInfo.shouldEnableWebsocket)
		{
			ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
			cOpt.connectionType(ConnectionTypes.WEBSOCKET);
			cOpt.wSocketOpts().protocols(consumerCmdLineParser.protocolList());
		}
		else if (chnlInfo.shouldEnableHttp)
		{
			ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
			cOpt.connectionType(ConnectionTypes.HTTP);
			cOpt.tunnelingInfo().tunnelingType("http");
			setHTTPConfiguration(cOpt);
		}

		/* Setup proxy if configured */
		if (consumerCmdLineParser.enableProxy())
		{
			String proxyHostName = consumerCmdLineParser.proxyHostname();
			if ( proxyHostName == null)
			{
				System.err.println("Error: Proxy hostname not provided.");
				System.exit(CodecReturnCodes.FAILURE);
			}
			String proxyPort = consumerCmdLineParser.proxyPort();
			if ( proxyPort == null)
			{
				System.err.println("Error: Proxy port number not provided.");
				System.exit(CodecReturnCodes.FAILURE);
			}


			chnlInfo.connectOptions.connectionList().get(0).connectOptions().tunnelingInfo().HTTPproxy(true);
			chnlInfo.connectOptions.connectionList().get(0).connectOptions().tunnelingInfo().HTTPproxyHostName(proxyHostName);
			try
			{
				chnlInfo.connectOptions.connectionList().get(0).connectOptions().tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort));
			}
			catch(Exception e)
			{
				System.err.println("Error: Proxy port number not provided.");
				System.exit(CodecReturnCodes.FAILURE);
			}
			// credentials
			if (chnlInfo.connectOptions.connectionList().get(0).connectOptions().tunnelingInfo().HTTPproxy())
			{
				setCredentials(chnlInfo.connectOptions.connectionList().get(0).connectOptions());
			}
		}

		// handle basic tunnel stream configuration
		if (chnlInfo.connectionArg.tunnel() && tunnelStreamHandler == null)
		{
			tsServiceName = chnlInfo.connectionArg.tsService();
			tunnelStreamHandler = new TunnelStreamHandler(chnlInfo.connectionArg.tunnelAuth(), chnlInfo.connectionArg.tunnelDomain());
		}

		if (consumerCmdLineParser.cacheOption())
		{
			initializeCache(chnlInfo.cacheInfo);
			if (chnlInfo.dictionary != null)
				initializeCacheDictionary(chnlInfo.cacheInfo, chnlInfo.dictionary);

			if (cacheDisplayStr == null)
			{
				cacheDisplayStr = new StringBuilder();
				cacheEntryBuffer = CodecFactory.createBuffer();
				cacheEntryBuffer.data(ByteBuffer.allocate(6144));
			}
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

	private void displayCache(ChannelInfo chnlInfo)
	{
		System.out.println("\nStarting Cache Display ");

		if (chnlInfo.reactorChannel.channel() != null)
		{
			cacheDisplayStr.setLength(0);
			cacheDisplayStr.append("Channel :");
			cacheDisplayStr.append(chnlInfo.reactorChannel.channel().selectableChannel());
			System.out.println(cacheDisplayStr.toString());
		}

		if (chnlInfo.dictionary == null)
		{
			System.out.println("\tDictionary for decoding cache entries is not available\n");
			return;
		}

		if (chnlInfo.cacheInfo.cache != null)
		{
			cacheDisplayStr.setLength(0);
			cacheDisplayStr.append("Total Items in Cache: ");
			cacheDisplayStr.append(chnlInfo.cacheInfo.cache.entryCount());
			cacheDisplayStr.append("\n");
			System.out.println(cacheDisplayStr.toString());
		}

		displayCacheDomain(chnlInfo, DomainTypes.MARKET_PRICE, false);
		displayCacheDomain(chnlInfo, DomainTypes.MARKET_PRICE, true);

		displayCacheDomain(chnlInfo, DomainTypes.MARKET_BY_ORDER, false);
		displayCacheDomain(chnlInfo, DomainTypes.MARKET_BY_ORDER, true);

		displayCacheDomain(chnlInfo, DomainTypes.MARKET_BY_PRICE, false);
		displayCacheDomain(chnlInfo, DomainTypes.MARKET_BY_PRICE, true);

		displayCacheDomain(chnlInfo, DomainTypes.YIELD_CURVE, false);
		displayCacheDomain(chnlInfo, DomainTypes.YIELD_CURVE, true);

		System.out.println("Cache Display Complete\n");
	}

	private void displayCacheDomain(ChannelInfo chnlInfo, int domainType, boolean isPrivateStream)
	{
		Iterator<Map.Entry<StreamIdKey, WatchListEntry>> iter = chnlInfo.itemWatchList.iterator();
		while (iter.hasNext())
		{
			WatchListEntry entry = iter.next().getValue();

			if (entry.cacheEntry != null && entry.domainType == domainType && entry.isPrivateStream == isPrivateStream)
			{
				cacheDisplayStr.setLength(0);
				cacheDisplayStr.append("ItemName: ");
				cacheDisplayStr.append(entry.itemName);
				cacheDisplayStr.append("\n");
				cacheDisplayStr.append("Domain:\t");
				cacheDisplayStr.append(DomainTypes.toString(domainType));
				if (isPrivateStream)
					cacheDisplayStr.append("\tPrivate Stream");
				cacheDisplayStr.append("\n");

				cacheDisplayStr.append(entry.itemState.toString());
				cacheDisplayStr.append("\n");
				System.out.println(cacheDisplayStr.toString());

				int ret = decodeEntryFromCache(chnlInfo, entry.cacheEntry, domainType);
				if (ret != CodecReturnCodes.SUCCESS)
				{
					cacheDisplayStr.setLength(0);
					cacheDisplayStr.append("Error decoding cache content: ");
					cacheDisplayStr.append(ret);
					System.out.println(cacheDisplayStr.toString());
				}
			}
			else if (entry.domainType == domainType && entry.isPrivateStream == isPrivateStream)
			{
				if ( entry.itemState.streamState() == StreamStates.CLOSED )
					continue;

				cacheDisplayStr.setLength(0);
				cacheDisplayStr.append(entry.itemName);
				cacheDisplayStr.append("\tno data in cache\n");
				System.out.println(cacheDisplayStr.toString());
			}
		}
	}

	private int decodeEntryFromCache(ChannelInfo chnlInfo, PayloadEntry cacheEntry, int domainType)
	{
		int ret = CodecReturnCodes.SUCCESS;
		EncodeIterator eIter = CodecFactory.createEncodeIterator();
		DecodeIterator dIter = CodecFactory.createDecodeIterator();
		int majorVersion;
		int minorVersion;

		cacheEntryBuffer.data().clear();
		if (chnlInfo.reactorChannel != null)
		{
			majorVersion = chnlInfo.reactorChannel.majorVersion();
			minorVersion = chnlInfo.reactorChannel.minorVersion();
		}
		else
		{
			majorVersion =  chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion();
			minorVersion =  chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion();
		}

		eIter.clear();
		eIter.setBufferAndRWFVersion(cacheEntryBuffer, majorVersion, minorVersion);

		chnlInfo.cacheInfo.cursor.clear();
		if ( (ret = cacheEntry.retrieve(eIter, chnlInfo.cacheInfo.cursor, chnlInfo.cacheInfo.cacheError)) != CodecReturnCodes.SUCCESS)
		{
			cacheDisplayStr.setLength(0);
			cacheDisplayStr.append("Failed retrieving cache entry.\n\tError ");
			cacheDisplayStr.append(chnlInfo.cacheInfo.cacheError.errorId());
			cacheDisplayStr.append(" : ");
			cacheDisplayStr.append(chnlInfo.cacheInfo.cacheError.text());
			System.out.println(cacheDisplayStr.toString());
			return ret;
		}
		else
		{
			dIter.clear();
			dIter.setBufferAndRWFVersion(cacheEntryBuffer, majorVersion, minorVersion);
			cacheDisplayStr.setLength(0);

			switch (domainType)
			{
				case DomainTypes.MARKET_PRICE:
					ret = chnlInfo.marketPriceHandler.decodePayload(dIter, chnlInfo.dictionary, cacheDisplayStr);
					break;

				case DomainTypes.MARKET_BY_ORDER:
					ret = chnlInfo.marketByOrderHandler.decodePayload(dIter, chnlInfo.dictionary, cacheDisplayStr);
					break;

				case DomainTypes.MARKET_BY_PRICE:
					ret = chnlInfo.marketByPriceHandler.decodePayload(dIter, chnlInfo.dictionary, cacheDisplayStr);
					break;

				case DomainTypes.YIELD_CURVE:
					ret = chnlInfo.yieldCurveHandler.decodePayload(dIter, chnlInfo.dictionary);
					break;

				default:
					break;
			}
			if (ret > CodecReturnCodes.SUCCESS)
				ret = CodecReturnCodes.SUCCESS;
		}

		return ret;
	}

	private void displayReactorChannelStats(ChannelInfo chnlInfo)
	{
		ReactorChannelStats stats = ReactorFactory.createReactorChannelStats();
		chnlInfo.reactorChannel.getReactorChannelStats(stats);

		reactorChannelStats.bytesRead(overflowSafeAggregate(reactorChannelStats.bytesRead(), stats.bytesRead()));
		reactorChannelStats.uncompressedBytesRead(overflowSafeAggregate(reactorChannelStats.uncompressedBytesRead(), stats.uncompressedBytesRead()));
		reactorChannelStats.bytesWritten(overflowSafeAggregate(reactorChannelStats.bytesWritten(), stats.bytesWritten()));
		reactorChannelStats.uncompressedBytesWritten(overflowSafeAggregate(reactorChannelStats.uncompressedBytesWritten(), stats.uncompressedBytesWritten()));
		reactorChannelStats.pingsReceived(overflowSafeAggregate(reactorChannelStats.pingsReceived(), stats.pingsReceived()));
		reactorChannelStats.pingsSent(overflowSafeAggregate(reactorChannelStats.pingsSent(), stats.pingsSent()));

		System.out.println("Message Details:");
		System.out.printf("Bytes read=%d\n", reactorChannelStats.bytesRead());
		System.out.printf("Uncompressed bytes read=%d\n\n", reactorChannelStats.uncompressedBytesRead());
		System.out.printf("Bytes written=%d\n", reactorChannelStats.bytesWritten());
		System.out.printf("Uncompressed bytes written=%d\n\n", reactorChannelStats.uncompressedBytesWritten());
		System.out.printf("Pings sent=%d\n", reactorChannelStats.pingsSent());
		System.out.printf("Pings received=%d\n\n", reactorChannelStats.pingsReceived());
	}

	int overflowSafeAggregate(int a, int b)
	{
		long sum = (long)a + (long)b;
		if (sum < Integer.MAX_VALUE)
			return (int)sum;
		else
			return Integer.MAX_VALUE;
	}

	private void setItemState(ChannelInfo chnlInfo, int streamState, int dataState, int stateCode )
	{
		Iterator<Map.Entry<StreamIdKey, WatchListEntry>> iter = chnlInfo.itemWatchList.iterator();
		while (iter.hasNext())
		{
			WatchListEntry entry = iter.next().getValue();
			entry.itemState.streamState(streamState);
			entry.itemState.dataState(dataState);
			entry.itemState.code(stateCode);
		}
	}

	private void setEncryptedConfiguration(ConnectOptions options)
	{
		setHTTPConfiguration(options);

		String keyFile = consumerCmdLineParser.keyStoreFile();
		String keyPasswd = consumerCmdLineParser.keystorePassword();

		if (keyFile != null && !keyFile.isEmpty() )
		{
			options.encryptionOptions().KeystoreFile(keyFile);
		}
		if (keyPasswd != null && !keyPasswd.isEmpty())
		{
			options.encryptionOptions().KeystorePasswd(keyPasswd);
		}

		options.encryptionOptions().KeystoreType("JKS");
		options.encryptionOptions().SecurityProtocol("TLS");
		options.encryptionOptions().SecurityProvider("SunJSSE");
		options.encryptionOptions().KeyManagerAlgorithm("SunX509");
		options.encryptionOptions().TrustManagerAlgorithm("PKIX");
	}


	private void setHTTPConfiguration(ConnectOptions options)
	{
		options.tunnelingInfo().objectName("");
		options.tunnelingInfo().KeystoreType("JKS");
		options.tunnelingInfo().SecurityProtocol("TLS");
		options.tunnelingInfo().SecurityProvider("SunJSSE");
		options.tunnelingInfo().KeyManagerAlgorithm("SunX509");
		options.tunnelingInfo().TrustManagerAlgorithm("PKIX");
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

		String proxyUsername = consumerCmdLineParser.proxyUsername();
		if ( proxyUsername == null)
		{
			System.err.println("Error: Proxy username not provided.");
			System.exit(CodecReturnCodes.FAILURE);
		}

		String proxyPasswd = consumerCmdLineParser.proxyPassword();
		if ( proxyPasswd == null)
		{
			System.err.println("Error: Proxy password not provided.");
			System.exit(CodecReturnCodes.FAILURE);
		}
		String proxyDomain = consumerCmdLineParser.proxyDomain();
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

		String proxyKrbfile = consumerCmdLineParser.krbFile();
		if (proxyKrbfile == null)
		{
			System.err.println("Error: Proxy krbfile not provided.");
			System.exit(CodecReturnCodes.FAILURE);
		}
		options.credentialsInfo().HTTPproxyKRB5configFile(proxyKrbfile);

	}


	private void createItemLists(ChannelInfo chnlInfo)
	{
		// add specified items to item watch list
		if (chnlInfo.connectionArg.itemList() != null)
		{
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
						if (!itemArg.enablePrivateStream())
						{
							chnlInfo.mbpItemList.add(itemArg.itemName());
						}
						else
						{
							chnlInfo.mbppsItemList.add(itemArg.itemName());
						}
						break;
					case DomainTypes.YIELD_CURVE:
						if (!itemArg.enablePrivateStream())
						{
							chnlInfo.ycItemList.add(itemArg.itemName());
						}
						else
						{
							chnlInfo.ycpsItemList.add(itemArg.itemName());
						}
						break;
					case DomainTypes.SYMBOL_LIST:
						chnlInfo.slItemList.add(itemArg.itemName());
						break;
					default:
						break;
				}
			}
		}
	}

	private void sendSymbolListRequests(ChannelInfo chnlInfo)
	{
		if (chnlInfo.slItemList.size() == 0)
			return;

		if (!chnlInfo.serviceInfo.checkHasInfo())
		{
			uninitialize();
			System.exit(ReactorReturnCodes.FAILURE);
		}

		Service.ServiceInfo info = chnlInfo.serviceInfo.info();
		if (info.qosList().size() > 0)
		{
			Qos qos = info.qosList().get(0);
			chnlInfo.symbolListHandler.qos().dynamic(qos.isDynamic());
			chnlInfo.symbolListHandler.qos().rate(qos.rate());
			chnlInfo.symbolListHandler.qos().timeliness(qos.timeliness());
		}
		else
		{
			chnlInfo.symbolListHandler.qos().dynamic(false);
			chnlInfo.symbolListHandler.qos().rate(QosRates.TICK_BY_TICK);
			chnlInfo.symbolListHandler.qos().timeliness(QosTimeliness.REALTIME);
		}
		chnlInfo.symbolListHandler.capabilities().addAll(info.capabilitiesList());
		chnlInfo.symbolListHandler.serviceId(chnlInfo.serviceInfo.serviceId());
		String cmdSLName = chnlInfo.slItemList.get(0);
		if (cmdSLName == null)
		{
			chnlInfo.symbolListHandler.symbolListName().data(info.itemList().data(), info.itemList().position(), info.itemList().length());
		}
		else
		{
			chnlInfo.symbolListHandler.symbolListName().data(cmdSLName);
		}
		if (chnlInfo.symbolListHandler.sendRequest(chnlInfo.reactorChannel, errorInfo) != CodecReturnCodes.SUCCESS)
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

	private void sendMBPRequests(ChannelInfo chnlInfo)
	{
		if (chnlInfo.marketByPriceHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.mbpItemList, false, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
		{
			if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
				chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
			{
				System.out.println(errorInfo.error().text());
				uninitialize();
				System.exit(ReactorReturnCodes.FAILURE);
			}
		}

		if (chnlInfo.mbppsItemList.size() > 0 && !chnlInfo.mbppsRequestSent)
		{
			if (chnlInfo.marketByPriceHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.mbppsItemList, true, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
			{
				if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
					chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
				{
					System.out.println(errorInfo.error().text());
					uninitialize();
					System.exit(ReactorReturnCodes.FAILURE);
				}
			}
			chnlInfo.mbppsRequestSent = true;
		}
	}

	private void sendMBORequests(ChannelInfo chnlInfo)
	{
		if (chnlInfo.marketByOrderHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.mboItemList, false, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
		{
			if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
				chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
			{
				System.out.println(errorInfo.error().text());
				uninitialize();
				System.exit(ReactorReturnCodes.FAILURE);
			}
		}

		if (chnlInfo.mbopsItemList.size() > 0 && !chnlInfo.mbopsRequestSent)
		{
			if (chnlInfo.marketByOrderHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.mbopsItemList, true, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
			{
				if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
					chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
				{
					System.out.println(errorInfo.error().text());
					uninitialize();
					System.exit(ReactorReturnCodes.FAILURE);
				}
			}
			chnlInfo.mbopsRequestSent = true;
		}
	}

	private void sendMPRequests(ChannelInfo chnlInfo)
	{
		if (chnlInfo.marketPriceHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.mpItemList, false, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
		{
			if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
				chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
			{
				System.out.println(errorInfo.error().text());
				uninitialize();
				System.exit(ReactorReturnCodes.FAILURE);
			}
		}

		if (chnlInfo.mppsItemList.size() > 0 && !chnlInfo.mppsRequestSent)
		{
			if (chnlInfo.marketPriceHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.mppsItemList, true, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
			{
				if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
					chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
				{
					System.out.println(errorInfo.error().text());
					uninitialize();
					System.exit(ReactorReturnCodes.FAILURE);
				}
			}
			chnlInfo.mppsRequestSent = true;
		}
	}

	private void sendYieldCurveRequests(ChannelInfo chnlInfo)
	{
		if (chnlInfo.yieldCurveHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.ycItemList, false, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
		{
			if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
				chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
			{
				System.out.println(errorInfo.error().text());
				uninitialize();
				System.exit(ReactorReturnCodes.FAILURE);
			}
		}

		if (chnlInfo.ycpsItemList.size() > 0 && !chnlInfo.ycpsRequestSent)
		{
			if (chnlInfo.yieldCurveHandler.sendItemRequests(chnlInfo.reactorChannel, chnlInfo.ycpsItemList, true, chnlInfo.loginRefresh, chnlInfo.serviceInfo, errorInfo) != CodecReturnCodes.SUCCESS)
			{
				if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
					chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
				{
					System.out.println(errorInfo.error().text());
					uninitialize();
					System.exit(ReactorReturnCodes.FAILURE);
				}
			}
			chnlInfo.ycpsRequestSent = true;
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

		// close item streams if opened
		chnlInfo.marketPriceHandler.closeStreams(chnlInfo.reactorChannel, errorInfo);
		chnlInfo.marketByOrderHandler.closeStreams(chnlInfo.reactorChannel, errorInfo);
		chnlInfo.marketByPriceHandler.closeStreams(chnlInfo.reactorChannel, errorInfo);
		chnlInfo.symbolListHandler.closeStream(chnlInfo.reactorChannel, errorInfo);
		chnlInfo.yieldCurveHandler.closeStreams(chnlInfo.reactorChannel, errorInfo);
	}

	/* Uninitializes the Value Add consumer application. */
	private void uninitialize()
	{
		System.out.println("Consumer unitializing and exiting...");

		for (ChannelInfo chnlInfo : chnlInfoList)
		{
			// close items streams
			closeItemStreams(chnlInfo);

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

			uninitializeCache(chnlInfo.cacheInfo);
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

	public static void main(String[] args) throws Exception
	{
		Consumer consumer = new Consumer();
		consumer.init(args);
		consumer.run();
		consumer.uninitialize();
		System.exit(0);
	}
}
