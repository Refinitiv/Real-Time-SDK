/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series100.ex100_MP_Streaming;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.examples.training.common.CommandLine;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println(refreshMsg);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println(updateMsg);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println(statusMsg);
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
    // APIQA
    private static final String DEFAULT_SERVICE_NAME_1 = "DIRECT_FEED";
    private static final String DEFAULT_SERVICE_NAME_2 = "DIRECT_FEED";
    private static final String DEFAULT_CONSUMER_NAME = "Consumer_7";
    private static final String DEFAULT_ITEM_NAME_1 = "IBM.N";
    private static final String DEFAULT_ITEM_NAME_2 = "TRI.N";

    private static final String SERVICE_NAME_1 = "serviceName1";
    private static final String SERVICE_NAME_2 = "serviceName2";
    private static final String ITEM_NAME_1 = "itemName1";
    private static final String ITEM_NAME_2 = "itemName2";

    private static void addCommandLineArgs()
    {
        CommandLine.programName("Consumer");

        CommandLine.addOption(SERVICE_NAME_1, DEFAULT_SERVICE_NAME_1, "Specifies first service name. Default value is DIRECT_FEED");
        CommandLine.addOption(SERVICE_NAME_2, DEFAULT_SERVICE_NAME_2, "Specifies second service name. Default value is DIRECT_FEED");
        CommandLine.addOption(ITEM_NAME_1, DEFAULT_ITEM_NAME_1, "Specifies first item name. Default value is IBM.N");
        CommandLine.addOption(ITEM_NAME_2, DEFAULT_ITEM_NAME_2, "Specifies second item name. Default value is TRI.N");
    }

    private static void init(String[] args) {
        // process command line args
        addCommandLineArgs();
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ex)
        {
            finishWithError(ex.getMessage());
        }
    }

    private static void finishWithError(String message) {
        System.err.println("Error loading command line arguments:\t");
        System.err.println(message);
        System.err.println();
        System.err.println(CommandLine.optionHelpString());
        System.out.println("Consumer exits...");
        System.exit(-1);
    }
    // APIQA END

	public static void main(String[] args)
	{
                // APIQA
                init(args);
               // APIQA END

		OmmConsumer consumer = null;
		try
		{
		        // APIQA
                        AppClient appClient1 = new AppClient();
                        AppClient appClient2 = new AppClient();

                        consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
                                                                .consumerName(DEFAULT_CONSUMER_NAME));
                        consumer.registerClient(EmaFactory.createReqMsg()
                                                .serviceName(CommandLine.value(SERVICE_NAME_1))
                                                .name(CommandLine.value(ITEM_NAME_1)), appClient1, 0);
                        consumer.registerClient(EmaFactory.createReqMsg()
                                                .serviceName(CommandLine.value(SERVICE_NAME_2))
                                                .name(CommandLine.value(ITEM_NAME_2)), appClient2, 0);
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


