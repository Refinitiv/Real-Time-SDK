///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex170_MP_ChannelInfo;

import com.refinitiv.ema.access.Msg;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.OmmConsumerErrorClient;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
	boolean updateCalled = false;	
	OmmConsumer _ommConsumer = null;
	
	public void onRefreshMsg( RefreshMsg refreshMsg, OmmConsumerEvent event )
	{
		System.out.println( refreshMsg + "\nevent channel info (refresh)\n" + event.channelInformation() );
	}

	public void onUpdateMsg( UpdateMsg updateMsg, OmmConsumerEvent event ) 
	{
		if (!updateCalled)
		{
			updateCalled = true;
			System.out.println( updateMsg + "\nevent channel info (update)\n" + event.channelInformation() );
			//API QA
			System.out.println("Test getMaxOutputBuffers() : " + event.channelInformation().maxOutputBuffers()); 
			System.out.println("Test getGuaranteedOutputBuffers() : " + event.channelInformation().guaranteedOutputBuffers());
			System.out.println("Test getCompressionThreshold() : " + event.channelInformation().compressionThreshold());
			//END API QA
		}
		else
			System.out.println( "skipped printing updateMsg" );			
	}

	public void onStatusMsg( StatusMsg statusMsg, OmmConsumerEvent event ) 
	{
		System.out.println( statusMsg + "\nevent channel info (status)\n" + event.channelInformation() );
	}

	public void onGenericMsg( GenericMsg genericMsg, OmmConsumerEvent consumerEvent ){}
	public void onAckMsg( AckMsg ackMsg, OmmConsumerEvent consumerEvent ){}
	public void onAllMsg( Msg msg, OmmConsumerEvent consumerEvent ){}
	
}
//API QA
class AppErrorClient implements OmmConsumerErrorClient
{
	public void onInvalidHandle(long handle, String text)
	{
		System.out.println("onInvalidHandle callback function" + "\nInvalid handle: " + handle + "\nError text: " + text); 
	}

	public void onInvalidUsage(String text, int errorCode) {
		System.out.println("onInvalidUsage callback function" + "\nError text: " + text +" , Error code: " + errorCode); 
	}
}
//END API QA
public class Consumer 
{
	//API QA
	static int maxOutputBuffers = 2000;
	static int guaranteedOutputBuffers = 2000;
	static int highWaterMark = 1000;
	static int serverNumPoolBuffers = 3000;
	static int compressionThreshold = 40;

	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -maxOutputBuffers : value of maxOutputBuffer to modify.\r\n" 
	    		+ "  -guaranteedOutputBuffers : value of guaranteedOutputBuffers to modify.\n"
	    		+ "  -highWaterMark : value of highWaterMark to modify.\r\n" 
	    		+ "  -serverNumPoolBuffers : value of serverNumPoolBuffer to modify.\r\n" 
	    		+ "  -compressionThreshold : value of compressionThreshold to modify.\n"
	    		+ "\n");
	}
	static boolean readCommandlineArgs(String[] args)
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
	            else if ("-maxOutputBuffers".equals(args[argsCount]))
				{
	            	maxOutputBuffers = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : maxOutputBuffers;
					++argsCount;				
				}
	            else if ("-guaranteedOutputBuffers".equals(args[argsCount]))
				{
	            	guaranteedOutputBuffers = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : guaranteedOutputBuffers;
					++argsCount;				
				}
	            else if ("-highWaterMark".equals(args[argsCount]))
				{
	            	highWaterMark = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : highWaterMark;
					++argsCount;				
				}
				else if ("-serverNumPoolBuffers".equals(args[argsCount]))
				{
					serverNumPoolBuffers = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : serverNumPoolBuffers;
					++argsCount;		
				}	
				else if ("-compressionThreshold".equals(args[argsCount]))
				{
					compressionThreshold = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : compressionThreshold;
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

	//END API QA
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			if (!readCommandlineArgs(args))
                return;
			AppErrorClient appErrorClient = new AppErrorClient();
			ChannelInformation ci = EmaFactory.createChannelInformation();

			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig( "EmaConfig.xml" ).username( "user" ),appErrorClient);
			consumer.channelInformation( ci );
			System.out.println( "channel information (consumer):\n\t" + ci );
			consumer.registerClient( EmaFactory.createReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), appClient, 0);
			//API QA
			System.out.println("Modify maxOutputBuffers to " + maxOutputBuffers );
			System.out.println("Modify guaranteedOutputBuffers to " + guaranteedOutputBuffers );
			System.out.println("Modify highWaterMark to " + highWaterMark );
			System.out.println("Modify serverNumPoolBuffers to " + serverNumPoolBuffers );
			System.out.println("Modify compressionThreshold to " + compressionThreshold );
			consumer.modifyIOCtl(1, maxOutputBuffers); // maxNumBuffer
			consumer.modifyIOCtl(2, guaranteedOutputBuffers); //guaranteedOutputBuffers
			consumer.modifyIOCtl(3, highWaterMark); //highWaterMark
			consumer.modifyIOCtl(8, serverNumPoolBuffers); //serverNumPoolBuffers
			consumer.modifyIOCtl(9, compressionThreshold); //compressionThreshold
			// END API QA

			Thread.sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println( excp.getMessage() );
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
