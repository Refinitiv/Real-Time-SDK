///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license 
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.               
// *|           Copyright (C) 2019,2022 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series200.ex220_MBP_PrivateStream;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig.OperationModel;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;


import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());
		
		if (DataType.DataTypes.MAP == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().map());
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		if (DataType.DataTypes.MAP == updateMsg.payload().dataType())
			decode(updateMsg.payload().map());
		
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
			switch (mapEntry.key().dataType())
                        {
                                case DataTypes.BUFFER :
                                        System.out.println("Action: " + mapEntry.mapActionAsString() + ", key value: " + mapEntry.key().buffer().toString() + "\n");
                                        break;
                                case DataTypes.ASCII :
                                        System.out.println("Action: " + mapEntry.mapActionAsString() + ", key value: " + mapEntry.key().ascii().toString() + "\n");
                                        break;
                                case DataTypes.RMTES :
                                        System.out.println("Action: " + mapEntry.mapActionAsString() + ", key value: " + mapEntry.key().rmtes().toString() + "\n");
                                        break;
                                default:
                                        break;
                        }

			if (DataTypes.FIELD_LIST == mapEntry.loadType())
			{
				System.out.println("Entry data:");
				decode(mapEntry.fieldList());
				System.out.println();
			}
		}
	}
	
	void decode(FieldList fieldList)
	{
		for (FieldEntry fieldEntry : fieldList)
		{
			System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.code())
				System.out.println(" blank");
			else
				switch (fieldEntry.loadType())
				{
				case DataTypes.REAL :
					System.out.println(fieldEntry.real().asDouble());
					break;
				case DataTypes.DATE :
					System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
					break;
				case DataTypes.TIME :
					System.out.println(fieldEntry.time().hour() + " / " + fieldEntry.time().minute() + " / " + fieldEntry.time().second() + fieldEntry.time().millisecond());
					break;
				case DataTypes.INT :
					System.out.println(fieldEntry.intValue());
					break;
				case DataTypes.UINT :
					System.out.println(fieldEntry.uintValue());
					break;
				case DataTypes.ASCII :
					System.out.println(fieldEntry.ascii());
					break;
				case DataTypes.ENUM :
					System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
					break;
				case DataTypes.RMTES :
					System.out.println(fieldEntry.rmtes());
					break;
				case DataTypes.ERROR :
					System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default :
					System.out.println();
					break;
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
			
			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
													.operationModel(OperationModel.USER_DISPATCH)
													.host("localhost:14002").username("user"));
			
			consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_MARKET_BY_PRICE)
															.serviceName("DIRECT_FEED").name("BBH.ITC")
															.privateStream(true), appClient);
			
			long startTime = System.currentTimeMillis();
			while (startTime + 60000 > System.currentTimeMillis())
				consumer.dispatch(10);		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
		}
		catch (OmmException excp)
		{
			System.out.println(excp);
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
