/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
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
import com.refinitiv.eta.valueadd.reactor.ReactorConnectInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyGroup;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServerInfo;

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


	private String protocolList = "tr_json2";

	private static final int defaultRuntime = 600;

	private int MAX_ITEMS = 128;
	private int ITEMS_MIN_STREAM_ID = 5;

	
	private ArrayList<ItemInfo> itemList = new ArrayList<ItemInfo>();
	
	private String configFileName;
	
	private ObjectMapper mapper = new ObjectMapper();
	private JsonNode root;
	
	private ReactorConnectOptions reactorConnectOpts = ReactorFactory.createReactorConnectOptions();

	class ItemInfo
	{
		int	domain;
		String name;
		String serviceName;
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

	public boolean init(String[]args, ChannelInfo channelInfo)
	{
		boolean ret;


		if ((ret = parseArgs(args, channelInfo)) == false )
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
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.MARKET_PRICE);

			itemNames = CommandLine.values("mbo");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.MARKET_BY_ORDER);

			itemNames = CommandLine.values("mbp");
			if ( itemNames != null && itemNames.size() > 0 )
				parseItems(itemNames, DomainTypes.MARKET_BY_PRICE);
			
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
		
		if(itemList.size() == 0)
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
			arrayNode = (ArrayNode)root.get("WSBGroups");
			
			if(arrayNode != null)
			{
				if(arrayNode.isArray() == false)
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
					
					if(tmpNode.asText().toLowerCase().equals("login"))
					{
						wsbGroup.warmStandbyMode(ReactorWarmStandbyMode.LOGIN_BASED);
					}
					else if(tmpNode.asText().toLowerCase().equals("service"))
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
						if(tmpChannelNode.isArray() == false)
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
			
			root = mapper.readTree(jsonText);
			arrayNode = (ArrayNode)root.get("ConnectionList");
			
			if(arrayNode != null)
			{
				if(arrayNode.isArray() == false)
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
		
		reactorConnectOpts.reconnectAttemptLimit(reconnectCount()); // attempt to recover forever
		reactorConnectOpts.reconnectMinDelay(500); // 0.5 second minimum
		reactorConnectOpts.reconnectMaxDelay(3000); // 3 second maximum
		

		
		return true;
	}
	
	ReactorConnectOptions connectOpts()
	{
		return reactorConnectOpts;
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

	String applicationId()
	{
		return CommandLine.value("aid");
	}

	String protocolList()
	{
		return Objects.nonNull(CommandLine.value("pl")) ? CommandLine.value("pl") : protocolList;
	}
	
	int reconnectCount()
	{
		return CommandLine.intValue("reconnectCount");
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
	
	private void parseWsbChannel(JsonNode channelJson, ReactorWarmStandbyServerInfo wsbServerInfo, ChannelInfo channelInfo)
	{
		ArrayNode tmpJsonNode;
		
		tmpJsonNode = (ArrayNode)channelJson.get("ServiceList");
		
		if(tmpJsonNode != null)
		{
			if(tmpJsonNode.isArray() == false)
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
		String localHostName = null;

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
		
		if(connectInfo.connectOptions().connectionType() == ConnectionTypes.ENCRYPTED)
		{
			setEncryptedConfiguration(connectInfo.connectOptions());
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

		if (keyFile != null && !keyFile.isEmpty())
		{
			options.encryptionOptions().KeystoreFile(keyFile);
		}
		if (keyPasswd != null && !keyFile.isEmpty())
		{
			options.encryptionOptions().KeystorePasswd(keyPasswd);
		}
		
		options.encryptionOptions().KeystoreType("JKS");
		options.encryptionOptions().SecurityProtocol("TLS");
		options.encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
		options.encryptionOptions().SecurityProvider("SunJSSE");
		options.encryptionOptions().KeyManagerAlgorithm("SunX509");
		options.encryptionOptions().TrustManagerAlgorithm("PKIX");
	}

	void addCommandLineArgs()
	{
		CommandLine.programName("WatchlistConsumer");
		CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain. Item names can be optionally <service_name>:<item_name> to specify a specific service");
		CommandLine.addOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
		CommandLine.addOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");

		CommandLine.addOption("configJson", defaultConfigFile,  "Json config file path");
		CommandLine.addOption("runTime", defaultRuntime, "Program runtime in seconds");
		CommandLine.addOption("x", "Provides XML tracing of messages.");
		
		CommandLine.addOption("keyfile", "", "Keystore file for encryption");
		CommandLine.addOption("keypasswd", "", "Keystore password");
		
		CommandLine.addOption("s", defaultServiceName,  "Default service name for requests. This will be used for dictionary requests as well as any items that do not have a default service specified in -mp, -mbo, or -mbp.");

		CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
		CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
		CommandLine.addOption("aid", "", "Specifies the Application ID.");
		
		CommandLine.addOption("u", "Login user name. Default is system user name.");
		CommandLine.addOption("passwd", "Password for the user name.");
		CommandLine.addOption("clientId", "Specifies the client Id for Refinitiv login V2, or specifies a unique ID for application making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.");
		CommandLine.addOption("clientSecret", "Specifies the associated client Secret with a provided clientId for V2 logins.");
		CommandLine.addOption("jwkFile", "Specifies the file location containing the JWK encoded private key for V2 logins.");
		CommandLine.addOption("tokenURLV1", "Specifies the token URL for V1 token oauthpasswd grant type.");
		CommandLine.addOption("tokenURLV2", "Specifies the token URL for V2 token oauthclientcreds grant type.");
		CommandLine.addOption("tokenScope", "", "Specifies the token scope.");
		CommandLine.addOption("audience", "", "Optionally specifies the audience used with V2 JWT logins");
		CommandLine.addOption("serviceDiscoveryURL", "Specifies the service discovery URL.");
		CommandLine.addOption("takeExclusiveSignOnControl", "true", "Specifies the exclusive sign on control to force sign-out for the same credentials., default is true");
		
		CommandLine.addOption("reconnectCount", -1, "Reconnection attempt count");

	}
}



