///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex501_PreferredHost_ProgCfg;

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
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (!updateCalled)
        {
            System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

            if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
                decode(updateMsg.payload().fieldList());
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
    private static final boolean DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS = true;
    private static final String DEFAULT_DETECTION_TIME_SCHEDULE = "";
    private static final int DEFAULT_DETECTION_TIME_INTERVAL = 15;
    private static final String DEFAULT_CHANNEL_NAME = "Channel_1";


    private static void addCommandLineArgs()
    {
        CommandLine.programName("Consumer");

        CommandLine.addOption("enablePH", DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS, "Enables preferred host feature");
        CommandLine.addOption("detectionTimeSchedule", DEFAULT_DETECTION_TIME_SCHEDULE, "Specifies Cron time format for detection time schedule");
        CommandLine.addOption("detectionTimeInterval", DEFAULT_DETECTION_TIME_INTERVAL, "Specifies detection time interval in seconds. 0 indicates that the detection time interval is disabled");
        CommandLine.addOption("channelNamePreferred", DEFAULT_CHANNEL_NAME, "Specifies a channel name in the Channel or ChannelSet element. Empty string indicates the first channel name in the ChannelSet is used");
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
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ex.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.out.println("Consumer exits...");
            System.exit(-1);
        }
    }

    private static Map createProgrammaticConfig(boolean enablePreferredHostOptions, int detectionTimeInterval,
                                                String detectionTimeSchedule, String channelName)
    {
        Map innerMap = EmaFactory.createMap();
        Map configMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1" ));

        // ConsumerGroup
        // Consumer_1
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", "Channel_2, Channel_1" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "PreferredChannelName", channelName ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
        if (detectionTimeSchedule != null && !detectionTimeSchedule.isEmpty()) {
            innerElementList.add(EmaFactory.createElementEntry().ascii( "PHDetectionTimeSchedule", detectionTimeSchedule ));
        }
        innerElementList.add(EmaFactory.createElementEntry().uintValue( "EnablePreferredHostOptions", enablePreferredHostOptions ? 1 : 0 ));
        innerElementList.add(EmaFactory.createElementEntry().uintValue( "PHDetectionTimeInterval", detectionTimeInterval ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ItemCountHint", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ServiceCountHint", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ObeyOpenWindow", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "PostAckTimeout", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "RequestTimeout", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxOutstandingPosts", 5000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "DispatchTimeoutApiThread", 1 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "HandleException", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "PipePort", 4001 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 10 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 2000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 6000 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToFile", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceWrite", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceRead", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTracePing", 0 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "MsgKeyInUpdates", 1 ));

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        // ChannelGroup
        // Channel_1
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "InitializationTimeout", 30000));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        // Channel_2
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "InitializationTimeout", 30000));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14003"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        // DictionaryGroup
        // Dictionary_1
        innerElementList.add(EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        return configMap;
    }

    public static void main(String[] args)
    {
        init(args);

        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            boolean enablePreferredHostOptions = CommandLine.booleanValue("enablePH");
            int detectionTimeInterval = CommandLine.intValue("detectionTimeInterval");
            String detectionTimeSchedule = CommandLine.value("detectionTimeSchedule");
            String channelName = CommandLine.value("channelNamePreferred");

            Map progConfig = createProgrammaticConfig(enablePreferredHostOptions, detectionTimeInterval, detectionTimeSchedule, channelName);
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config(progConfig));
            consumer.registerClient(EmaFactory.createReqMsg()
                                                .serviceName("DIRECT_FEED")
                                                .name("IBM.N"), appClient);

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


