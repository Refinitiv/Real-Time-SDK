///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex130_MP_UserDisp;

import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig.OperationModel;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

//APIQA
class DispatchThread extends Thread
{
	OmmConsumer myCons;
	public DispatchThread(OmmConsumer cons) {
		myCons = cons;
	}
	
	public void run() {
		while (true)
			myCons.dispatch(10);
	}
}
//END APIQA


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());
		
    //APIQA
	//	if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
	//		decode(refreshMsg.payload().fieldList());
    //END APIQA
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());
		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
	
	void decode(FieldList fieldList)
	{
		for (FieldEntry fieldEntry : fieldList)
		{
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
			
       //APIQA
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().consumerName("Consumer_2")
													.operationModel(OperationModel.USER_DISPATCH)
													.username("user"));
			
		// 	consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient, 0);
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			String itemPreName = "RTR";
			StringBuilder itemName = new StringBuilder();
			long[] handles = new long[10000];
			
			DispatchThread dThread = new DispatchThread(consumer);
			dThread.start();
			
		while (true) {	
			System.out.println("############## Starting a new iteration ###############");
			Thread.sleep(1000);
			
			for (int idx = 0; idx < 10000; ++idx)
			{
				itemName.append(itemPreName).append(idx).append(".N");
				reqMsg.clear().serviceName("DIRECT_FEED").name(itemName.toString());
				handles[idx] = consumer.registerClient(reqMsg, appClient);
				itemName.setLength(0);
			}
															
			Thread.sleep(980000);
		
					
			for (int idx = 0; idx < 10000; ++idx)
			{			
				 consumer.unregister(handles[idx]);
				
			}
		}
		
		}
       //END APIQA
		catch (OmmException excp )
		{
			System.out.println(excp);
		}
		catch (InterruptedException excp )
		{
			System.out.println(excp);
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
