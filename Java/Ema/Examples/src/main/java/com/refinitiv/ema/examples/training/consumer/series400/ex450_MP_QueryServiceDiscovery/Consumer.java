///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license                             --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.                         --
// *|                See the project's LICENSE.md for details.                                         --
// *|           Copyright (C) 2020-2022 Refinitiv. All rights reserved.                                --
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex450_MP_QueryServiceDiscovery;

import com.refinitiv.ema.access.Msg;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ServiceEndpointDiscovery;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryClient;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryEvent;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryOption;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryResp;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryInfo;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Objects;

class AppClient implements OmmConsumerClient, ServiceEndpointDiscoveryClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println(refreshMsg);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println(updateMsg);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println(statusMsg);
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	public void onSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent event)
	{
		System.out.println(serviceEndpointResp); // dump service discovery endpoints
		String endPoint = null;
		String port = null;
		for(ServiceEndpointDiscoveryInfo info : serviceEndpointResp.serviceEndpointInfoList())
		{
			if(info.locationList().size() >= 2 && info.locationList().get(0).startsWith(Consumer.location)) // Get an endpoint that provides auto failover for the specified location.
			{
				endPoint = info.endpoint();
				port = info.port();
				break;
			}
			// Try to get backups and keep looking for main case. Keep only the first item met.
			else if(info.locationList().size() > 0 && info.locationList().get(0).startsWith(Consumer.location) &&
					endPoint == null && port == null)
			{
				endPoint = info.endpoint();
				port = info.port();
			}
		}

		if(Objects.nonNull(endPoint) && Objects.nonNull(port)) {
			Consumer.host = endPoint;
			Consumer.port = port;
		}
	}

	public void onError(String errorText, ServiceEndpointDiscoveryEvent event)
	{
		System.out.println("Failed to query RDP service discovery. Error text: " + errorText);
	}
}

public class Consumer
{
	static String userName;
	static String password;
	static String clientId;
	static String clientSecret;
	static String clientJwk;
	static String audience;
	static String proxyHostName;
	static String proxyPort = "-1";
	static String proxyUserName;
	static String proxyPassword;
	static String proxyDomain;
	static String proxyKrb5Configfile;
	static boolean takeExclusiveSignOnControl = true;
	static boolean connectWebSocket = false;
	public static String host;
	public static String port;
	public static String location = "us-east-1";

	public static String itemName = "IBM.N";

	public static String tokenUrlV1 = null;
	public static String tokenUrlV2 = null;
	public static String serviceDiscoveryUrl = null;

	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -username machine ID to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory for V1 password grant).\n"
	    		+ "  -password password to perform authorization with the token \r\n"
	    		+ "\tservice (mandatory for V1 password grant).\n"
	    		+ "  -location location to get an endpoint from RDP service \r\n"
	    		+ "\tdiscovery. Defaults to \"us-east-1\" (optional).\n"
	    		+ "  -clientId client ID for application making the request to(mandatory for V1 password grant and V2 client credentials grant) \r\n"
	    		+ "  -clientSecret service account secret (mandatory for V2 client credentials grant).\n"
	    		+ "  -jwkFile file containing the private JWK encoded in JSON format. (mandatory for V2 client credentials grant with JWT)\n"
	    		+ "  -audience Audience value for JWT (optional for V2 oAuth client credentials with JWT).\n"
	    		+ "  -websocket Use the WebSocket transport protocol (optional) \r\n"
	    		+ "\tRDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
	    		+ "  -keyfile keystore file for encryption.\n"
	    		+ "  -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials(optional).\r\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
				+ "  -tokenURL V1 URL to perform authentication to get access and refresh tokens (optional).\n"
				+ "  -tokenURLV1 V1 URL to perform authentication to get access and refresh tokens (optional).\n"
				+ "  -tokenURLV2 V2 URL to perform authentication to get access and refresh tokens (optional).\n"
				+ "  -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).\n"
	    		+ "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -itemName Request item name (optional).\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n" 
	    		+ "  -pdomain Proxy Domain (optional).\n"
	    		+ "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos \r\n" 
	    		+ "\tand Kerberos authentications (optional).\n"
	    		+ "\n");
	}
	
	static boolean readCommandlineArgs(String[] args, OmmConsumerConfig config)
	{
	    try
	    {
	        int argsCount = 0;

	        while (argsCount < args.length)
	        {
	            if (0 == args[argsCount].compareTo("-?"))
	            {
	                printHelp();
	                return false;
	            }
	            else if ("-username".equals(args[argsCount]))
    			{
	            	userName = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-password".equals(args[argsCount]))
    			{
	            	password = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-clientId".equals(args[argsCount]))
    			{
	            	clientId = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-clientSecret".equals(args[argsCount]))
    			{
	            	clientSecret = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-audience".equals(args[argsCount]))
    			{
	            	audience = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-jwkFile".equals(args[argsCount]))
    			{
	            	String jwkFile = argsCount < (args.length-1) ? args[++argsCount] : null;
	            	if(jwkFile != null)
	            	{
		            	try
						{
							// Get the full contents of the JWK file.
							byte[] jwkBuffer = Files.readAllBytes(Paths.get(jwkFile));
							clientJwk = new String(jwkBuffer);
						}
						catch(Exception e)
						{
							System.err.println("Error loading JWK file: " + e.getMessage());
							System.err.println();
							System.out.println("Consumer exits...");
							System.exit(CodecReturnCodes.FAILURE);
						} 
	            	}
    				++argsCount;				
    			}
	            else if ("-location".equals(args[argsCount]))
    			{
	            	location = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-keyfile".equals(args[argsCount]))
    			{
    				config.tunnelingKeyStoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
    				config.tunnelingSecurityProtocol("TLS");
    				++argsCount;				
    			}	
    			else if ("-keypasswd".equals(args[argsCount]))
    			{
    				config.tunnelingKeyStorePasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-itemName".equals(args[argsCount]))
    			{
    				itemName = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;
    			}
    			else if ("-ph".equals(args[argsCount]))
    			{
    				proxyHostName = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-pp".equals(args[argsCount]))
    			{
    				proxyPort = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-plogin".equals(args[argsCount]))
    			{
    				proxyUserName = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-ppasswd".equals(args[argsCount]))
    			{
    				proxyPassword = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-pdomain".equals(args[argsCount]))
    			{
    				proxyDomain = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-krbfile".equals(args[argsCount]))
    			{
    				proxyKrb5Configfile = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-takeExclusiveSignOnControl".equals(args[argsCount]))
    			{
    				String takeExclusiveSignOnControlStr = argsCount < (args.length-1) ? args[++argsCount] : null;
    				
    				if(takeExclusiveSignOnControlStr != null)
    				{
    					if(takeExclusiveSignOnControlStr.equalsIgnoreCase("true"))
    						takeExclusiveSignOnControl = true;
    					else if (takeExclusiveSignOnControlStr.equalsIgnoreCase("false"))
    						takeExclusiveSignOnControl = false;
    				}
    				
    				++argsCount;				
    			}
    			else if ("-websocket".equals(args[argsCount]))
    			{
    				connectWebSocket = true;
    				++argsCount;
    			}
				else if ("-tokenURL".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) ) {
						tokenUrlV1 = args[++argsCount];
						config.tokenServiceUrlV1( tokenUrlV1 );
					}
					++argsCount;
				}
				else if ("-tokenURLV1".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) ) {
						tokenUrlV1 = args[++argsCount];
						config.tokenServiceUrlV1( tokenUrlV1 );
					}
					++argsCount;
				}
				else if ("-tokenURLV2".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) ) {
						tokenUrlV2 = args[++argsCount];
						config.tokenServiceUrlV2( tokenUrlV2 );
					}
					++argsCount;
				}
				else if ("-serviceDiscoveryURL".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) ) {
						serviceDiscoveryUrl = args[++argsCount];
						config.serviceDiscoveryUrl( serviceDiscoveryUrl );
					}
					++argsCount;
				}
    			else // unrecognized command line argument
    			{
    				printHelp();
    				return false;
    			}			
    		}
	        
	        if ( userName == null || password == null)
			{
	        	if(clientId == null || (clientSecret == null && clientJwk == null))
	        	{
					System.out.println("Username/password/clientId or clientId/clientSecret or jwkFile must be specified on the command line. Exiting...");
					printHelp();
					return false;
	        	}
			}
        }
        catch (Exception e)
        {
        	printHelp();
            return false;
        }
		
		return true;
	}
	
	static void createProgramaticConfig(Map configDb)
	{
		Map elementMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
		
		if(connectWebSocket)
		{
			// Use FileDictionary instead of ChannelDictionary as WebSocket connection has issue to download dictionary from Refinitiv Data Platform
			innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
		}
		
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("ConsumerList", elementMap));
		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));

		if(connectWebSocket)
		{
			innerElementList.add(EmaFactory.createElementEntry().ascii("EncryptedProtocolType", "EncryptedProtocolType::RSSL_WEBSOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("WsProtocols", "tr_json2"));
		}
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("Host", host));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", port));
		innerElementList.add(EmaFactory.createElementEntry().intValue("EnableSessionManagement", 1));
		
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("ChannelList", elementMap));
		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();
		
		if(connectWebSocket)
		{
			innerElementList.add( EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary" ));
			innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary" ));
			innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
			elementMap.add( EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList ));
			innerElementList.clear();
		
			elementList.add( EmaFactory.createElementEntry().map( "DictionaryList", elementMap ));
			elementMap.clear();
		
			configDb.add( EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();
		}
	}
	
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		ServiceEndpointDiscovery serviceDiscovery = null;
		try
		{
			AppClient appClient = new AppClient();
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			Map configDb = EmaFactory.createMap();
			
			if (!readCommandlineArgs(args, config)) return;

			serviceDiscovery = EmaFactory.createServiceEndpointDiscovery(tokenUrlV1, tokenUrlV2, serviceDiscoveryUrl);
			
			/* If username, password, and clientId are present, this is a V1 password grant connection */ 
			if ( password != null )
			{
				serviceDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOption().username(userName)
						.password(password).clientId(clientId)
						.transport(connectWebSocket ? ServiceEndpointDiscoveryOption.TransportProtocol.WEB_SOCKET : ServiceEndpointDiscoveryOption.TransportProtocol.TCP)
						.takeExclusiveSignOnControl(takeExclusiveSignOnControl)
						.proxyHostName(proxyHostName).proxyPort(proxyPort).proxyUserName(proxyUserName)
						.proxyPassword(proxyPassword).proxyDomain(proxyDomain).proxyKRB5ConfigFile(proxyKrb5Configfile), appClient);
			}
			else
			{
				if(clientJwk == null)
				{
					/* V2 client credentials grant */
					serviceDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOption().clientId(clientId).clientSecret(clientSecret)
							.transport(connectWebSocket ? ServiceEndpointDiscoveryOption.TransportProtocol.WEB_SOCKET : ServiceEndpointDiscoveryOption.TransportProtocol.TCP)
							.proxyHostName(proxyHostName).proxyPort(proxyPort).proxyUserName(proxyUserName)
							.proxyPassword(proxyPassword).proxyDomain(proxyDomain).proxyKRB5ConfigFile(proxyKrb5Configfile), appClient);
				}
				else
				{
					if(audience == null)
					{
						/* V2 client credentials grant with JWK */
						serviceDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOption().clientId(clientId).clientJWK(clientJwk)
								.transport(connectWebSocket ? ServiceEndpointDiscoveryOption.TransportProtocol.WEB_SOCKET : ServiceEndpointDiscoveryOption.TransportProtocol.TCP)
								.proxyHostName(proxyHostName).proxyPort(proxyPort).proxyUserName(proxyUserName)
								.proxyPassword(proxyPassword).proxyDomain(proxyDomain).proxyKRB5ConfigFile(proxyKrb5Configfile), appClient);
					}
					else
					{
						/* V2 client credentials grant with JWK */
						serviceDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOption().clientId(clientId).clientJWK(clientJwk).audience(audience)
								.transport(connectWebSocket ? ServiceEndpointDiscoveryOption.TransportProtocol.WEB_SOCKET : ServiceEndpointDiscoveryOption.TransportProtocol.TCP)
								.proxyHostName(proxyHostName).proxyPort(proxyPort).proxyUserName(proxyUserName)
								.proxyPassword(proxyPassword).proxyDomain(proxyDomain).proxyKRB5ConfigFile(proxyKrb5Configfile), appClient);
					}
				}
			}
			
			if ( host == null || port == null )
			{
				System.out.println("Both hostname and port are not avaiable for establishing a connection with Refinitiv Real-Time - Optimized. Exiting...");
				return;
			}
			
			createProgramaticConfig(configDb);
			
			if ( (proxyHostName == null) && (proxyPort == "-1") )
			{
				if ( password != null )
				{
					consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").username(userName).password(password)
						.clientId(clientId).takeExclusiveSignOnControl(takeExclusiveSignOnControl).config(configDb));
				}
				else
				{
					if(clientJwk == null)
					{
						consumer = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientId(clientId).clientSecret(clientSecret).config(configDb));
					}
					else
					{
						if(audience == null)
						{
							consumer = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientId(clientId).clientJWK(clientJwk).config(configDb));
						}
						else
						{
							consumer = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientId(clientId).clientJWK(clientJwk).audience(audience).config(configDb));
						}
					}
				}
			}
			else
			{
				if ( password != null )
				{
					consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").username(userName).password(password)
						.clientId(clientId).config(configDb).takeExclusiveSignOnControl(takeExclusiveSignOnControl)
						.tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
						.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
						.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile));
				}
				else
				{
					if(clientJwk == null)
					{
						consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientId(clientId).clientSecret(clientSecret).config(configDb)
								.tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
								.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
								.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile));
					}
					else
					{
						if(audience == null)
						{
							consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientId(clientId).clientJWK(clientJwk).config(configDb)
									.tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
									.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
									.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile));
						}
						else
						{
							consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientId(clientId).clientJWK(clientJwk).config(configDb).audience(audience)
									.tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
									.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
									.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile));
						}
					}
				}
			}
					
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name(itemName), appClient);
			
			Thread.sleep(900000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
			if (serviceDiscovery != null) serviceDiscovery.uninitialize();
		}
	}
}
