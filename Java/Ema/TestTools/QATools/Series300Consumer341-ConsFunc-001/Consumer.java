///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series300.ex341_MP_OffStreamPost;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;


class AppClient implements OmmConsumerClient
{
	private static int postId = 1;
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		if ( refreshMsg.domainType() == EmaRdm.MMT_LOGIN && 
				refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
				refreshMsg.state().dataState() == OmmState.DataState.OK )
			{
				PostMsg postMsg = EmaFactory.createPostMsg();
				RefreshMsg nestedRefreshMsg = EmaFactory.createRefreshMsg();
				FieldList nestedFieldList = EmaFactory.createFieldList();
				
				//FieldList is a collection in java
				nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
				nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
				nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
				nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));
				
				nestedRefreshMsg.payload(nestedFieldList ).complete(true);
				
				((OmmConsumer)event.closure()).submit( postMsg.postId( postId++ ).serviceName( "DIRECT_FEED" )
															.name( "TRI.N" ).solicitAck( true ).complete(true)
															.payload(nestedRefreshMsg), event.handle() );
			}

		decode( refreshMsg );
			
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		decode( updateMsg );
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());
		
		System.out.println();
	}
	
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event)
	{
		System.out.println("Received AckMsg. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		decode( ackMsg );
		
		System.out.println();
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

	void decode( Msg msg )
	{
		switch ( msg.attrib().dataType() )
		{
		case DataTypes.ELEMENT_LIST:
			decode( msg.attrib().elementList() );
			break;
		case DataTypes.FIELD_LIST:
			decode( msg.attrib().fieldList() );
			break;
		default:
			break;
		}

		switch ( msg.payload().dataType() )
		{
		case  DataTypes.ELEMENT_LIST:
			decode( msg.payload().elementList() );
			break;
		case DataTypes.FIELD_LIST:
			decode(msg.payload().fieldList() );
			break;
		default:
			break;
		}
	}
	
	void decode(ElementList elementList)
	{
		for (ElementEntry elementEntry : elementList)
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
			
			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
		
			long loginHandle = consumer.registerClient(reqMsg.domainType( EmaRdm.MMT_LOGIN ), appClient, consumer);
			
			long handle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient, consumer);
            //APIQA
		//	Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
			
			Thread.sleep(3000);
			int id = 2;
			
			PostMsg postMsg = EmaFactory.createPostMsg();
			RefreshMsg nestedRefreshMsg = EmaFactory.createRefreshMsg();
			FieldList nestedFieldList = EmaFactory.createFieldList();
			//FieldList is a collection in java
			nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
			nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
			nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
			nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));
			
			nestedRefreshMsg.payload(nestedFieldList ).complete(true);
			System.out.println(postMsg);
			
			while (true) {
				consumer.submit( postMsg.postId( id++ ).serviceName( "DIRECT_FEED" )
													.name( "TRI.N" ).solicitAck( true ).complete(true)
													.payload(nestedRefreshMsg), loginHandle);
				postMsg.clear();
				
				Thread.sleep(20000);
			}	
		//END APIQA	
				
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
