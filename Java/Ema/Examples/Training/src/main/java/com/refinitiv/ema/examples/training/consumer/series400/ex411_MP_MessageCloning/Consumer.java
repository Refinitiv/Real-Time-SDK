///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex411_MP_MessageCloning;

import com.refinitiv.ema.access.Msg;

import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;


class AppClient implements OmmConsumerClient, Runnable
{
	private LinkedBlockingDeque<Msg> _messageQueue;
	private LinkedBlockingDeque<RefreshMsg> _refreshMsgPool;
	private LinkedBlockingDeque<UpdateMsg> _updateMsgPool;
	private LinkedBlockingDeque<StatusMsg> _statusMsgPool;
	
	private ExecutorService _executor;
	private volatile boolean _exit;
	
	private int _updateBufferSize;
	private volatile int _updatePoolSize;
	
	public AppClient(int updatePoolSize, int updateBufferSize)
	{
		_updateBufferSize = updateBufferSize;
		_updatePoolSize = updatePoolSize;
		
		_messageQueue = new LinkedBlockingDeque<Msg>();
		_refreshMsgPool = new LinkedBlockingDeque<RefreshMsg>();	
		_updateMsgPool = new LinkedBlockingDeque<UpdateMsg>();
		_statusMsgPool = new LinkedBlockingDeque<StatusMsg>();
		
		for(int i = 0; i < updatePoolSize; i++)
		{
			_updateMsgPool.add(EmaFactory.createUpdateMsg(updateBufferSize));
		}
		
		_executor = Executors.newSingleThreadExecutor();
		_executor.execute(this);
	}
	
	void exit()
	{
		_exit = true;
		_executor.shutdown();
		try {
			_executor.awaitTermination(5, TimeUnit.SECONDS);
		} catch (InterruptedException excp) {
			System.out.println(excp.getMessage());
		}
	}
	
	RefreshMsg getRefreshMsg()
	{
		RefreshMsg refershMsg = _refreshMsgPool.poll();
		if(refershMsg == null)
		{
			refershMsg = EmaFactory.createRefreshMsg(2560);
		}
		
		return refershMsg;
	}
	
	UpdateMsg getUpdateMsg()
	{
		UpdateMsg updateMsg = _updateMsgPool.poll();
		if(updateMsg == null)
		{
			updateMsg = EmaFactory.createUpdateMsg(_updateBufferSize);
		}
		
		return updateMsg;
	}
	
	StatusMsg getStatusMsg()
	{
		StatusMsg statusMsg = _statusMsgPool.poll();
		if(statusMsg == null)
		{
			statusMsg = EmaFactory.createStatusMsg(512);
		}
		
		return statusMsg;
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		RefreshMsg copyRefreshMsg = getRefreshMsg();
		refreshMsg.copy(copyRefreshMsg);
		_messageQueue.add(copyRefreshMsg);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		UpdateMsg copyUpdateMsg = getUpdateMsg();
		updateMsg.copy(copyUpdateMsg);
		_messageQueue.add(copyUpdateMsg);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		StatusMsg copyStatusMsg = getStatusMsg();
		statusMsg.copy(copyStatusMsg);
		_messageQueue.add(copyStatusMsg);
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	@Override
	public void run() {
		
		while(!_exit)
		{
			Msg queuedMessage = _messageQueue.poll();
			if(queuedMessage != null)
			{
				System.out.println("Item Name: " + (queuedMessage.hasName() ? queuedMessage.name() : "<not set>"));
				
				switch(queuedMessage.dataType())
				{
				case DataType.DataTypes.REFRESH_MSG:
					RefreshMsg refreshMsg = (RefreshMsg)queuedMessage;
					
					System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
					System.out.println("Item State: " + refreshMsg.state());
					
					if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
						System.out.println(refreshMsg.payload().fieldList());
					
					_refreshMsgPool.add(refreshMsg.clear());
					
					break;
				case DataType.DataTypes.UPDATE_MSG:
					UpdateMsg updateMsg = (UpdateMsg)queuedMessage;
					
					System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
					
					if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
						System.out.println(updateMsg.payload().fieldList());
					
					updateMsg.clear();
					
					if(_updateMsgPool.size() < _updatePoolSize) // Checks to limit the size of the pool
						_updateMsgPool.add(updateMsg);
					else
						updateMsg = null;
					
					break;
				case DataType.DataTypes.STATUS_MSG:
					StatusMsg statusMsg = (StatusMsg)queuedMessage;
					
					System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));
					
					if (statusMsg.hasState())
						System.out.println("Item State: " +statusMsg.state());
					
					_statusMsgPool.add(statusMsg.clear());
					
					break;
				}
			}
			else
			{
				try {
					TimeUnit.MICROSECONDS.sleep(1); // Waits for a message from the message queue to avoid CPU peak. 
				} catch (InterruptedException excp) {
					System.out.println(excp.getMessage());
				}
			}
		}
	}
}

public class Consumer 
{
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		
		AppClient appClient = new AppClient(300, 2048); // Specifies number of the UpdateMsg object in the pool for reuse and the buffer size of the UpdateMsg
		
		try
		{	
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("LSEG.L"), appClient);
			
			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
			appClient.exit();
		}
	}
}
