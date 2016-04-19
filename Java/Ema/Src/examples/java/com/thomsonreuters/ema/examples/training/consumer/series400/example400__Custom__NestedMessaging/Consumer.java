///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2015. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series400.example400__Custom__NestedMessaging;

import com.thomsonreuters.ema.access.Attrib;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.Payload;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;


class AppClient implements OmmConsumerClient
{
	long count = 0;
	OmmConsumer _ommConsumer = null;

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		decode(refreshMsg);

		// open a sub stream (a.k.a. nested message request)
		if (refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
				refreshMsg.state().dataState() == OmmState.DataState.OK &&
				refreshMsg.domainType() == 200)
		{
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			reqMsg.name(".DJI").privateStream(true).serviceId(refreshMsg.serviceId()).streamId(1);
			_ommConsumer.submit(EmaFactory.createGenericMsg().payload(reqMsg), event.handle());
		}
			
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		decode(updateMsg);
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());
	
		decode(statusMsg);
		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Generic. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		decode(genericMsg);
		
		System.out.println();
	}
	
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

	void decode(StatusMsg statusMsg)
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " + statusMsg.state());
	}
	
	void decode(Payload payload)
	{
		switch (payload.dataType())
		{
		case DataTypes.ELEMENT_LIST:
			decode(payload.elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(payload.fieldList());
			break;
		}
	}
	
	void decode(Attrib attrib)
	{
		switch (attrib.dataType())
		{
		case DataTypes.ELEMENT_LIST:
			decode(attrib.elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(attrib.fieldList());
			break;
		}		
	}

	void decode(GenericMsg genMsg)
	{
		if (genMsg.hasServiceId())
			System.out.println("ServiceId: " + genMsg.serviceId());

		if (genMsg.hasPartNum())
			System.out.println("PartNum:  " + genMsg.partNum());

		if (genMsg.hasSeqNum())
			System.out.println("SeqNum:   " + genMsg.seqNum());

		switch (genMsg.attrib().dataType())
		{
		case DataTypes.ELEMENT_LIST:
			decode(genMsg.attrib().elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(genMsg.attrib().fieldList());
			break;
		}

		switch (genMsg.payload().dataType())
		{
		case DataTypes.ELEMENT_LIST:
			decode(genMsg.payload().elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(genMsg.payload().fieldList());
			break;
		case DataTypes.REFRESH_MSG:
			decode(genMsg.payload().refreshMsg());
			break;
		case DataTypes.UPDATE_MSG:
			decode(genMsg.payload().updateMsg());
			break;
		case DataTypes.STATUS_MSG:
			decode(genMsg.payload().statusMsg());
			break;
		}
	}

	void decode(ElementList elementList)
	{
		for (ElementEntry elementEntry : elementList)
		{
			System.out.println("Name: " + elementEntry.name() + " DataType: " + DataType.asString(elementEntry.load().dataType()) + " Value: ");

			if (Data.DataCode.BLANK == elementEntry.code())
				System.out.println(" blank");
			else
				switch (elementEntry.loadType())
			{
				case DataTypes.REAL:
					System.out.println(elementEntry.real().asDouble());
					break;
				case DataTypes.DATE:
					System.out.println(elementEntry.date().day() + " / " + elementEntry.date().month() + " / " + elementEntry.date().year());
					break;
				case DataTypes.TIME:
					System.out.println(elementEntry.time().hour() + ":" + elementEntry.time().minute() + ":" + elementEntry.time().second() + ":" + elementEntry.time().millisecond());
					break;
				case DataTypes.INT:
					System.out.println(elementEntry.intValue());
					break;
				case DataTypes.UINT:
					System.out.println(elementEntry.uintValue());
					break;
				case DataTypes.ASCII:
					System.out.println(elementEntry.ascii());
					break;
				case DataTypes.ENUM:
					System.out.println(elementEntry.enumValue());
					break;
				case DataTypes.ERROR:
					System.out.println(elementEntry.error().errorCode() + " (" + elementEntry.error().errorCodeAsString() + ")");
					break;
				default:
					System.out.println();
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
					System.out.println(fieldEntry.enumValue());
					break;
				case DataTypes.ERROR :
					System.out.println(fieldEntry.error().errorCode() +" (" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default :
					System.out.println();
					break;
				}
		}
	}
	
	void setOmmConsumer(OmmConsumer consumer)
	{
		_ommConsumer = consumer;
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
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().username("user"));
			
			appClient.setOmmConsumer(consumer);
			
			long handle = consumer.registerClient(EmaFactory.createReqMsg().domainType(200).serviceName("DIRECT_FEED")
																																   .name("IBM.XYZ").privateStream(true), appClient, (Integer)1);

			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp);
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
