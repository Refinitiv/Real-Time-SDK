///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------
//
//APIQA this file is a standalone tool. See qa_readme.txt for details about this tool.

package com.rtsdk.ema.examples.training.consumer.series100.example130__MarketPrice__UserDisp;

import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmArray;

import java.util.ArrayList;
import java.util.ConcurrentModificationException;

import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerConfig.OperationModel;
import com.rtsdk.ema.perftools.common.PerfToolsReturnCodes;
import com.rtsdk.ema.perftools.common.XmlItemInfoList;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());
		
		if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
//		System.out.println("+++++++  ThreadId: " + Thread.currentThread().getId());
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
}


class DispatchThread extends Thread {
	OmmConsumer myCons;

	
	public DispatchThread(OmmConsumer cons) {
		myCons = cons;		
	}
	
	public void run() {

		long startTime = System.currentTimeMillis();
		try {
			while (startTime + 500000 > System.currentTimeMillis()) 
				myCons.dispatch();		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
		} catch (ConcurrentModificationException e) {
			e.printStackTrace();
		} catch (Exception e) {
			System.out.println(e.getStackTrace());
		}
		finally 
		{
		//	System.out.println("====  Runtime expired ====  ThreadId: " + Thread.currentThread().getId());
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
			
			XmlItemInfoList _xmlItemInfoList;
			_xmlItemInfoList = new XmlItemInfoList(1000);
			String fileName = "1k.xml";
			if (_xmlItemInfoList.parseFile(fileName) == PerfToolsReturnCodes.FAILURE)
	        {
	        	System.out.printf("Failed to load item list from file '%s'.\n", fileName);
				System.exit(-1);
			}
				ArrayList<String> items1 = new ArrayList<>();
				ArrayList<String> items2 = new ArrayList<>();
				ArrayList<String> items3 = new ArrayList<>();
				
				for (int i=0; i<_xmlItemInfoList.itemInfoList().length; i++) {
					switch (i%3) 
					{
					case 0: 
						items1.add(_xmlItemInfoList.itemInfoList()[i].name());
						break;
					case 1:
						items2.add(_xmlItemInfoList.itemInfoList()[i].name());
						break;
					case 2:
						items3.add(_xmlItemInfoList.itemInfoList()[i].name());
						break;
						default:
							System.out.println("Skipping items");
							break;
					} 
				}
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
													.operationModel(OperationModel.USER_DISPATCH)
													.host("localhost:14002").username("user"));
			
			ElementList batch = EmaFactory.createElementList();
			ElementList batchSnapshot = EmaFactory.createElementList();			
			OmmArray array = EmaFactory.createOmmArray();
			OmmArray snapshot = EmaFactory.createOmmArray();
			for (int i = 0; i < items1.size(); i++ )
				array.add(EmaFactory.createOmmArrayEntry().ascii(items1.get(i)));
			
			for (int i = 0; i < items2.size(); i++ )
				snapshot.add(EmaFactory.createOmmArrayEntry().ascii(items2.get(i)));			

			batch.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));
			
			batchSnapshot.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, snapshot));
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").payload(batch), appClient);
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").payload(batchSnapshot).interestAfterRefresh(false), appClient);
			
			ElementList view = EmaFactory.createElementList();
			OmmArray arrayView = EmaFactory.createOmmArray();
			
			arrayView.fixedWidth(2);
			arrayView.add(EmaFactory.createOmmArrayEntry().intValue(22));
			arrayView.add(EmaFactory.createOmmArrayEntry().intValue(25));

			view.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
			view.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, arrayView));				
			
			for (int i = 0; i < items3.size(); i++ )
				consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name(items3.get(i)).payload(view), appClient);	
			DispatchThread thread1 = new DispatchThread(consumer);
			DispatchThread thread2 = new DispatchThread(consumer);
			DispatchThread thread3 = new DispatchThread(consumer);
			DispatchThread thread4 = new DispatchThread(consumer);
			DispatchThread thread5 = new DispatchThread(consumer);
			DispatchThread thread6 = new DispatchThread(consumer);
			DispatchThread thread7 = new DispatchThread(consumer);
			DispatchThread thread8 = new DispatchThread(consumer);
			
			thread1.start();
			thread2.start();
			thread3.start();
			thread4.start();
			thread5.start();
			thread6.start();
			thread7.start();
			thread8.start();

			Thread.sleep(600000);
		}
		catch (InterruptedException excp)
		{
			System.out.println(excp);
		}
		catch (OmmException excp)
		{
			System.out.println(excp);
		}
		catch (Exception excp)
		{
			System.out.println(excp);
		}
		finally 
		{
			System.out.println("Uninitializing Consumer");
			if (consumer != null) consumer.uninitialize();
		}
	}
}
