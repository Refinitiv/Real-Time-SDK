///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming;

import com.rtsdk.ema.access.*;

class AppErrorClient implements OmmConsumerErrorClient {
	@Override
	public void onInvalidHandle(long handle, String text) {
		System.out.println("onInvalidHandle: "+handle+" text: "+text );
	}
	public void onInvalidUsage(java.lang.String text, int errorCode) {
		System.out.println("onInvalidUsage text: "+text+" errorCode: "+errorCode );
	}
}

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
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		AppErrorClient appErrorClient = new AppErrorClient();
		try
		{
			AppClient appClient = new AppClient();
			
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
			consumer = EmaFactory.createOmmConsumer(config.host("localhost:14002").username("user"), appErrorClient);
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.serviceName("DIRECT_FEED").name("IBM.N"), appClient);
			
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


