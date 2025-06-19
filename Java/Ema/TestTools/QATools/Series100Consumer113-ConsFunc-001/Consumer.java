/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series100.ex113_MP_SessionMgmt;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

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
}

public class Consumer {
	
	static String userName;
	static String password;
	static String clientId;
	
	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -username machine ID to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory).\n"
	    		+ "  -password password to perform authorization with the token \r\n"
	    		+ "\tservice (mandatory).\n"
	    		+ "  -clientId client ID for application making the request to \r\n" 
	    		+ "\tLDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
	    		+ "  -keyfile keystore file for encryption.\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
	    		+ "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
	    		+ "  -ph Proxy host name (optional).\n"
	    		+ "  -pp Proxy port number (optional).\n"
	    		+ "  -plogin User name on proxy server (optional).\n"
	    		+ "  -ppasswd Password on proxy server (optional).\n" 
	    		+ "  -pdomain Proxy Domain (optional).\n"
	    		+ "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos \r\n" 
	    		+ "\tand Kerberos authentications (optional).\n"
	    		+ "  -tokenServiceUrl \r\n" 
	    		+ "  -serviceDiscoveryUrl \r\n" 
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
	            	config.clientId(clientId);
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
    				config.tunnelingProxyHostName(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-pp".equals(args[argsCount]))
    			{
    				config.tunnelingProxyPort(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-plogin".equals(args[argsCount]))
    			{
    				config.tunnelingCredentialUserName(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-ppasswd".equals(args[argsCount]))
    			{
    				config.tunnelingCredentialPasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-pdomain".equals(args[argsCount]))
    			{
    				config.tunnelingCredentialDomain(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
    			else if ("-krbfile".equals(args[argsCount]))
    			{
    				config.tunnelingCredentialKRB5ConfigFile(argsCount < (args.length-1) ? args[++argsCount] : null);
    				++argsCount;				
    			}
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
    			else // unrecognized command line argument
    			{
    				printHelp();
    				return false;
    			}			
    		}
	        
	        if ( userName == null || password == null || clientId == null)
			{
				System.out.println("Username, password, and clientId all must be specified on the command line. Exiting...");
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

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
			if (!readCommandlineArgs(args, config))
                return;
			
			AppClient appClient = new AppClient();
			
			consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_4").username(userName).password(password));
			
			consumer.registerClient( EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name("BBL.BK"), appClient);
			
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
