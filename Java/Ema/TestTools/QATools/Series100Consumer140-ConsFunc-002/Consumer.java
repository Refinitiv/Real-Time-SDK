///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex140_MBO_Streaming;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.EmaUtility;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
	        //APIQA	
		System.out.println("Clone Refresh Msg");
	        RefreshMsg cloneRefreshMsg = EmaFactory.createRefreshMsg(refreshMsg); 
		System.out.println("Item Name: " + (cloneRefreshMsg.hasName() ? cloneRefreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (cloneRefreshMsg.hasServiceName() ? cloneRefreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + cloneRefreshMsg.state());
		
		if (DataType.DataTypes.MAP == cloneRefreshMsg.payload().dataType())
			decode(cloneRefreshMsg.payload().map());
		
	        //END APIQA	
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
	        //APIQA	
		System.out.println("Clone Update Msg");
	        UpdateMsg cloneUpdateMsg = EmaFactory.createUpdateMsg(updateMsg); 
		System.out.println("Item Name: " + (cloneUpdateMsg.hasName() ? cloneUpdateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (cloneUpdateMsg.hasServiceName() ? cloneUpdateMsg.serviceName() : "<not set>"));
		
		if (DataType.DataTypes.MAP == cloneUpdateMsg.payload().dataType())
			decode(cloneUpdateMsg.payload().map());
		
	        //END APIQA	
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
	
	void decode(Map map)
	{
		if (DataTypes.FIELD_LIST == map.summaryData().dataType())
		{
			System.out.println("Map Summary data:");
			decode(map.summaryData().fieldList());
			System.out.println();
		}
		
		for (MapEntry mapEntry : map)
		{
			if (DataTypes.BUFFER == mapEntry.key().dataType())
				System.out.println("Action: " + mapEntry.mapActionAsString() + " key value: " + EmaUtility.asHexString(mapEntry.key().buffer().buffer()));

			if (DataTypes.FIELD_LIST == mapEntry.loadType())
			{
				System.out.println("Entry data:");
				decode(mapEntry.fieldList());
				System.out.println();
			}
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
			
			consumer.registerClient( EmaFactory.createReqMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER)
															.serviceName("DIRECT_FEED").name("AAO.V"), appClient, 0);
			
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
