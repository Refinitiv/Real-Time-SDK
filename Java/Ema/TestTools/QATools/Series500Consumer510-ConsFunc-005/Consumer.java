 ///*|--------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	                 --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.                       --
// *|                See the project's LICENSE.md for details.                  					 --
// *|              Copyright (C) 2024 LSEG. All rights reserved.            		                 --
///*|--------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex510_RequestRouting_FileCfg;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ServiceList;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

import com.refinitiv.ema.access.ReqMsg.Rate;
import com.refinitiv.ema.access.ReqMsg.Timeliness;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmConsumerClient
{
	List<ChannelInformation> channelInList = new ArrayList<ChannelInformation>();
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		//API QA
		if (refreshMsg.domainType() == 1)
		{
			System.out.println(refreshMsg + "\nevent session info (refresh)\n");
			printSessionInfo(event);
		}
		else 
		{
			System.out.println( refreshMsg + "\nevent channel info (refresh)\n" + event.channelInformation() );
		}
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		//API QA
		if (updateMsg.domainType() == 1)
		{
			System.out.println(updateMsg + "\nevent session info (update)\n");
			printSessionInfo(event);
		}
		else 
		{
			System.out.println( updateMsg + "\nevent channel info (update)\n" + event.channelInformation() );
		}
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		//API QA
		if (statusMsg.domainType() == 1)
		{
			System.out.println(statusMsg + "\nevent session info (status)\n");
			printSessionInfo(event);
		}
		else 
		{
			System.out.println( statusMsg + "\nevent channel info (status)\n" + event.channelInformation() );
		}
	}
	void printSessionInfo(OmmConsumerEvent event)
	{
		event.sessionChannelInfo(channelInList);
		
		for(ChannelInformation channelInfo : channelInList) 
		{
			System.out.println(channelInfo);
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
	//API QA
	public static int _TEST = 0;
	public static int _domainType = 6;
	public static String _itemName = "LSEG.L";
	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
	    		+ "  -i to identify itemName.\n"
	    		+ "  -domain follow by domainType mp=MARKET_PRICE, mbp=MARKET_BY_PRICE, mbo=MARKET_BY_ORDER.\n"
	    		+ "  -test0 Testing default, no qos in ReqMsg.\n"
	    		+ "  -test11 Testing ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK).\n"
	    		+ "  -test12 Testing ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK).\n"
	    		+ "  -test21 Testing ReqMsg.qos(qos(Timeliness.REALTIME, Rate.JIT_CONFLATED).\n"
	    		+ "\n");
	}
	static boolean readCommandlineArgs(String[] args)
	{
		    try
		    {
		        int argsCount = 0;
		        boolean test0 = false;
		        boolean test11 = false;
		        boolean test12 = false;
		        boolean test21 = false;
		        String domainName = "mp";

		        while (argsCount < args.length)
		        {
		            if (0 == args[argsCount].compareTo("-?"))
		            {
		                printHelp();
		                return false;
		            }
		            else if ("-i".equals(args[argsCount]))
					{
						Consumer._itemName = argsCount < (args.length-1) ? args[++argsCount] : null;
						++argsCount;
					}
		            
		            else if ("-domain".equals(args[argsCount]))
					{
						domainName = argsCount < (args.length-1) ? args[++argsCount] : null;
						if (domainName.equals("mp"))
							Consumer._domainType = EmaRdm.MMT_MARKET_PRICE;
						else if (domainName.equals("mbp"))
							Consumer._domainType = EmaRdm.MMT_MARKET_BY_PRICE;
						else if (domainName.equals("mbo"))
							Consumer._domainType = EmaRdm.MMT_MARKET_BY_ORDER;
						else
							System.out.println("Incorect domain type, turn to default mp.");
						++argsCount;
					}		           
		        	else if ("-test0".equals(args[argsCount]))	   
	    			{
	    				test0 = true;
	    				if (test0)
	    					Consumer._TEST = 0;
	    				++argsCount;
	    			}
		        	else if ("-test11".equals(args[argsCount]))	   
	    			{
	    				test11 = true;
	    				if (test11)
	    					Consumer._TEST = 11;
	    				++argsCount;
	    			}
		        	else if ("-test12".equals(args[argsCount]))	   
	    			{
	    				test12 = true;
	    				if (test12)
	    					Consumer._TEST = 12;
	    				++argsCount;
	    			}	  
		        	else if ("-test21".equals(args[argsCount]))	   
	    			{
	    				test21 = true;
	    				if (test21)
	    					Consumer._TEST = 21;
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
	// END API QA
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			//API QA
			if ( !readCommandlineArgs(args) ) return;
			//END API QA
			AppClient appClient = new AppClient();
			
			/* Create a service list which can subscribe data using any concrete services in this list */
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().consumerName("Consumer_10").addServiceList(serviceList), appClient);
			//API QA
			switch(Consumer._TEST)
			{
				default:
				case 0:
					System.out.println("***APIQA TEST 0 : ReqMsg does NOT set qos and same itemName, one is normal, one is private stream.***"); 
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).privateStream(true), appClient);
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName), appClient);
					break;
				case 11:
					System.out.println("***APIQA TEST 11 : ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK), one is normal, one is private stream.***"); 
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).qos(Timeliness.REALTIME, Rate.TICK_BY_TICK).privateStream(true), appClient);
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).qos(Timeliness.REALTIME, Rate.TICK_BY_TICK), appClient);
					break;
				case 12:
					System.out.println("***APIQA TEST 12 : ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK), one is normal, one is private stream.***"); 
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK).privateStream(true), appClient);
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK), appClient);
					break;
				case 21:
					System.out.println("***APIQA TEST 21 : ReqMsg.qos(Timeliness.REALTIME, Rate.JIT_CONFLATED), one is normal, one is private stream.***"); 
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).qos(Timeliness.REALTIME, Rate.JIT_CONFLATED).privateStream(true), appClient);
					consumer.registerClient(EmaFactory.createReqMsg().domainType(Consumer._domainType).serviceListName("SVG1").name(Consumer._itemName).qos(Timeliness.REALTIME, Rate.JIT_CONFLATED), appClient);
					break;
					
			}			
			//END API QA
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
