/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.codec.DataTypes.FIELD_LIST;
import static com.refinitiv.eta.codec.DataTypes.MSG;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.*;
import static org.junit.Assert.*;

@RunWith(Parameterized.class)
public class PostMsgTest {
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
    int streamId = MsgClasses.POST;
    Buffer errorBuffer = CodecFactory.createBuffer();
    GetJsonErrorParams errorParams = ConverterFactory.createJsonErrorParams();

    public class MsgParameters {
        private int id;
        private boolean extendedHeader;
        private boolean postId;
        private boolean seqNum;
        private boolean partNum;
        private boolean postComplete;
        private boolean ack;
        private boolean permData;
        private boolean postUserRights;
        private int containerType;
    }


    private MsgParameters msgParameters = new MsgParameters();

    public PostMsgTest(int id,
                       boolean extendedHeader,
                       boolean postId,
                       boolean seqNum,
                       boolean partNum,
                       boolean postComplete,
                       boolean ack,
                       boolean permData,
                       boolean postUserRights,
                       int containerType) {
        msgParameters.id = id;
        msgParameters.extendedHeader = extendedHeader;
        msgParameters.postId = postId;
        msgParameters.seqNum = seqNum;
        msgParameters.partNum = partNum;
        msgParameters.postComplete = postComplete;
        msgParameters.ack = ack;
        msgParameters.permData = permData;
        msgParameters.postUserRights = postUserRights;
        msgParameters.containerType = containerType;
    }

    private Msg generateMsg() {
        PostMsg postMsg = (PostMsg) CodecFactory.createMsg();
        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        postMsg.encodedDataBody(emptyBuffer);

        postMsg.msgClass(MsgClasses.POST);
        postMsg.streamId(MsgClasses.POST);
        postMsg.domainType(DomainTypes.MARKET_PRICE);
        postMsg.containerType(msgParameters.containerType);

        postMsg.applyHasMsgKey();
        postMsg.msgKey().applyHasServiceId();
        postMsg.msgKey().serviceId(MSGKEY_SVC_ID);

        postMsg.msgKey().name(MSG_KEY_NAME_BUFFER);
        postMsg.msgKey().applyHasName();
        postMsg.msgKey().nameType(InstrumentNameTypes.CONTRIBUTOR);
        postMsg.msgKey().applyHasNameType();

        if (msgParameters.extendedHeader) {
            postMsg.applyHasExtendedHdr();
            postMsg.extendedHeader(EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.postId) {
            postMsg.applyHasPostId();
            postMsg.postId(msgParameters.id);
        }

        if (msgParameters.seqNum) {
            postMsg.applyHasSeqNum();
            postMsg.seqNum(101L);
        }

        if (msgParameters.partNum) {
            postMsg.applyHasPartNum();
            postMsg.partNum(20);
        }

        if (msgParameters.postComplete) {
            postMsg.applyPostComplete();
        }

        if (msgParameters.ack) {
            postMsg.applyAck();
        }

        if (msgParameters.permData) {
            postMsg.applyHasPermData();
            postMsg.permData().data("PostMsg.permissionData");
        }

        if (msgParameters.postUserRights) {
            postMsg.applyHasPostUserRights();
            postMsg.postUserRights(PostUserRights.CREATE | PostUserRights.DELETE);
        }

        postMsg.postUserInfo().userId(msgParameters.id);
        postMsg.postUserInfo().userAddr(300);

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());

        assertFalse(postMsg.encodeInit(encodeIter, 0) < SUCCESS);
        switch (msgParameters.containerType) {
            case MSG:
                writeBasicMsg(postMsg);
                break;
            case FIELD_LIST:
                encodeSampleFieldList(postMsg);
                break;
        }
        assertEquals(postMsg.encodeComplete(encodeIter, true), SUCCESS);

        origMsgBuffer.data().flip();
        postMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return postMsg;
    }

    @Before
    public void init() {
        jsonMsg.clear();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        resultMsg = CodecFactory.createMsg();
        origMsgBuffer = CodecFactory.createBuffer();
        origMsgBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
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

    private void encodeFieldList() {
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();
        fieldList.clear();
        fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(encodeIter, null, 0));
        entry.fieldId(245);
        entry.dataType(DataTypes.REAL);
        assertEquals(CodecReturnCodes.SUCCESS, entry.encode(encodeIter, CodecFactory.createReal()));
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(encodeIter, true));
    }

    private void checkForPostMsgEqualsTo(PostMsg expectedMsg, PostMsg equalsToMsg) {
        checkMsgsAreEqual(expectedMsg, equalsToMsg);

        assertEquals(expectedMsg.checkHasExtendedHdr(), equalsToMsg.checkHasExtendedHdr());

        if (expectedMsg.checkHasExtendedHdr()) {
            assertTrue(expectedMsg.extendedHeader().equals(equalsToMsg.extendedHeader()));
        }

        assertEquals(expectedMsg.checkHasPostId(), equalsToMsg.checkHasPostId());
        if (expectedMsg.checkHasPostId()) {
            assertEquals(expectedMsg.postId(), equalsToMsg.postId());
        }

        assertEquals(expectedMsg.checkHasSeqNum(), equalsToMsg.checkHasSeqNum());
        if (expectedMsg.checkHasSeqNum()) {
            assertEquals(expectedMsg.seqNum(), equalsToMsg.seqNum());
        }

        assertEquals(expectedMsg.checkHasPartNum(), equalsToMsg.checkHasPartNum());
        if (expectedMsg.checkHasPartNum()) {
            assertEquals(expectedMsg.partNum(), equalsToMsg.partNum());
        }

        assertEquals(expectedMsg.checkPostComplete(), equalsToMsg.checkPostComplete());
        assertEquals(expectedMsg.checkAck(), equalsToMsg.checkAck());

        assertEquals(expectedMsg.checkHasPermData(), equalsToMsg.checkHasPermData());
        if (expectedMsg.checkHasPermData()) {
            assertEquals(expectedMsg.permData(), equalsToMsg.permData());
        }

        assertEquals(expectedMsg.checkHasPostUserRights(), equalsToMsg.checkHasPostUserRights());
        if (expectedMsg.checkHasPostUserRights()) {
            assertEquals(expectedMsg.postUserRights(), equalsToMsg.postUserRights());
        }

        assertEquals(expectedMsg.postUserInfo() != null, equalsToMsg.postUserInfo() != null);
        if (expectedMsg.postUserInfo() != null) {
            assertEquals(expectedMsg.postUserInfo().userAddr(), equalsToMsg.postUserInfo().userAddr());
            assertEquals(expectedMsg.postUserInfo().userId(), equalsToMsg.postUserInfo().userId());
        }
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

        assertEquals("check class", MsgClasses.POST, resultMsg.msgClass());

        checkForPostMsgEqualsTo((PostMsg) origMsg, (PostMsg) resultMsg);
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][]{
                /* Test with/without ExtendedHeader, PostId, SeqNum, PartNum, PostComplete, ACK, PermData, PostUserRights*/

                /* Defaults */
                {0, false, false, false, false, false, false, false, false, MSG},

                /* ExtendedHeader */
                {1, true, false, false, false, false, false, false, false, MSG},
                /*PostId*/
                {2, false, true, false, false, false, false, false, false, MSG},
                /*SeqNum*/
                {3, false, false, true, false, false, false, false, false, MSG},
                /*PartNum*/
                {4, false, false, false, true, false, false, false, false, MSG},
                /*PostComplete*/
                {5, false, false, false, false, true, false, false, false, MSG},
                /*ACK*/
                {6, false, false, false, false, false, true, false, false, MSG},
                /*PermData*/
                {7, false, false, false, false, false, false, true, false, MSG},
                /*PostUserRights*/
                {8, false, false, false, false, false, true, false, true, MSG},
                /*Everything*/
                {9, true, true, true, true, true, true, true, true, MSG},
                {10, true, true, true, true, true, true, true, true, FIELD_LIST}
        });
    }
}
