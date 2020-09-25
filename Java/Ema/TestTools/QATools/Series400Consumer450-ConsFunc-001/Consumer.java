///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex450_MP_QueryServiceDiscovery;

import com.refinitiv.ema.access.Msg;

import java.util.List;

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
import com.refinitiv.ema.access.ServiceEndpointDiscoveryClient;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryEvent;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryOption;
import com.refinitiv.ema.access.ServiceEndpointDiscoveryResp;

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
		
		for(int index = 0; index < serviceEndpointResp.serviceEndpointInfoList().size(); index++)
		{
			List<String> locationList = serviceEndpointResp.serviceEndpointInfoList().get(index).locationList();
			
			if(locationList.size() == 2) // Get an endpoint that provides auto failover for the specified location.
			{
				if(locationList.get(0).startsWith(Consumer.location))
				{
					Consumer.host = serviceEndpointResp.serviceEndpointInfoList().get(index).endpoint();
					Consumer.port = serviceEndpointResp.serviceEndpointInfoList().get(index).port();
					break;
				}
			}
		}
	}

	public void onError(String errorText, ServiceEndpointDiscoveryEvent event)
	{
		System.out.println("Failed to query Refinitiv Data Platform service discovery. Error text: " + errorText);
	}
}

public class Consumer
{
	static String userName;
	static String password;
	static String clientId;
	static String proxyHostName;
	static String proxyPort = "-1";
	static String proxyUserName;
	static String proxyPassword;
	static String proxyDomain;
	static String proxyKrb5Configfile;
	public static String host;
	public static String port;
	public static String location = "us-east";
	// API QA
	static double tokenReissueRatio;
	static int restRequestTimeout;
	static int reissueTokenAttemptLimit;
	static int reissueTokenAttemptInterval;
	// END API QA
	
	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -username machine ID to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory).\n"
	    		+ "  -password password to perform authorization with the token \r\n"
	    		+ "\tservice (mandatory).\n"
	    		+ "  -location location to get an endpoint from RDP-RT service \r\n"
	    		+ "\tdiscovery. Defaults to \"us-east\" (optional).\n"
	    		+ "  -clientId client ID for application making the request to \r\n" 
	    		+ "\tRDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
	    		+ "  -keyfile keystore file for encryption (mandatory).\n"
	    		+ "  -keypasswd keystore password for encryption (mandatory).\n"
	    		+ "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n" 
	    		+ "  -pdomain Proxy Domain (optional).\n"
	    		+ "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos \r\n" 
	    		+ "\tand Kerberos authentications (optional).\n"
	    		// API QA
	    		+ "  -tokenServiceUrl (for QA testing).\n"
	    		+ "  -serviceDiscoveryUrl (for QA testing).\n"
	    		+ "  -restRequestTimeout (for QA testing).\n"
	    		+ "  -tokenReissueRatio (for QA testing).\n"
	    		+ "  -reissueTokenAttemptLimit (for QA testing).\n"
	    		+ "  -reissueTokenAttemptInterval (for QA testing).\n"
	    		// END API QA
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
	            // API QA
    			else if ("-tokenServiceUrl".equals(args[argsCount]))
    			{
    				config.tokenServiceUrl(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-serviceDiscoveryUrl".equals(args[argsCount]))
    			{
    				config.serviceDiscoveryUrl(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-restRequestTimeout".equals(args[argsCount]))
     			{			
     				restRequestTimeout = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : null;
    				++argsCount;	
     			}
    			else if ("-reissueTokenAttemptInterval".equals(args[argsCount]))
     			{			
    				reissueTokenAttemptInterval = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : null;
    				++argsCount;	
     			}
    			else if ("-reissueTokenAttemptLimit".equals(args[argsCount]))
     			{			
     				reissueTokenAttemptLimit = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : null;
    				++argsCount;	
     			}
    			else if ("-tokenReissueRatio".equals(args[argsCount]))
     			{			
     				tokenReissueRatio = argsCount < (args.length-1) ? Double.parseDouble(args[++argsCount]) : null;
    				++argsCount;	
     			}
	            
	            // END API QA
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
    			else // unrecognized command line argument
    			{
    				printHelp();
    				return false;
    			}			
    		}
	        
	        if ( userName == null || password == null || clientId == null)
			{
				System.out.println("Username, password, and clientId must be specified on the command line. Exiting...");
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
	
		// API QA	
		//elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1" ));
		//innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
		innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
		innerElementList.add(EmaFactory.createElementEntry().uintValue("RestRequestTimeOut", restRequestTimeout));
		innerElementList.add(EmaFactory.createElementEntry().doubleValue("TokenReissueRatio", tokenReissueRatio ));
		innerElementList.add(EmaFactory.createElementEntry().intValue("ReissueTokenAttemptLimit", reissueTokenAttemptLimit ));
        innerElementList.add(EmaFactory.createElementEntry().intValue("ReissueTokenAttemptInterval",reissueTokenAttemptInterval ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_2" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Logger", "Logger_1" ));
		
		// END API QA
		innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
		
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("ConsumerList", elementMap));
		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Host", host));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", port));
		innerElementList.add(EmaFactory.createElementEntry().intValue("EnableSessionManagement", 1));
		
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("ChannelList", elementMap));
		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList));
	}
	
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		ServiceEndpointDiscovery serviceDiscovery = null;
		try
		{
			AppClient appClient = new AppClient();
			serviceDiscovery = EmaFactory.createServiceEndpointDiscovery();
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			Map configDb = EmaFactory.createMap();
			
			if (!readCommandlineArgs(args, config)) return;
			
			serviceDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOption().username(userName)
					.password(password).clientId(clientId).transport(ServiceEndpointDiscoveryOption.TransportProtocol.TCP)
					.proxyHostName(proxyHostName).proxyPort(proxyPort).proxyUserName(proxyUserName)
					.proxyPassword(proxyPassword).proxyDomain(proxyDomain).proxyKRB5ConfigFile(proxyKrb5Configfile), appClient);
			
			if ( host == null || port == null )
			{
				System.out.println("Both hostname and port are not avaiable for establishing a connection with Refinitiv Real-Time Optimized. Exiting...");
				return;
			}
			
			createProgramaticConfig(configDb);
			
			if ( (proxyHostName == null) && (proxyPort == "-1") )
			{
				consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").username(userName).password(password)
					.clientId(clientId).config(configDb));
			}
			else
			{
				consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").username(userName).password(password)
					.clientId(clientId).config(configDb).tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
					.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
					.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile));
			}
					
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name("BBL.BK"), appClient);
			
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
