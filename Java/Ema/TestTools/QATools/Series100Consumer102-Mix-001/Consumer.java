///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series100.ex102_MP_Snapshot;

import com.refinitiv.ema.access.*;

import java.util.Iterator;

class AppClient implements OmmConsumerClient
{
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        if (refreshMsg.hasName())
            System.out.println("Item Name: " + refreshMsg.name());

        if (refreshMsg.hasServiceName())
            System.out.println("Service Name: " + refreshMsg.serviceName());

        System.out.println("Item State: " + refreshMsg.state());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().fieldList());

        System.out.println("\n");
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (updateMsg.hasName())
            System.out.println("Item Name: " + updateMsg.name());

        if (updateMsg.hasServiceName())
            System.out.println("Service Name: " + updateMsg.serviceName());

        if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
            decode(updateMsg.payload().fieldList());

        System.out.println("\n");
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        if (statusMsg.hasName())
            System.out.println("Item Name: " + statusMsg.name());

        if (statusMsg.hasServiceName())
            System.out.println("Service Name: " + statusMsg.serviceName());

        if (statusMsg.hasState())
            System.out.println("Service State: " + statusMsg.state());

        System.out.println("\n");
    }
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

    void decode(FieldList fieldList)
    {
        Iterator<FieldEntry> iter = fieldList.iterator();
        FieldEntry fieldEntry;
        while (iter.hasNext())
        {
            fieldEntry = iter.next();
            System.out.println("Fid: " + fieldEntry.fieldId() + " Name: " + fieldEntry.name() + " value: " + fieldEntry.load());
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
		   // APIQA 	
			AppClient appClient = new AppClient();
            AppClient appClient1 = new AppClient();
			
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().consumerName("Consumer_1"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.serviceName("NI_PUB").name("IBM.N").interestAfterRefresh(false), appClient);
            consumer.registerClient(reqMsg.serviceName("ELEKTRON_DD").name("IBM.N").interestAfterRefresh(false), appClient1);
            // APIQA END
			
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


