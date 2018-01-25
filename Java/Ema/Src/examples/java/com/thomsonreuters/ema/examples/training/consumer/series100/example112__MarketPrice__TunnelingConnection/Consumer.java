///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2016. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series100.example112__MarketPrice__TunnelingConnection;


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

public class Consumer 
{
	static void printHelp()
	{

	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
	    		+ "  if the application will attempt to make an http or encrypted \n "
	    		+ "           connection, ChannelType must need to be set to ChannelType::RSSL_HTTP \n"
	    		+ "            or ChannelType::RSSL_ENCRYPTED in EMA configuration file.\n"
	    		+ "  -ph Proxy host name.\n"
	    		+ "  -pp Proxy port number.\n"
	    		+ "  -plogin User name on proxy server.\n"
	    		+ "  -ppasswd Password on proxy server.\n" 
	    		+ "  -pdomain Proxy Domain.\n"
	    		+ "  -krbfile Proxy KRB file.\n" 
	    		+ "  -keyfile keystore file for encryption.\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
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
	    				config.tunnelingProxyHostName(args[++argsCount]);
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
	    			else // unrecognized command line argument
	    			{
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

	
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
			if (!readCommandlineArgs(args, config))
                return;
			
			AppClient appClient = new AppClient();
			
			consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_3"));
			
			consumer.registerClient( EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient, 0);
			
			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
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


