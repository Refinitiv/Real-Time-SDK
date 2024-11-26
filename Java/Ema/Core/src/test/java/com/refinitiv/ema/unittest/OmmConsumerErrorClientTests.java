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

public class OmmConsumerErrorClientTests {

    private static final String FIELD_ABSENT_DICTIONARY_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/absent_fid_RDMFieldDictionary";
    private static final String FIELD_DICTIONARY_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/RDMTestDictionary";
    private static final String ENUM_TABLE_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/testenumtype.def";
    private static final String ERROR_TEXT_FOR_JSON_CONVERTER = "Failed to convert JSON message: {\"ID\":3,\"Type\":\"Error\",\"Text\":\"JSON Converter error: encountered unexpected fid = BID while decoding FieldEntry\"";

    static AtomicReference<AssertionError> assertionError = new AtomicReference<>();
    static AtomicBoolean onJsonConverterErrorCalled = new AtomicBoolean(false);

    static class AppConsumerClient implements OmmConsumerClient
    {
        public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event) {}

        public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {}

        public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {}

        public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}

        public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}

        public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){
            System.out.println("Consumer: \n" + msg);
        }
    }

    static class AppErrorClient implements OmmConsumerErrorClient
    {
        public void onInvalidHandle(long handle, String text) {}

        public void onJsonConverterError(ConsumerSessionInfo consumerSessionInfo, int errorCode, String text) {
            System.out.println("onJsonConverterError callback function" + "\nError text: " + text +" , Error code: " + errorCode);
            onJsonConverterErrorCalled.set(true);
            try {
                TestUtilities.checkResult(text.contains(ERROR_TEXT_FOR_JSON_CONVERTER));
            } catch (AssertionError err) {
                assertionError.set(err);
            }
        }
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

        public void onAllMsg(Msg msg, OmmProviderEvent event){
            System.out.println("Provider: \n" + msg);
        }

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
            innerElementList.add( EmaFactory.createElementEntry().intValue( "SendJsonConvError", 1 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));

            innerMap.add( EmaFactory.createMapEntry().keyAscii( "Provider_A", MapEntry.MapAction.ADD, innerElementList));
            innerElementList.clear();

            elementList.add( EmaFactory.createElementEntry().map( "IProviderList", innerMap ));
            innerMap.clear();

            configMap.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList ));
            elementList.clear();

            innerElementList.add( EmaFactory.createElementEntry().ascii( "ServerType", "ServerType::RSSL_WEBSOCKET" ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::None" ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 30000 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1 ));
            innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxFragmentSize", 6144 ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", port));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "WsProtocols", "rssl.json.v2, rssl.rwf, tr_json2"));

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

    private static Map createProgrammaticConfig(String port) {
        Map innerMap = EmaFactory.createMap();
        Map configMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();

        elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_A" ));

        // ConsumerGroup
        // Consumer_A
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Channel", "Channel_A" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
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
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 3000 ));
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
        innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_WEBSOCKET" ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::None"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "InitializationTimeout", 30000));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", port));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 1));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "WsMaxMsgSize", 61440));
        innerElementList.add(EmaFactory.createElementEntry().ascii("WsProtocols", "rssl.json.v2, rssl.rwf, tr_json2"));

        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_A", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        // DictionaryGroup
        // Dictionary_1
        innerElementList.add(EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", FIELD_ABSENT_DICTIONARY_FILE_NAME));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", ENUM_TABLE_FILE_NAME));
        innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        return configMap;
    }

    @Test
    public void fidIsAbsentInDictionary_onJsonConverterErrorCallback() {
        TestUtilities.printTestHead("fidIsAbsentInDictionary_onJsonConverterErrorCallback", "Receiving Fid doesn't defined in Dictionary, error handling by onJsonConverterError callback");

        final String port = "14002";
        OmmConsumer consumer = null;
        OmmConsumerErrorClientTests.ProviderThread providerThread = new OmmConsumerErrorClientTests.ProviderThread(port);

        try {
            providerThread.start();
            Thread.sleep(1000);

            OmmConsumerErrorClientTests.AppConsumerClient consumerClient = new OmmConsumerErrorClientTests.AppConsumerClient();
            AppErrorClient appErrorClient = new AppErrorClient();

            Map progConfig = createProgrammaticConfig(port);
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config(progConfig),
                    appErrorClient);
            consumer.registerClient(EmaFactory.createReqMsg()
                    .serviceName("DIRECT_FEED")
                    .name("IBM.N"), consumerClient);

            Thread.sleep(6000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

        }
        catch (InterruptedException | OmmException ex) {
            System.out.println(ex.getMessage());
            TestUtilities.checkResult(false);
        }
        finally {
            providerThread.shutdown();
            if (consumer != null) {
                consumer.uninitialize();
            }
        }

        TestUtilities.checkResult("There is no call of onJsonConverterErrorCalled", onJsonConverterErrorCalled.get());
        checkForAssertionError("Error message text for json converter error doesn't match expected");

        System.out.println("End fidIsAbsentInDictionary_onJsonConverterErrorCallback");
        System.out.println();
    }

    @Test
    public void fidIsAbsentInDictionary_UserDispatch_ExceptionThrown() {
        TestUtilities.printTestHead("fidIsAbsentInDictionary_UserDispatch_ExceptionThrown", "Receiving Fid doesn't defined in Dictionary, there is no OmmConsumerErrorClient, USER_DISPATCH operation model, OmmException is thrown");

        final String port = "14003";
        boolean JsonConverterErrorThrown = false;
        OmmConsumer consumer = null;
        OmmConsumerErrorClientTests.ProviderThread providerThread = new OmmConsumerErrorClientTests.ProviderThread(port);

        try {
            providerThread.start();
            Thread.sleep(1000);

            OmmConsumerErrorClientTests.AppConsumerClient consumerClient = new OmmConsumerErrorClientTests.AppConsumerClient();

            Map progConfig = createProgrammaticConfig(port);
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
                    .config(progConfig)
                    .operationModel(OmmConsumerConfig.OperationModel.USER_DISPATCH));
            consumer.registerClient(EmaFactory.createReqMsg()
                    .serviceName("DIRECT_FEED")
                    .name("IBM.N"), consumerClient);

            long startTime = System.currentTimeMillis();
            while (startTime + 6000 > System.currentTimeMillis())
                consumer.dispatch(10);

        }
        catch (InterruptedException ex) {
            System.out.println(ex.getMessage());
        }
        catch (OmmException ex) {
            System.out.println(ex.getMessage());
            JsonConverterErrorThrown = true;
            TestUtilities.checkResult(ex.getMessage().contains(ERROR_TEXT_FOR_JSON_CONVERTER));
        }
        finally {
            providerThread.shutdown();
            if (consumer != null) {
                consumer.uninitialize();
            }
        }

        TestUtilities.checkResult("Json Converter Error isn't thrown", JsonConverterErrorThrown);

        System.out.println("End fidIsAbsentInDictionary_UserDispatch_ExceptionThrown");
        System.out.println();
    }

    @Test
    public void fidIsAbsentInDictionary_ApiDispatch_NoException() {
        TestUtilities.printTestHead("fidIsAbsentInDictionary_ApiDispatch_NoException", "Receiving Fid doesn't defined in Dictionary, there is no OmmConsumerErrorClient, API_DISPATCH operation model, OmmException isn't thrown");

        final String port = "14004";
        OmmConsumer consumer = null;
        OmmConsumerErrorClientTests.ProviderThread providerThread = new OmmConsumerErrorClientTests.ProviderThread(port);

        try {
            providerThread.start();
            Thread.sleep(1000);

            OmmConsumerErrorClientTests.AppConsumerClient consumerClient = new OmmConsumerErrorClientTests.AppConsumerClient();

            Map progConfig = createProgrammaticConfig(port);
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config(progConfig));
            consumer.registerClient(EmaFactory.createReqMsg()
                    .serviceName("DIRECT_FEED")
                    .name("IBM.N"), consumerClient);

            Thread.sleep(6000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

        }
        catch (InterruptedException ex) {
            System.out.println(ex.getMessage());
        }
        catch (OmmException ex) {
            System.out.println(ex.getMessage());
            TestUtilities.checkResult(false);
        }
        finally {
            providerThread.shutdown();
            if (consumer != null) {
                consumer.uninitialize();
            }
        }

        System.out.println("End fidIsAbsentInDictionary_ApiDispatch_NoException");
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
