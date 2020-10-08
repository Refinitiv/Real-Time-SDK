///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex112_MP_TunnelingConnection;


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
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.ElementList;

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
		//APIQA
    static Map createProgramaticConfig()
    {
        Map innerMap = EmaFactory.createMap();
        Map configMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_3" ));

        innerElementList.add(EmaFactory.createElementEntry().ascii( "Channel", "Channel_1" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ItemCountHint", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ServiceCountHint", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ObeyOpenWindow", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "PostAckTimeout", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "RequestTimeout", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxOutstandingPosts", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "DispatchTimeoutApiThread", 1 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "HandleException", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReactorEventFdPort", 45000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "PipePort", 4001 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 10 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 2000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 6000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MsgKeyInUpdates", 1 ));
innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_3", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_ENCRYPTED" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::None"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "P_ObjectName"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));
        innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyHost", "proxyHostToConnectTo"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyPort", "proxyPortToConnectTo"));
innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
        innerMap.clear();
        return configMap;
    }
		//END APIQA
 	
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
			if (!readCommandlineArgs(args, config))
                return;
			
			AppClient appClient = new AppClient();
			
		//APIQA
			consumer  = EmaFactory.createOmmConsumer(config.config(createProgramaticConfig()).consumerName("Consumer_3"));
			
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


