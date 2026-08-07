/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series300.ex331_Directory_Streaming;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.domain.directory.DirectoryRefresh;
import com.refinitiv.ema.domain.directory.DirectoryRequest;
import com.refinitiv.ema.domain.directory.DirectoryStatus;
import com.refinitiv.ema.domain.directory.DirectoryUpdate;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;


class AppClient implements OmmConsumerClient
{
    private final DirectoryRefresh directoryRefresh =  EmaFactory.Domain.createDirectoryRefresh();
    private final DirectoryStatus directoryStatus =  EmaFactory.Domain.createDirectoryStatus();
    private final DirectoryUpdate directoryUpdate =  EmaFactory.Domain.createDirectoryUpdate();

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

        if (refreshMsg.domainType() == EmaRdm.MMT_DIRECTORY)
        {
            directoryRefresh.clear();
            directoryRefresh.message(refreshMsg);
            System.out.println(directoryRefresh);
        }
        else
        {
            System.out.println(refreshMsg);
        }

        System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

        if (updateMsg.domainType() == EmaRdm.MMT_DIRECTORY)
        {
            directoryUpdate.clear();
            directoryUpdate.message(updateMsg);
            System.out.println(directoryUpdate);
        }
        else
        {
            System.out.println(updateMsg);
        }

        System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.domainType() == EmaRdm.MMT_DIRECTORY)
        {
            directoryStatus.clear();
            directoryStatus.message(statusMsg);
            System.out.println(directoryStatus);
        }
        else
        {
            if (statusMsg.hasState())
                System.out.println("Item State: " +statusMsg.state());
        }

        System.out.println();
	}
	
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event) {}
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event){}
	public void onAllMsg(Msg msg, OmmConsumerEvent event){}
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

            DirectoryRequest directoryRequest =  EmaFactory.Domain.createDirectoryRequest();
            directoryRequest.serviceName("DIRECT_FEED");

            consumer.registerClient(directoryRequest.message(), appClient);
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient);

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
