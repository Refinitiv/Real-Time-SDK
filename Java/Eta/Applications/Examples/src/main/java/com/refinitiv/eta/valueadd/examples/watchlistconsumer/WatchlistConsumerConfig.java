/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArg;
import com.refinitiv.eta.valueadd.examples.common.ItemArg;

public class WatchlistConsumerConfig
{
	private String publisherId;
	private String publisherAddress;
	private boolean enableEncrypted;
	private boolean enableHttp;
	private boolean cacheOption;
	private int cacheInterval;

	List<ConnectionArg> connectionList = new ArrayList<ConnectionArg>();

	// default server host name
	private static final String defaultSrvrHostname = "localhost";

	// default server port number
	private static final String defaultSrvrPortNo = "14002";

	// default service name
	private static final String defaultServiceName = "DIRECT_FEED";

	// default item name
	private static final String defaultItemName = "TRI.N";


	private String protocolList = "tr_json2";

	private static final int defaultRuntime = 600;

	private int MAX_ITEMS = 128;
	private int ITEMS_MIN_STREAM_ID = 5;

	private ArrayList<ItemInfo> itemList = new ArrayList<ItemInfo>();
	private ArrayList<ItemInfo> providedItemList = new ArrayList<ItemInfo>();
	
	private String startingHostName;
	private String startingPort;
	private String standbyHostName;
	private String standbyPort;
	private String warmStandbyMode;
	private boolean enableWarmStandby;
	
	private String securityProtocol;
	private String securityProtocolVersions;

	class ItemInfo
	{
		int	domain;
		String name;
		int	streamId;
		boolean	symbolListData;
		boolean isPrivateStream;
		boolean isBatchStream;
		State state = CodecFactory.createState();

		public int domain()
		{
			return domain;
		}
		public void domain(int domainType)
		{
			this.domain = domainType;
		}
		public String name()
		{
			return name;
		}
		public void name(String name)
		{
			this.name = name;
		}
		public int streamId()
		{
			return streamId;
		}
		public void streamId(int streamId)
		{
			this.streamId = streamId;
		}
		public boolean symbolListData()
		{
			return symbolListData;
		}
		public void symbolListData(boolean symbolListData)
		{
			this.symbolListData = symbolListData;
		}
		public boolean isPrivateStream()
		{
			return isPrivateStream;
		}
		public void privateStream(boolean isPrivateStream)
		{
			this.isPrivateStream = isPrivateStream;
		}
		public boolean isBatchStream()
		{
			return isBatchStream;
		}
		public void batchStream(boolean isBatchStream)
		{
			this.isBatchStream = isBatchStream;
		}
		public State state()
		{
			return state;
		}
		public void state(State state)
		{
			this.state = state;
		}
	}

	public boolean init(String[]args)
	{
		boolean ret;


		if ((ret = parseArgs(args)) == false )
		{
			return ret;
		}

		List<ConnectionArg> connectionList = connectionList();
		ConnectionArg conn = connectionList.get(0);
		for ( ItemArg itemArg : conn.itemList())
		{
			addItem(itemArg.itemName(), itemArg.domain(), itemArg.symbolListData(), itemArg.enablePrivateStream());
		}
		return true;
	}


	public void addItem(String itemName, int domainType, boolean isSymbolList, boolean isPrivate)
	{
		ItemInfo itemInfo = new ItemInfo();
		itemInfo.domain(domainType);
		itemInfo.name(itemName);
		itemInfo.symbolListData(isSymbolList);
		itemInfo.privateStream(isPrivate);
		itemInfo.streamId(ITEMS_MIN_STREAM_ID + itemList.size());
		itemList.add(itemInfo);

		if (itemList.size() >= MAX_ITEMS)
		{
			System.out.println("Config Error: Example only supports up to %d items " + MAX_ITEMS);
			System.exit(-1);
		}
	}

	public ItemInfo getItemInfo(int streamId)
	{
		if (streamId > 0)
		{
			if (streamId >= ITEMS_MIN_STREAM_ID && streamId < itemList.size() + ITEMS_MIN_STREAM_ID)
				return itemList.get(streamId - ITEMS_MIN_STREAM_ID);
			else
				return null;
		}
		else if (streamId < 0)
		{
			for(int i = 0; i < providedItemList.size(); ++i)
			{
				if (providedItemList.get(i).streamId() == streamId)
					return providedItemList.get(i);
			}
			return null;
		}
		else
			return null;
	}

	public ItemInfo addProvidedItemInfo(int streamId, MsgKey msgKey, int domainType)
	{
		ItemInfo item = null;

		/* Check if item is already present. */
		for(int i = 0; i < providedItemList.size(); ++i)
		{
			if (providedItemList.get(i).streamId == streamId)
			{
				item = providedItemList.get(i);
				break;
			}
		}

		/* Add new item. */
		if (item == null)
		{
			if (providedItemList.size() == MAX_ITEMS)
			{
				System.out.println("Too many provided items.\n");
				return null;
			}
			item = new ItemInfo();
			providedItemList.add(item);
		}

		if ((msgKey.flags() & MsgKeyFlags.HAS_NAME)> 0)
		{
			item.name = msgKey.name().toString();
		}
		else
		{
			item.name = null;
		}

		item.streamId(streamId);
		item.domain(domainType);

		return item;
	}

	public void removeProvidedItemInfo(ItemInfo item)
	{
		int i = 0;
		boolean found = false;
		for(i = 0; i < providedItemList.size(); ++i)
		{
			if (providedItemList.get(i).streamId == item.streamId)
			{
				found = true;
				break;
			}
		}
		if ( found )
		{
			providedItemList.remove(i);
		}
		return;
	}

	public boolean parseArgs(String[] args)
	{
		for (int i = 0; i < args.length; i++)
		{
			if (args[i].contains("runtime"))
			{
				args[i] = args[i].replace("runtime", "runTime");
			}
			if (args[i].contains("uname"))
			{
				args[i] = args[i].replace("uname", "u");
			}
			if (args[i].contains("connectionType"))
			{
				args[i] = args[i].replace("connectionType", "c");
			}
		}

		addCommandLineArgs();
		try
		{
			CommandLine.parseArgs(args);

			ConnectionArg connectionArg = new ConnectionArg();

			String connectionType = CommandLine.value("c");
			if (connectionType.equals("socket"))
			{
				connectionArg.connectionType(ConnectionTypes.SOCKET);
			}
			else if (connectionType.equals("encrypted"))
			{
				connectionArg.connectionType(ConnectionTypes.ENCRYPTED);
				enableEncrypted = true;
			}
			else if (connectionType.equals("http"))
			{
				connectionArg.connectionType(ConnectionTypes.HTTP);
				enableHttp = true;
			}
			else if (connectionType.equals("websocket"))
			{
				connectionArg.connectionType(ConnectionTypes.WEBSOCKET);
			}
			
			if (CommandLine.hasArg("startingHostName") && (CommandLine.hasArg("startingPort") && CommandLine.hasArg("standbyHostName")
					&& CommandLine.hasArg("standbyPort") && CommandLine.hasArg("warmStandbyMode")))
			{
				startingHostName = CommandLine.value("startingHostName");
				startingPort = CommandLine.value("startingPort");
				standbyHostName = CommandLine.value("standbyHostName");
				standbyPort = CommandLine.value("standbyPort");
				warmStandbyMode = CommandLine.value("warmStandbyMode");
				enableWarmStandby = true;
			}
			else
				enableWarmStandby = false;

			if(CommandLine.hasArg("encryptedConnectionType"))
			{
				String encryptedConnectionType = CommandLine.value("encryptedConnectionType");
				if (encryptedConnectionType.equals("socket"))
				{
					connectionArg.encryptedConnectionType(ConnectionTypes.SOCKET);
				}
				else if (encryptedConnectionType.equals("http"))
				{
					connectionArg.encryptedConnectionType(ConnectionTypes.HTTP);
				}
				else if (encryptedConnectionType.equals("websocket"))
				{
					connectionArg.encryptedConnectionType(ConnectionTypes.WEBSOCKET);
				}
			}
			else
			{
				if(connectionArg.connectionType() == ConnectionTypes.ENCRYPTED)
				{
					connectionArg.encryptedConnectionType(ConnectionTypes.HTTP);
				}
			}
			
			if (CommandLine.hasArg("spTLSv1.2") && CommandLine.hasArg("spTLSv1.3"))
			{
				// Enable both TLS 1.2 and 1.3
				securityProtocol = "TLS";
				securityProtocolVersions = "1.2,1.3";
			}
			else if (CommandLine.hasArg("spTLSv1.2"))
			{
				// Enable TLS 1.2
				securityProtocol = "TLS";
				securityProtocolVersions = "1.2";
			}
			else if (CommandLine.hasArg("spTLSv1.3"))
			{
				// Enable TLS 1.3
				securityProtocol = "TLS";
				securityProtocolVersions = "1.3";
			}
			else
			{
				// Default always sets both TLS versions 1.2 and 1.3.
				securityProtocol = "TLS";
				securityProtocolVersions = "1.2,1.3";
			}

			connectionArg.service(serviceName());


			if(CommandLine.value("h") == null)
			{
				if(enableSessionManagement() == false)
				{
					connectionArg.hostname(defaultSrvrHostname);
				}
			}
			else
			{
				connectionArg.hostname(CommandLine.value("h"));
			}

			if(CommandLine.value("p") == null)
			{
				if(enableSessionManagement() == false)
				{
					connectionArg.port(defaultSrvrPortNo);
				}
			}
			else
			{
				connectionArg.port(CommandLine.value("p"));
			}

			connectionArg.interfaceName(CommandLine.value("if"));

			if (CommandLine.hasArg("tsServiceName"))
			{
				connectionArg.tsService(CommandLine.value("tsServiceName"));
			}

			if (CommandLine.hasArg("tunnel"))
			{
				connectionArg.tunnel(CommandLine.booleanValue("tunnel"));
			}

			if (CommandLine.hasArg("tsAuth"))
				connectionArg.tunnelAuth(CommandLine.booleanValue("tsAuth"));
			if (CommandLine.hasArg("tsDomain"))
				connectionArg.tunnelDomain(CommandLine.intValue("tsDomain"));

			List<ItemArg> itemList = new ArrayList<ItemArg>();

			List<String> itemNames = CommandLine.values("mp");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.MARKET_PRICE, false, false, itemList);

			itemNames = CommandLine.values("mbo");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.MARKET_BY_ORDER, false, false, itemList);

			itemNames = CommandLine.values("mbp");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.MARKET_BY_PRICE, false, false, itemList);

			itemNames = CommandLine.values("yc");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.YIELD_CURVE, false, false, itemList);

			itemNames = CommandLine.values("sl");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.SYMBOL_LIST, false, false, itemList);

			itemNames = CommandLine.values("sld");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.SYMBOL_LIST, false, true, itemList);

			if (itemList.size() == 0 && !CommandLine.hasArg("tunnel"))
			{
				ItemArg itemArg = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName, false);
				itemList.add(itemArg);
			}

			if ( tsServiceName() == null || tsServiceName().equals(""))
				connectionArg.tsService(connectionArg.service());

			connectionArg.itemList(itemList);
			connectionList.add(connectionArg);

			String value = CommandLine.value("publisherInfo");
			if (value!= null)
			{
				String [] pieces = value.split(",");

				if( pieces.length > 1 )
				{
					publisherId = pieces[0];

					try
					{
						Integer.parseInt(publisherId);
					}
					catch(Exception e)
					{
						System.err.println("Error loading command line arguments:\t");
						System.out.println("publisherId within publisherinfo should be an integer number");
						System.out.println("Consumer exits...");
						System.exit(CodecReturnCodes.FAILURE);
					}
					publisherAddress = pieces[1];
				}
			}
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

		return true;
	}

	String serviceName()
	{
		return CommandLine.value("s");
	}

	String tsServiceName()
	{
		return CommandLine.value("tsServiceName");
	}

	List<ConnectionArg> connectionList()
	{
		return connectionList;
	}

	String userName()
	{
		return CommandLine.value("u");
	}

	String password()
	{
		return CommandLine.value("passwd");
	}

	boolean enableView()
	{
		return CommandLine.booleanValue("view");
	}

	boolean enablePost()
	{
		return CommandLine.booleanValue("post");
	}

	boolean enableOffpost()
	{
		return CommandLine.booleanValue("offpost");
	}

	String publisherId()
	{
		return publisherId;
	}

	String publisherAddress()
	{
		return publisherAddress;
	}

	boolean enableSnapshot()
	{
		return CommandLine.booleanValue("snapshot");
	}

	int runtime()
	{
		return CommandLine.intValue("runTime");
	}

	boolean enableXmlTracing()
	{
		return CommandLine.booleanValue("x");
	}

	boolean enableSessionManagement()
	{
		return CommandLine.booleanValue("sessionMgnt");
	}

	boolean enableEncrypted()
	{
		return enableEncrypted;
	}

	String clientId()
	{
		return CommandLine.value("clientId");
	}
	
	String clientSecret()
	{
		return CommandLine.value("clientSecret");
	}
	
	String jwkFile()
	{
		return CommandLine.value("jwkFile");
	}
	
	String tokenUrlV1()
	{
		return CommandLine.value("tokenURLV1");
	}
	
	String tokenUrlV2()
	{
		return CommandLine.value("tokenURLV2");
	}
	
	String tokenScope()
	{
		return CommandLine.value("tokenScope");
	}
	
	String audience()
	{
		return CommandLine.value("audience");
	}
	
	String serviceDiscoveryURL()
	{
		return CommandLine.value("serviceDiscoveryURL");
	}

	boolean takeExclusiveSignOnControl()
	{
		return CommandLine.booleanValue("takeExclusiveSignOnControl");
	}

	String location()
	{
		return CommandLine.value("l");
	}

	boolean queryEndpoint()
	{
		return CommandLine.booleanValue("query");
	}

	boolean enableHttp()
	{
		return enableHttp;
	}

	boolean enableProxy()
	{
		return CommandLine.booleanValue("proxy");
	}

	String proxyHostname()
	{
		return CommandLine.value("ph");
	}

	String proxyPort()
	{
		return CommandLine.value("pp");
	}

	String proxyUsername()
	{
		return CommandLine.value("plogin");
	}

	String proxyPassword()
	{
		return CommandLine.value("ppasswd");
	}

	String proxyDomain()
	{
		return CommandLine.value("pdomain");
	}

	String restProxyHost() {
		return CommandLine.value("restProxyHost");
	}
	
	String restProxyPort() {
		return CommandLine.value("restProxyPort");
	}
	
	String restProxyUserName() {
		return CommandLine.value("restProxyUserName");
	}
	
	String restProxyPasswd() {
		return CommandLine.value("restProxyPasswd");
	}
	
	String restProxyDomain() {
		return CommandLine.value("restProxyDomain");
	}
	
	String restProxyKrb5ConfigFile() {
		return CommandLine.value("restProxyKrb5ConfigFile");
	}

	String krbFile()
	{
		return CommandLine.value("krbfile");
	}

	String keyStoreFile()
	{
		return CommandLine.value("keyfile");
	}

	String keystorePassword()
	{
		return CommandLine.value("keypasswd");
	}

	boolean cacheOption()
	{
		return cacheOption;
	}

	int cacheInterval()
	{
		return cacheInterval;
	}

	String authenticationToken()
	{
		return CommandLine.value("at");
	}

	String authenticationExtended()
	{
		return CommandLine.value("ax");
	}

	String applicationId()
	{
		return CommandLine.value("aid");
	}

	boolean enableRTT() {
		return CommandLine.booleanValue("rtt");
	}

	String protocolList()
	{
		return Objects.nonNull(CommandLine.value("pl")) ? CommandLine.value("pl") : protocolList;
	}

	int itemCount()
	{
		return itemList.size();
	}

	public List<ItemInfo> itemList()
	{
		return itemList;
	}
	
	String startingHostName()
	{
		return startingHostName;
	}
	
	String startingPort()
	{
		return startingPort;
	}
	
	String standbyHostName()
	{
		return standbyHostName;
	}
	
	String standbyPort()
	{
		return standbyPort;
	}
	
	String warmStandbyMode()
	{
		return warmStandbyMode;
	}
	
	boolean enableWarmStandby()
	{
		return enableWarmStandby;
	}
	
	String securityProtocol()
	{
		return securityProtocol;
	}
	
	String securityProtocolVersions()
	{
		return securityProtocolVersions;
	}

	private void parseItems(List<String> itemNames, int domain, boolean isPrivateStream, boolean isSymbollistData, List<ItemArg> itemList)
	{
		for ( String itemName : itemNames)
		{
			ItemArg itemArg = new ItemArg();
			itemArg.domain(domain);

			if (isPrivateStream)
				itemArg.enablePrivateStream(true);
			else
				itemArg.enablePrivateStream(false);
			if (isSymbollistData)
				itemArg.symbolListData(true);

			itemArg.itemName(itemName);
			itemList.add(itemArg);

		}
	}

	void addCommandLineArgs()
	{
		CommandLine.programName("WatchlistConsumer");
		CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain.");
		CommandLine.addOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
		CommandLine.addOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");
		CommandLine.addOption("yc", "For each occurrence, requests item using Yield Curve domain. Default is no yield curve requests.");
		CommandLine.addOption("view", "Specifies each request using a basic dynamic view. Default is false.");
		CommandLine.addOption("post", "Specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream). Default is false.");
		CommandLine.addOption("offpost", "Specifies that the application should attempt to send post messages on the login stream (i.e., off-stream).");
		CommandLine.addOption("publisherInfo", "Specifies that the application should add user provided publisher Id and publisher ipaddress when posting");
		CommandLine.addOption("snapshot", "Specifies each request using non-streaming. Default is false(i.e. streaming requests.)");
		CommandLine.addOption("sl", "Requests symbol list using Symbol List domain. (symbol list name optional). Default is no symbol list requests.");
		CommandLine.addOption("sld", "Requests item on the Symbol List domain and data streams for items on that list.  Default is no symbol list requests.");
		CommandLine.addOption("h", "Server host name");
		CommandLine.addOption("p", "Server port number");
		CommandLine.addOption("if", (String)null, "Interface name");
		CommandLine.addOption("s", defaultServiceName, "Service name");
		CommandLine.addOption("u", "Login user name. Default is system user name.");
		CommandLine.addOption("passwd", "Password for the user name.");
		CommandLine.addOption("pl", "protocol list (defaults to rssl.rwf, tr_json2, rssl.json.v2)");
		CommandLine.addOption("c", "Socket", "Specifies the connection type that the connection should use. Possible values are: 'socket', 'http', 'websocket', 'encrypted'");
		CommandLine.addOption("encryptedConnectionType", "", "Specifies the encrypted connection type that will be used by the consumer, if the 'encrypted' type is selected.  Possible values are 'socket', 'websocket' or 'http'");

		CommandLine.addOption("runTime", defaultRuntime, "Program runtime in seconds");
		CommandLine.addOption("x", "Provides XML tracing of messages.");

		CommandLine.addOption("proxy", "Specifies that the application will make a tunneling connection (http or encrypted) through a proxy server, default is false");
		CommandLine.addOption("ph", "", "Proxy server host name");
		CommandLine.addOption("pp", "", "Proxy port number");
		CommandLine.addOption("plogin", "", "User Name on proxy server");
		CommandLine.addOption("ppasswd", "", "Password on proxy server");
		CommandLine.addOption("pdomain", "", "Proxy server domain");
		CommandLine.addOption("krbfile", "", "KRB File location and name");
		CommandLine.addOption("keyfile", "", "Keystore file location and name");
		CommandLine.addOption("keypasswd", "", "Keystore password");

		CommandLine.addOption("tunnel", "", "(optional) enables consumer to open tunnel stream and send basic text messages");
		CommandLine.addOption("tsServiceName", "", "(optional) specifies the service name for tunnel stream messages (if not specified, the service name specified in -c/-tcp is used");
		CommandLine.addOption("tsAuth", "", "(optional) causes consumer to request authentication when opening a tunnel stream. This applies to basic tunnel streams");
		CommandLine.addOption("tsDomain", "", "(optional) specifes the domain a consumer uses when opening a tunnel stream. This applies to basic tunnel streams");

		CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
		CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
		CommandLine.addOption("aid", "", "Specifies the Application ID.");

		CommandLine.addOption("sessionMgnt", "(optional) Enable Session Management in the reactor.");
		CommandLine.addOption("l", "(optional) Specifies a location to get an endpoint from service endpoint information. Defaults to us-east-1.");
		CommandLine.addOption("query", "", "(optional) Queries EDP service discovery to get an endpoint according to a specified connection type and location.");
		CommandLine.addOption("clientId", "Specifies the client Id for Refinitiv login V2, or specifies a unique ID with login V1 for applications making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.");
		CommandLine.addOption("clientSecret", "Specifies the associated client Secret with a provided clientId for V2 logins.");
		CommandLine.addOption("jwkFile", "Specifies the file location containing the JWK encoded private key for V2 logins.");
		CommandLine.addOption("tokenURLV1", "Specifies the token URL for V1 token oauthpasswd grant type.");
		CommandLine.addOption("tokenURLV2", "Specifies the token URL for V2 token oauthclientcreds grant type.");
		CommandLine.addOption("tokenScope", "", "Specifies the token scope.");
		CommandLine.addOption("audience", "", "Optionally specifies the audience used with V2 JWT logins");
		CommandLine.addOption("serviceDiscoveryURL", "Specifies the service discovery URL.");

		CommandLine.addOption("rtt", false, "(optional) Enable RTT support in the WatchList");
		CommandLine.addOption("takeExclusiveSignOnControl", "true", "Specifies the exclusive sign on control to force sign-out for the same credentials., default is true");
	
		CommandLine.addOption("startingHostName", "", "Specifies the hostname of the starting server in a Warm Standby environment.");
		CommandLine.addOption("startingPort", "", "Specifies the port of the starting server in a Warm Standby environment.");
		CommandLine.addOption("standbyHostName", "", "Specifies the hostname of the standby server in a Warm Standby environment.");
		CommandLine.addOption("standbyPort", "", "Specifies the port of the standby server in a Warm Standby environment.");
		CommandLine.addOption("warmStandbyMode", "", "Specifies the Warm Standby Connection Mode, set either to Login or Service.");
		
		CommandLine.addOption("restProxyHost", "", "Specifies the hostname of the proxy server for rest protocol connections.");
		CommandLine.addOption("restProxyPort", "", "Specifies the port of the proxy server for rest protocol connections.");
		CommandLine.addOption("restProxyUserName", "", "Specifies the user name for the proxy server during rest protocol connections.");
		CommandLine.addOption("restProxyPasswd", "", "Specifies the password for the proxy server during rest protocol connections.");
		CommandLine.addOption("restProxyDomain", "", "Specifies the domain of the proxy server for rest protocol connections.");
		CommandLine.addOption("restProxyKrb5ConfigFile", "", "Specifies the kerberos5 config file used for the proxy server for rest protocol connections.");
		CommandLine.addOption("spTLSv1.2", "Specifies for an encrypted connection to be able to use TLS 1.2. Default enables both TLS version 1.2 and 1.3.");
		CommandLine.addOption("spTLSv1.3", "Specifies for an encrypted connection to be able to use TLS 1.3. Default enables both TLS version 1.2 qnd 1.3.");

	}
}

