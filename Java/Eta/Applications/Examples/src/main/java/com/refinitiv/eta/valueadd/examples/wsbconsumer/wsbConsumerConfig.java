/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2024-2025 LSEG. All rights reserved.    --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.wsbconsumer;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.StringTokenizer;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.EncryptionOptions;
import com.refinitiv.eta.valueadd.reactor.*;

public class wsbConsumerConfig
{	

	// default server host name
	private static final String defaultSrvrHostname = "localhost";

	// default server port number
	private static final String defaultSrvrPortNo = "14002";


	// default service name
	private static final String defaultServiceName = "DIRECT_FEED";

	// default item name
	private static final String defaultItemName = "TRI.N";
	
	private static final String defaultConfigFile = "WSBConfig.json";


	private static final String protocolList = "tr_json2";

	private static final int defaultRuntime = 600;

	private static final int MAX_ITEMS = 128;
	private static final int ITEMS_MIN_STREAM_ID = 5;

	// default preferred host options
	private static final int CONNECT_LIST_INDEX = 0;
	private static final int WSB_GROUP_LIST_INDEX = 0;
	private static final int DETECTION_TIME_INTERVAL = 0;
	private static final String DETECTION_TIME_SCHEDULE = "";

	private final ArrayList<ItemInfo> itemList = new ArrayList<>();
	private String[] securityProtocolVersions = new String[] {"1.3", "1.2"};
	private String configFileName;
	
	private final ObjectMapper mapper = new ObjectMapper();
	private JsonNode root;
	
	private final ReactorConnectOptions reactorConnectOpts = ReactorFactory.createReactorConnectOptions();
	private ReactorPreferredHostOptions reactorPreferredHostOpts = ReactorFactory.createReactorPreferredHostOptions();

	static class ItemInfo
	{
		int	domain;
		String name;
		String serviceName;
		int	streamId;
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
		public String serviceName()
		{
			return serviceName;
		}
		public void serviceName(String serviceName)
		{
			this.serviceName = serviceName;
		}
		public int streamId()
		{
			return streamId;
		}
		public void streamId(int streamId)
		{
			this.streamId = streamId;
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
	
	private enum EncryptionProtocolFlags {
		NONE(0),
		TLSV1_2(0x4),
		TLSV1_3(0x8);
		
	    private int value;

	    private EncryptionProtocolFlags(int value) {
	        this.value = value;
	    }
	}

	public boolean init(String[]args, ChannelInfo channelInfo)
	{
		boolean ret;


		if (!(ret = parseArgs(args, channelInfo)))
		{
			return ret;
		}
		return true;
	}


	public void addItem(String itemName, String serviceName, int domainType)
	{
		ItemInfo itemInfo = new ItemInfo();
		itemInfo.domain(domainType);
		itemInfo.name(itemName);
		itemInfo.serviceName(serviceName);
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
		else
			return null;
	}

	public boolean parseArgs(String[] args, ChannelInfo chnlInfo)
	{
		reactorConnectOpts.clear();

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
		}

		addCommandLineArgs();
		try
		{
			CommandLine.parseArgs(args);
			
			configFileName = CommandLine.value("configJson");

			List<String> itemNames = CommandLine.values("mp");
			if ( itemNames != null && !itemNames.isEmpty())
				parseItems(itemNames, DomainTypes.MARKET_PRICE);

			itemNames = CommandLine.values("mbo");
			if ( itemNames != null && !itemNames.isEmpty())
				parseItems(itemNames, DomainTypes.MARKET_BY_ORDER);

			itemNames = CommandLine.values("mbp");
			if ( itemNames != null && !itemNames.isEmpty())
				parseItems(itemNames, DomainTypes.MARKET_BY_PRICE);

			if(isIoctlArgsIncorrect()) {
				throw new IllegalArgumentException("\nioctlInterval should have a positive value if any ioctl parameters are specified");
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
		
		if(itemList.isEmpty())
		{
			addItem(defaultItemName, serviceName(), DomainTypes.MARKET_PRICE);
		}
		
		try
		{
			byte[] jsonFile = Files.readAllBytes(Paths.get(configFileName));
			String jsonText = new String(jsonFile);
			ArrayNode arrayNode;
			ArrayNode tmpChannelNode;
			JsonNode tmpArrayNode;
			JsonNode tmpNode;
			
			root = mapper.readTree(jsonText);

			reactorPreferredHostOpts = parsePreferredHostOptions();

			arrayNode = (ArrayNode)root.get("WSBGroups");
			
			if(arrayNode != null)
			{
				if(!arrayNode.isArray())
				{
					System.err.println("WSB Group is not a Json array");
					System.exit(CodecReturnCodes.FAILURE);
				}
				
				for(int i = 0; i < arrayNode.size(); i++)
				{
					tmpArrayNode = arrayNode.get(i);
					tmpNode = tmpArrayNode.get("WSBMode");
					ReactorWarmStandbyGroup wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
					
					if(tmpNode == null)
					{
						System.err.println("WSB Group is missing a WSBMode.  Acceptable inputs are login or service");
						System.exit(CodecReturnCodes.FAILURE);
					}
					
					if(tmpNode.asText().equalsIgnoreCase("login"))
					{
						wsbGroup.warmStandbyMode(ReactorWarmStandbyMode.LOGIN_BASED);
					}
					else if(tmpNode.asText().equalsIgnoreCase("service"))
					{
						wsbGroup.warmStandbyMode(ReactorWarmStandbyMode.SERVICE_BASED);
					}
					else
					{
						System.err.println("WSB Group has an invalid WSBMode.  Acceptable inputs are login or service");
						System.exit(CodecReturnCodes.FAILURE);
					}
					
					tmpNode = tmpArrayNode.get("WSBActive");
					
					if(tmpNode == null)
					{
						System.err.println("WSB Group is missing an active channel configruation.");
						System.exit(CodecReturnCodes.FAILURE);
					}
					
					parseWsbChannel(tmpNode, wsbGroup.startingActiveServer(), chnlInfo);
					
					tmpChannelNode = (ArrayNode)tmpArrayNode.get("WSBStandby");
					
					if(tmpChannelNode != null)  
					{
						if(!tmpChannelNode.isArray())
						{
							System.err.println("WSB Group's WSBStandby list is not a JSON array.");
							System.exit(CodecReturnCodes.FAILURE);
						}
						
						for(int j = 0; j < tmpChannelNode.size(); j++)
						{
							tmpNode = tmpChannelNode.get(j);
							ReactorWarmStandbyServerInfo newWsbInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
							
							parseWsbChannel(tmpNode, newWsbInfo, chnlInfo);
							
							wsbGroup.standbyServerList().add(newWsbInfo);	
						}
					}
					
					reactorConnectOpts.reactorWarmStandbyGroupList().add(wsbGroup);
				}
			}
			
			//root = mapper.readTree(jsonText);
			arrayNode = (ArrayNode)root.get("ConnectionList");
			
			if(arrayNode != null)
			{
				if(!arrayNode.isArray())
				{
					System.err.println("ConnectionList is not a Json array");
					System.exit(CodecReturnCodes.FAILURE);
				}
				
				for(int i = 0; i < arrayNode.size(); i++)
				{
					tmpNode = arrayNode.get(i);
					
					if(tmpNode != null)  
					{					
						ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
							
						parseChannel(tmpNode, connectInfo, chnlInfo);
							
						reactorConnectOpts.connectionList().add(connectInfo);	
					}
				}
			}						
		}
		catch(Exception e)
		{
			System.err.println("Error loading file: " + e.getMessage());
			System.err.println();
			System.err.println(CommandLine.optionHelpString());
			System.out.println("Consumer exits...");
			System.exit(CodecReturnCodes.FAILURE);
		}
		
		reactorConnectOpts.reconnectAttemptLimit(reconnectAttemptLimit()); // attempt to recover forever
		reactorConnectOpts.reconnectMinDelay(reconnectMinDelay()); // minimum delay
		reactorConnectOpts.reconnectMaxDelay(reconnectMaxDelay()); // maximum delay

		return true;
	}

	private boolean isIoctlArgsIncorrect() {
		return (ioctlInterval() <= 0) &&
				(hasIoctlEnablePH() ||
						hasIoctlConnectListIndex() ||
						hasIoctlDetectionTimeInterval() ||
						hasIoctlDetectionTimeSchedule() ||
						hasIoctlWarmstandbyGroupListIndex() ||
						hasIoctlFallBackWithinWSBGroup());
	}

	ReactorConnectOptions connectOpts()
	{
		return reactorConnectOpts;
	}

	ReactorPreferredHostOptions preferredHostOpts()
	{
		return reactorPreferredHostOpts;
	}

	String userName()
	{
		return CommandLine.value("u");
	}

	String password()
	{
		return CommandLine.value("passwd");
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
	
	String serviceName()
	{
		return CommandLine.value("s");
	}

	String keyStoreFile()
	{
		return CommandLine.value("keyfile");
	}

	String keystorePassword()
	{
		return CommandLine.value("keypasswd");
	}

	String securityProvider()
	{
		return CommandLine.value("securityProvider");
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

	String applicationId()
	{
		return CommandLine.value("aid");
	}

	String protocolList()
	{
		return Objects.nonNull(CommandLine.value("pl")) ? CommandLine.value("pl") : protocolList;
	}
	
	int reconnectAttemptLimit()
	{
		return CommandLine.intValue("reconnectAttemptLimit");
	}

	int reconnectMinDelay()
	{
		return CommandLine.intValue("reconnectMinDelay");
	}

	int reconnectMaxDelay()
	{
		return CommandLine.intValue("reconnectMaxDelay");
	}

	int fallBackInterval()
	{
		return CommandLine.intValue("fallBackInterval");
	}

	int ioctlInterval()
	{
		return CommandLine.intValue("ioctlInterval");
	}

	boolean ioctlEnablePH()
	{
		return CommandLine.booleanValue("ioctlEnablePH");
	}

	boolean hasIoctlEnablePH()
	{
		return CommandLine.hasArg("ioctlEnablePH");
	}

	int ioctlConnectListIndex()
	{
		return CommandLine.intValue("ioctlConnectListIndex");
	}

	boolean hasIoctlConnectListIndex()
	{
		return CommandLine.hasArg("ioctlConnectListIndex");
	}

	int ioctlDetectionTimeInterval()
	{
		return CommandLine.intValue("ioctlDetectionTimeInterval");
	}

	boolean hasIoctlDetectionTimeInterval()
	{
		return CommandLine.hasArg("ioctlDetectionTimeInterval");
	}

	String ioctlDetectionTimeSchedule()
	{
		return CommandLine.value("ioctlDetectionTimeSchedule");
	}

	boolean hasIoctlDetectionTimeSchedule()
	{
		return CommandLine.hasArg("ioctlDetectionTimeSchedule");
	}

	int ioctlWarmstandbyGroupListIndex()
	{
		return CommandLine.intValue("ioctlWarmstandbyGroupListIndex");
	}

	boolean hasIoctlWarmstandbyGroupListIndex()
	{
		return CommandLine.hasArg("ioctlWarmstandbyGroupListIndex");
	}

	boolean ioctlFallBackWithinWSBGroup()
	{
		return CommandLine.booleanValue("ioctlFallBackWithinWSBGroup");
	}

	boolean hasIoctlFallBackWithinWSBGroup()
	{
		return CommandLine.hasArg("ioctlFallBackWithinWSBGroup");
	}
	
	int itemCount()
	{
		return itemList.size();
	}

	public List<ItemInfo> itemList()
	{
		return itemList;
	}
	
	private void parseItems(List<String> itemNames, int domain)
	{
		for ( String itemName : itemNames)
		{
			StringTokenizer token = new StringTokenizer(itemName, ":");
			String name;
			String serviceName;
			if(token.countTokens() == 1)
			{
				addItem(token.nextToken(), serviceName(), domain);
			}
			else if(token.countTokens() == 2)
			{
				serviceName = token.nextToken();
				name = token.nextToken();
				addItem(name, serviceName, domain);
			}
			else
			{
				System.err.println("Invalid item name.  This needs to be either <item_name> or <service_name>:<item_name");
				System.err.println();
				System.err.println(CommandLine.optionHelpString());
				System.out.println("Consumer exits...");
				System.exit(CodecReturnCodes.FAILURE);
			}
		}
	}

	private ReactorPreferredHostOptions parsePreferredHostOptions()
	{
		JsonNode preferredHostNode = root.get("PreferredHost");

		ReactorPreferredHostOptions rphOptions = ReactorFactory.createReactorPreferredHostOptions();
		if(preferredHostNode != null)
		{
			JsonNode tmpNode = preferredHostNode.get("enablePH");

			if(tmpNode != null)
			{
                rphOptions.isPreferredHostEnabled(tmpNode.asBoolean());
			}

			tmpNode = preferredHostNode.get("fallBackWithinWSBGroup");

			if(tmpNode != null)
			{
                rphOptions.fallBackWithInWSBGroup(tmpNode.asBoolean());
			}

			tmpNode = preferredHostNode.get("detectionTimeInterval");

			if(tmpNode != null)
			{
				rphOptions.detectionTimeInterval(tmpNode.asInt());
			} else {
				rphOptions.detectionTimeInterval(DETECTION_TIME_INTERVAL);
			}

			tmpNode = preferredHostNode.get("connectListIndex");

			if(tmpNode != null)
			{
				rphOptions.connectionListIndex(tmpNode.asInt());
			} else {
				rphOptions.connectionListIndex(CONNECT_LIST_INDEX);
			}

			tmpNode = preferredHostNode.get("warmstandbyGroupListIndex");

			if(tmpNode != null)
			{
				rphOptions.warmStandbyGroupListIndex(tmpNode.asInt());
			} else {
				rphOptions.warmStandbyGroupListIndex(WSB_GROUP_LIST_INDEX);
			}

			tmpNode = preferredHostNode.get("detectionTimeSchedule");

			if(tmpNode != null)
			{
				rphOptions.detectionTimeSchedule(tmpNode.asText(DETECTION_TIME_SCHEDULE));
			} else {
				rphOptions.detectionTimeSchedule(DETECTION_TIME_SCHEDULE);
			}
		} else {
			System.out.println("Preferred host options is missing. Default preferred host options will be used.");
		}

		return rphOptions;
	}

	private void parseWsbChannel(JsonNode channelJson, ReactorWarmStandbyServerInfo wsbServerInfo, ChannelInfo channelInfo)
	{
		ArrayNode tmpJsonNode;
		
		tmpJsonNode = (ArrayNode)channelJson.get("ServiceList");
		
		if(tmpJsonNode != null)
		{
			if(!tmpJsonNode.isArray())
			{
				System.err.println("WSB Channel's Service List is not an array.");
				System.exit(CodecReturnCodes.FAILURE);
			}
			
			for(int i = 0; i < tmpJsonNode.size(); i++)
			{
				Buffer serviceName = CodecFactory.createBuffer();
				serviceName.data(tmpJsonNode.get(i).asText());
				wsbServerInfo.perServiceBasedOptions().serviceNameList().add(serviceName);
			}
		}
			
		parseChannel(channelJson, wsbServerInfo.reactorConnectInfo(), channelInfo);
	}
	
	private void parseChannel(JsonNode channelJson, ReactorConnectInfo connectInfo, ChannelInfo channelInfo)
	{
		JsonNode tmpJsonNode;
		
		String localIPaddress = null;
		String localHostName;

		try
		{
			localIPaddress = InetAddress.getLocalHost().getHostAddress();
			localHostName = InetAddress.getLocalHost().getHostName();
		}
		catch (UnknownHostException e)
		{
			localHostName = localIPaddress;
		}
		
		tmpJsonNode = channelJson.get("Host");
		if(tmpJsonNode == null)
		{
			connectInfo.connectOptions().unifiedNetworkInfo().address(defaultSrvrHostname);
		}		
		else
		{
			connectInfo.connectOptions().unifiedNetworkInfo().address(tmpJsonNode.asText());
		}
		
		tmpJsonNode = channelJson.get("Port");
		if(tmpJsonNode == null)
		{
			connectInfo.connectOptions().unifiedNetworkInfo().serviceName(defaultSrvrPortNo);
		}		
		else
		{
			connectInfo.connectOptions().unifiedNetworkInfo().serviceName(tmpJsonNode.asText());
		}
		
		tmpJsonNode = channelJson.get("Interface");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().unifiedNetworkInfo().interfaceName(tmpJsonNode.asText());
		}

		tmpJsonNode = channelJson.get("ConnType");
		if(tmpJsonNode == null)
		{
			connectInfo.connectOptions().connectionType(ConnectionTypes.SOCKET);
		}		
		else
		{
			connectInfo.connectOptions().connectionType(tmpJsonNode.asInt());
		}
		
		tmpJsonNode = channelJson.get("EncryptedConnType");
		if(tmpJsonNode == null)
		{
			connectInfo.connectOptions().encryptionOptions().connectionType(ConnectionTypes.SOCKET);
		}		
		else
		{
			connectInfo.connectOptions().encryptionOptions().connectionType(tmpJsonNode.asInt());
		}
		tmpJsonNode = channelJson.get("EncryptionProtocolFlags");
		if(tmpJsonNode != null)
		{
			
			int encryptionProtocolFlags = tmpJsonNode.asInt();
			if (encryptionProtocolFlags == EncryptionProtocolFlags.TLSV1_2.value)
			{
				// Enable TLS 1.2
				securityProtocolVersions = new String []{"1.2"};
			}
			if (encryptionProtocolFlags == EncryptionProtocolFlags.TLSV1_3.value)
			{
				// Enable TLS 1.3
				securityProtocolVersions = new String []{"1.3"};
			}
		}
		
		if(connectInfo.connectOptions().connectionType() == ConnectionTypes.ENCRYPTED)
		{
			setEncryptedConfiguration(connectInfo.connectOptions());
		}
		
		tmpJsonNode = channelJson.get("ProtocolList");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().wSocketOpts().protocols(tmpJsonNode.asText());
		}
		else
		{
			connectInfo.connectOptions().wSocketOpts().protocols("rssl.rwf, rssl.json.v2");
		}
		
		tmpJsonNode = channelJson.get("SessionMgnt");
		if(tmpJsonNode != null)
		{
			connectInfo.enableSessionManagement(tmpJsonNode.asBoolean());
		}
		
		tmpJsonNode = channelJson.get("Location");
		if(tmpJsonNode != null)
		{
			connectInfo.location(tmpJsonNode.asText());
		}
		
		tmpJsonNode = channelJson.get("ProxyHost");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().tunnelingInfo().HTTPproxy(true);
			connectInfo.connectOptions().tunnelingInfo().HTTPproxyHostName(tmpJsonNode.asText());
			connectInfo.connectOptions().credentialsInfo().HTTPproxyLocalHostname(localHostName);
		}
		
		tmpJsonNode = channelJson.get("ProxyPort");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().tunnelingInfo().HTTPproxyPort(Integer.parseInt(tmpJsonNode.asText()));
		}
		
		tmpJsonNode = channelJson.get("ProxyUserName");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().credentialsInfo().HTTPproxyUsername(tmpJsonNode.asText());
		}
		
		tmpJsonNode = channelJson.get("ProxyPassword");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().credentialsInfo().HTTPproxyPasswd(tmpJsonNode.asText());
		}
		
		tmpJsonNode = channelJson.get("ProxyDomain");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().credentialsInfo().HTTPproxyPasswd(tmpJsonNode.asText());
		}
		
		tmpJsonNode = channelJson.get("Krbfile");
		if(tmpJsonNode != null)
		{
			connectInfo.connectOptions().credentialsInfo().HTTPproxyKRB5configFile(tmpJsonNode.asText());
		}
		
		connectInfo.connectOptions().majorVersion(Codec.majorVersion());
		connectInfo.connectOptions().minorVersion(Codec.minorVersion());
		connectInfo.connectOptions().guaranteedOutputBuffers(1000);
		
		connectInfo.connectOptions().userSpecObject(channelInfo);
		
	}

	private void setEncryptedConfiguration(ConnectOptions options)
	{
		String keyFile = keyStoreFile();
		String keyPasswd = keystorePassword();
		String securityProvider = securityProvider();

		if (keyFile != null && !keyFile.isEmpty())
		{
			options.encryptionOptions().KeystoreFile(keyFile);
		}
		if (keyPasswd != null && !keyPasswd.isEmpty())
		{
			options.encryptionOptions().KeystorePasswd(keyPasswd);
		}
		if (securityProvider != null && !securityProvider.isEmpty())
		{
			options.encryptionOptions().SecurityProvider(securityProvider);
		}
		options.encryptionOptions().SecurityProtocolVersions(securityProtocolVersions);
	}

	void addCommandLineArgs()
	{
		CommandLine.programName("wsbConsumer");
		CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain. Item names can be optionally <service_name>:<item_name> to specify a specific service");
		CommandLine.addOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
		CommandLine.addOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");

		CommandLine.addOption("configJson", defaultConfigFile,  "Json config file path");
		CommandLine.addOption("runTime", defaultRuntime, "Program runtime in seconds");
		CommandLine.addOption("x", "Provides XML tracing of messages.");
		
		CommandLine.addOption("keyfile", "", "Keystore file for encryption");
		CommandLine.addOption("keypasswd", "", "Keystore password");
		CommandLine.addOption("securityProvider", "", "Specifies security provider, default is SunJSSE, also supports Conscrypt");

		CommandLine.addOption("s", defaultServiceName,  "Default service name for requests. This will be used for dictionary requests as well as any items that do not have a default service specified in -mp, -mbo, or -mbp.");

		CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
		CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
		CommandLine.addOption("aid", "", "Specifies the Application ID.");
		
		CommandLine.addOption("u", "Login user name. Default is system user name.");
		CommandLine.addOption("passwd", "Password for the user name.");
		CommandLine.addOption("clientId", "Specifies the client Id for V2 authentication OR specifies a unique ID, also known as AppKey generated by an AppGenerator, for V1 authentication usedwhen connecting to Real-Time Optimized.");
		CommandLine.addOption("clientSecret", "Specifies the associated client Secret with a provided clientId for V2 logins.");
		CommandLine.addOption("jwkFile", "Specifies the file location containing the JWK encoded private key for V2 logins.");
		CommandLine.addOption("tokenURLV1", "Specifies the token URL for V1 token oauthpasswd grant type.");
		CommandLine.addOption("tokenURLV2", "Specifies the token URL for V2 token oauthclientcreds grant type.");
		CommandLine.addOption("tokenScope", "", "Specifies the token scope.");
		CommandLine.addOption("audience", "", "Optionally specifies the audience used with V2 JWT logins");
		CommandLine.addOption("serviceDiscoveryURL", "Specifies the service discovery URL.");
		CommandLine.addOption("takeExclusiveSignOnControl", "true", "Specifies the exclusive sign on control to force sign-out for the same credentials., default is true");

		CommandLine.addOption("fallBackInterval", 0, "Specifies time interval (in second) in application before Ad Hoc Fallback function is invoked. O indicates that function won't be invoked. Default is 0");
		CommandLine.addOption("ioctlInterval", 0, "Specifies time interval (in second) before IOCtl function is invoked. O indicates that function won't be invoked. Default is 0");
		CommandLine.addOption("ioctlEnablePH", false, "Enables Preferred host feature. Possible values are true/false");
		CommandLine.addOption("ioctlConnectListIndex", 0, "Specifies the preferred host as the index in the connection list");
		CommandLine.addOption("ioctlDetectionTimeInterval", 0, "Specifies time interval (in second) to switch over to a preferred host. 0 indicates that the detection time interval is disabled");
		CommandLine.addOption("ioctlDetectionTimeSchedule", "", "Specifies Cron time format to switch over to a preferred host");
		CommandLine.addOption("ioctlWarmstandbyGroupListIndex", 0, "Specifies the preferred WSB group as the index in the WarmStandbyGroup list");
		CommandLine.addOption("ioctlFallBackWithinWSBGroup", false, "Specifies whether to fallback within a WSB group instead of moving into a preferred WSB group. Possible values are true/false");
		
		CommandLine.addOption("restProxyHost", "", "Specifies the hostname of the proxy server for rest protocol connections.");
		CommandLine.addOption("restProxyPort", "", "Specifies the port of the proxy server for rest protocol connections.");
		CommandLine.addOption("restProxyUserName", "", "Specifies the user name for the proxy server during rest protocol connections.");
		CommandLine.addOption("restProxyPasswd", "", "Specifies the password for the proxy server during rest protocol connections.");
		CommandLine.addOption("restProxyDomain", "", "Specifies the domain of the proxy server for rest protocol connections.");
		CommandLine.addOption("restProxyKrb5ConfigFile", "", "Specifies the kerberos5 config file used for the proxy server for rest protocol connections.");

		CommandLine.addOption("reconnectAttemptLimit", -1, "Specifies the maximum number of reconnection attempts. Default is -1");
		CommandLine.addOption("reconnectMinDelay", 500, "Specifies minimum delay (in milliseconds) between reconnection attempts. Default is 500");
		CommandLine.addOption("reconnectMaxDelay", 6000, "Specifies maximum delay (in milliseconds) between reconnection attempts. Default is 6000");

	}
}



