/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.wsbconsumer;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.Dictionary.VerbosityValues;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.shared.network.ChannelHelper;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;
import com.refinitiv.eta.valueadd.examples.wsbconsumer.wsbConsumerConfig.ItemInfo;
import com.refinitiv.eta.valueadd.reactor.*;

import static java.util.concurrent.TimeUnit.NANOSECONDS;

/**
 * <p>
 * This is a main class to run the ETA Value Add warm standby consumer application.
 * </p>
 * <H2>Summary</H2>
 * <p>
 * This is the main file for the wsbConsumer application.  It is a single-threaded
 * client application that utilizes the ETA Reactor's watchlist and the ETA Reactor's 
 * Warm Standby functionality to provide recovery of data.
 *
 * The main consumer file provides the callback for channel events and 
 * the default callback for processing RsslMsgs. The main function
 * Initializes the ETA Reactor, makes the desired connections, and
 * dispatches for events.
 * This application makes use of the RDM package for easier decoding of Login &amp; Source Directory
 * messages.
 * </p>
 * <p>
 * This application supports consuming Level I Market Price, Level II Market By Price, and 
 * Level II Market By Order data. 
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
 * <H2>Json Configuration file format</H2>
 * <p>
 * The Json format is 2 named arrays.  Please note that these are all case sensitive:
 * </p>
 * <ul>
 * <li>WSBGroups : Array of warm standby groups consisting of an active server and standby servers.
 * <li>ConnectionList : Ordered list of connections. 
 * </ul>
 * <p>
 * WSBGroups values:
 * </p>
 * <ul>
 * <li>	WSBMode : string either "login" or "service", indicating that this warm standby group is using a Login or Service based configuration.
 * <li> WSBActive : object containing a channel configuration for the active warm standby group.  Please see below for the channel formatting.
 * <li>	WSBStandby : Array of channel objects. Please see below for the channel formatting.
 * </ul>
 * <p>
 * ConnectionList values: This is an ordered list of channel objects. 
 * </p>
 * <p>
 * Channel object format:
 * </p>
 * <ul>
 * <li>	Name : String channel name that is used in the application.
 * <li>	Host : String host name.
 * <li>	Port : String port.
 * <li>	Interface : String interface.
 * <li>	ConnType : numeric connection protocol type.  See {@link com.refinitiv.eta.transport.ConnectionTypes} for acceptable values.
 * <li>	EncryptedConnType : numeric encrypted connection protocol type.  See {@link com.refinitiv.eta.transport.ConnectionTypes} for acceptable values.
 * <li> EncryptionProtocolFlags : numeric flag, 4 - TLS1.2, 8 - TLS1.3; default is both TLS1.2, TLS1.3
 * <li>	SessionMgnt : boolean value.  If true, session management will be turned on for this channel, and the watchlist will automatically get an access token and
 * 				  optionally perform service discovery if host and port are not specified.
 * <li>	ProxyHost : String proxy host.
 * <li>	ProxyPort : String proxy port.
 * <li>	ProxyUserName : String user name for proxy authentication.
 * <li>	ProxyPassword : String password for proxy authentication.
 * <li>	ProxyDomain : String domain for proxy authentication.
 * </li>
 * </ul>
 * <p>
 * Example config:
 * </p>
 * <p>
 * {
 *
 *	"WSBGroups" : [
 *		{
 *			"WSBMode" : "login",
 *			"WSBActive" : {
 *				"Host" : "localhost",
 *				"Port" : "14002"
 *			},
 *			"WSBStandby" : [
 *				{
 *					"Host" : "localhost",
 *					"Port" : "14000"
 *				}
 *			]
 *		}
 *	],
 *	"ConnectionList" : [
 *	{
 *		"Host" : "CONNECTION 3",
 *		"Port" : "14002",
 *		"SessionMgnt" : false,
 *		"ConnType": 0
 *	},
 *	{
 *		"Host" : "us-east-2-aws-2-med.optimized-pricing-api.refinitiv.net",
 *		"Port" : "14002",
 *		"SessionMgnt" : true,
 *		"ConnType": 1,
 *		"EncryptedConnType" : 1,
 *		"EncryptionProtocolFlags" : 4
 *	}
 *	]
 * }
 * </p>
 * 
 * <H2>Setup Environment</H2>
 * <p>
 * The RDMFieldDictionary and enumtype.def files could be located in the
 * directory of execution or this application will request dictionary from
 * provider.
 * </p> 
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * </p>
 * <p>
 * Linux: ./gradlew runWsbConsumer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runWsbConsumer -PcommandLineArgs="arguments"<br>
 * </p>
 * <br>
 * <p>
 * Arguments are listed below.
 * </p>
 * <ul>
 * <li>-mp Market Price domain item name. Default is <i>TRI.N</i>. The user can
 * specify multiple -mp instances, where each occurrence is associated with a
 * single item. For example, specifying -mp TRI -mp GOOG will provide content
 * for two MarketPrice items. A service can be specified for each item with:
 * <li>	service_name:item_name
 * <li>-mbo Market By Order domain item name. No default. The user can specify
 * multiple -mbo instances, where each occurrence is associated with a single
 * item.
 * <li>-mbp market By Price domain item name. No default. The user can specify
 * multiple -mbp instances, where each occurrence is associated with a single
 * item.
 * <li>-x Provides XML tracing of messages.
 * <li>-runTime run time. Default is 600 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * <li>-keyfile keystore file for encryption.
 * <li>-keypasswd keystore password for encryption.
 * <li>-u Login user name. Default is system user name.
 * <li>-passwd Password for the user name.
 * <li>-clientId Specifies a unique ID for application making the request to RDP token service, also known as AppKey generated using an AppGenerator.
 * <li>-clientSecret Specifies the associated client Secret with a provided clientId for V2 logins.
 * <li>-jwkFile Specifies the file containing the JWK encoded private key for V2 JWT logins.
 * <li>-tokenURLV1 Specifies the token URL for V1 token oauthpasswd grant type.
 * <li>-tokenURLV2 Specifies the token URL for V2 token oauthclientcreds grant type.
 * <li>
 * <li>-restProxyHost rest proxy host name.
 * <li>-restProxyPort rest proxy host port.
 * <li>-restProxyUserName rest proxy user name.
 * <li>-restProxyPasswd rest proxy user password.
 * <li>-restProxyDomain rest proxy domain.
 * <li>-restProxyKrb5ConfigFile rest proxy Krb5 file.
 * <li>
 * </ul>
 */
public class wsbConsumer implements ConsumerCallback,
		ReactorJsonConversionEventCallback, ReactorServiceNameToIdCallback
{
	private final String FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
	private final String ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";

	private Reactor reactor;
	private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
	private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
	private ReactorJsonConverterOptions jsonConverterOptions = ReactorFactory.createReactorJsonConverterOptions();
	private wsbConsumerConfig watchlistConsumerConfig;
	private Selector selector;

	private long runtime;
	private Error error;    // error information

	private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	ReactorOAuthCredential reactorOAuthCredential = ReactorFactory.createReactorOAuthCredential();

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
	
	ChannelInfo chnlInfo = new ChannelInfo();
	
	private HashMap<String, Service> serviceMap = new HashMap<String, Service>();

	public static final int MAX_MSG_SIZE = 1024;

	public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

	boolean fieldDictionaryLoaded;
	boolean enumDictionaryLoaded;

	private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
	private ItemRequest itemRequest;
	Buffer payload;

	private Map<ReactorChannel, Integer> socketFdValueMap = new HashMap<>();

	public wsbConsumer()
	{
		error = TransportFactory.createError();

		dispatchOptions.maxMessages(1000);
		_finalStatusEvent = true;

		closetime = 10; // 10 sec

		itemDecoder = new ItemDecoder();

		itemRequest = createItemRequest();
		itemsRequested = false;

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

		watchlistConsumerConfig = new wsbConsumerConfig();
		watchlistConsumerConfig.addCommandLineArgs();

		if (!watchlistConsumerConfig.init(args, chnlInfo))
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
		
		// Set the Token Generator URL locations, if specified
		if (watchlistConsumerConfig.tokenUrlV1() != null && !watchlistConsumerConfig.tokenUrlV1().isEmpty())
		{
			reactorOptions.tokenServiceURL_V1().data(watchlistConsumerConfig.tokenUrlV1());
		}
		
		if (watchlistConsumerConfig.tokenUrlV2() != null && !watchlistConsumerConfig.tokenUrlV2().isEmpty())
		{
			reactorOptions.tokenServiceURL_V2().data(watchlistConsumerConfig.tokenUrlV2());
		}
		
		if (watchlistConsumerConfig.serviceDiscoveryURL() != null && !watchlistConsumerConfig.serviceDiscoveryURL().isEmpty())
		{
			reactorOptions.serviceDiscoveryURL().data(watchlistConsumerConfig.serviceDiscoveryURL());
		}

		if (watchlistConsumerConfig.restProxyHost() != null && !watchlistConsumerConfig.restProxyHost().isEmpty())
		{
			reactorOptions.restProxyOptions().proxyHostName().data(watchlistConsumerConfig.restProxyHost());
		}
		
		if (watchlistConsumerConfig.restProxyPort() != null && !watchlistConsumerConfig.restProxyPort().isEmpty())
		{
			reactorOptions.restProxyOptions().proxyPort().data(watchlistConsumerConfig.restProxyPort());
		}
		
		if (watchlistConsumerConfig.restProxyUserName() != null && !watchlistConsumerConfig.restProxyUserName().isEmpty())
		{
			reactorOptions.restProxyOptions().proxyUserName().data(watchlistConsumerConfig.restProxyUserName());
		}
		
		if (watchlistConsumerConfig.restProxyPasswd() != null && !watchlistConsumerConfig.restProxyPasswd().isEmpty())
		{
			reactorOptions.restProxyOptions().proxyPassword().data(watchlistConsumerConfig.restProxyPasswd());
		}
		
		if (watchlistConsumerConfig.restProxyDomain() != null && !watchlistConsumerConfig.restProxyDomain().isEmpty())
		{
			reactorOptions.restProxyOptions().proxyDomain().data(watchlistConsumerConfig.restProxyDomain());
		}
		
		if (watchlistConsumerConfig.restProxyKrb5ConfigFile() != null && !watchlistConsumerConfig.restProxyKrb5ConfigFile().isEmpty())
		{
			reactorOptions.restProxyOptions().proxyKrb5ConfigFile().data(watchlistConsumerConfig.restProxyKrb5ConfigFile());
		}
		
		// create reactor
		reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
		if (errorInfo.code() != ReactorReturnCodes.SUCCESS)
		{
			System.out.println("createReactor() failed: " + errorInfo.toString());
			System.exit(ReactorReturnCodes.FAILURE);
		}

		jsonConverterOptions.dataDictionary(itemDecoder.getDictionary());
		jsonConverterOptions.serviceNameToIdCallback(this);
		jsonConverterOptions.jsonConversionEventCallback(this);

		// Initialize the JSON converter
		if ( reactor.initJsonConverter(jsonConverterOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
		{
			System.out.println("Reactor.initJsonConverter() failed: " + errorInfo.toString());
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
		// initialize channel info
		initChannelInfo(chnlInfo);

		// connect channel
		int ret;
		if ((ret = reactor.connect(chnlInfo.connectOptions, chnlInfo.consumerRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
		{
			System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
			System.exit(ReactorReturnCodes.FAILURE);
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
				int ret;
				ret = reactor.dispatchAll(keySet, dispatchOptions, errorInfo);
				if (ret == ReactorReturnCodes.FAILURE)
				{
					System.out.println("ReactorChannel dispatch failed: " + ret + "(" + errorInfo.error().text() + ")");
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
		}
	}

	/* Requests the desired items.  */
	int requestItems(Reactor reactor, ReactorChannel channel)
	{
		if (itemsRequested)
			return ReactorReturnCodes.SUCCESS;

		itemsRequested = true;

		for(int itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemList().size(); ++itemListIndex)
		{
			int ret = CodecReturnCodes.SUCCESS;

			itemRequest.clear();

			itemRequest.applyStreaming();

			itemRequest.addItem(watchlistConsumerConfig.itemList().get(itemListIndex).name);

			int domainType = watchlistConsumerConfig.itemList().get(itemListIndex).domain();

			itemRequest.domainType(domainType);
			itemRequest.streamId(watchlistConsumerConfig.itemList().get(itemListIndex).streamId);


			itemRequest.encode();

			submitOptions.clear();
			submitOptions.serviceName(watchlistConsumerConfig.itemList().get(itemListIndex).serviceName());

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

		switch(event.eventType())
		{
			case ReactorChannelEventTypes.CHANNEL_UP:
			{
				if (event.reactorChannel().selectableChannel() != null)
					System.out.println("Channel Up Event: " + event.reactorChannel().selectableChannel());
				else
					System.out.println("Channel Up Event");
				// register selector with channel event's reactorChannel

				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					/* Add all the current selectable channels */
					for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
					{
						// define socket fd value
						final int fdSocketId =
								ChannelHelper.defineFdValueOfSelectableChannel(event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i));
						socketFdValueMap.put(event.reactorChannel(), fdSocketId);
						try
						{
							event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).register(selector,
									SelectionKey.OP_READ,
									event.reactorChannel());
						}
						catch (ClosedChannelException e)
						{
							System.out.println("selector register failed: " + e.getLocalizedMessage());
						}
						
					}
				}
				else
				{
					// define socket fd value
					final int fdSocketId =
							ChannelHelper.defineFdValueOfSelectableChannel(event.reactorChannel().channel().selectableChannel());
					socketFdValueMap.put(event.reactorChannel(), fdSocketId);
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
				}	

				break;
			}
			case ReactorChannelEventTypes.FD_CHANGE:
			{
				System.out.println("Channel Change - Old Channel: "
								   + event.reactorChannel().oldSelectableChannel() + " New Channel: "
								   + event.reactorChannel().selectableChannel());
				
				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					/* Remove all the old keys from the selectable key list */
					SelectableChannel oldSelectChannel;
					/* Remove all the old keys from the selectable key list */
					for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().size(); i++)
					{
						oldSelectChannel = event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i);
						if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(oldSelectChannel))
						{
							SelectionKey key = oldSelectChannel.keyFor(selector);
							System.out.println("Removing socket from list" + oldSelectChannel);
							if (key != null)
								key.cancel();
						}
					}
					
					/* Add all the current selectable channels */
					for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
					{
						// define socket fd value
						final int fdSocketId =
								ChannelHelper.defineFdValueOfSelectableChannel(event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i));
						socketFdValueMap.put(event.reactorChannel(), fdSocketId);
						try
						{
							if(event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).keyFor(selector) == null)
							{
								System.out.println("Adding socket to list" + event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i));
								event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).register(selector,
										SelectionKey.OP_READ,
										event.reactorChannel());
							}
						}
						catch (Exception e)
						{
							System.out.println("selector register failed: " + e.getLocalizedMessage());
						}
						
					}
				}
				else
				{

					// cancel old reactorChannel select
					SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
					if (key != null)
						key.cancel();
	
					// define new socket fd value
					final int fdSocketId =
							ChannelHelper.defineFdValueOfSelectableChannel(event.reactorChannel().channel().selectableChannel());
					socketFdValueMap.put(event.reactorChannel(), fdSocketId);
	
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
				}
				break;
			}
			case ReactorChannelEventTypes.CHANNEL_READY:
			{
				if (event.reactorChannel().selectableChannel() != null)
					System.out.println("Channel Ready Event: " + event.reactorChannel().selectableChannel());
				else
					System.out.println("Channel Ready Event");

				break;

			}
			case ReactorChannelEventTypes.CHANNEL_OPENED:
			{
				// set ReactorChannel on ChannelInfo, again need this?
				chnlInfo.reactorChannel = event.reactorChannel();

				if (fieldDictionaryLoaded && enumDictionaryLoaded
					|| itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile)
					requestItems(reactor, event.reactorChannel());
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
				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					/* Remove all the old keys from the selectable key list */
					event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().forEach((oldSelectChannel) -> {
						// cancel old reactorChannel select if it's not in the current list
						if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(oldSelectChannel))
						{
							SelectionKey key = oldSelectChannel.keyFor(selector);
							if (key != null)
								key.cancel();
						}
					});
				}
				else
				{
					if (event.reactorChannel().selectableChannel() != null)
					{
						SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
						if (key != null)
							key.cancel();
					}
				}

				itemsRequested = false;
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
				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					/* Remove all the old keys from the selectable key list */
					event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().forEach((oldSelectChannel) -> {
						// cancel old reactorChannel select if it's not in the current list
						if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(oldSelectChannel))
						{
							SelectionKey key = oldSelectChannel.keyFor(selector);
							if (key != null)
								key.cancel();
						}
					});
				}
				else
				{
					if (event.reactorChannel().selectableChannel() != null)
					{
						SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
						if (key != null)
							key.cancel();
					}
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
		String itemName = null;
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
				if ( refreshMsg.checkHasMsgKey())
				{
					if (refreshMsg.msgKey().checkHasName())
					{
						itemName = refreshMsg.msgKey().name().toString(); // Buffer
					}
				}
				else if (item != null)
				{
					itemName = item.name();
				}

				System.out.println("DefaultMsgCallback Refresh ItemName: " + itemName + " Domain: " + DomainTypes.toString(refreshMsg.domainType()) + ", StreamId: " + refreshMsg.streamId());

				System.out.println("                      State: "  + refreshMsg.state());

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

				System.out.println("DefaultMsgCallback Update ItemName: " + itemName + " Domain: " + DomainTypes.toString(updateMsg.domainType()) + ", StreamId: " + updateMsg.streamId());

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
					}
				}
				else if (item != null)
				{
					itemName = item.name();
				}

				System.out.println("DefaultMsgCallback Status -- ItemName: " + itemName + " Domain: " + DomainTypes.toString(statusMsg.domainType()) + ", StreamId: " + statusMsg.streamId());

				if ( statusMsg.checkHasState())
				{
					System.out.println(statusMsg.state());

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

				System.out.println(" State: "  + chnlInfo.loginRefresh.state());
				if ( chnlInfo.loginRefresh.checkHasUserName())
					System.out.println(" UserName: " + chnlInfo.loginRefresh.userName().toString());

				// get login reissue time from authenticationTTReissue
				if (chnlInfo.loginRefresh.checkHasAuthenticationTTReissue())
				{
					chnlInfo.loginReissueTime = chnlInfo.loginRefresh.authenticationTTReissue() * 1000;
					chnlInfo.canSendLoginReissue = watchlistConsumerConfig.enableSessionManagement() ? false : true;
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
				System.out.printf("RTT Response sent to provider by watchlist.\n\n");
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
				System.out.println(directoryRefresh.state().toString());

				serviceList = directoryRefresh.serviceList();

				for (Service service : serviceList)
				{
					if(service.info().serviceName().toString() != null)
					{
						Service tmpService = serviceMap.get(service.info().serviceName().toString());
						if(tmpService != null )
						{
							if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() )
							{
								chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
							}

							boolean updateServiceInfo = false;
							System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
							// update service cache - assume cache is built with previous refresh message
							if (service.serviceId() == tmpService.serviceId())
							{
								updateServiceInfo = true;
							}

							if (updateServiceInfo)
							{
								// update serviceInfo associated with requested service name
								if (service.copy(tmpService) < CodecReturnCodes.SUCCESS)
								{
									System.out.println("Service.copy() failure");
									uninitialize();
									System.exit(ReactorReturnCodes.FAILURE);
								}
							}
						}
						else
						{
							Service newService = DirectoryMsgFactory.createService();
							
							service.copy(newService);
							
							serviceMap.put(newService.info().serviceName().toString(), newService);
						}
					}
				}
				break;
			case UPDATE:
				DirectoryUpdate directoryUpdate = (DirectoryUpdate)event.rdmDirectoryMsg();

				System.out.println("Received Source Directory Update");
				System.out.println(directoryUpdate.toString());

				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.UPDATE));

				serviceList = directoryUpdate.serviceList();

				for (Service service : serviceList)
				{
					Service tmpService = serviceMap.get(service.info().serviceName().toString());
					if(tmpService != null )
					{
						if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() )
						{
							chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
						}


						boolean updateServiceInfo = false;
						System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
						// update service cache - assume cache is built with previous refresh message
						if (service.serviceId() == tmpService.serviceId())
						{
							updateServiceInfo = true;
						}

						if (updateServiceInfo)
						{
							// update serviceInfo associated with requested service name
							if (service.copy(tmpService) < CodecReturnCodes.SUCCESS)
							{
								System.out.println("Service.copy() failure");
								uninitialize();
								System.exit(ReactorReturnCodes.FAILURE);
							}
						}
					}
					else
					{
						Service newService = DirectoryMsgFactory.createService();
						
						service.copy(newService);
						
						serviceMap.put(newService.info().serviceName().toString(), newService);
					}
				}

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
		if ( serviceList != null )
		{
			for (Service service : serviceList)
			{
				System.out.println(" Service = " + service.serviceId() + " Action: " + MapEntryActions.toString(service.action()));

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
							itemDecoder.fieldDictionaryDownloadedFromNetwork =  true;
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
							itemDecoder.enumTypeDictionaryDownloadedFromNetwork =  true;
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

	@Override
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId, ReactorServiceNameToIdEvent serviceNameToIdEvent)
	{
		Service service = serviceMap.get(serviceNameToId.serviceName());

		/* Checks whether the service name is used by the channel. */
		if(service != null)
		{
			serviceNameToId.serviceId(service.serviceId());
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

	private void initChannelInfo(ChannelInfo chnlInfo)
	{
		// set up consumer role
		chnlInfo.consumerRole.defaultMsgCallback(this);
		chnlInfo.consumerRole.channelEventCallback(this);
		chnlInfo.consumerRole.loginMsgCallback(this);
		chnlInfo.consumerRole.directoryMsgCallback(this);
		chnlInfo.consumerRole.watchlistOptions().enableWatchlist(true);
		chnlInfo.consumerRole.watchlistOptions().itemCountHint(4);
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
		if (watchlistConsumerConfig.userName() != null && !watchlistConsumerConfig.userName().isEmpty())
		{
			chnlInfo.consumerRole.rdmLoginRequest().userName().data(watchlistConsumerConfig.userName());
			reactorOAuthCredential.userName().data(watchlistConsumerConfig.userName());
		}

		if (watchlistConsumerConfig.password() != null && !watchlistConsumerConfig.password().isEmpty())
		{
			reactorOAuthCredential.password().data(watchlistConsumerConfig.password());

		}

		if (watchlistConsumerConfig.clientId() != null && !watchlistConsumerConfig.clientId().isEmpty())
		{
			reactorOAuthCredential.clientId().data(watchlistConsumerConfig.clientId());
			if (watchlistConsumerConfig.clientSecret() != null && !watchlistConsumerConfig.clientSecret().isEmpty())
			{
				reactorOAuthCredential.clientSecret().data(watchlistConsumerConfig.clientSecret());
			}
			
			if(watchlistConsumerConfig.jwkFile() != null && !watchlistConsumerConfig.jwkFile().isEmpty())
			{
				try
				{
					// Get the full contents of the JWK file.
					byte[] jwkFile = Files.readAllBytes(Paths.get(watchlistConsumerConfig.jwkFile()));
					String jwkText = new String(jwkFile);
					
					reactorOAuthCredential.clientJwk().data(jwkText);
				}
				catch(Exception e)
				{
					System.err.println("Error loading JWK file: " + e.getMessage());
					System.err.println();
					System.err.println(CommandLine.optionHelpString());
					System.out.println("Consumer exits...");
					System.exit(CodecReturnCodes.FAILURE);
				}
			}
			
			reactorOAuthCredential.takeExclusiveSignOnControl(watchlistConsumerConfig.takeExclusiveSignOnControl());
			chnlInfo.consumerRole.reactorOAuthCredential(reactorOAuthCredential);
		}
		
		if (watchlistConsumerConfig.tokenScope() != null && !watchlistConsumerConfig.tokenScope().isEmpty())
		{
			reactorOAuthCredential.tokenScope().data(watchlistConsumerConfig.tokenScope());
			chnlInfo.consumerRole.reactorOAuthCredential(reactorOAuthCredential);
		}
		
		if (watchlistConsumerConfig.audience() != null && !watchlistConsumerConfig.audience().isEmpty())
		{
			reactorOAuthCredential.audience().data(watchlistConsumerConfig.audience());
			chnlInfo.consumerRole.reactorOAuthCredential(reactorOAuthCredential);
		}

		// use command line application id if specified
		if (watchlistConsumerConfig.applicationId() != null && !watchlistConsumerConfig.applicationId().equals(""))
		{
			chnlInfo.consumerRole.rdmLoginRequest().attrib().applicationId().data(watchlistConsumerConfig.applicationId());
		}

		chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
		chnlInfo.consumerRole.rdmLoginRequest().attrib().singleOpen(1);
		chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
		chnlInfo.consumerRole.rdmLoginRequest().attrib().allowSuspectData(1);

		if ((itemDecoder.fieldDictionaryLoadedFromFile == true &&
			itemDecoder.enumTypeDictionaryLoadedFromFile == true)) 
		{

			chnlInfo.dictionary = itemDecoder.getDictionary();
		}

		chnlInfo.connectOptions = watchlistConsumerConfig.connectOpts();
	}

	private void closeItemStreams(ChannelInfo chnlInfo)
	{
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

		// close items streams
		closeItemStreams(chnlInfo);

		// close ReactorChannel
		if (chnlInfo.reactorChannel != null)
		{
			chnlInfo.reactorChannel.close(errorInfo);
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

		closeItemStreams(chnlInfo);
	}

	String mapServiceActionString(int action)
	{
		if (action == MapEntryActions.DELETE) return "DELETE";
		else if ( action == MapEntryActions.ADD) return "ADD";
		else if (action == MapEntryActions.UPDATE) return "UPDATE";
		else
			return null;
	}


	public static void main(String[] args) throws Exception
	{
		wsbConsumer consumer = new wsbConsumer();
		consumer.init(args);
		consumer.run();
		consumer.uninitialize();
		System.exit(0);
	}
}
