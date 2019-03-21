///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2019. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series400.example450__MarketPrice__QueryServiceDiscovery;

import com.thomsonreuters.ema.access.Msg;

import java.util.List;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ServiceEndpointDiscovery;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.ServiceEndpointDiscoveryClient;
import com.thomsonreuters.ema.access.ServiceEndpointDiscoveryEvent;
import com.thomsonreuters.ema.access.ServiceEndpointDiscoveryOption;
import com.thomsonreuters.ema.access.ServiceEndpointDiscoveryResp;

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
		System.out.println("Failed to query EDP-RT service discovery. Error text: " + errorText);
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
	
	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -username user name to perform authentication with the token service.\n"
	    		+ "  -password password to perform authorization with the token service.\n"
	    		+ "  -location location to get an endpoint from EDP-RT service discovery. Defaults to \"us-east\"\n"
	    		+ "  -clientId client ID to perform authorization with the token service. The user name is used if not specified.\n"
	    		+ "  -keyfile keystore file for encryption.\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
	    		+ "  -ph Proxy host name.\n"
	    		+ "  -pp Proxy port number.\n"
	    		+ "  -plogin User name on proxy server.\n"
	    		+ "  -ppasswd Password on proxy server.\n" 
	    		+ "  -pdomain Proxy Domain.\n"
	    		+ "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos and Kerberos authentications\n"
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
	        
	        if ( userName == null || password == null )
			{
				System.out.println("Both username and password must be specified on the command line. Exiting...");
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
		try
		{
			AppClient appClient = new AppClient();
			ServiceEndpointDiscovery serviceDiscovery = EmaFactory.createServiceEndpointDiscovery();
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			Map configDb = EmaFactory.createMap();
			
			if (!readCommandlineArgs(args, config)) return;
			
			serviceDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOption().username(userName)
					.password(password).clientId(clientId).transport(ServiceEndpointDiscoveryOption.TransportProtocol.TCP)
					.proxyHostName(proxyHostName).proxyPort(proxyPort).proxyUserName(proxyUserName)
					.proxyPassword(proxyPassword).proxyDomain(proxyDomain).proxyKRB5ConfigFile(proxyKrb5Configfile), appClient);
			
			if ( host == null || port == null )
			{
				System.out.println("Both hostname and port are not avaiable for establishing a connection with ERT in cloud. Exiting...");
				return;
			}
			
			createProgramaticConfig(configDb);
			
			consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1").username(userName).password(password)
					.clientId(clientId).config(configDb)
					.tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
					.tunnelingCredentialUserName(proxyUserName).tunnelingCredentialPasswd(proxyPassword).tunnelingCredentialDomain(proxyDomain)
					.tunnelingCredentialKRB5ConfigFile(proxyKrb5Configfile));
					
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name("IBM.N"), appClient);
			
			Thread.sleep(900000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
