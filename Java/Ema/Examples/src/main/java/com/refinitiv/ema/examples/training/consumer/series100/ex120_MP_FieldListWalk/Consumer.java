///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex120_MP_FieldListWalk;

import java.util.Iterator;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		if (refreshMsg.hasName())
			System.out.println("Item Name: " + refreshMsg.name());
		
		if (refreshMsg.hasServiceName())
			System.out.println("Service Name: " + refreshMsg.serviceName());
		
		System.out.println("Item State: " + refreshMsg.state());
		
		if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());
		
		System.out.println("\n");
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		if (updateMsg.hasName())
			System.out.println("Item Name: " + updateMsg.name());
		
		if (updateMsg.hasServiceName())
			System.out.println("Service Name: " + updateMsg.serviceName());
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());
		
		System.out.println("\n");
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		if (statusMsg.hasName())
			System.out.println("Item Name: " + statusMsg.name());
		
		if (statusMsg.hasServiceName())
			System.out.println("Service Name: " + statusMsg.serviceName());
		
		if (statusMsg.hasState())
			System.out.println("Service State: " + statusMsg.state());
		
		System.out.println("\n");
	}

	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

								
	void decode(FieldList fieldList)
	{
		Iterator<FieldEntry> iter = fieldList.iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			System.out.println("Fid: " + fieldEntry.fieldId() + " Name: " + fieldEntry.name() + " value: " + fieldEntry.load());
		}
	}
}

public class Consumer 
{
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
			
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


