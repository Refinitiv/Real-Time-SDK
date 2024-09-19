///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2022 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex451_MP_OAuth2Callback_V2;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OAuth2CredentialRenewal;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ServiceEndpointDiscovery;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmOAuth2ConsumerClient;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryEvent;

class AppClient implements OmmConsumerClient
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

	public void onError(String errorText, ServiceEndpointDiscoveryEvent event)
	{
		System.out.println("Failed to query RDP service discovery. Error text: " + errorText);
	}
}

/* This is for example purposes, For best security, please use a proper credential store. */
class CredentialStore
{
	public String clientSecret;
	public String clientId;
	public OmmConsumer consumer;
}

/* Implementation of OmmOAuth2ConsumerClient.  This is a very basic callback that uses the closure to obtain the OmmConsumer and call submitOAuthCredentialRenewal.
 * This is intended to show functionality, so this example does not implement or use secure credential storage.
 */
class OAuthCallback implements OmmOAuth2ConsumerClient
{
	public void onOAuth2CredentialRenewal(OmmConsumerEvent event)
	{
		CredentialStore credentials = (CredentialStore)event.closure();
		
		OAuth2CredentialRenewal renewal = EmaFactory.createOAuth2CredentialRenewal();
		
		renewal.clientId(credentials.clientId);
		renewal.clientSecret(credentials.clientSecret);
		
		
		System.out.println("Submitting credentials due to token renewal");
		
		credentials.consumer.renewOAuthCredentials(renewal);
	}
}

public class Consumer
{
	static OAuthCallback oAuthCallback = new OAuthCallback();
	static CredentialStore credentials = new CredentialStore();
	static String proxyHostName;
	static String proxyPort = "-1";
	static String proxyUserName;
	static String proxyPassword;
	static String proxyDomain;
	static String proxyKrb5Configfile;
	static boolean connectWebSocket = false;
	public static String host = null;
	public static String port = null;

	// API QA
	static String proxyHostName_pc = null;
	static String proxyPort_pc = "-1";
	static boolean channelTypeEncrypted_pc = true;
	// END API QA
	public static String itemName = "IBM.N";

	public static String tokenUrlV2 = "https://api.refinitiv.com/auth/oauth2/v2/token";

	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -h hostname to connect to (mandatory)\r\n" 
	    		+ "  -p port to connect to (mandatory)\r\n" 
	    		+ "  -clientId machine account to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory).\n"
	    		+ "  -clientSecret associated client secret with the machine id \r\n"
	    		+ "\tservice (mandatory).\n"
	    		+ "  -websocket Use the WebSocket transport protocol (optional) \r\n"
	    		+ "  -keyfile keystore file for encryption(optional).\n"
	    		+ "  -keypasswd keystore password for encryption(if -keyfile is present, this is mandatory).\n"
				+ "  -tokenURLV2 URL to perform authentication to get access and refresh tokens (optional).\n"
	    		+ "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -itemName Request item name (optional).\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n" 
	    		+ "  -pdomain Proxy Domain (optional).\n"
	    		+ "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos \r\n"
	    		+ "  -spTLSv1.2 Enable TLS 1.2 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
	    		+ "  -spTLSv1.3 Enable TLS 1.3 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
	    		+ "\tand Kerberos authentications (optional).\n"
// API QA
				+ "\nOptional parameters for programmatic config.\n"
				+ "  -progph Proxy host name. Programmatic Config. (optional).\n"
				+ "  -progpp Proxy port number. Programmatic Config. (optional).\n"
				+ "  -progChannelType [encryped|socket] Channel type. By default: encrypted. Programmatic Config. (optional).\n"
// END API QA
	    		+ "\n");
	}
	
	static boolean readCommandlineArgs(String[] args, OmmConsumerConfig config)
	{
	    try
	    {
	        int argsCount = 0;
	        boolean tls12 = false;
	        boolean tls13 = false;

	        while (argsCount < args.length)
	        {
	            if (0 == args[argsCount].compareTo("-?"))
	            {
	                printHelp();
	                return false;
	            }
	            else if ("-clientId".equals(args[argsCount]))
    			{
	            	credentials.clientId = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
	            else if ("-clientSecret".equals(args[argsCount]))
    			{
	            	credentials.clientSecret = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-keyfile".equals(args[argsCount]))
    			{
    				config.tunnelingKeyStoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
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
    			else if ("-h".equals(args[argsCount]))
    			{
    				host = argsCount < (args.length-1) ? args[++argsCount] : null;
    				++argsCount;				
    			}
    			else if ("-p".equals(args[argsCount]))
    			{
    				port = argsCount < (args.length-1) ? args[++argsCount] : null;
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
    			else if ("-websocket".equals(args[argsCount]))
    			{
    				connectWebSocket = true;
    				++argsCount;
    			}
				else if ("-tokenURLV2".equals(args[argsCount]))
				{
					if ( argsCount < (args.length-1) ) {
						tokenUrlV2 = args[++argsCount];
						config.tokenServiceUrl( tokenUrlV2 );
					}
					++argsCount;
				}
    			else if ("-spTLSv1.2".equals(args[argsCount]))	   
    			{
    				tls12 = true;
    				++argsCount;
    			}
    			else if ("-spTLSv1.3".equals(args[argsCount]))
    			{
    				tls13 = true;
    				++argsCount;
    			}
// API QA
				else if ("-progph".equals(args[argsCount]))
				{
					proxyHostName_pc = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-progpp".equals(args[argsCount]))
				{
					proxyPort_pc = argsCount < (args.length-1) ? args[++argsCount] : null;
					++argsCount;
				}
				else if ("-progChannelType".equals(args[argsCount]))
				{
					if (argsCount < (args.length-1)) {
						if ("socket".equals(args[++argsCount]))
						{
							channelTypeEncrypted_pc = false;
						}
					}
					++argsCount;
				}
// END API QA
    			else // unrecognized command line argument
    			{
    				printHelp();
    				return false;
    			}			
    		}
	        
	        // Set security protocol versions of TLS based on configured values, with default having TLS 1.2 and 1.3 enabled
	        if ((tls12 && tls13) || (!tls12 && !tls13))
	        {
	        	config.tunnelingSecurityProtocol("TLS");
	        	config.tunnelingSecurityProtocolVersions(new String[] {"1.2", "1.3"});
	        }
	        else if (tls12)
	        {
	        	config.tunnelingSecurityProtocol("TLS");
	        	config.tunnelingSecurityProtocolVersions(new String[]{"1.2"});
	        }
	        else if (tls13)
	        {
	        	config.tunnelingSecurityProtocol("TLS");
	        	config.tunnelingSecurityProtocolVersions(new String[]{"1.3"});
	        }
	        
	        if (host == null || port == null || credentials.clientSecret == null || credentials.clientId == null)
			{
				System.out.println("host, port, ClientId and client Secret must be specified on the command line. Exiting...");
				printHelp();
				return false;
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
			// Use FileDictionary instead of ChannelDictionary as WebSocket connection has issue to download dictionary from Delivery Platform
			innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
		}
		
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("ConsumerList", elementMap));
		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();
		
		//innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
// API QA
		if (channelTypeEncrypted_pc) {
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
		}
		else if(connectWebSocket) {
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_WEBSOCKET"));
		}
		else {
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
		}

		if (proxyHostName_pc != null && !proxyHostName_pc.isEmpty()
			&& proxyPort_pc != null  && !proxyPort_pc.isEmpty() && (proxyPort_pc != "-1"))
		{
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyHost", proxyHostName_pc));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyPort", proxyPort_pc));
		}
// END API QA

		if(connectWebSocket)
		{
			if (channelTypeEncrypted_pc)  // API QA
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

			createProgramaticConfig(configDb);
			
			if ( (proxyHostName == null) && (proxyPort == "-1") )
			{
				System.out.println("Consumer_1  no proxy");
				consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientSecret(credentials.clientSecret)
					.clientId(credentials.clientId).tokenServiceUrlV2(tokenUrlV2).config(configDb), oAuthCallback, (Object)credentials);
			}
			else
			{
				consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").clientSecret(credentials.clientSecret)
					.clientId(credentials.clientId).config(configDb).tokenServiceUrlV2(tokenUrlV2)
					.tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
					.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
					.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile), oAuthCallback, (Object)credentials);
			}
			
			/* Set the consumer on the credential structure for the callback */
			credentials.consumer = consumer;
			
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
