/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series200.ex291_Custom_DownCast;

import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.OmmAscii;
import com.refinitiv.ema.access.OmmDate;
import com.refinitiv.ema.access.OmmEnum;
import com.refinitiv.ema.access.OmmError;
import com.refinitiv.ema.access.OmmInt;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmRmtes;
import com.refinitiv.ema.access.OmmTime;
import com.refinitiv.ema.access.OmmUInt;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType.DataTypes;
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
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure().hashCode());
		
		decodeRefreshMsg(refreshMsg);
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure().hashCode());
		
		decodeUpdateMsg(updateMsg);
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure().hashCode());
		
		decodeStatusMsg(statusMsg);
		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decodeRefreshMsg(RefreshMsg refreshMsg)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());

		System.out.println("Attribute");
		decode(refreshMsg.attrib().data());
		
		System.out.println("Payload");
		decode(refreshMsg.payload().data());
	}

	void decodeUpdateMsg(UpdateMsg updateMsg)
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		System.out.println("Attribute");
		decode(updateMsg.attrib().data());

		System.out.println("Payload");
		decode(updateMsg.payload().data());
	}
	
	void decodeStatusMsg(StatusMsg statusMsg)
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));
		
		if (statusMsg.hasState())
			System.out.println("Item State: " + statusMsg.state());
	}

	void decode(Data data)
	{
		if (Data.DataCode.BLANK == data.code())
			System.out.println("Blank data");
		else
			switch (data.dataType())
			{
			case DataTypes.REFRESH_MSG :
				decodeRefreshMsg((RefreshMsg)data);
				break;
			case DataTypes.UPDATE_MSG :
				decodeUpdateMsg((UpdateMsg)data);
				break;
			case DataTypes.STATUS_MSG :
				decodeStatusMsg((StatusMsg)data);
				break;
			case DataTypes.FIELD_LIST :
				decodeFieldList((FieldList)data);
				break;
			case DataTypes.MAP :
				decodeMap((Map)data);
				break;
			case DataTypes.NO_DATA :
				System.out.println("NoData");
				break;
			case DataTypes.TIME :
				System.out.println("OmmTime: " + ((OmmTime)data).toString());
				break;
			case DataTypes.DATE :
				System.out.println("OmmDate: " + ((OmmDate)data).toString());
				break;
			case DataTypes.REAL :
				System.out.println("OmmReal::asDouble: " + ((OmmReal)data).asDouble());
				break;
			case DataTypes.INT :
				System.out.println("OmmInt: " + ((OmmInt)data).intValue());
				break;
			case DataTypes.UINT :
				System.out.println("OmmUInt: " + ((OmmUInt)data).longValue());
				break;
			case DataTypes.ENUM :
				System.out.println("OmmEnum: " + ((OmmEnum)data).enumValue());
				break;
			case DataTypes.ASCII :
				System.out.println("OmmAscii: " + ((OmmAscii)data).ascii());
				break;
			case DataTypes.RMTES :
				System.out.println("OmmRmtes: " + ((OmmRmtes)data).rmtes());
				break;
			case DataTypes.ERROR :
				System.out.println("Decoding error: " + ((OmmError)data).errorCodeAsString());
				break;
			default :
				break;
			}
	}

	void decodeMap(Map map)
	{
		System.out.println("Map Summary");
		decode(map.summaryData().data());

		for (MapEntry mapEntry : map)
		{
			System.out.println("Action = " + mapEntry.mapActionAsString());

			System.out.println("Key");
			decode(mapEntry.key().data());

			System.out.println("Load");
			decode(mapEntry.load());
		}
	}

	void decodeFieldList(FieldList fieldList)
	{
		if (fieldList.hasInfo())
			System.out.println("FieldListNum: " + fieldList.infoFieldListNum() + " DictionaryId: " + fieldList.infoDictionaryId());

		for (FieldEntry fieldEntry : fieldList)
		{
			System.out.println("Load");
			decode(fieldEntry.load());
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
