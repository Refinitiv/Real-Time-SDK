///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------
//APIQA
package com.rtsdk.ema.examples.training.consumer.series400.ex410_MP_HorizontalScaling;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.Data;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.DataType.DataTypes;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.perftools.common.PerfToolsReturnCodes;
import com.rtsdk.ema.perftools.common.XmlItemInfoList;

class AppClient implements OmmConsumerClient
{
	String consumerInstanceName = null;
		public AppClient(String consumerInstanceName) {
				this.consumerInstanceName = consumerInstanceName;
					}
						
							public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
								{
								//	System.out.print("[" + consumerInstanceName + "] ");
								//	System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
								//	System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
								//	System.out.println("Item State: " + refreshMsg.state());
								//	if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
											//decode(refreshMsg.payload().fieldList());
									System.out.print(".");
								}
                                public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.print("[" + consumerInstanceName + "] ");
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			//decode(updateMsg.payload().fieldList());
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.print("[" + consumerInstanceName + "] ");
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
					System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default :
					System.out.println();
					break;
				}
		}
	}
}

class ConsumerInstance implements Runnable
{
	AppClient _appClient;
	OmmConsumer _consumer;
	ReqMsg _reqMsg;
	ExecutorService _executor;
	String[] items = null;
	String serviceName = null;
	
	public ConsumerInstance(String consumerInstanceName, String host, String username, String[] items, String serviceName)
	{
		_appClient = new AppClient(consumerInstanceName);
		_reqMsg = EmaFactory.createReqMsg();
		
		_consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host(host).username(username));
		
		this.items = items;
		this.serviceName = serviceName;
		
		_executor = Executors.newSingleThreadExecutor();
		
		_executor.execute(this);
	}
	
	public void openItem(String item, String serviceName)
	{
		_reqMsg.clear().name(item).serviceName(serviceName).interestAfterRefresh(false);
		
		_consumer.registerClient(_reqMsg, _appClient);
	}
	
	public void run()
	{
		try
		{
			for (String item:items) {
				try {
					Thread.sleep(10);
					_reqMsg.clear().name(item).serviceName(serviceName).interestAfterRefresh(false);
					_consumer.registerClient(_reqMsg, _appClient);
//					Thread.yield();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			Thread.sleep(60000);

			_executor.shutdown();
			_consumer.uninitialize();
			_executor.awaitTermination(5, TimeUnit.SECONDS);
		}
		catch (InterruptedException excp)
		{
			System.out.println(excp.getMessage());
		}	
	}	
}

public class Consumer 
{
	public static void main(String[] args)
	{
		try 
		{
			XmlItemInfoList _xmlItemInfoList;
			_xmlItemInfoList = new XmlItemInfoList(20000);
			String fileName = "20k.xml";
			if (_xmlItemInfoList.parseFile(fileName) == PerfToolsReturnCodes.FAILURE)
	        {
	        	System.out.printf("Failed to load item list from file '%s'.\n", fileName);
				System.exit(-1);
			}
			
			String server = "localhost:14002";
			boolean testThread = true;
			
			if (testThread) {
				ArrayList<String> items1 = new ArrayList<>();
				ArrayList<String> items2 = new ArrayList<>();
				for (int i=0; i<_xmlItemInfoList.itemInfoList().length; i++) {
					if (i%2 == 0) {
						items1.add(_xmlItemInfoList.itemInfoList()[i].name());
					} else {
						items2.add(_xmlItemInfoList.itemInfoList()[i].name());
					}
				}
				String[] itemsString1 = new String[items1.size()];
				itemsString1 = items1.toArray(itemsString1);
				String[] itemsString2 = new String[items2.size()];
				itemsString2 = items2.toArray(itemsString2);
				
				System.err.println("init consumer1");
				ConsumerInstance consumer1 = new ConsumerInstance("consumer1", server, "user1", itemsString1, "DIRECT_FEED");
				System.err.println("init consumer2");
				ConsumerInstance consumer2 = new ConsumerInstance("consumer2", server, "user2", itemsString2, "DIRECT_FEED");				
			} else {
				ArrayList<String> items = new ArrayList<>();
				for (int i=0; i<_xmlItemInfoList.itemInfoList().length; i++) {
					items.add(_xmlItemInfoList.itemInfoList()[i].name());
				}
				String[] itemsString = new String[items.size()];
				itemsString = items.toArray(itemsString);
				
				System.err.println("init consumer");
				ConsumerInstance consumer = new ConsumerInstance("consumer", server, "user", itemsString, "DIRECT_FEED");	
			}

			
			Thread.sleep(60000);
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp);
		}
	}
}
