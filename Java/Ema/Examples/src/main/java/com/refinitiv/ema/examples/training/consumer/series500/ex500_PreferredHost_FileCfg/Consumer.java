///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex500_PreferredHost_FileCfg;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;

class AppClient implements OmmConsumerClient
{
    private boolean updateCalled = false;

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());

        if (DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().fieldList());

        System.out.println("\nEvent channel info (refresh)\n" + event.channelInformation());
        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (!updateCalled)
        {
            updateCalled = true;
            System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

            if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
                decode(updateMsg.payload().fieldList());

            System.out.println("\nEvent channel info (update)\n" + event.channelInformation());
            System.out.println();
        }
        else {
            System.out.println("skipped printing updateMsg");
        }
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
            System.out.println("Item State: " +statusMsg.state());

        System.out.println("\nEvent channel info (status)\n" + event.channelInformation());
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
                    case DataTypes.DATETIME :
                        System.out.println(fieldEntry.dateTime().day() + " / " + fieldEntry.dateTime().month() + " / " +
                                fieldEntry.dateTime().year() + "." + fieldEntry.dateTime().hour() + ":" +
                                fieldEntry.dateTime().minute() + ":" + fieldEntry.dateTime().second() + ":" +
                                fieldEntry.dateTime().millisecond() + ":" + fieldEntry.dateTime().microsecond()+ ":" +
                                fieldEntry.dateTime().nanosecond());
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

public class Consumer
{
    private static final String DEFAULT_SERVICE_NAME = "DIRECT_FEED";
    private static final String INFRA_SERVICE_NAME = "ELEKTRON_DD";
    private static final String DEFAULT_CONSUMER_NAME = "Consumer_9";
    private static final String DEFAULT_ITEM_NAME = "IBM.N";

    public static void main(String[] args)
    {
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
                                                                .consumerName(DEFAULT_CONSUMER_NAME));
            consumer.registerClient(EmaFactory.createReqMsg()
                                                .serviceName(DEFAULT_SERVICE_NAME)
                                                .name(DEFAULT_ITEM_NAME), appClient, 0);

            int printInterval = 1;
            ChannelInformation ci = EmaFactory.createChannelInformation();
            for (int i = 0; i < 600; i++) {
                Thread.sleep(1000); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

                if ((i % printInterval == 0)) {
                    consumer.channelInformation(ci);
                    if (ci.channelState() != ChannelInformation.ChannelState.INACTIVE) {
                        System.out.println("\nChannel information (consumer):\n\t" + ci);
                        System.out.println();
                    }
                }
            }
        }
        catch (InterruptedException | OmmException ex)
        {
            System.out.println(ex.getMessage());
        }
        finally
        {
            if (consumer != null) consumer.uninitialize();
        }
    }
}


