/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

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
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.checkMsgStatusKeyAttrib;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.encodeStatusMsgKeyAttrib;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class StatusMsgTest {
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

    
    public class MessageParameters {
        public int id;
        boolean extendedHeader;
        boolean permData;
        boolean key;
        boolean groupId;
        boolean state;
        boolean clearCache;
        boolean privateStream;
        boolean postUserInfo;
        boolean reqKey;
        boolean qualifiedStream;
        public int domainType;
        public int streamId;
    }
    MessageParameters msgParameters = new MessageParameters();
    
    public StatusMsgTest(
            int id,
            boolean extendedHeader,
            boolean permData,
            boolean key,
            boolean groupId,
            boolean state,
            boolean clearCache,
            boolean privateStream,
            boolean postUserInfo,
            boolean reqKey,
            boolean qualifiedStream,
            int domainType,
            int streamId
    ) {
        msgParameters.id                = id;
        msgParameters.extendedHeader = extendedHeader; 
        msgParameters.permData = permData; 
        msgParameters.key = key; 
        msgParameters.groupId = groupId; 
        msgParameters.state = state; 
        msgParameters.clearCache = clearCache; 
        msgParameters.privateStream = privateStream; 
        msgParameters.postUserInfo = postUserInfo; 
        msgParameters.reqKey = reqKey; 
        msgParameters.qualifiedStream = qualifiedStream;
        msgParameters.domainType = domainType;
        msgParameters.streamId = streamId;
    }


    @Before
    public void init()
    {
        jsonMsg.clear();
        jsonBuffer = CodecFactory.createBuffer();
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
                new Class[] {JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(msgParameters.streamId);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        convRes = ConverterFactory.createConversionResults();
    }

    @Test
    public void testRwfToJson_Sunny() {
        Msg origMsg = generateMsg();
        assertEquals(SUCCESS, converter.convertRWFToJson(origMsg, rwfToJsonOptions, convRes, convError));
        assertEquals(SUCCESS, converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError));

        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));

        assertEquals("DecodeIter setup", SUCCESS, decodeIter.setBufferAndRWFVersion(
                jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion()));
        assertEquals("DecodeMsg", SUCCESS, resultMsg.decode(decodeIter));

        assertEquals("check class", MsgClasses.STATUS, resultMsg.msgClass());

        checkStatusMsgsAreEqual((StatusMsg) origMsg, (StatusMsg)resultMsg);
    }

    private void checkStatusMsgsAreEqual(StatusMsg expectedMsg, StatusMsg actualMsg) {
        JsonConverterTestUtils.checkMsgsAreEqual(expectedMsg, actualMsg);
        if (expectedMsg.domainType() == DomainTypes.LOGIN)
            checkMsgStatusKeyAttrib(actualMsg.msgKey());

        assertEquals(expectedMsg.checkClearCache(), actualMsg.checkClearCache());
        assertEquals(expectedMsg.checkHasExtendedHdr(), actualMsg.checkHasExtendedHdr());

        if (expectedMsg.checkHasExtendedHdr()) //redundant?
            assertTrue(expectedMsg.extendedHeader().equals(actualMsg.extendedHeader()));

        assertEquals(expectedMsg.checkHasPermData(), actualMsg.checkHasPermData());
        if (expectedMsg.checkHasPermData())
            assertTrue(expectedMsg.permData().equals(actualMsg.permData()));

        assertEquals(expectedMsg.checkHasPostUserInfo(), actualMsg.checkHasPostUserInfo());
        if (expectedMsg.checkHasPostUserInfo()) {
            assertEquals(expectedMsg.postUserInfo().userAddr(), actualMsg.postUserInfo().userAddr());
            assertEquals(expectedMsg.postUserInfo().userId(), actualMsg.postUserInfo().userId());
        }

        assertEquals(expectedMsg.checkPrivateStream(), actualMsg.checkPrivateStream());
        assertEquals(expectedMsg.checkQualifiedStream(), actualMsg.checkQualifiedStream());

        assertEquals(expectedMsg.checkHasState(), actualMsg.checkHasState());
        if (expectedMsg.checkHasState())
            assertTrue(expectedMsg.state().equals(actualMsg.state()));

        assertEquals(expectedMsg.checkHasPermData(), actualMsg.checkHasPermData());
        if(expectedMsg.checkHasPermData())
            assertTrue(expectedMsg.permData().equals(actualMsg.permData()));
    }

    private Msg generateMsg() {
        StatusMsg statusMsg = (StatusMsg) CodecFactory.createMsg();

        statusMsg.msgClass(MsgClasses.STATUS);

        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        statusMsg.encodedDataBody(emptyBuffer);

        statusMsg.streamId(msgParameters.streamId);
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.domainType(msgParameters.domainType);

        if (msgParameters.extendedHeader) {
            statusMsg.applyHasExtendedHdr();
            statusMsg.extendedHeader(JsonConverterTestUtils.EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.permData) {
            statusMsg.applyHasPermData();
            statusMsg.permData(JsonConverterTestUtils.PERM_DATA_BUFFER);
        }

        if (msgParameters.key) {
            statusMsg.applyHasMsgKey();
            statusMsg.msgKey().applyHasName();
            statusMsg.msgKey().name(JsonConverterTestUtils.MSG_KEY_NAME_BUFFER);
            statusMsg.msgKey().applyHasServiceId();
            statusMsg.msgKey().serviceId(JsonConverterTestUtils.MSGKEY_SVC_ID);
        }

        if (msgParameters.domainType == DomainTypes.LOGIN)
            encodeStatusMsgKeyAttrib(statusMsg.msgKey());

        if (msgParameters.groupId) {
            statusMsg.applyHasGroupId();
            statusMsg.groupId(JsonConverterTestUtils.GROUP_ID_BUFFER);
        }

        if (msgParameters.state) {
            statusMsg.applyHasState();
            statusMsg.state().streamState(StreamStates.CLOSED);
            statusMsg.state().dataState(DataStates.SUSPECT);
            statusMsg.state().code(StateCodes.NOT_FOUND);
            statusMsg.state().text(JsonConverterTestUtils.STATE_TEXT_BUFFER);
        }

        if (msgParameters.clearCache)
            statusMsg.applyClearCache();

        if (msgParameters.privateStream)
            statusMsg.applyPrivateStream();

        if (msgParameters.postUserInfo) {
            statusMsg.applyHasPostUserInfo();
            statusMsg.postUserInfo().userAddr(JsonConverterTestUtils.IP_ADDRESS_STR);
            statusMsg.postUserInfo().userId(JsonConverterTestUtils.USER_ID);
        }

        if (msgParameters.qualifiedStream)
            statusMsg.applyQualifiedStream();

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        assertEquals(SUCCESS, statusMsg.encode(iter));

        origMsgBuffer.data().flip();
        statusMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return statusMsg;
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][]{
                /* Test with/without ExtendedHeader, PermData, MsgKey, GroupId, State, ClearCache, PrivateStream, PostUserInfo, ReqKey, Qualified */

                /* Defaults */
                { 0, false, false, false, false, false, false, false, false, false, false, DomainTypes.SOURCE, 3},

                /* ExtendedHeader */
                { 1, true, false, true, false, false, false, false, false, false, false, DomainTypes.LOGIN, 1},

                /* PermData */
                { 2, false, true, false, false, false, false, false, false, false, false, DomainTypes.DICTIONARY, 2},

                /* MsgKey */
                { 3, false, false, true, false, false, false, false, false, false, false, DomainTypes.MARKET_BY_ORDER, 3},

                /* GroupId (Group ID isn't used in JSON so it shouldn't appear) */
                { 4, false, false, false, true, false, false, false, false, false, false, DomainTypes.SOURCE, 5},

                /* State */
                { 5, false, false, true, false, true, false, false, false, false, false, DomainTypes.LOGIN, 3},

                /* ClearCache */
                { 6, false, false, false, false, false, true, false, false, false, false, DomainTypes.SOURCE, 5},

                /* PrivateStream */
                { 7, false, false, false, false, false, false, true, false, false, false, DomainTypes.MARKET_BY_PRICE, 6},

                /* PostUserInfo */
                { 8, false, false, true, false, false, false, false, true, false, false, DomainTypes.LOGIN, 1},

                /* ReqKey */
                { 9, false, false, false, false, false, false, false, false, true, false, DomainTypes.YIELD_CURVE, 2},

                /* Qualified */
                {10, false, false, false, false, false, false, false, false, false, true, DomainTypes.MARKET_MAKER, 4},

                /* Everything */
                {11, true, true, true, true, true, true, true, true, true, true, DomainTypes.DICTIONARY, 3}

        });
    }
}
