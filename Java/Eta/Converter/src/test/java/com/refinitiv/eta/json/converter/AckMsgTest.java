package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class AckMsgTest {
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
    int streamId = MsgClasses.ACK;

    public class MsgParameters {
        private int id;
        private boolean extendedHeader;
        private boolean text;
        private boolean privateStream;
        private boolean seqNum;
        private boolean nakCode;
        private boolean qualifiedStream;
    }


    private MsgParameters msgParameters = new MsgParameters();

    public AckMsgTest(int id,
                      boolean extendedHeader,
                      boolean text,
                      boolean privateStream,
                      boolean seqNum,
                      boolean nakCode,
                      boolean qualifiedStream) {
        msgParameters.id = id;
        msgParameters.extendedHeader = extendedHeader;
        msgParameters.text = text;
        msgParameters.seqNum = seqNum;
        msgParameters.privateStream = privateStream;
        msgParameters.nakCode = nakCode;
        msgParameters.qualifiedStream = qualifiedStream;
    }

    private Msg generateMsg() {
        AckMsg ackMsg = (AckMsg) CodecFactory.createMsg();
        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        ackMsg.encodedDataBody(emptyBuffer);

        ackMsg.msgClass(MsgClasses.ACK);
        ackMsg.streamId(MsgClasses.ACK);
        ackMsg.domainType(DomainTypes.MARKET_PRICE);
        ackMsg.containerType(DataTypes.NO_DATA);

        ackMsg.applyHasMsgKey();
        ackMsg.msgKey().applyHasServiceId();
        ackMsg.msgKey().serviceId(JsonConverterTestUtils.MSGKEY_SVC_ID);

        ackMsg.msgKey().name(JsonConverterTestUtils.MSG_KEY_NAME_BUFFER);
        ackMsg.msgKey().applyHasName();

        if (msgParameters.extendedHeader) {
            ackMsg.applyHasExtendedHdr();
            ackMsg.extendedHeader(JsonConverterTestUtils.EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.text) {
            final Buffer textBuffer = CodecFactory.createBuffer();
            textBuffer.data("Received an on-stream post message on a stream that does not have an item open");
            ackMsg.applyHasText();
            ackMsg.text(textBuffer);
        }

        if (msgParameters.seqNum) {
            ackMsg.applyHasSeqNum();
            ackMsg.seqNum(101L);
        }

        if (msgParameters.privateStream) {
            ackMsg.applyPrivateStream();
        }

        if (msgParameters.nakCode) {
            ackMsg.applyHasNakCode();
            ackMsg.nakCode(NakCodes.INVALID_CONTENT);
        }

        if (msgParameters.qualifiedStream) {
            ackMsg.applyQualifiedStream();
        }

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        assertEquals(SUCCESS, ackMsg.encode(iter));

        origMsgBuffer.data().flip();
        ackMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return ackMsg;
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
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
    }

    private void checkForAckMsgEqualsTo(AckMsg expectedMsg, AckMsg equalsToMsg) {
        JsonConverterTestUtils.checkMsgsAreEqual(expectedMsg, equalsToMsg);

        assertEquals(expectedMsg.checkHasExtendedHdr(), equalsToMsg.checkHasExtendedHdr());

        if (expectedMsg.checkHasExtendedHdr()) {
            assertTrue(expectedMsg.extendedHeader().equals(equalsToMsg.extendedHeader()));
        }

        assertEquals(expectedMsg.checkHasText(), equalsToMsg.checkHasText());
        if (expectedMsg.checkHasText()) {
            assertEquals(expectedMsg.text(), equalsToMsg.text());
        }

        assertEquals(expectedMsg.checkPrivateStream(), equalsToMsg.checkPrivateStream());

        assertEquals(expectedMsg.checkHasSeqNum(), equalsToMsg.checkHasSeqNum());
        if (expectedMsg.checkHasSeqNum()) {
            assertEquals(expectedMsg.seqNum(), equalsToMsg.seqNum());
        }

        assertEquals(expectedMsg.checkHasNakCode(), equalsToMsg.checkHasNakCode());
        if (expectedMsg.checkHasNakCode()) {
            assertEquals(expectedMsg.nakCode(), equalsToMsg.nakCode());
        }

        assertEquals(expectedMsg.checkQualifiedStream(), equalsToMsg.checkQualifiedStream());
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

        assertEquals("check class", MsgClasses.ACK, resultMsg.msgClass());

        checkForAckMsgEqualsTo((AckMsg) origMsg, (AckMsg) resultMsg);
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][]{
                /* Test with/without ExtendedHeader, Text, SeqNum, PrivateStream, NAKCode, QualifiedStream*/

                /* Defaults */
                {0, false, true, false, false, true, false},

                /* ExtendedHeader */
                {1, true, true, false, false, true, false},
                /*No TEXT*/
                {2, false, false, false, false, true, false},
                /*Seq Num*/
                {3, false, true, true, false, true, false},
                /*Private Stream*/
                {4, false, true, false, true, true, false},
                /*No NAK CODE*/
                {5, false, true, false, false, false, false},
                /*Qualified stream*/
                {6, false, true, false, false, true, true},
                /* Everything */
                {7, true, true, true, true, true, true}
        });
    }
}
