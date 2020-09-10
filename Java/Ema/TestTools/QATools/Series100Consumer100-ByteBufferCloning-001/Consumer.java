///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming;

import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerConfig;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;

import java.nio.ByteBuffer;

class AppClient implements OmmConsumerClient
{

	private static byte[] newPermissionBytes = new byte[]{0x10, 0x20, 0x30, 0x40};
	private static byte[] newExtendedHeader = new byte[]{0x50, 0x51, 0x52, 0x53, 0x54};
	private static String newName = "TRI.N";

	public String replace(String src) {
		return src.replaceAll("03 01 2c 56 25 c0", "10 20 30 40")
				.replaceAll("61 62 63 64 65 66 67 45 4e 44", "50 51 52 53 54")
				.replaceAll("IBM.N", "TRI.N")
				.replaceAll("serviceId=\"1\"", "serviceId=\"123\"");
	}

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("onRefreshMsg START");
		String expectedMsgString = refreshMsg.toString();
		System.out.println(expectedMsgString);

		System.out.println("---");
		RefreshMsg clonedMsg = EmaFactory.createRefreshMsg(refreshMsg);
		String clonedMsgString = clonedMsg.toString();
		System.out.println(clonedMsgString);
		assert expectedMsgString.equals(clonedMsgString) : "RefreshMsg.toString|() calls should return equal strings";

		System.out.println("---Altering");
		clonedMsg.permissionData(ByteBuffer.wrap(newPermissionBytes));
		clonedMsg.extendedHeader(ByteBuffer.wrap(newExtendedHeader));
		clonedMsg.name(newName);
		clonedMsg.serviceId(123);
		clonedMsgString = clonedMsg.toString();
		System.out.println(clonedMsgString);
		expectedMsgString = replace(expectedMsgString);
		assert expectedMsgString.equals(clonedMsgString) : "RefreshMsg.toString|() should match replaced string";

		System.out.println("onRefreshMsg DONE");
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {
		System.out.println("onUpdateMsg START");
		String expectedMsgString = updateMsg.toString();
		System.out.println(expectedMsgString);

		System.out.println("---");
		UpdateMsg clonedMsg = EmaFactory.createUpdateMsg(updateMsg);
		String clonedMsgString = clonedMsg.toString();
		System.out.println(clonedMsgString);
		assert expectedMsgString.equals(clonedMsgString) : "UpdateMsg.toString|() calls should return equal strings";
		System.out.println("onUpdateMsg DONE");
	}
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {
		System.out.println("onStatusMsg START");
		String expectedMsgString = statusMsg.toString();
		System.out.println(expectedMsgString);

		System.out.println("---");
		StatusMsg clonedMsg = EmaFactory.createStatusMsg(statusMsg);
		String clonedMsgString = clonedMsg.toString();
		System.out.println(clonedMsgString);
		assert expectedMsgString.equals(clonedMsgString) : "StatusMsg.toString|() calls should return equal strings";
		System.out.println("onStatusMsg END");
	}
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){
		System.out.println("onGenericMsg START");
		String expectedMsgString = genericMsg.toString();
		System.out.println(expectedMsgString);

		System.out.println("---");
		GenericMsg clonedMsg = EmaFactory.createGenericMsg(genericMsg);
		String clonedMsgString = clonedMsg.toString();
		System.out.println(clonedMsgString);
		assert expectedMsgString.equals(clonedMsgString) : "GenericMsg.toString|() calls should return equal strings";
		System.out.println("onGenericMsg END");
	}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){
		System.out.println("onAckMsg START");
		String expectedMsgString = ackMsg.toString();
		System.out.println(expectedMsgString);

		System.out.println("---");
		AckMsg clonedMsg = EmaFactory.createAckMsg(ackMsg);
		String clonedMsgString = clonedMsg.toString();
		System.out.println(clonedMsgString);
		assert expectedMsgString.equals(clonedMsgString) : "AckMsg.toString|() calls should return equal strings";
		System.out.println("onAckMsg END");
	}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
			consumer  = EmaFactory.createOmmConsumer(config.host("localhost:14002").username("user"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.serviceName("DIRECT_FEED").name("IBM.N"), appClient);
			
			Thread.sleep(4000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
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


