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

public class OmmProviderErrorClientTests {

    private static final String FIELD_ABSENT_DICTIONARY_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/absent_fid_RDMFieldDictionary";
    private static final String FIELD_DICTIONARY_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/RDMTestDictionary";
    private static final String ENUM_TABLE_FILE_NAME = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/testenumtype.def";
    private static final String ERROR_TEXT_FOR_JSON_CONVERTER = "Failed to convert JSON message: {\"ID\":3,\"Type\":\"Error\",\"Text\":\"JSON Converter error: encountered unexpected fid = BID while decoding FieldEntry\"";

    static AtomicReference<AssertionError> assertionError = new AtomicReference<>();
    static AtomicBoolean onJsonConverterErrorCalled = new AtomicBoolean(false);

    static class AppConsumerClient implements OmmConsumerClient
    {
        private static int postId = 1;

        public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event) {
            if ( refreshMsg.domainType() == EmaRdm.MMT_MARKET_PRICE &&
                    refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
                    refreshMsg.state().dataState() == OmmState.DataState.OK )
            {
                PostMsg postMsg = EmaFactory.createPostMsg();
                RefreshMsg nestedRefreshMsg = EmaFactory.createRefreshMsg();
                FieldList nestedFieldList = EmaFactory.createFieldList();

                //FieldList is a collection in java
                nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
                nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
                nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));

                nestedRefreshMsg.payload(nestedFieldList ).complete(true);

                ((OmmConsumer)event.closure()).submit( postMsg
                            .postId( postId++ )
                            .serviceName( "DIRECT_FEED" )
                            .name( "IBM.N" )
                            .solicitAck(true)
                            .complete(true)
                            .payload(nestedRefreshMsg), event.handle() );
            }
        }

        public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {}

        public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {}

        public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}

        public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}

        public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){
            System.out.println("Consumer: \n" + msg);
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

        public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){
            if(postMsg.solicitAck()) {
                AckMsg ackMsg = EmaFactory.createAckMsg();
                if(postMsg.hasSeqNum()){
                    ackMsg.seqNum(postMsg.seqNum());
                }
                if(postMsg.hasName()){
                    ackMsg.name(postMsg.name());
                }
                if(postMsg.hasServiceId()){
                    ackMsg.serviceId(postMsg.serviceId());
                }
                ackMsg.ackId(postMsg.postId())
                        .domainType(postMsg.domainType());
                event.provider().submit(ackMsg, event.handle());
            }
        }

        public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}

        public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}

        public void onAllMsg(Msg msg, OmmProviderEvent event){
            System.out.println("Provider: \n" + msg);
        }

        void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
        {
            ElementList elementList = EmaFactory.createElementList();
            elementList.add(EmaFactory.createElementEntry().uintValue("SupportOMMPost", 1));
            event.provider().submit(EmaFactory.createRefreshMsg()
                                        .domainType(EmaRdm.MMT_LOGIN)
                                        .name(reqMsg.name())
                                        .nameType(EmaRdm.USER_NAME)
                                        .complete(true)
                                        .solicited(true)
                                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted")
                                        .attrib(elementList), event.handle());
        }

        void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event) {
            if( itemHandle != 0 )
            {
                processInvalidItemRequest(reqMsg, event);
                return;
            }

            FieldList fieldList = EmaFactory.createFieldList();
            fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

            event.provider().submit( EmaFactory.createRefreshMsg()
                            .name(reqMsg.name())
                            .serviceId(reqMsg.serviceId())
                            .solicited(true)
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed")
                            .payload(fieldList)
                            .complete(true), event.handle() );

            itemHandle = event.handle();
        }

        void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
        {
            event.provider().submit( EmaFactory.createStatusMsg()
                                .name(reqMsg.name())
                                .serviceName(reqMsg.serviceName())
                                .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
                    event.handle() );
        }
    }

    static class AppErrorClient implements OmmProviderErrorClient {
        @Override
        public void onInvalidHandle(long handle, String text) {}

        @Override
        public void onJsonConverterError(ProviderSessionInfo providerSessionInfo, int errorCode, String text) {
            System.out.printf("\nonJsonConverter callback function\n" +
                    "Error text: %s\n" +
                    "Error code: %d\n" +
                    "Closing the client channel\n", text, errorCode
            );
            providerSessionInfo.getProvider().closeChannel(providerSessionInfo.getClientHandle());
            onJsonConverterErrorCalled.set(true);
            try {
                TestUtilities.checkResult(text.contains(ERROR_TEXT_FOR_JSON_CONVERTER));
            } catch (AssertionError err) {
                assertionError.set(err);
            }
        }
    }

    static class ConsumerThread extends Thread {
        private OmmConsumer consumer = null;
        private final String port;

        public ConsumerThread(String port) {
            super();
            this.port = port;
        }

        Map createProgrammaticConfig() {
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
            innerElementList.add( EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary" ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", FIELD_DICTIONARY_FILE_NAME ));
            innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", ENUM_TABLE_FILE_NAME ));
            innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
            innerElementList.clear();

            elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
            innerMap.clear();

            configMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
            elementList.clear();

            return configMap;
        }

        public void run() {
            try {
                OmmProviderErrorClientTests.AppConsumerClient consumerClient = new OmmProviderErrorClientTests.AppConsumerClient();

                Map progConfig = createProgrammaticConfig();
                consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config(progConfig));
                consumer.registerClient(EmaFactory.createReqMsg()
                        .serviceName("DIRECT_FEED")
                        .name("IBM.N"), consumerClient, consumer);

                Thread.sleep(6000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
            }
            catch (Exception e) {
                // Do nothing
            } finally {
                // Shutdown consumer
                if (consumer != null) {
                    consumer.uninitialize();
                }
            }
        }

        public void shutdown() {
            interrupt();
        }
    }

    private static Map createProgrammaticConfig(String port)
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
        innerElementList.add(EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", FIELD_ABSENT_DICTIONARY_FILE_NAME));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", ENUM_TABLE_FILE_NAME));
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

    @Test
    public void fidIsAbsentInDictionary_onJsonConverterErrorCallback() {
        TestUtilities.printTestHead("fidIsAbsentInDictionary_onJsonConverterErrorCallback", "Receiving Fid doesn't defined in Dictionary, error handling by onJsonConverterError callback");

        final String port = "14005";
        OmmProvider provider = null;
        OmmProviderErrorClientTests.ConsumerThread consumerThread = new OmmProviderErrorClientTests.ConsumerThread(port);

        try {
            OmmProviderErrorClientTests.AppProviderClient providerClient = new OmmProviderErrorClientTests.AppProviderClient();
            OmmProviderErrorClientTests.AppErrorClient appErrorClient = new OmmProviderErrorClientTests.AppErrorClient();

            provider = EmaFactory.createOmmProvider( EmaFactory.createOmmIProviderConfig()
                    .config( createProgrammaticConfig(port)), providerClient, appErrorClient);

            consumerThread.start();

            while( providerClient.itemHandle == 0 ) {
                Thread.sleep(1000);
            }
        }
        catch (InterruptedException | OmmException ex) {
            System.out.println(ex.getMessage());
            TestUtilities.checkResult(false);
        }
        finally {
            consumerThread.shutdown();
            if (provider != null) {
                provider.uninitialize();
            }
        }

        TestUtilities.checkResult("There is no call of onJsonConverterErrorCalled", onJsonConverterErrorCalled.get());
        checkForAssertionError("Error message text for json converter error doesn't match expected");

        System.out.println("End fidIsAbsentInDictionary_onJsonConverterErrorCallback");
        System.out.println();
    }

    @Test
    public void fidIsAbsentInDictionary_UserDispatch_ExceptionThrown() {
        TestUtilities.printTestHead("fidIsAbsentInDictionary_UserDispatch_ExceptionThrown", "Receiving Fid doesn't defined in Dictionary, there is no OmmProviderErrorClient, USER_DISPATCH operation model, OmmException is thrown");

        final String port = "14006";
        boolean JsonConverterErrorThrown = false;
        OmmProvider provider = null;
        OmmProviderErrorClientTests.ConsumerThread consumerThread = new OmmProviderErrorClientTests.ConsumerThread(port);

        try {
            OmmProviderErrorClientTests.AppProviderClient providerClient = new OmmProviderErrorClientTests.AppProviderClient();

            provider = EmaFactory.createOmmProvider( EmaFactory.createOmmIProviderConfig()
                    .config(createProgrammaticConfig(port)).operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH), providerClient );

            consumerThread.start();

            while( providerClient.itemHandle == 0 ) {
                provider.dispatch(1000);
                Thread.sleep(1000);
            }

            provider.dispatch(3000);
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
            consumerThread.shutdown();
            if (provider != null) {
                provider.uninitialize();
            }
        }

        TestUtilities.checkResult("Json Converter Error isn't thrown", JsonConverterErrorThrown);

        System.out.println("End fidIsAbsentInDictionary_UserDispatch_ExceptionThrown");
        System.out.println();
    }

    @Test
    public void fidIsAbsentInDictionary_ApiDispatch_NoException() {
        TestUtilities.printTestHead("fidIsAbsentInDictionary_ApiDispatch_NoException", "Receiving Fid doesn't defined in Dictionary, there is no OmmProviderErrorClient, API_DISPATCH operation model, OmmException isn't thrown");

        final String port = "14007";
        OmmProvider provider = null;
        OmmProviderErrorClientTests.ConsumerThread consumerThread = new OmmProviderErrorClientTests.ConsumerThread(port);

        try {
            OmmProviderErrorClientTests.AppProviderClient providerClient = new OmmProviderErrorClientTests.AppProviderClient();

            provider = EmaFactory.createOmmProvider( EmaFactory.createOmmIProviderConfig()
                    .config( createProgrammaticConfig(port)), providerClient);

            consumerThread.start();

            while( providerClient.itemHandle == 0 ) {
                Thread.sleep(1000);
            }
        }
        catch (InterruptedException ex) {
            System.out.println(ex.getMessage());
        }
        catch (OmmException ex) {
            System.out.println(ex.getMessage());
            TestUtilities.checkResult(false);
        }
        finally {
            consumerThread.shutdown();
            if (provider != null) {
                provider.uninitialize();
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
