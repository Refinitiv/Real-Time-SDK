///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series300.ex340_MP_OnStreamPost;

import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.Data;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.DataType.DataTypes;
import com.rtsdk.ema.access.ElementEntry;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;


class AppClient implements OmmConsumerClient
{
        private static int postId = 1;
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		if (refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
			refreshMsg.state().dataState() == OmmState.DataState.OK)
		{
			PostMsg postMsg = EmaFactory.createPostMsg();
			UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
			FieldList nestedFieldList = EmaFactory.createFieldList();

			nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
			nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
			nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
			nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));
			
			nestedUpdateMsg.payload(nestedFieldList );
				
			((OmmConsumer)event.closure()).submit( postMsg.postId( postId++ ).serviceId( 1701 )
														.name( "IBM.N" ).solicitAck( true ).complete(true)
														.payload(nestedUpdateMsg), event.handle() );
		}

		decode(refreshMsg);
		//APIQA	
		System.out.println();
		System.out.println("Clone RefreshMsg");
                RefreshMsg cloneRefreshMsg = EmaFactory.createRefreshMsg(refreshMsg); 
                System.out.println(cloneRefreshMsg);
		
                System.out.println("Clone Refresh Item State: " + cloneRefreshMsg.state());
		//END APIQA	
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		decode(updateMsg);
		
		//APIQA	
		System.out.println();
		System.out.println("Clone UpdateMsg");
                UpdateMsg cloneUpdateMsg = EmaFactory.createUpdateMsg(updateMsg); 
                System.out.println(cloneUpdateMsg);
		decode(cloneUpdateMsg);
		//END APIQA	
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " + statusMsg.state());
		
		//APIQA	
		System.out.println();
		System.out.println("Clone StatusMsg");
                StatusMsg cloneStatusMsg = EmaFactory.createStatusMsg(statusMsg); 
                System.out.println(cloneStatusMsg);
		if (cloneStatusMsg.hasState())
			System.out.println("Item State: " + cloneStatusMsg.state());
		//END APIQA	
	}
	
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event)
	{
		System.out.println("Received AckMsg. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		decode(ackMsg);
		
		//APIQA	
		System.out.println();
		System.out.println("Clone AckMsg");
                AckMsg cloneAckMsg = EmaFactory.createAckMsg(ackMsg); 
                System.out.println(cloneAckMsg);
		decode(cloneAckMsg);
		//END APIQA	
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event){}
	public void onAllMsg(Msg msg, OmmConsumerEvent event){}

	void decode( AckMsg ackMsg )
	{
		if ( ackMsg.hasMsgKey() )
			System.out.println("Item Name: " + ( ackMsg.hasName() ? ackMsg.name() : "not set" ) +  "\nService Name: " 
					+ ( ackMsg.hasServiceName() ? ackMsg.serviceName() : "not set" ) );

		System.out.println("Ack Id: "  + ackMsg.ackId());

		if ( ackMsg.hasNackCode() )
			System.out.println("Nack Code: " + ackMsg.nackCodeAsString());

		if ( ackMsg.hasText() )
			System.out.println("Text: " + ackMsg.text());

		switch ( ackMsg.attrib().dataType() )
		{
		case DataTypes.ELEMENT_LIST:
			decode( ackMsg.attrib().elementList() );
			break;
		case DataTypes.FIELD_LIST:
			decode( ackMsg.attrib().fieldList() );
			break;
		default:
			break;
		}

		switch ( ackMsg.payload().dataType() )
		{
		case DataTypes.ELEMENT_LIST:
			decode( ackMsg.payload().elementList() );
			break;
		case DataTypes.FIELD_LIST:
			decode( ackMsg.payload().fieldList() );
			break;
		default:
			break;
		}
	}

	void decode(Msg msg)
	{
		switch(msg.attrib().dataType())
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

		switch(msg.payload().dataType())
		{
		case  DataTypes.ELEMENT_LIST:
			decode(msg.payload().elementList());
			break;
		case DataTypes.FIELD_LIST:
			decode(msg.payload().fieldList());
			break;
		default:
			break;
		}
	}
	
	void decode(ElementList elementList)
	{
		for(ElementEntry elementEntry : elementList)
		{
			System.out.print(" Name = " + elementEntry.name() + " DataType: " + DataType.asString(elementEntry.load().dataType()) + " Value: ");

			if (Data.DataCode.BLANK == elementEntry.code())
				System.out.println(" blank");
			else
				switch (elementEntry.loadType())
				{
				case DataTypes.REAL :
					System.out.println(elementEntry.real().asDouble());
					break;
				case DataTypes.DATE :
					System.out.println(elementEntry.date().day() + " / " + elementEntry.date().month() + " / " + elementEntry.date().year());
					break;
				case DataTypes.TIME :
					System.out.println(elementEntry.time().hour() + ":" + elementEntry.time().minute() + ":" + elementEntry.time().second() + ":" + elementEntry.time().millisecond());
					break;
				case DataTypes.INT :
					System.out.println(elementEntry.intValue());
					break;
				case DataTypes.UINT :
					System.out.println(elementEntry.uintValue());
					break;
				case DataTypes.ASCII :
					System.out.println(elementEntry.ascii());
					break;
				case DataTypes.ENUM :
					System.out.println(elementEntry.enumValue());
					break;
				case DataTypes.RMTES :
					System.out.println(elementEntry.rmtes());
					break;
				case DataTypes.ERROR :
					System.out.println(elementEntry.error().errorCode() +" (" + elementEntry.error().errorCodeAsString() + ")");
					break;
				default :
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
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient, consumer);

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
