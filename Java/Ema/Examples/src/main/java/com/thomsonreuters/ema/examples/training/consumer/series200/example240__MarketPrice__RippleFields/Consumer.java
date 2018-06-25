///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2016. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series200.example240__MarketPrice__RippleFields;

import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;


class AppClient implements OmmConsumerClient
{
	double BID, BID_1, BID_2, ASK, ASK_1, ASK_2 = 0;

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());

		System.out.println("Item Handle: " + event.handle() + " Item Closure: " + event.closure().hashCode());

		if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		System.out.println("Item Handle: " + event.handle() + " Item Closure: " + event.closure().hashCode());

		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());
		
		System.out.println();
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
		for(FieldEntry fieldEntry : fieldList)
		{
			switch (fieldEntry.loadType())
			{
				case DataTypes.REAL :
					if (fieldEntry.fieldId() == 22)	// Display data for BID field name and its ripple fields
					{
						if (fieldEntry.rippleTo(fieldEntry.rippleTo()) == 24)
							BID_2 = BID_1;

						if (fieldEntry.rippleTo() == 23)
							BID_1 = BID;

						BID = fieldEntry.real().asDouble();

						System.out.println("DataType: " + DataType.asString(fieldEntry.load().dataType()));
						System.out.println("Name: " + fieldEntry.name() + " (" + fieldEntry.fieldId() + ") Value: " + BID);
						System.out.println("Name: " + fieldEntry.rippleToName() + " (" + fieldEntry.rippleTo() + ") Value: " + BID_1);
						System.out.println("Name: " + fieldEntry.rippleToName(fieldEntry.rippleTo()) + " (" + fieldEntry.rippleTo(fieldEntry.rippleTo())
						+ ") Value: " + BID_2);
					}
					else if (fieldEntry.fieldId() == 25) // Display data for ASK field name and its ripple fields
					{
						if (fieldEntry.rippleTo(fieldEntry.rippleTo()) == 27)
							ASK_2 = ASK_1;

						if (fieldEntry.rippleTo() == 26)
							ASK_1 = ASK;

						ASK = fieldEntry.real().asDouble();

						System.out.println("DataType: " + DataType.asString(fieldEntry.load().dataType()));
						System.out.println("Name: " + fieldEntry.name() + " (" + fieldEntry.fieldId() + ") Value: " + ASK);
						System.out.println("Name: " + fieldEntry.rippleToName() + " (" + fieldEntry.rippleTo() + ") Value: " + ASK_1);
						System.out.println("Name: " + fieldEntry.rippleToName(fieldEntry.rippleTo()) + " (" + fieldEntry.rippleTo(fieldEntry.rippleTo())
						+ ") Value: " + ASK_2);
					}
					break;
				default:
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
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient, 1);
			
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
