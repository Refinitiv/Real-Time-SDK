///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series200.ex280_MP_Perf;

import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmArray;
import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.OmmAscii;
import com.rtsdk.ema.access.OmmBuffer;
import com.rtsdk.ema.access.OmmDate;
import com.rtsdk.ema.access.OmmDateTime;
import com.rtsdk.ema.access.OmmError;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmRmtes;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.OmmTime;
import com.rtsdk.ema.access.OmmUtf8;
import com.rtsdk.ema.access.Payload;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.Data;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.DataType.DataTypes;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmQos;


class AppClient implements OmmConsumerClient
{
	long updateCount = 0;
	long refreshCount = 0;
	long statusCount = 0;
	Payload payload;

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		++refreshCount;
		
		payload = refreshMsg.payload();
		if (DataType.DataTypes.FIELD_LIST == payload.dataType())
			decode(payload.fieldList());
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		++updateCount;
		
		payload = updateMsg.payload();
		if (DataType.DataTypes.FIELD_LIST == payload.dataType())
			decode(payload.fieldList());
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		++statusCount;
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	@SuppressWarnings("unused")
	void decode(FieldList fieldList)
	{
		try
		{
			for(FieldEntry fieldEntry : fieldList)
			{
				if (Data.DataCode.NO_CODE == fieldEntry.code())
					switch (fieldEntry.loadType())
					{
						case DataTypes.REAL :
						{
							OmmReal re = fieldEntry.real();
						}
						break;
						case DataTypes.DATE :
						{
							OmmDate date = fieldEntry.date();
						}
						break;
						case DataTypes.TIME :
						{
							OmmTime time = fieldEntry.time();
						}
						break;
						case DataTypes.DATETIME :
						{
							OmmDateTime dateTime = fieldEntry.dateTime();
						}
						break;
						case DataTypes.INT :
						{
							long value = fieldEntry.intValue();
						}
						break;
						case DataTypes.UINT :
						{
							long value = fieldEntry.uintValue();
						}
						break;
						case DataTypes.FLOAT :
						{
							float value = fieldEntry.floatValue();
						}
						break;
						case DataTypes.DOUBLE :
						{
							double value = fieldEntry.doubleValue();
						}
						break;
						case DataTypes.QOS :
						{
							OmmQos value = fieldEntry.qos();
						}
						break;
						case DataTypes.STATE :
						{
							OmmState value = fieldEntry.state();
						}
						break;
						case DataTypes.ASCII :
						{
							OmmAscii asciiString = fieldEntry.ascii();
						}
						break;
						case DataTypes.RMTES :
						{
							OmmRmtes rmtesBuffer = fieldEntry.rmtes();
						}
						break;
						case DataTypes.UTF8 :
						{
							OmmUtf8 utf8Buffer = fieldEntry.utf8();
						}
						break;
						case DataTypes.BUFFER :
						{
							OmmBuffer value = fieldEntry.buffer();
						}
						break;
						case DataTypes.ENUM :
						{
							int value = fieldEntry.enumValue();
						}
						break;
						case DataTypes.ARRAY :
						{
							OmmArray value = fieldEntry.array();
						}
						break;
						case DataTypes.ERROR :
						{
							OmmError error = fieldEntry.error();
						}
						break;
						default :
						break;
					}
			}
		}
		catch (OmmException excp)
		{
			System.out.println(excp);
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
			
			String itemPreName = "RTR";
			StringBuilder itemName = new StringBuilder();
			for (int idx = 0; idx < 1000; ++idx)
			{
				itemName.append(itemPreName).append(idx).append(".N");
				reqMsg.clear().serviceName("DIRECT_FEED").name(itemName.toString());
				consumer.registerClient(reqMsg, appClient);
				itemName.setLength(0);
			}

			StringBuilder display = new StringBuilder();
			for (int idx = 0; idx < 300; ++idx)
			{
				Thread.sleep(1000);
				
				display.append("total refresh count: " ).append(appClient.refreshCount).append("\ttotal status count: ").append(appClient.statusCount)
							.append("\tupdate rate (per sec): ").append(appClient.updateCount);
				System.out.println(display.toString());
				
				display.setLength(0);
				appClient.updateCount = 0;
			}
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
