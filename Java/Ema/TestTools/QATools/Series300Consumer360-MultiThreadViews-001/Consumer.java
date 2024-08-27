///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2024 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series300.ex360_MP_View;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;


class AppClient implements OmmConsumerClient
{
	static boolean DEBUG = true;
	
	AppClient(boolean debug)
	{
		DEBUG = debug;
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		if (!DEBUG)
			return;

		System.out.println(Thread.currentThread().toString());
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>" + " thread: " + Thread.currentThread().toString()));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());

		if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());
		
		if (DEBUG)
		{
			System.out.println();	
		}
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		if (!DEBUG)
			return;
		
		System.out.println(Thread.currentThread().toString());
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"+ " thread: " + Thread.currentThread()));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		if (!DEBUG)
			return;
		
		System.out.println(Thread.currentThread().toString());
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"+ " thread: " + Thread.currentThread()));
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
		if (!DEBUG)
			return;
		System.out.println(Thread.currentThread().toString());
		for (FieldEntry fieldEntry : fieldList)
		{
			System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: " + " thread: " + Thread.currentThread());

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
					System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default :
					System.out.println();
					break;
				}
		}
	}
}

class consThread extends Thread
{
	final static boolean DEBUG = true;
	
	OmmConsumer consumer = null;
	ElementList view;
	AppClient appClient = new AppClient(DEBUG);
	String uniqueName;
	
	consThread(ElementList view, String uniqueName)
	{
		this.view = view;
		this.uniqueName = uniqueName;
		
		consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
		
	}
	
	@Override
	public void run() {
		// TODO Auto-generated method stub
		ReqMsg request = EmaFactory.createReqMsg();
		request.clear();
		
		for (int i = 0; i < 1000; ++i)
		{	
			long itemHandle = consumer.registerClient(request.serviceName("DIRECT_FEED").name("IBM.N").payload(view), appClient);
			
			if (DEBUG)
				System.out.println(uniqueName + "   registered - Trial " + i);

			try {
				Thread.sleep(i);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			request.clear();
			try {
				consumer.reissue(request, itemHandle);
			}
			catch (OmmInvalidUsageException excp)
			{
				if (DEBUG)
					System.out.println(excp.getMessage());
			}
			
			
			try {
				Thread.sleep(i);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			consumer.unregister(itemHandle);
			
			if (DEBUG)
				System.out.println(uniqueName + "   unregistered - Trial " + i);
				
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
			ElementList view = EmaFactory.createElementList();
			OmmArray array = EmaFactory.createOmmArray();
			
			array.fixedWidth(2);
			array.add(EmaFactory.createOmmArrayEntry().intValue(22));
			array.add(EmaFactory.createOmmArrayEntry().intValue(25));

			view.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
			view.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, array));
			

			consThread consumerThread_One = new consThread(view, "A");
			consumerThread_One.start();
			consThread consumerThread_Two = new consThread(view, "B");
			consumerThread_Two.start();
			consThread consumerThread_Three = new consThread(view, "C");
			consumerThread_Three.start();
			consThread consumerThread_Four = new consThread(view, "D");
			consumerThread_Four.start();
			consThread consumerThread_Five = new consThread(view, "E");
			consumerThread_Five.start();
			consThread consumerThread_Six = new consThread(view, "F");
			consumerThread_Six.start();
			consThread consumerThread_Seven = new consThread(view, "G");
			consumerThread_Seven.start();
			
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

