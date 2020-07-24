///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license                             --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.                         --
// *|                See the project's LICENSE.md for details.                                         --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.                                   --
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series100.example113__MarketPrice__SessionManagement;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;

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

	static String itemName = "IBM.N";

	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -username machine ID to perform authorization with the\r\n" 
	    		+ "\ttoken service (mandatory).\n"
	    		+ "  -password password to perform authorization with the token \r\n"
	    		+ "\tservice (mandatory).\n"
	    		+ "  -clientId client ID for application making the request to \r\n" 
	    		+ "\tEDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
	    		+ "  -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials(optional).\r\n"
	    		+ "  -keyfile keystore file for encryption (mandatory).\n"
	    		+ "  -keypasswd keystore password for encryption (mandatory).\n"
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
    			else if ("-itemName".equals(args[argsCount]))
    			{
    				itemName = argsCount < (args.length-1) ? args[++argsCount] : null;
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
    			else if ("-takeExclusiveSignOnControl".equals(args[argsCount]))
    			{
    				String takeExclusiveSignOnControl = argsCount < (args.length-1) ? args[++argsCount] : null;
    				
    				if(takeExclusiveSignOnControl != null)
    				{
    					if(takeExclusiveSignOnControl.equalsIgnoreCase("true"))
    						config.takeExclusiveSignOnControl(true);
    					else if (takeExclusiveSignOnControl.equalsIgnoreCase("false"))
    						config.takeExclusiveSignOnControl(false);
    				}
    				
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
			
			consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_3").username(userName).password(password));
			
			consumer.registerClient( EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name(itemName), appClient);
			
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
