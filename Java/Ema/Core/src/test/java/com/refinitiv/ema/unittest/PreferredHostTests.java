///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

import org.junit.Test;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

public class PreferredHostTests {

    private static final String OLD_DETECTION_TIME_SCHEDULE = "";
    private static final String NEW_DETECTION_TIME_SCHEDULE = "0 * * ? * *";
    private static final int OLD_DETECTION_TIME_INTERVAL = 15;
    private static final int NEW_DETECTION_TIME_INTERVAL = 5;
    private static final String OLD_CHANNEL_NAME = "Channel_B";
    private static final String NEW_CHANNEL_NAME = "Channel_A";
    private static final String FIELD_DICTIONARY_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/RDMTestDictionary";
    private static final String ENUM_TABLE_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/testenumtype.def";

    static AtomicReference<AssertionError> assertionError = new AtomicReference<>();
    static AtomicBoolean isNewPHOptionsApplied = new AtomicBoolean(false);
    static AtomicBoolean isFallbackPreferredHostCalled = new AtomicBoolean(false);
    static AtomicBoolean isRefreshMessageReceived = new AtomicBoolean(false);

    static class AppConsumerClient implements OmmConsumerClient
    {
        public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event) {
            System.out.println("Refresh message is received");
            // System.out.println("Event channel info (refresh)\n" + event.channelInformation());
            try {
                if(!isNewPHOptionsApplied.get()) {
                    PreferredHostInfo preferredHostInfo = event.channelInformation().preferredHostInfo();
                    TestUtilities.checkResult(preferredHostInfo.isPreferredHostEnabled());
                    TestUtilities.checkResult(preferredHostInfo.getWsbChannelName().isEmpty());
                    TestUtilities.checkResult(OLD_CHANNEL_NAME, preferredHostInfo.getChannelName());
                    TestUtilities.checkResult(OLD_DETECTION_TIME_INTERVAL, preferredHostInfo.getDetectionTimeInterval());
                    TestUtilities.checkResult(OLD_DETECTION_TIME_SCHEDULE, preferredHostInfo.getDetectionTimeSchedule());
                }

                if (isFallbackPreferredHostCalled.get()) {
                    isRefreshMessageReceived.set(true);
                    TestUtilities.checkResult(event.channelInformation().port() == 14003);
                }
            } catch (AssertionError err) {
                assertionError.set(err);
            }
        }

        public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {
            System.out.println("Update message is received");
            // System.out.println("Event channel info (update)\n" + event.channelInformation());
            try {
                if (isNewPHOptionsApplied.get()) {
                    PreferredHostInfo preferredHostInfo = event.channelInformation().preferredHostInfo();
                    TestUtilities.checkResult(preferredHostInfo.isPreferredHostEnabled());
                    TestUtilities.checkResult(preferredHostInfo.getWsbChannelName().isEmpty());
                    TestUtilities.checkResult(NEW_DETECTION_TIME_INTERVAL, preferredHostInfo.getDetectionTimeInterval());
                    TestUtilities.checkResult(NEW_DETECTION_TIME_SCHEDULE, preferredHostInfo.getDetectionTimeSchedule());
                }
            } catch (AssertionError err) {
                assertionError.set(err);
            }
        }

        public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {
            System.out.println("Status message is received");
            // System.out.println("Event channel info (status)\n" + event.channelInformation());
        }

        public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
        public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
        public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
    }

    static class AppProviderClient implements OmmProviderClient
    {
        public long itemHandle = 0;

        public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event) {
            switch (reqMsg.domainType())
            {
                case EmaRdm.MMT_LOGIN :
                    processLoginRequest(reqMsg, event);
                    break;
                case EmaRdm.MMT_MARKET_PRICE :
                    processMarketPriceRequest(reqMsg, event);
                    break;
                default :
                    processInvalidItemRequest(reqMsg, event);
                    break;
            }
        }

        public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event){}
        public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
        public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
        public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
        public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
        public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
        public void onAllMsg(Msg msg, OmmProviderEvent event){}

        void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event) {
            event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
                            nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
                            state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
                    event.handle() );
        }

        void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event) {
            if( itemHandle != 0 )
            {
                processInvalidItemRequest(reqMsg, event);
                return;
            }

            FieldList fieldList = EmaFactory.createFieldList();
            fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
            fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

            event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
                            state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
                            payload(fieldList).complete(true),
                    event.handle() );

            itemHandle = event.handle();
        }

        void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
        {
            event.provider().submit( EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
                            state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
                    event.handle() );
        }
    }

    static class ProviderThread extends Thread {

        private OmmProvider provider = null;
        private final String port;

        public ProviderThread(String port) {
            super();
            this.port = port;
        }

        Map createProgrammaticConfig()
        {
            Map innerMap = EmaFactory.createMap();
            Map configMap = EmaFactory.createMap();
            ElementList elementList = EmaFactory.createElementList();
            ElementList innerElementList = EmaFactory.createElementList();

            elementList.add( EmaFactory.createElementEntry().ascii( "DefaultIProvider", "Provider_A" ));

            innerElementList.add( EmaFactory.createElementEntry().ascii( "Server", "Server_A" ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "Directory", "Directory_2" ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "ItemCountHint", 10000 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceCountHint", 10000 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "DispatchTimeoutUserThread", 500 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "RefreshFirstRequired", 1 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));

            innerMap.add( EmaFactory.createMapEntry().keyAscii( "Provider_A", MapEntry.MapAction.ADD, innerElementList));
            innerElementList.clear();

            elementList.add( EmaFactory.createElementEntry().map( "IProviderList", innerMap ));
            innerMap.clear();

            configMap.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList ));
            elementList.clear();

            innerElementList.add( EmaFactory.createElementEntry().ascii( "ServerType", "ServerType::RSSL_SOCKET" ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib" ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 30000 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1 ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", port));

            innerMap.add( EmaFactory.createMapEntry().keyAscii( "Server_A", MapEntry.MapAction.ADD, innerElementList));
            innerElementList.clear();

            elementList.add( EmaFactory.createElementEntry().map( "ServerList", innerMap ));
            innerMap.clear();

            configMap.add( EmaFactory.createMapEntry().keyAscii("ServerGroup", MapEntry.MapAction.ADD, elementList ));
            elementList.clear();

            innerElementList.add( EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary" ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", FIELD_DICTIONARY_FILE_NAME ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", ENUM_TABLE_FILE_NAME ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryItemName", "RWFFld" ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefItemName", "RWFEnum" ));
            innerMap.add( EmaFactory.createMapEntry().keyAscii( "Dictionary_3", MapEntry.MapAction.ADD, innerElementList ));
            innerElementList.clear();

            elementList.add( EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
            innerMap.clear();

            configMap.add( EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
            elementList.clear();

            Map serviceMap = EmaFactory.createMap();

            innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceId", 1 ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "Vendor", "company name" ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "IsSource", 0 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "AcceptingConsumerStatus", 0 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "SupportsQoSRange", 0 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue(  "SupportsOutOfBandSnapshots", 0 ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "ItemList", "#.itemlist" ));

            OmmArray array = EmaFactory.createOmmArray();
            array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_DICTIONARY" ));
            array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_MARKET_PRICE" ));
            array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_MARKET_BY_PRICE" ));
            array.add( EmaFactory.createOmmArrayEntry().ascii( "200" ));
            innerElementList.add( EmaFactory.createElementEntry().array( "Capabilities", array ));
            array.clear();

            array.add( EmaFactory.createOmmArrayEntry().ascii( "Dictionary_3" ));
            innerElementList.add( EmaFactory.createElementEntry().array( "DictionariesProvided", array ));
            array.clear();

            array.add( EmaFactory.createOmmArrayEntry().ascii( "Dictionary_3" ));
            innerElementList.add( EmaFactory.createElementEntry().array( "DictionariesUsed", array ));
            array.clear();

            ElementList inner2 = EmaFactory.createElementList();

            Series series = EmaFactory.createSeries();
            inner2.add( EmaFactory.createElementEntry().ascii( "Timeliness", "Timeliness::RealTime" ));
            inner2.add( EmaFactory.createElementEntry().ascii( "Rate", "Rate::TickByTick" ));
            series.add( EmaFactory.createSeriesEntry().elementList( inner2 ));
            inner2.clear();

            inner2.add( EmaFactory.createElementEntry().intValue( "Timeliness", 100 ));
            inner2.add( EmaFactory.createElementEntry().intValue( "Rate", 100 ));
            series.add( EmaFactory.createSeriesEntry().elementList( inner2 ));
            inner2.clear();

            innerElementList.add( EmaFactory.createElementEntry().series( "QoS", series ));

            elementList.add( EmaFactory.createElementEntry().elementList( "InfoFilter", innerElementList ));
            innerElementList.clear();

            innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceState", 1 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "AcceptingRequests", 1 ));
            elementList.add( EmaFactory.createElementEntry().elementList( "StateFilter", innerElementList ));
            innerElementList.clear();

            innerElementList.add(EmaFactory.createElementEntry().intValue("OpenLimit", 5));
            innerElementList.add(EmaFactory.createElementEntry().intValue("OpenWindow", 5));
            innerElementList.add(EmaFactory.createElementEntry().intValue("LoadFactor", 1));
            elementList.add(EmaFactory.createElementEntry().elementList("LoadFilter", innerElementList));
            innerElementList.clear();

            serviceMap.add( EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED", MapEntry.MapAction.ADD, elementList ));
            elementList.clear();
            innerMap.add( EmaFactory.createMapEntry().keyAscii( "Directory_2", MapEntry.MapAction.ADD, serviceMap ));

            elementList.add( EmaFactory.createElementEntry().ascii( "DefaultDirectory", "Directory_2" ));
            elementList.add( EmaFactory.createElementEntry().map( "DirectoryList", innerMap ));
            innerMap.clear();

            configMap.add( EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList ));

            return configMap;
        }

        public void run() {
            try {
                AppProviderClient providerClient = new AppProviderClient();
                FieldList fieldList = EmaFactory.createFieldList();

                provider = EmaFactory.createOmmProvider( EmaFactory.createOmmIProviderConfig()
                                                                .config( createProgrammaticConfig()), providerClient);

                while( providerClient.itemHandle == 0 ) {
                    Thread.sleep(1000);
                }

                for( int i = 0; i < 60; i++ )
                {
                    fieldList.clear();
                    fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                    fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));

                    provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient.itemHandle );

                    Thread.sleep(1000);
                }
            }
            catch (Exception e) {
                // Do nothing
            } finally {
                // Shutdown provider
                if (provider != null) {
                    provider.uninitialize();
                }
            }
        }

        public void shutdown() {
            interrupt();
        }
    }

    private static PreferredHostOptions getPreferredHostOptions(boolean enablePreferredHostOptions, boolean fallBackWithInWSBGroup,
                                                             int detectionTimeInterval, String detectionTimeSchedule,
                                                             String channelName, String wsbChannelName) {

        PreferredHostOptions preferredHostOptions = EmaFactory.createPreferredHostOptions();
        preferredHostOptions.setPreferredHostEnabled(enablePreferredHostOptions);
        preferredHostOptions.setChannelName(channelName);
        preferredHostOptions.setDetectionTimeInterval(detectionTimeInterval);
        preferredHostOptions.setDetectionTimeSchedule(detectionTimeSchedule);
        preferredHostOptions.setWsbChannelName(wsbChannelName);
        preferredHostOptions.setFallBackWithInWSBGroup(fallBackWithInWSBGroup);
        return preferredHostOptions;
    }

    private static Map createProgrammaticConfig(boolean enablePreferredHostOptions, int detectionTimeInterval,
                                                String detectionTimeSchedule, String channelName) {
        Map innerMap = EmaFactory.createMap();
        Map configMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_A" ));

        // ConsumerGroup
        // Consumer_A
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", "Channel_B, Channel_A" ));
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

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_A", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        // ChannelGroup
        // Channel_A
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "InitializationTimeout", 30000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_A", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        // Channel_B
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "InitializationTimeout", 30000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14003"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_B", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        // DictionaryGroup
        // Dictionary_1
        innerElementList.add(EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::ChannelDictionary"));
        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        return configMap;
    }

    @Test
    public void callModifyIOCtl_ChangePreferredHostSettings() {
        TestUtilities.printTestHead("callModifyIOCtl_ChangePreferredHostSettings", "EMA call ModifyIOCtl, change preferred host settings");

        OmmConsumer consumer = null;
        PreferredHostTests.ProviderThread providerThread1 = new PreferredHostTests.ProviderThread("14002");
        PreferredHostTests.ProviderThread providerThread2 = new PreferredHostTests.ProviderThread("14003");

        try {
            providerThread1.start();
            providerThread2.start();
            Thread.sleep(1000);

            PreferredHostTests.AppConsumerClient consumerClient = new PreferredHostTests.AppConsumerClient();

            boolean enablePreferredHostOptions = true;
            int detectionTimeInterval = OLD_DETECTION_TIME_INTERVAL;
            String detectionTimeSchedule = OLD_DETECTION_TIME_SCHEDULE;
            String channelName = OLD_CHANNEL_NAME;

            Map progConfig = createProgrammaticConfig(enablePreferredHostOptions, detectionTimeInterval,
                                                        detectionTimeSchedule, channelName);
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config(progConfig));
            consumer.registerClient(EmaFactory.createReqMsg()
                    .serviceName("DIRECT_FEED")
                    .name("IBM.N"), consumerClient);

            Thread.sleep(2000);

            boolean fallBackWithInWSBGroup = true;
            detectionTimeInterval = NEW_DETECTION_TIME_INTERVAL;
            detectionTimeSchedule = NEW_DETECTION_TIME_SCHEDULE;
            channelName = NEW_CHANNEL_NAME;
            String wsbChannelName = "";

            PreferredHostOptions preferredHostOptions = getPreferredHostOptions(enablePreferredHostOptions,
                    fallBackWithInWSBGroup, detectionTimeInterval, detectionTimeSchedule, channelName,
                    wsbChannelName);
            consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, preferredHostOptions);
            System.out.println("ModifyIOCtl() is called!");
            isNewPHOptionsApplied.set(true);

            Thread.sleep(2000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

        }
        catch (InterruptedException | OmmException ex) {
            System.out.println(ex.getMessage());
            TestUtilities.checkResult(false);
        }
        finally {
            providerThread1.shutdown();
            providerThread2.shutdown();
            if (consumer != null) {
                consumer.uninitialize();
            }
        }

        checkForAssertionError("Preferred host attributes were incorrect");

        System.out.println("End EMA call ModifyIOCtl");
        System.out.println();
    }

    @Test
    public void callFallbackPreferredHost_SwitchToPreferredHost() {
        TestUtilities.printTestHead("callFallbackPreferredHost_SwitchToPreferredHost", "EMA call FallbackPreferredHost, switch to preferred host");

        OmmConsumer consumer = null;
        PreferredHostTests.ProviderThread providerThread1 = new PreferredHostTests.ProviderThread("14002");
        PreferredHostTests.ProviderThread providerThread2 = new PreferredHostTests.ProviderThread("14003");

        try {
            providerThread1.start();
            providerThread2.start();
            Thread.sleep(2000);

            PreferredHostTests.AppConsumerClient consumerClient = new PreferredHostTests.AppConsumerClient();

            isNewPHOptionsApplied.set(true);
            Map progConfig = createProgrammaticConfig(true, NEW_DETECTION_TIME_INTERVAL,
                                                            NEW_DETECTION_TIME_SCHEDULE, NEW_CHANNEL_NAME);
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config(progConfig));
            consumer.registerClient(EmaFactory.createReqMsg()
                    .serviceName("DIRECT_FEED")
                    .name("IBM.N"), consumerClient);

            Thread.sleep(1000);

            PreferredHostOptions preferredHostOptions = getPreferredHostOptions(true, true,
                    NEW_DETECTION_TIME_INTERVAL, NEW_DETECTION_TIME_SCHEDULE, OLD_CHANNEL_NAME, "");
            consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, preferredHostOptions);
            System.out.println("ModifyIOCtl() is called!");

            Thread.sleep(1000); // modifyIOCtl needs some time to process PH changes

            consumer.fallbackPreferredHost();
            System.out.println("FallbackPreferredHost() is called!");
            isFallbackPreferredHostCalled.set(true);

            Thread.sleep(5000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
        }
        catch (InterruptedException | OmmException ex) {
            System.out.println(ex.getMessage());
            TestUtilities.checkResult(false);
        }
        finally {
            providerThread1.shutdown();
            providerThread2.shutdown();
            if (consumer != null) {
                consumer.uninitialize();
            }
        }

        checkForAssertionError("Something was wrong during fallback to preferred host");
        TestUtilities.checkResult("There is no switch to preferred host after fallbackPreferredHost is called", isRefreshMessageReceived.get());

        System.out.println("End EMA call FallbackPreferredHost");
        System.out.println();
    }

    private void checkForAssertionError(String errorMessage) {
        AssertionError possibleError = assertionError.get();
        if(possibleError != null) {
            possibleError.printStackTrace();
        }
        try {
            TestUtilities.checkResult(errorMessage, possibleError == null);
        } finally {
            assertionError.set(null);
        }
    }
}
