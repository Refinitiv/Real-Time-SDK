package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.codec.DataTypes.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class GenericMsgTest {

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
    int streamId = MsgClasses.GENERIC;

    public class MsgParameters {
        private int id;
        private boolean extendedHeader;
        private boolean permData;
        private boolean seqNum;
        private boolean partNum;
        private boolean messageComplete;
        private boolean secondarySecNum;
        private boolean providerDriven;
        private int containerType;
    }


    private MsgParameters msgParameters = new MsgParameters();

    public GenericMsgTest(int id,
                          boolean extendedHeader,
                          boolean permData,
                          boolean seqNum,
                          boolean partNum,
                          boolean messageComplete,
                          boolean secondarySecNum,
                          boolean providerDriven,
                          int containerType) {
        msgParameters.id = id;
        msgParameters.extendedHeader = extendedHeader;
        msgParameters.permData = permData;
        msgParameters.seqNum = seqNum;
        msgParameters.partNum = partNum;
        msgParameters.messageComplete = messageComplete;
        msgParameters.secondarySecNum = secondarySecNum;
        msgParameters.providerDriven = providerDriven;
        msgParameters.containerType = containerType;
    }

    private Msg generateMsg() {
        GenericMsg genericMsg = (GenericMsg) CodecFactory.createMsg();
        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        genericMsg.encodedDataBody(emptyBuffer);

        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.streamId(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.MARKET_PRICE);
        genericMsg.containerType(msgParameters.containerType);

        genericMsg.applyHasMsgKey();
        genericMsg.msgKey().applyHasServiceId();
        genericMsg.msgKey().serviceId(JsonConverterTestUtils.MSGKEY_SVC_ID);

        genericMsg.msgKey().name(JsonConverterTestUtils.MSG_KEY_NAME_BUFFER);
        genericMsg.msgKey().applyHasName();

        if (msgParameters.extendedHeader) {
            genericMsg.applyHasExtendedHdr();
            genericMsg.extendedHeader(JsonConverterTestUtils.EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.permData) {
            genericMsg.applyHasPermData();
            Buffer permissionData = CodecFactory.createBuffer();
            permissionData.data("GenericMsg.permissionData");
            genericMsg.permData(permissionData);
        }

        if (msgParameters.seqNum) {
            genericMsg.applyHasSeqNum();
            genericMsg.seqNum(101L);
        }

        if (msgParameters.partNum) {
            genericMsg.applyHasPartNum();
            genericMsg.partNum(30);
        }

        if (msgParameters.messageComplete) {
            genericMsg.applyMessageComplete();
        }

        if (msgParameters.secondarySecNum) {
            genericMsg.applyHasSecondarySeqNum();
        }

        if (msgParameters.providerDriven) {
            genericMsg.applyProviderDriven();
        }

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        switch (msgParameters.containerType) {
            case NO_DATA:
                assertEquals(SUCCESS, genericMsg.encode(iter));
                break;
            case FIELD_LIST:
                genericMsg.encodeInit(iter, 0);
                JsonConverterTestUtils.encodeSampleFieldList(genericMsg);
                genericMsg.encodeComplete(iter, true);
                break;
            case MAP:
                genericMsg.encodeInit(iter, 0);
                JsonConverterTestUtils.encodeMap(genericMsg);
                genericMsg.encodeComplete(iter, true);
                break;
        }

        origMsgBuffer.data().flip();
        genericMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return genericMsg;
    }

    @Before
    public void init() {
        jsonMsg.clear();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "src/test/resources/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE));
        resultMsg = CodecFactory.createMsg();
        origMsgBuffer = CodecFactory.createBuffer();
        origMsgBuffer.data(ByteBuffer.allocate(JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE));
        decodeIter.clear();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
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

    private void checkForGenericMsgEqualsTo(GenericMsg expectedMsg, GenericMsg equalsToMsg) {
        JsonConverterTestUtils.checkMsgsAreEqual(expectedMsg, equalsToMsg);

        assertEquals(expectedMsg.checkHasExtendedHdr(), equalsToMsg.checkHasExtendedHdr());

        if (expectedMsg.checkHasExtendedHdr()) {
            assertTrue(expectedMsg.extendedHeader().equals(equalsToMsg.extendedHeader()));
        }

        assertEquals(expectedMsg.checkHasPermData(), equalsToMsg.checkHasPermData());
        if (expectedMsg.checkHasPermData()) {
            assertEquals(expectedMsg.permData(), equalsToMsg.permData());
        }

        assertEquals(expectedMsg.checkHasSeqNum(), equalsToMsg.checkHasSeqNum());
        if (expectedMsg.checkHasSeqNum()) {
            assertEquals(expectedMsg.seqNum(), equalsToMsg.seqNum());
        }

        assertEquals(expectedMsg.checkHasPartNum(), equalsToMsg.checkHasPartNum());
        if (expectedMsg.checkHasPartNum()) {
            assertEquals(expectedMsg.partNum(), equalsToMsg.partNum());
        }

        assertEquals(expectedMsg.checkMessageComplete(), equalsToMsg.checkMessageComplete());

        assertEquals(expectedMsg.checkHasSecondarySeqNum(), equalsToMsg.checkHasSecondarySeqNum());
        if (expectedMsg.checkHasSecondarySeqNum()) {
            assertEquals(expectedMsg.secondarySeqNum(), equalsToMsg.secondarySeqNum());
        }

        assertEquals(expectedMsg.checkIsProviderDriven(), equalsToMsg.checkIsProviderDriven());
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

        assertEquals("check class", MsgClasses.GENERIC, resultMsg.msgClass());

        checkForGenericMsgEqualsTo((GenericMsg) origMsg, (GenericMsg) resultMsg);
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][]{
                /* Test with/without ExtendedHeader, PermData, SeqNum, PartNum, MessageComplete, SecondarySecNumber,
                * ProviderDriven*/

                /* Defaults */
                {0, false, true, false, false, false, false, false, NO_DATA},

                /* ExtendedHeader */
                {1, true, false, false, false, false, false, false, NO_DATA},

                /* PermData */
                {2, false, true, false, false, false, false, false, NO_DATA},
                /* Seq Num */
                {3, false, false, true, false, false, false, false, NO_DATA},
                /* Part Number */
                {4, false, false, true, true, false, false, false, NO_DATA},
                /* Message Complete */
                {5, false, false, true, false, true, false, false, NO_DATA},
                /* Secondary Sec Number */
                {6, false, false, false, false, false, true, false, NO_DATA},
                /* Provider Driven*/
                {7, false, false, false, false, false, false, true, NO_DATA},
                /* DEFAULT FIELD_LIST */
                {8, false, true, false, false, false, false, false, FIELD_LIST},
                /* DEFAULT MAP */
                {9, false, true, false, false, false, false, false, MAP}
        });
    }


}
