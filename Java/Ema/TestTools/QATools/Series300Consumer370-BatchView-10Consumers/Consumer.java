///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series300.ex370_MP_Batch;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.rdm.EmaRdm;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		RefreshMsg msgClone = EmaFactory.createRefreshMsg(refreshMsg);
		System.out.println(msgClone);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		UpdateMsg msgClone = EmaFactory.createUpdateMsg(updateMsg);
		System.out.println(msgClone);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		StatusMsg msgClone = EmaFactory.createStatusMsg(statusMsg);
		System.out.println(msgClone);
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
	public static void main(String[] args) throws InterruptedException
	{
		{
            OmmConsumer consumer = null;
            OmmConsumerConfig config = null;
			//APIQA bring multiple consumers up and down to trigger any memory issues
            for (int i = 0; i < 10; i++)
            {
                try
                {
                	AppClient appClient = new AppClient();
                    config = EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user");
                    consumer = EmaFactory.createOmmConsumer(config);

                    long itemHandle = consumer.registerClient(EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name("IBM.N"), appClient); 

                    OmmArray batchArray = EmaFactory.createOmmArray();
                    batchArray.add(EmaFactory.createOmmArrayEntry().ascii("TRI.N"));
                    batchArray.add(EmaFactory.createOmmArrayEntry().ascii("IBM.N"));
                    batchArray.add(EmaFactory.createOmmArrayEntry().ascii("MSFT.O"));
                    
                    OmmArray viewArray = EmaFactory.createOmmArray();
                    viewArray.fixedWidth(2);
                    viewArray.add(EmaFactory.createOmmArrayEntry().intValue(22));
                    viewArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList batchView = EmaFactory.createElementList();
                    batchView.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, batchArray));
                    batchView.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
                    batchView.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, viewArray));

                    consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_DIRECTORY).serviceName("ELEKTRON_DD").payload(batchView), appClient);

                    Thread.sleep(3000);
                }
                catch (InterruptedException | OmmException excp)
        		{
        			System.out.println(excp.getMessage());
        		}
        		finally 
        		{
        			if (consumer != null) consumer.uninitialize();
        		}

                Thread.sleep(1000);

                System.gc();
            }
        }

		System.out.println("Waiting to exit application");
        System.gc();
        System.gc();
        Thread.sleep(600000); // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
	}
}



