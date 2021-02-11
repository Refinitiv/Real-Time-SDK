package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.ViewTypes;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class RequestMsgTest {
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    Buffer jsonBuffer;
    Msg resultMsg;
    Buffer origMsgBuffer;
    EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
    DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    int streamId = 5;
    ConversionResults convRes;

    private final static String batchItemNames[] = {"SIX", "TWELVE", "FOURTEEN"};
    private final Buffer batchItemNamesBuffers[] = {CodecFactory.createBuffer(), CodecFactory.createBuffer(), CodecFactory.createBuffer()};
    private final static int viewArrayItems[] = {22, 25};
    private final Int viewArrayItemsInt[] = {CodecFactory.createInt(), CodecFactory.createInt()};
    private int providePermissionProfile = 1;
    private int providePermissionExpressions = 1;
    private int allowSuspectData = 1;
    private Buffer userName;
    private Buffer position;
    private Buffer password;
    private Buffer appId;
    private Buffer appName;
    private UInt uint;

    public class MessageParameters
    {
        public int id;
        public boolean extendedHeader;
        public boolean priority;
        public boolean streaming;
        public boolean keyInUpdates;
        public boolean confInfoInUpdates;
        public boolean noRefresh;
        public boolean qos;
        public boolean worstQos;
        public boolean privateStream;
        public boolean pause;
        public boolean batch;
        public boolean view;
        public boolean qualified;
        public boolean keyIdentifier;
        public int domain;
        public int streamId;
        public boolean hasFilter;
        public int filter;

        @Override
        public String toString() {
            return "MessageParameters{" +
                    "id=" + id +
                    ", extendedHeader=" + extendedHeader +
                    ", priority=" + priority +
                    ", streaming=" + streaming +
                    ", keyInUpdates=" + keyInUpdates +
                    ", confInfoInUpdates=" + confInfoInUpdates +
                    ", noRefresh=" + noRefresh +
                    ", qos=" + qos +
                    ", worstQos=" + worstQos +
                    ", privateStream=" + privateStream +
                    ", pause=" + pause +
                    ", batch=" + batch +
                    ", view=" + view +
                    ", qualified=" + qualified +
                    ", keyIdentifier=" + keyIdentifier +
                    '}';
        }
    }
    MessageParameters msgParameters = new MessageParameters();

    public RequestMsgTest(
            int id,
            boolean extendedHeader,
            boolean priority,
            boolean streaming,
            boolean keyInUpdates,
            boolean confInfoInUpdates,
            boolean noRefresh,
            boolean qos,
            boolean worstQos,
            boolean privateStream,
            boolean pause,
            boolean batch,
            boolean view,
            boolean qualified,
            boolean keyIdentifier,
            int domain,
            int streamId,
            boolean hasFilter,
            int filter
    ) {
        msgParameters.id                = id;
        msgParameters.extendedHeader    = extendedHeader;
        msgParameters.priority          = priority;
        msgParameters.streaming         = streaming;
        msgParameters.keyInUpdates      = keyInUpdates;
        msgParameters.confInfoInUpdates = confInfoInUpdates;
        msgParameters.noRefresh         = noRefresh;
        msgParameters.qos               = qos;
        msgParameters.worstQos          = worstQos;
        msgParameters.privateStream     = privateStream;
        msgParameters.pause             = pause;
        msgParameters.batch             = batch;
        msgParameters.view              = view;
        msgParameters.qualified         = qualified;
        msgParameters.keyIdentifier     = keyIdentifier;
        msgParameters.domain            = domain;
        msgParameters.streamId          = streamId;
        msgParameters.hasFilter         = hasFilter;
        msgParameters.filter            = filter;

        for (int i=0; i< batchItemNames.length; i++)
            batchItemNamesBuffers[i].data(batchItemNames[i]);

        for (int i=0; i< viewArrayItems.length; i++)
            viewArrayItemsInt[i].value(viewArrayItems[i]);
    }

    private Msg generateMsg() {
        RequestMsg requestMsg = (RequestMsg) CodecFactory.createMsg();
        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        requestMsg.encodedDataBody(emptyBuffer);

        requestMsg.msgClass(MsgClasses.REQUEST);

        requestMsg.streamId(msgParameters.streamId);
        requestMsg.domainType(msgParameters.domain);

        Qos qos = CodecFactory.createQos();
        qos.timeliness(QosTimeliness.REALTIME);
        qos.rate(QosRates.TICK_BY_TICK);

        Qos worstQos = CodecFactory.createQos();
        worstQos.timeliness(QosTimeliness.DELAYED_UNKNOWN);
        worstQos.rate(QosRates.JIT_CONFLATED);

        if (msgParameters.extendedHeader) {
            requestMsg.applyHasExtendedHdr();
            requestMsg.extendedHeader(EXTENDED_HEADER_BUFFER);
        }

        final int PRIORITY_CLASS = 2;
        final int PRIORITY_COUNT = 65535;
        if (msgParameters.priority) {
            requestMsg.applyHasPriority();
            requestMsg.priority().priorityClass(PRIORITY_CLASS);
            requestMsg.priority().count(PRIORITY_COUNT);
        }

        if (msgParameters.streaming)
            requestMsg.applyStreaming();

        if (msgParameters.keyInUpdates)
            requestMsg.applyMsgKeyInUpdates();

        if (msgParameters.confInfoInUpdates)
            requestMsg.applyConfInfoInUpdates();

        if (msgParameters.noRefresh)
            requestMsg.applyNoRefresh();

        if (msgParameters.qos) {
            requestMsg.applyHasQos();
            qos.copy(requestMsg.qos());
        }

        if (msgParameters.worstQos) {
            requestMsg.applyHasWorstQos();
            worstQos.copy(requestMsg.worstQos());
        }

        if (msgParameters.privateStream)
            requestMsg.applyPrivateStream();

        if (msgParameters.pause)
            requestMsg.applyPause();

        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().serviceId(MSGKEY_SVC_ID);

        encodeMsgKeyAttrib(msgParameters.domain, requestMsg.msgKey());

        if (msgParameters.hasFilter) {
            requestMsg.msgKey().applyHasFilter();
            requestMsg.msgKey().filter(msgParameters.filter);
        }

        if (msgParameters.keyIdentifier) {
            requestMsg.msgKey().applyHasIdentifier();
            requestMsg.msgKey().identifier(MSGKEY_IDENTIFIER);
        }

        if (msgParameters.batch)
            requestMsg.applyHasBatch();
        else {
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name(MSG_KEY_NAME_BUFFER);
        }

        if (msgParameters.view)
            requestMsg.applyHasView();

        if (msgParameters.qualified)
            requestMsg.applyQualifiedStream();

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        if (msgParameters.view || msgParameters.batch) {
            requestMsg.containerType(DataTypes.ELEMENT_LIST);
            encodeBatchView(requestMsg);
        } else {
            requestMsg.containerType(DataTypes.NO_DATA);
        }
        assertEquals(SUCCESS, requestMsg.encode(iter));

        origMsgBuffer.data().flip();
        requestMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return requestMsg;
    }

    private void encodeMsgKeyAttrib(int domainType, MsgKey msgKey) {
        switch (domainType) {
            case DomainTypes.LOGIN:
                encodeLoginRequestMsgKeyAttrib(msgKey);
                break;
        }
    }

    private void encodeLoginRequestMsgKeyAttrib(MsgKey msgKey) {

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();
        EncodeIterator attribIter = CodecFactory.createEncodeIterator();
        Buffer tmp = CodecFactory.createBuffer();
        tmp.data(ByteBuffer.allocate(200));
        attribIter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        elementList.applyHasStandardData();
        if (elementList.encodeInit(attribIter, null, 0) != CodecReturnCodes.SUCCESS)
            assert(false);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        if (element.encode(attribIter, appId) != CodecReturnCodes.SUCCESS)
            assert(false);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        if (element.encode(attribIter, appName) != CodecReturnCodes.SUCCESS)
            assert(false);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        if (element.encode(attribIter, position) != CodecReturnCodes.SUCCESS)
            assert(false);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.PASSWORD);
        if (element.encode(attribIter, password) != CodecReturnCodes.SUCCESS)
            assert(false);

        uint.value(providePermissionProfile);
        element.dataType(DataTypes.UINT);
        element.name(ElementNames.PROV_PERM_PROF);
        element.encode(attribIter, uint);

        uint.value(providePermissionExpressions);
        element.dataType(DataTypes.UINT);
        element.name(ElementNames.PROV_PERM_EXP);
        element.encode(attribIter, uint);

        element.dataType(DataTypes.UINT);
        element.name(ElementNames.ALLOW_SUSPECT_DATA);
        uint.value(allowSuspectData);
        element.encode(attribIter, uint);

        if (elementList.encodeComplete(attribIter, true) != CodecReturnCodes.SUCCESS)
            assert(false);

        msgKey.applyHasAttrib();
        msgKey.attribContainerType(DataTypes.ELEMENT_LIST);
        msgKey.encodedAttrib(tmp);
    }

    private void checkMsgKeyAttrib(int domainType, MsgKey msgKey) {
        switch (domainType) {
            case DomainTypes.LOGIN:
                checkLoginRequestMsgKeyAttrib(msgKey);
                break;
        }
    }

    private void checkLoginRequestMsgKeyAttrib(MsgKey msgKey) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(msgKey.encodedAttrib(), Codec.majorVersion(), Codec.minorVersion());
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Buffer tmp = CodecFactory.createBuffer();
        UInt uint = CodecFactory.createUInt();
        elementList.decode(decIter, null);
        int entriesCount = 0;
        while (elementEntry.decode(decIter) != END_OF_CONTAINER) {
            if (elementEntry.name().equals(ElementNames.APPNAME)) {
                assertTrue(elementEntry.dataType() == DataTypes.ASCII_STRING);
                tmp.decode(decIter);
                assertTrue(tmp.equals(appName));
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.APPID)) {
                assertTrue(elementEntry.dataType() == DataTypes.ASCII_STRING);
                tmp.decode(decIter);
                assertTrue(tmp.equals(appId));
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.POSITION)) {
                assertTrue(elementEntry.dataType() == DataTypes.ASCII_STRING);
                tmp.decode(decIter);
                assertTrue(tmp.equals(position));
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.PASSWORD)) {
                assertTrue(elementEntry.dataType() == DataTypes.ASCII_STRING);
                tmp.decode(decIter);
                assertTrue(tmp.equals(password));
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.PROV_PERM_PROF)) {
                assertTrue(elementEntry.dataType() == DataTypes.UINT);
                uint.decode(decIter);
                assertTrue(uint.toLong() == providePermissionProfile);
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.PROV_PERM_EXP)) {
                assertTrue(elementEntry.dataType() == DataTypes.UINT);
                uint.decode(decIter);
                assertTrue(uint.toLong() == providePermissionExpressions);
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.ALLOW_SUSPECT_DATA)) {
                assertTrue(elementEntry.dataType() == DataTypes.UINT);
                uint.decode(decIter);
                assertTrue(uint.toLong() == allowSuspectData);
                entriesCount++;
            }
        }

        assertEquals(7, entriesCount);
    }

    private void encodeBatchView(RequestMsg requestMsg) {
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer dataBuffer = CodecFactory.createBuffer();
        dataBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        iter.setBufferAndRWFVersion(dataBuffer, Codec.majorVersion(), Codec.minorVersion());

        ElementList elementList = CodecFactory.createElementList();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
        assertEquals(SUCCESS, elementList.encodeInit(iter, null, 0));

        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Buffer nameBuffer = CodecFactory.createBuffer();

        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        //batch entry
        if (msgParameters.batch) {
            nameBuffer.data(BATCH_REQUEST_NAME_STR);
            elementEntry.name(nameBuffer);
            elementEntry.dataType(DataTypes.ARRAY);
            assertEquals(SUCCESS, elementEntry.encodeInit(iter, 0));

//            array.itemLength(0);//array elements length is dynamic
            array.primitiveType(DataTypes.ASCII_STRING);
            assertEquals(SUCCESS, array.encodeInit(iter));

            assertEquals(SUCCESS, arrayEntry.encode(iter, batchItemNamesBuffers[0]));
            assertEquals(SUCCESS, arrayEntry.encode(iter, batchItemNamesBuffers[1]));
            assertEquals(SUCCESS, arrayEntry.encode(iter, batchItemNamesBuffers[2]));

            assertEquals(SUCCESS, array.encodeComplete(iter, true));

            assertEquals(SUCCESS, elementEntry.encodeComplete(iter, true));
        }

        //view entry
        if (msgParameters.view) {
            nameBuffer.data(VIEW_TYPE_STR);
            elementEntry.name(nameBuffer);
            elementEntry.dataType(DataTypes.UINT);
            UInt viewType = CodecFactory.createUInt();
            viewType.value(ViewTypes.FIELD_ID_LIST);
            assertEquals(SUCCESS, elementEntry.encode(iter, viewType));

            nameBuffer.data(VIEW_NAME_STR);
            elementEntry.name(nameBuffer);
            elementEntry.dataType(DataTypes.ARRAY);
            assertEquals(SUCCESS, elementEntry.encodeInit(iter, 0));

//            array.clear();
//            array.itemLength(0);//array elements length is dynamic
            array.primitiveType(DataTypes.INT);
            assertEquals(SUCCESS, array.encodeInit(iter));

            assertEquals(SUCCESS, arrayEntry.encode(iter, viewArrayItemsInt[0]));
            assertEquals(SUCCESS, arrayEntry.encode(iter, viewArrayItemsInt[1]));

            assertEquals(SUCCESS, array.encodeComplete(iter, true));

            assertEquals(SUCCESS, elementEntry.encodeComplete(iter, true));
        }

        assertEquals(SUCCESS, elementList.encodeComplete(iter, true));

        requestMsg.encodedDataBody(dataBuffer);
    }

    @Before
    public void init()
    {
        jsonMsg.clear();
        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        resultMsg = CodecFactory.createMsg();
        origMsgBuffer = CodecFactory.createBuffer();
        origMsgBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        decodeIter.clear();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[] {JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(msgParameters.streamId);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        convRes = ConverterFactory.createConversionResults();

        userName = CodecFactory.createBuffer();
        position = CodecFactory.createBuffer();
        password = CodecFactory.createBuffer();
        appId = CodecFactory.createBuffer();
        appName = CodecFactory.createBuffer();
        uint = CodecFactory.createUInt();
        userName.data("UserName");
        position.data("position");
        password.data("password");
        appId.data("AppId");
        appName.data("AppName");
    }

    @Test
    public void testRwfToJson_Sunny() {
        Msg origMsg = generateMsg();
        assertEquals(SUCCESS, converter.convertRWFToJson(origMsg, rwfToJsonOptions, convRes, convError));
        assertEquals(SUCCESS, converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError));

        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));

        assertEquals("DecodeIter setup", SUCCESS, decodeIter.setBufferAndRWFVersion(jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion()));
        assertEquals("DecodeMsg", SUCCESS, resultMsg.decode(decodeIter));

        assertEquals("check class", MsgClasses.REQUEST, resultMsg.msgClass());

        checkRequestMsgsAreEqual((RequestMsg) origMsg, (RequestMsg)resultMsg);
    }

    private void checkRequestMsgsAreEqual(RequestMsg expectedMsg, RequestMsg actualMsg) {
        checkMsgsAreEqual(expectedMsg, actualMsg);
        checkMsgKeyAttrib(actualMsg.domainType(), actualMsg.msgKey());

        assertEquals(expectedMsg.checkConfInfoInUpdates(), actualMsg.checkConfInfoInUpdates());
        assertEquals(expectedMsg.checkHasExtendedHdr(), actualMsg.checkHasExtendedHdr());

        if (expectedMsg.checkHasExtendedHdr()) //redundant?
            assertTrue(expectedMsg.extendedHeader().equals(actualMsg.extendedHeader()));
        assertEquals(expectedMsg.checkMsgKeyInUpdates(), actualMsg.checkMsgKeyInUpdates());

        assertEquals(expectedMsg.checkMsgKeyInUpdates(), actualMsg.checkMsgKeyInUpdates());
        assertEquals(expectedMsg.checkHasView(), actualMsg.checkHasView());
        assertEquals(expectedMsg.checkStreaming(), actualMsg.checkStreaming());
        assertEquals(expectedMsg.checkNoRefresh(), actualMsg.checkNoRefresh());
        assertEquals(expectedMsg.checkPrivateStream(), actualMsg.checkPrivateStream());
        assertEquals(expectedMsg.checkPause(), actualMsg.checkPause());
        assertEquals(expectedMsg.checkHasPriority(), actualMsg.checkHasPriority());
        if (expectedMsg.checkHasPriority()) {
            assertEquals(expectedMsg.priority().priorityClass(), actualMsg.priority().priorityClass());
            assertEquals(expectedMsg.priority().count(), actualMsg.priority().count());
        }
        assertEquals(expectedMsg.checkHasQos(), actualMsg.checkHasQos());
        if (expectedMsg.checkHasQos())
            checkQoSsAreEqual(expectedMsg.qos(), actualMsg.qos());

        assertEquals(expectedMsg.checkHasWorstQos(), actualMsg.checkHasWorstQos());
        if (expectedMsg.checkHasWorstQos())
            checkQoSsAreEqual(expectedMsg.worstQos(), actualMsg.worstQos());

        assertEquals(expectedMsg.checkQualifiedStream(), actualMsg.checkQualifiedStream());

        assertEquals(expectedMsg.checkHasBatch(), actualMsg.checkHasBatch());
    }


    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][] {
                /* Test with/without ExtendedHeader, Priority, Streaming, KeyInUpdates, ConfInfoInUpdates, NoRefresh, Qos, WorstQos,
                 * PrivateStream, Pause, Batch, View, Qualified, KeyIdentifier, domain, streamId, hasFilter, filter; */

                /* Defaults */
                { 0, false, false, true, true, false, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* ExtendedHeader */
                { 1, true, false, true, true, false, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* Priority */
                { 2, false, true, true, true, false, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* Streaming {false}, */
                { 3, false, false, false, true, false, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* KeyInUpdates {false}, */
                { 4, false, false, true, false, false, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* ConfInfoInUpdates */
                { 5, false, false, true, true, true, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* NoRefresh */
                { 6, false, false, true, true, false, true, false, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* Qos */
                { 7, false, false, true, true, false, false, true, false, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* WorstQoS */
                { 8, false, false, true, true, false, false, false, true, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 5, false, 0},

                /* Qos and WorstQos */
                { 9, false, false, true, true, false, false, true, true, false, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* PrivateStream */
                {10, false, false, true, true, false, false, false, false, true, false, false, false, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* Pause */
                {11, false, false, true, true, false, false, false, false, false, true, false, false, false, false, DomainTypes.MARKET_PRICE, 3, false, 0},

                /* Batch */
                {12, false, false, true, true, false, false, false, false, false, false, true, false, false, false, DomainTypes.MARKET_BY_PRICE, 1, false, 0},

                /* View */
                {13, false, false, true, true, false, false, false, false, false, false, false, true, false, false, DomainTypes.MARKET_PRICE, 1, false, 0},

                /* Batch and View */
                {14, false, false, true, true, false, false, false, false, false, false, true, true, false, false, DomainTypes.MARKET_BY_TIME, 1, false, 0},

                /* Qualified */
                {15, false, false, true, true, false, false, false, false, false, false, false, false, true, false, DomainTypes.MARKET_BY_ORDER, 1, true, 5},

                /* KeyIdentifier */
                {16, false, false, true, true, false, false, false, false, false, false, false, false, false, true, DomainTypes.MARKET_BY_TIME, 2, true, 7},

                /* Login Request */
                {17, false, false, true, false, false, true, false, false, false, true, false, false, false, false, DomainTypes.LOGIN, 1, false, 0},

                /* Dictionary Request */
                {18, false, false, false, false, false, true, false, false, false, false, false, false, false, false, DomainTypes.DICTIONARY, 2, true, 3},

                /* Directory Request */
                {19, false, false, false, false, false, true, false, false, false, false, false, false, false, false, DomainTypes.SOURCE, 2, true, 3}
        });
    }
}
