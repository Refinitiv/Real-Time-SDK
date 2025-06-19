/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series100.ex112_MP_TunnelingConnection;


import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.PostMsg;
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
	    		+ "  -spTLSv1.2 Enable TLS 1.2 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
	    		+ "  -spTLSv1.3 Enable TLS 1.3 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
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
			consumer.registerClient( EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("TRI.N"), appClient, 0);
			consumer.registerClient( EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("GOOG.L"), appClient, 0);
			
			int postId = 1;
			Thread.sleep(5000); // Wait for connection to come up
			for (int i = 0; i < 60; i++)
			{
				PostMsg postMsg = EmaFactory.createPostMsg();
				UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
				FieldList nestedFieldList = EmaFactory.createFieldList();

				nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
				nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
				nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
				nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));
				
				nestedUpdateMsg.payload(nestedFieldList );
				try {
				consumer.submit( postMsg.postId( postId++ ).serviceId( 1 ) 
															.name( "IBM.N" ).solicitAck( true ).complete(true)
															.payload(nestedUpdateMsg), 1 );
				}
				catch (Exception e)
				{
					System.out.println("failed");
				}
				Thread.sleep(1000);
			}
						
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


