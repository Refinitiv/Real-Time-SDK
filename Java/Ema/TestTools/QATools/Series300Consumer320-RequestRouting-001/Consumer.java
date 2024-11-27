///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series300.ex320_Custom_GenericMsg;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.Msg;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmConsumerConfig.OperationModel;


class AppClient implements OmmConsumerClient
{
	long count = 0;
	OmmConsumer _ommConsumer = null;
	//API QA
	List<ChannelInformation> channelInList = new ArrayList<ChannelInformation>();

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		// submit a generic message when stream becomes open / ok
		if (refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
				refreshMsg.state().dataState() == OmmState.DataState.OK)
		{
			ElementList elementList = EmaFactory.createElementList();
			elementList.add(EmaFactory.createElementEntry().intValue("value", ++count));
			_ommConsumer.submit(EmaFactory.createGenericMsg().domainType(200).name("genericMsg").payload(elementList), event.handle());
		}
		
		decode(refreshMsg);
		
		System.out.println();
		//API QA	
		System.out.println("event session info (refresh)\n");
		printSessionInfo(event);		
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		decode(updateMsg);
		
		System.out.println();
		//API QA	
		System.out.println("event session info (update)\n");
		printSessionInfo(event);			
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " + statusMsg.state());
		
		System.out.println();
		//API QA	
		System.out.println("event session info (status)\n");
		printSessionInfo(event);
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Generic. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().intValue("value", ++count));
		_ommConsumer.submit(EmaFactory.createGenericMsg().domainType(200).name("genericMsg").payload(elementList), event.handle());

		decode(genericMsg);
		System.out.println();
		//API QA	
		System.out.println("event session info (generic)\n");
		printSessionInfo(event);
	}
	
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(Msg msg)
	{
		switch (msg.attrib().dataType())
		{
		case DataTypes.ELEMENT_LIST:
			decode(msg.attrib().elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(msg.attrib().fieldList());
			break;
		default:
			break;
		}

		switch (msg.payload().dataType())
		{
		case DataTypes.ELEMENT_LIST:
			decode(msg.payload().elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(msg.payload().fieldList());
			break;
		default:
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
		default:
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
		default:
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
				case DataTypes.RMTES :
					System.out.println(elementEntry.rmtes());
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
					System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
					break;
				case DataTypes.RMTES :
					System.out.println(fieldEntry.rmtes());
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
	//API QA
	void printSessionInfo(OmmConsumerEvent event)
	{
		event.sessionChannelInfo(channelInList);
			
		for(ChannelInformation channelInfo : channelInList) 
		{
			System.out.println(channelInfo);
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
			
			//consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().operationModel(OperationModel.USER_DISPATCH).host("localhost:14002").username("user"));
			//API QA
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().operationModel(OperationModel.USER_DISPATCH).consumerName("Consumer_10").username("user"));
			
			appClient.setOmmConsumer(consumer);
			
			consumer.registerClient(EmaFactory.createReqMsg().domainType(200).serviceName("DIRECT_FEED").name("IBM.XYZ"), appClient, 0);

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
