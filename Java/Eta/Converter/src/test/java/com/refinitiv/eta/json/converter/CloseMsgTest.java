/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
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
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class CloseMsgTest {

    Buffer jsonBuffer;
    Msg resultMsg;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    Buffer origMsgBuffer;
    EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
    DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    int streamId = MsgClasses.CLOSE;

    public class MsgParameters {
        private int id;
        private boolean extendedHeader;
        private boolean ack;
        private boolean isBatchClose;
    }

    private MsgParameters msgParameters = new MsgParameters();

    public CloseMsgTest(int id,
                        boolean extendedHeader,
                        boolean ack,
                        boolean isBatchClose) {
        msgParameters.id = id;
        msgParameters.extendedHeader = extendedHeader;
        msgParameters.ack = ack;
        msgParameters.isBatchClose = isBatchClose;
    }

    private Msg generateMsg() {
        CloseMsg closeMsg = (CloseMsg) CodecFactory.createMsg();
        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        closeMsg.encodedDataBody(emptyBuffer);

        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(MsgClasses.CLOSE);
        closeMsg.domainType(DomainTypes.MARKET_PRICE);
        closeMsg.containerType(DataTypes.NO_DATA);

        if (msgParameters.extendedHeader) {
            closeMsg.applyHasExtendedHdr();
            closeMsg.extendedHeader(JsonConverterTestUtils.EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.ack) {
            closeMsg.applyAck();
        }

        if (msgParameters.isBatchClose)
            encodeBatchClose(closeMsg);

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        assertEquals(SUCCESS, closeMsg.encode(iter));

        origMsgBuffer.data().flip();
        closeMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return closeMsg;
    }

    private void encodeBatchClose(CloseMsg closeMsg) {
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer elemListBuf = CodecFactory.createBuffer();
        elemListBuf.data(ByteBuffer.allocate(100));
        iter.setBufferAndRWFVersion(elemListBuf, Codec.majorVersion(), Codec.minorVersion());

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        elementList.applyHasStandardData();
        elementList.encodeInit(iter, null, 0);
        elementEntry.dataType(DataTypes.ARRAY);
        elementEntry.name(ElementNames.BATCH_STREAMID_LIST);
        elementEntry.encodeInit(iter, 0);
        array.primitiveType(DataTypes.INT);
        array.encodeInit(iter);
        Int id = CodecFactory.createInt();
        id.value(1);
        arrayEntry.encode(iter, id);
        id.value(2);
        arrayEntry.encode(iter, id);
        id.value(3);
        arrayEntry.encode(iter, id);
        id.value(4);
        arrayEntry.encode(iter, id);
        array.encodeComplete(iter, true);
        elementEntry.encodeComplete(iter, true);
        elementList.encodeComplete(iter, true);
        closeMsg.containerType(DataTypes.ELEMENT_LIST);
        closeMsg.encodedDataBody(elemListBuf);
    }

    @Before
    public void init() {
        jsonMsg.clear();
        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE));
        resultMsg = CodecFactory.createMsg();
        origMsgBuffer = CodecFactory.createBuffer();
        origMsgBuffer.data(ByteBuffer.allocate(JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE));
        decodeIter.clear();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[]{JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(streamId);
        getJsonMsgOptions.isCloseMsg(true);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
    }

    private void checkForCloseMsgEqualsTo(CloseMsg expectedMsg, CloseMsg equalsToMsg) {

        assertEquals(expectedMsg.msgClass(), equalsToMsg.msgClass());
        assertEquals(expectedMsg.domainType(), equalsToMsg.domainType());
        if (!msgParameters.isBatchClose) {
            assertEquals(expectedMsg.streamId(), equalsToMsg.streamId());
        } else {
            assertEquals(expectedMsg.containerType(), equalsToMsg.containerType());
            ElementList elementList = CodecFactory.createElementList();
            ElementEntry elementEntry = CodecFactory.createElementEntry();
            Array array = CodecFactory.createArray();
            ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
            Int id = CodecFactory.createInt();
            DecodeIterator decIter = CodecFactory.createDecodeIterator();
            decIter.setBufferAndRWFVersion(equalsToMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
            elementList.decode(decIter, null);
            elementEntry.decode(decIter);
            assertTrue(elementEntry.name().equals(ElementNames.BATCH_STREAMID_LIST));
            assertTrue(elementEntry.dataType() == DataTypes.ARRAY);
            int i = 1;
            array.decode(decIter);
            while (arrayEntry.decode(decIter) != END_OF_CONTAINER) {
                id.decode(decIter);
                assertEquals(id.toLong(), i);
                i++;
            }
            assertEquals(5, i);
        }

        assertEquals(expectedMsg.checkHasExtendedHdr(), equalsToMsg.checkHasExtendedHdr());
        if (expectedMsg.checkHasExtendedHdr()) {
            assertTrue(expectedMsg.extendedHeader().equals(equalsToMsg.extendedHeader()));
        }

        assertEquals(expectedMsg.checkAck(), equalsToMsg.checkAck());
    }

    @Test
    public void testRwfToJson_Sunny() {
        Msg origMsg = generateMsg();
        ConversionResults intVal = ConverterFactory.createConversionResults();
        assertEquals(SUCCESS, converter.convertRWFToJson(origMsg, rwfToJsonOptions, intVal, convError));
        assertEquals(SUCCESS, converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError));

        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));

        assertEquals("DecodeIter setup", SUCCESS, decodeIter.setBufferAndRWFVersion(
                jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion()));
        assertEquals("DecodeMsg", SUCCESS, resultMsg.decode(decodeIter));

        assertEquals("check class", MsgClasses.CLOSE, resultMsg.msgClass());

        checkForCloseMsgEqualsTo((CloseMsg) origMsg, (CloseMsg) resultMsg);
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][]{
                /* Test with/without ExtendedHeader, Text, SeqNum, PrivateStream, NAKCode, QualifiedStream*/

                /* Defaults */
                {0, false, false, true},

                /* ExtendedHeader */
                {1, true, false, false},

                /* ACK */
                {2, false, true, true},

                /*Extended header && ACK*/
                {3, true, true, false}
        });
    }

}
