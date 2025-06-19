/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			/* Create a service list which can subscribe data using any concrete services in this list */
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().consumerName("Consumer_10").addServiceList(serviceList).username("user"), appClient);
		    consumer.registerClient(EmaFactory.createReqMsg().serviceListName("SVG1").name("LSEG.L"), appClient);
	        consumer.registerClient(EmaFactory.createReqMsg().serviceListName("SVG1").name("TRI.N"), appClient);

	        Thread.sleep(10000);
	        System.out.println("\nAPIQA calls consumer.unitialize()!!\n");
	        consumer.uninitialize();
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
