///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex500_PreferredHost_FileCfg;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.examples.training.common.CommandLine;

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

        // APIQA
//        System.out.println("\nEvent channel info (refresh)\n" + event.channelInformation());
        // APIQA END
        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (!updateCalled)
        {
            // APIQA
//            updateCalled = true;
            // APIQA END

            System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

            if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
                decode(updateMsg.payload().fieldList());

            // APIQA
//            System.out.println("\nEvent channel info (update)\n" + event.channelInformation());
            // APIQA END
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

        // APIQA
//        System.out.println("\nEvent channel info (status)\n" + event.channelInformation());
        // APIQA END
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
    // APIQA
    private static final String DEFAULT_SERVICE_NAME_1 = "DIRECT_FEED";
    private static final String DEFAULT_SERVICE_NAME_2 = "ELEKTRON_DD";
    private static final String DEFAULT_CONSUMER_NAME = "Consumer_9";
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
        CommandLine.addOption(SERVICE_NAME_2, DEFAULT_SERVICE_NAME_2, "Specifies second service name. Default value is ELEKTRON_DD");
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

            int printInterval = 5;
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


