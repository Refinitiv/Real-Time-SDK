///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex460_MP_RTT;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.domain.login.Login;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.rdm.DomainTypes;

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
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){
		if (genericMsg.domainType() == DomainTypes.LOGIN && genericMsg.payload().dataType() == DataTypes.ELEMENT_LIST) {
			System.out.println("Received login RTT message from Provider");
			ElementList data = genericMsg.payload().elementList();
			for ( ElementEntry elem : data) {
				if (elem.name().equals(EmaRdm.ENAME_TICKS)) {
					System.out.println("        Ticks: " + elem.uintValue());
				}
				if (elem.name().equals(EmaRdm.ENAME_LATENCY)) {
					System.out.println("        Last Latency: " + elem.uintValue());
				}
			}
		}
	}
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
			
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();

			consumer  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_7"));

			Login.LoginReq loginReq = EmaFactory.Domain.createLoginReq();
			consumer.registerClient(loginReq.message(), appClient);
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient, 0);

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


