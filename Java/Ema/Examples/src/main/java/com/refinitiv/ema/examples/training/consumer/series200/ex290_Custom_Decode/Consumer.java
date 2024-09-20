///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license 
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details. 
// *|           Copyright (C) 2019,2022 LSEG. All rights reserved.  
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series200.ex290_Custom_Decode;

import com.refinitiv.ema.access.Attrib;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Payload;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Handle: " + event.handle() + " Closure: " + event.closure().hashCode());
		
		decode(refreshMsg);
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Handle: " + event.handle() + " Closure: " + event.closure().hashCode());
		
		decode(updateMsg);
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		System.out.println("Item Handle: " + event.handle() + " Closure: " + event.closure().hashCode());

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());
		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(RefreshMsg refreshMsg)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());
		
		decode(refreshMsg.attrib());
		
		decode(refreshMsg.payload());
	}
	
	void decode(UpdateMsg updateMsg)
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

		decode(updateMsg.attrib());
		
		decode(updateMsg.payload());
	}

	void decode(Attrib attrib)
	{
		System.out.println("Attribute");
		
		switch (attrib.dataType())
		{
			case DataTypes.FIELD_LIST :
				decode(attrib.fieldList());
				break;
			case DataTypes.MAP :
				decode(attrib.map());
				break;
			default:
				break;
		}
	}
	
	void decode(Payload payload)
	{
		System.out.println("Payload");

		switch (payload.dataType())
		{
			case DataTypes.FIELD_LIST :
				decode(payload.fieldList());
				break;
			case DataTypes.MAP :
				decode(payload.map());
				break;
			case DataTypes.REFRESH_MSG :
				decode(payload.refreshMsg());
			break;
			case DataTypes.UPDATE_MSG :
				decode(payload.updateMsg());
				break;
			default:
				break;
		}
	}
	
	void decode(Map map)
	{
		switch (map.summaryData().dataType())
		{
			case DataTypes.FIELD_LIST :
				decode(map.summaryData().fieldList());
				break;
			case DataTypes.MAP :
				decode(map.summaryData().map());
				break;
			case DataTypes.REFRESH_MSG :
				decode(map.summaryData().refreshMsg());
				break;
			case DataTypes.UPDATE_MSG :
				decode(map.summaryData().updateMsg());
				break;
			default:
				break;
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
			
			switch (mapEntry.loadType())
			{
				case DataTypes.FIELD_LIST :
					decode(mapEntry.fieldList());
					break;
				case DataTypes.MAP :
					decode(mapEntry.map());
					break;
				case DataTypes.REFRESH_MSG :
					decode(mapEntry.refreshMsg());
					break;
				case DataTypes.UPDATE_MSG :
					decode(mapEntry.updateMsg());
					break;
				default:
					break;
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
				case DataTypes.FIELD_LIST :
					System.out.println(",  contains FieldList.");
					decode(fieldEntry.fieldList());
					break;
				case DataTypes.MAP :
					System.out.println(",  contains map.");
					decode(fieldEntry.map());
					break;
				case DataTypes.REFRESH_MSG :
					System.out.println(",  contains refresh message.");
					decode(fieldEntry.refreshMsg());
					break;
				case DataTypes.UPDATE_MSG :
					System.out.println(",  contains update message.");
					decode(fieldEntry.updateMsg());
					break;
				case DataTypes.REAL :
					System.out.println(fieldEntry.real().asDouble());
					break;
				case DataTypes.DATE :
					System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
					break;
				case DataTypes.TIME :
					System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
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
					System.out.println(fieldEntry.error().errorCode() + " (" + fieldEntry.error().errorCodeAsString() + ")");
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
			
			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
			
			// request a custom domain (133) item IBM.XYZ
			consumer.registerClient(EmaFactory.createReqMsg().domainType(133).serviceName("DIRECT_FEED").name("IBM.XYZ"), appClient, 1);
			
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
