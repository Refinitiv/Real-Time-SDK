///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex502_PreferredHost_Ioctl;

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

        System.out.println();

        //System.out.println("Event channel info (refresh)\n" + event.channelInformation());
        //System.out.println();
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

            System.out.println();
        }
        else {
            System.out.println("skipped printing updateMsg");
        }

        //System.out.println("Event channel info (update)\n" + event.channelInformation());
        //System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
            System.out.println("Item State: " +statusMsg.state());

        System.out.println();

        //System.out.println("Event channel info (status)\n" + event.channelInformation());
        //System.out.println();
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

    // APIQA END

    private static final int DEFAULT_IOCTL_CALL_TIME_INTERVAL = 0;
    private static final int DEFAULT_FALLBACK_CALL_TIME_INTERVAL = 0;

    private static final boolean DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS = false;
    private static final String DEFAULT_DETECTION_TIME_SCHEDULE = "";
    private static final int DEFAULT_DETECTION_TIME_INTERVAL = 0;
    private static final String DEFAULT_CHANNEL_NAME = "";
    private static final String DEFAULT_WSB_CHANNEL_NAME = "";
    private static final boolean DEFAULT_FALLBACK_WITHIN_WSB_GROUP = false;

    private static final String FALLBACK_INTERVAL = "fallBackInterval";
    private static final String IOCTL_INTERVAL = "ioctlInterval";
    private static final String IOCTL_ENABLE_PH = "ioctlEnablePH";
    private static final String IOCTL_FALLBACK_WITHIN_WSB_GROUP = "ioctlFallBackWithinWSBGroup";
    private static final String IOCTL_DETECTION_TIME_INTERVAL = "ioctlDetectionTimeInterval";
    private static final String IOCTL_DETECTION_TIME_SCHEDULE  = "ioctlDetectionTimeSchedule";
    private static final String IOCTL_CHANNEL_NAME = "ioctlChannelName";
    private static final String IOCTL_WSB_GROUP = "ioctlWarmStandbyGroup";

    private static void addCommandLineArgs()
    {
        CommandLine.programName("Consumer");

        CommandLine.addOption(IOCTL_INTERVAL, DEFAULT_IOCTL_CALL_TIME_INTERVAL, "Specifies sleep time in application before call IOCtl is invoked. O indicates that function won't be invoked");
        CommandLine.addOption(FALLBACK_INTERVAL, DEFAULT_FALLBACK_CALL_TIME_INTERVAL, "Specifies sleep time in application before call Ad Hoc Fallback Function is invoked. O indicates that function won't be invoked");

        CommandLine.addOption(IOCTL_ENABLE_PH, DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS, "Enables preferred host feature");
        CommandLine.addOption(IOCTL_DETECTION_TIME_SCHEDULE, DEFAULT_DETECTION_TIME_SCHEDULE, "Specifies Cron time format for detection time schedule");
        CommandLine.addOption(IOCTL_DETECTION_TIME_INTERVAL, DEFAULT_DETECTION_TIME_INTERVAL, "Specifies detection time interval in seconds. 0 indicates that the detection time interval is disabled");
        CommandLine.addOption(IOCTL_CHANNEL_NAME, DEFAULT_CHANNEL_NAME, "Specifies a channel name in the Channel or ChannelSet element. Empty string indicates the first channel name in the ChannelSet is used");
        CommandLine.addOption(IOCTL_WSB_GROUP, DEFAULT_WSB_CHANNEL_NAME, "Specifies a WSB channel name in the WarmStandbyChannelSet element. Empty string indicates the first WSB channel name in the WarmStandbyChannelSet is used");
        CommandLine.addOption(IOCTL_FALLBACK_WITHIN_WSB_GROUP, DEFAULT_FALLBACK_WITHIN_WSB_GROUP, "Specifies whether to fallback within a WSB group instead of moving into a preferred WSB group");

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

    private static PreferredHostOptions getPreferredHostOptions(PreferredHostInfo currentPreferredHostInfo,
                                                             boolean enablePreferredHostOptions, boolean fallBackWithInWSBGroup,
                                                             int detectionTimeInterval, String detectionTimeSchedule,
                                                             String channelName, String wsbChannelName) {

        PreferredHostOptions preferredHostOptions = EmaFactory.createPreferredHostOptions();

        boolean ioctlEnablePH = CommandLine.hasArg(IOCTL_ENABLE_PH) ?
                enablePreferredHostOptions : currentPreferredHostInfo.isPreferredHostEnabled();
        preferredHostOptions.setPreferredHostEnabled(ioctlEnablePH);

        String ioctlChannelName = CommandLine.hasArg(IOCTL_CHANNEL_NAME) ?
                channelName : currentPreferredHostInfo.getChannelName();
        preferredHostOptions.setChannelName(ioctlChannelName);

        long ioctlDetectionTimeInterval = CommandLine.hasArg(IOCTL_DETECTION_TIME_INTERVAL) ?
                detectionTimeInterval : currentPreferredHostInfo.getDetectionTimeInterval();
        preferredHostOptions.setDetectionTimeInterval(ioctlDetectionTimeInterval);

        String ioctlDetectionTimeSchedule = CommandLine.hasArg(IOCTL_DETECTION_TIME_SCHEDULE) ?
                detectionTimeSchedule.trim() : currentPreferredHostInfo.getDetectionTimeSchedule();
        preferredHostOptions.setDetectionTimeSchedule(ioctlDetectionTimeSchedule);

        String ioctlWarmStandbyGroup = CommandLine.hasArg(IOCTL_WSB_GROUP) ?
                wsbChannelName : currentPreferredHostInfo.getWsbChannelName();
        preferredHostOptions.setWsbChannelName(ioctlWarmStandbyGroup);

        boolean ioctlFallBackWithinWSBGroup = CommandLine.hasArg(IOCTL_FALLBACK_WITHIN_WSB_GROUP) ?
                fallBackWithInWSBGroup : currentPreferredHostInfo.isFallBackWithInWSBGroup();
        preferredHostOptions.setFallBackWithInWSBGroup(ioctlFallBackWithinWSBGroup);

        return preferredHostOptions;
    }

    private static boolean isPHInfoChanged(PreferredHostInfo currentPreferredHostInfo, boolean enablePreferredHostOptions,
                                           boolean fallBackWithInWSBGroup, String channelName, String wsbChannelName,
                                           int detectionTimeInterval, String detectionTimeSchedule) {

        if (currentPreferredHostInfo == null) {
            return true;
        }

        boolean changed = false;

        if (CommandLine.hasArg(IOCTL_ENABLE_PH)) {
            if (currentPreferredHostInfo.isPreferredHostEnabled() != enablePreferredHostOptions) {
                changed = true;
            }
        }

        if (CommandLine.hasArg(IOCTL_FALLBACK_WITHIN_WSB_GROUP)) {
            if (currentPreferredHostInfo.isFallBackWithInWSBGroup() != fallBackWithInWSBGroup) {
                changed = true;
            }
        }

        if (CommandLine.hasArg(IOCTL_CHANNEL_NAME)) {
            if (!currentPreferredHostInfo.getChannelName().equals(channelName)) {
                changed = true;
            }
        }

        if (CommandLine.hasArg(IOCTL_WSB_GROUP)) {
            if (!currentPreferredHostInfo.getWsbChannelName().equals(wsbChannelName)) {
                changed = true;
            }
        }

        if (CommandLine.hasArg(IOCTL_DETECTION_TIME_INTERVAL)) {
            if (currentPreferredHostInfo.getDetectionTimeInterval() != detectionTimeInterval) {
                changed = true;
            }
        }

        if (CommandLine.hasArg(IOCTL_DETECTION_TIME_SCHEDULE)) {
            if (!currentPreferredHostInfo.getDetectionTimeSchedule().equals(detectionTimeSchedule)) {
                changed = true;
            }
        }

        return changed;
    }

    private static boolean isIoctlArgsIncorrect(int ioctlInterval) {
        return (ioctlInterval <= 0) &&
                (CommandLine.hasArg(IOCTL_ENABLE_PH) ||
                        CommandLine.hasArg(IOCTL_CHANNEL_NAME) ||
                        CommandLine.hasArg(IOCTL_DETECTION_TIME_INTERVAL) ||
                        CommandLine.hasArg(IOCTL_DETECTION_TIME_SCHEDULE) ||
                        CommandLine.hasArg(IOCTL_WSB_GROUP) ||
                        CommandLine.hasArg(IOCTL_FALLBACK_WITHIN_WSB_GROUP));
    }

    private static void printModifyIOCtlData(int ioctlInterval, int current) {
        int timeLeft = ioctlInterval - current;
        if (timeLeft > 0) {
            StringBuilder sb = new StringBuilder();
            sb.append("\n\tIoctl call to update PreferredHostOptions:\n");
            sb.append("\tTime interval: ");
            sb.append(ioctlInterval);
            sb.append("\n\tRemaining time: ");
            sb.append(timeLeft);
            sb.append("\n");
            System.out.println(sb);
        }
    }

    private static void printFallbackPreferredHostData(int fallbackInterval, int current) {
        int timeLeft = fallbackInterval - current;
        if (timeLeft > 0) {
            StringBuilder sb = new StringBuilder();
            sb.append("\n\tFallback to preferred host:\n");
            sb.append("\tTime interval: ");
            sb.append(fallbackInterval);
            sb.append("\n\tRemaining time: ");
            sb.append(timeLeft);
            sb.append("\n");
            System.out.println(sb);
        }
    }

    public static void main(String[] args)
    {
        init(args);

        OmmConsumer consumer = null;
        try
        {
            // APIQA
            AppClient appClient1 = new AppClient();
            AppClient appClient2 = new AppClient();
            // APIQA END
            ChannelInformation ci = EmaFactory.createChannelInformation();

            int ioctlInterval = CommandLine.intValue(IOCTL_INTERVAL);
            int fallbackInterval = CommandLine.intValue(FALLBACK_INTERVAL);

            boolean enablePreferredHostOptions = CommandLine.booleanValue(IOCTL_ENABLE_PH);
            boolean fallBackWithInWSBGroup = CommandLine.booleanValue(IOCTL_FALLBACK_WITHIN_WSB_GROUP);
            int detectionTimeInterval = CommandLine.intValue(IOCTL_DETECTION_TIME_INTERVAL);
            String detectionTimeSchedule = CommandLine.value(IOCTL_DETECTION_TIME_SCHEDULE);
            String channelName = CommandLine.value(IOCTL_CHANNEL_NAME);
            String wsbChannelName = CommandLine.value(IOCTL_WSB_GROUP);

            if (isIoctlArgsIncorrect(ioctlInterval)) {
                finishWithError("\tioctlInterval should have a positive value if any ioctl parameters are specified");
            }

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
                                                                .consumerName(DEFAULT_CONSUMER_NAME));
            // APIQA
            consumer.registerClient(EmaFactory.createReqMsg()
                                        .serviceName(CommandLine.value(SERVICE_NAME_1))
                                        .name(CommandLine.value(ITEM_NAME_1)), appClient1, 0);
            consumer.registerClient(EmaFactory.createReqMsg()
                                        .serviceName(CommandLine.value(SERVICE_NAME_2))
                                        .name(CommandLine.value(ITEM_NAME_2)), appClient2, 0);
            // APIQA END

            boolean isModifyIOCtlDone = false;
            int printInterval = 1;
            consumer.channelInformation(ci);
            System.out.println("\nInitial channel information (consumer):\n\t" + ci);
            System.out.println();

            long startTime = System.currentTimeMillis();
            for (int i = 0; i < 600; i++) {
                Thread.sleep(1000); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

                if (i % printInterval == 0 && (isModifyIOCtlDone || ioctlInterval <= 0)) {
                    consumer.channelInformation(ci);
                    if (ci.channelState() != ChannelInformation.ChannelState.INACTIVE) {
                        System.out.println("\nChannel information (consumer):\n\t" + ci + "\n");
                    }
                }

                long currentTime = System.currentTimeMillis();
                int period = (int) (currentTime - startTime) / 1000;
                if (ioctlInterval > 0 && period >= ioctlInterval && !isModifyIOCtlDone) {
                    boolean isChanged = isPHInfoChanged(ci.preferredHostInfo(), enablePreferredHostOptions,
                            fallBackWithInWSBGroup, channelName, wsbChannelName, detectionTimeInterval, detectionTimeSchedule);

                    if (isChanged) {
                        PreferredHostOptions preferredHostOptions = getPreferredHostOptions(ci.preferredHostInfo(),
                                enablePreferredHostOptions, fallBackWithInWSBGroup, detectionTimeInterval,
                                detectionTimeSchedule, channelName, wsbChannelName);
                        consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, preferredHostOptions);
                        System.out.println("ModifyIOCtl() is called!");
                    }

                    isModifyIOCtlDone = true;
                }

                if (fallbackInterval > 0 && period >= fallbackInterval) {
                    consumer.fallbackPreferredHost();
                    System.out.println( "FallbackPreferredHost() is called!");
                    startTime = System.currentTimeMillis();
                }

                if (i % printInterval == 0) {
                    if(ioctlInterval > 0 && !isModifyIOCtlDone) {
                        printModifyIOCtlData(ioctlInterval, period);
                    }

                    if(fallbackInterval > 0) {
                        printFallbackPreferredHostData(fallbackInterval, period);
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


