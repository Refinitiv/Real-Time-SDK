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
import com.refinitiv.eta.rdm.UpdateEventTypes;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.codec.CodecReturnCodes.ENCODE_CONTAINER;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class UpdateMsgTest {

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

    public class MessageParameters
    {
        public int id;
        public int containerType;
        public boolean extendedHeader;
        public boolean permData;
        public boolean key;
        public boolean seqNum;
        public boolean conflationInfo;
        public boolean doNotCache;
        public boolean doNotConflate;
        public boolean doNotRipple;
        public boolean postUserInfo;
        public boolean discardable;
        public int domainType;
        public int streamId;

        @Override
        public String toString() {
            return "MessageParameters{" +
                    "id=" + id +
                    ", containerType=" + containerType +
                    ", extendedHeader=" + extendedHeader +
                    ", permData=" + permData +
                    ", key=" + key +
                    ", seqNum=" + seqNum +
                    ", conflationInfo=" + conflationInfo +
                    ", doNotCache=" + doNotCache +
                    ", doNotConflate=" + doNotConflate +
                    ", doNotRipple=" + doNotRipple +
                    ", postUserInfo=" + postUserInfo +
                    ", discardable=" + discardable +
                    '}';
        }
    }
    MessageParameters msgParameters = new MessageParameters();

    public UpdateMsgTest(
            int id,
            int containerType,
            boolean extendedHeader,
            boolean permData,
            boolean key,
            boolean seqNum,
            boolean conflationInfo,
            boolean doNotCache,
            boolean doNotConflate,
            boolean doNotRipple,
            boolean postUserInfo,
            boolean discardable,
            int domainType,
            int streamId
    ) {
        msgParameters.id                = id;
        msgParameters.containerType = containerType;
        msgParameters.extendedHeader = extendedHeader;
        msgParameters.permData = permData;
        msgParameters.key = key;
        msgParameters.seqNum = seqNum;
        msgParameters.conflationInfo = conflationInfo;
        msgParameters.doNotCache = doNotCache;
        msgParameters.doNotConflate = doNotConflate;
        msgParameters.doNotRipple = doNotRipple;
        msgParameters.postUserInfo = postUserInfo;
        msgParameters.discardable = discardable;
        msgParameters.domainType = domainType;
        msgParameters.streamId = streamId;
    }

    private Msg generateMsg() {
        UpdateMsg updateMsg = (UpdateMsg) CodecFactory.createMsg();

        updateMsg.msgClass(MsgClasses.UPDATE);

        updateMsg.streamId(msgParameters.streamId);
        updateMsg.containerType(msgParameters.containerType);
        updateMsg.domainType(msgParameters.domainType);
        updateMsg.updateType(UpdateEventTypes.QUOTE);

        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        updateMsg.encodedDataBody(emptyBuffer);


        if (msgParameters.extendedHeader) {
            updateMsg.applyHasExtendedHdr();
            updateMsg.extendedHeader(JsonConverterTestUtils.EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.permData) {
            updateMsg.applyHasPermData();
            updateMsg.permData(JsonConverterTestUtils.PERM_DATA_BUFFER);
        }

        if (msgParameters.key) {
            updateMsg.applyHasMsgKey();
            updateMsg.msgKey().applyHasName();
            updateMsg.msgKey().name(JsonConverterTestUtils.MSG_KEY_NAME_BUFFER);
            updateMsg.msgKey().applyHasServiceId();
            updateMsg.msgKey().serviceId(JsonConverterTestUtils.MSGKEY_SVC_ID);
        }

        if (msgParameters.seqNum) {
            updateMsg.applyHasSeqNum();
            updateMsg.seqNum(JsonConverterTestUtils.SEQ_NUM);
        }

        final int CONFLATION_COUNT = 15;
        final int CONFLATION_TIME = 32768;
        if (msgParameters.conflationInfo) {
            updateMsg.applyHasConfInfo();
            updateMsg.conflationCount(CONFLATION_COUNT);
            updateMsg.conflationTime(CONFLATION_TIME);
        }

        if (msgParameters.doNotCache)
            updateMsg.applyDoNotCache();

        if (msgParameters.doNotConflate)
            updateMsg.applyDoNotConflate();

        if (msgParameters.doNotRipple)
            updateMsg.applyDoNotRipple();

        if (msgParameters.postUserInfo) {
            updateMsg.applyHasPostUserInfo();
            updateMsg.postUserInfo().userAddr(JsonConverterTestUtils.IP_ADDRESS_STR);
            updateMsg.postUserInfo().userId(JsonConverterTestUtils.USER_ID);
        }

        if (msgParameters.discardable)
            updateMsg.applyDiscardable();

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        switch (msgParameters.containerType) {
            case DataTypes.FIELD_LIST:
                assertEquals(ENCODE_CONTAINER, updateMsg.encodeInit(iter, 0));
                encodeSampleFieldList(updateMsg);
                assertEquals(SUCCESS, updateMsg.encodeComplete(iter, true));
                break;
            case DataTypes.MAP:
                assertEquals(ENCODE_CONTAINER, updateMsg.encodeInit(iter, 0));
                encodeMap(updateMsg);
                assertEquals(SUCCESS, updateMsg.encodeComplete(iter, true));
                break;
            case DataTypes.NO_DATA:
                if (msgParameters.domainType == DomainTypes.SOURCE) {
                    updateMsg.containerType(DataTypes.MAP);
                    encodeDirectoryServiceList(updateMsg);
                }
                assertEquals(SUCCESS, updateMsg.encode(iter));
                break;
            default:
                assertTrue("Unhandled container type", false);
        }

        origMsgBuffer.data().flip();
        updateMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return updateMsg;
    }

    @Before
    public void init()
    {
        jsonMsg.clear();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
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

        assertEquals("DecodeIter setup", SUCCESS, decodeIter.setBufferAndRWFVersion(jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion()));
        assertEquals("DecodeMsg", SUCCESS, resultMsg.decode(decodeIter));

        assertEquals("check class", MsgClasses.UPDATE, resultMsg.msgClass());

        checkUpdateMsgsAreEqual((UpdateMsg) origMsg, (UpdateMsg)resultMsg);
    }

    private void checkUpdateMsgsAreEqual(UpdateMsg expectedMsg, UpdateMsg actualMsg) {
        JsonConverterTestUtils.checkMsgsAreEqual(expectedMsg, actualMsg);

        assertEquals(expectedMsg.updateType(), actualMsg.updateType());
        assertEquals(expectedMsg.checkHasExtendedHdr(), actualMsg.checkHasExtendedHdr());

        if (expectedMsg.checkHasExtendedHdr()) //redundant?
            assertTrue(expectedMsg.extendedHeader().equals(actualMsg.extendedHeader()));

        assertEquals(expectedMsg.conflationCount(), actualMsg.conflationCount());
        assertEquals(expectedMsg.conflationTime(), actualMsg.conflationTime());

        assertEquals(expectedMsg.checkDoNotConflate(), actualMsg.checkDoNotConflate());
        assertEquals(expectedMsg.checkDoNotCache(), actualMsg.checkDoNotCache());
        assertEquals(expectedMsg.checkDoNotRipple(), actualMsg.checkDoNotRipple());
        assertEquals(expectedMsg.checkHasPermData(), actualMsg.checkHasPermData());
        if(expectedMsg.checkHasPermData())
            assertTrue(expectedMsg.permData().equals(actualMsg.permData()));

        assertEquals(expectedMsg.checkHasPostUserInfo(), actualMsg.checkHasPostUserInfo());
        if (expectedMsg.checkHasPostUserInfo()) {
            assertEquals(expectedMsg.postUserInfo().userAddr(), actualMsg.postUserInfo().userAddr());
            assertEquals(expectedMsg.postUserInfo().userId(), actualMsg.postUserInfo().userId());
        }

        assertEquals(expectedMsg.checkHasSeqNum(), actualMsg.checkHasSeqNum());
        if (expectedMsg.checkHasSeqNum())
            assertEquals(expectedMsg.seqNum(), actualMsg.seqNum());

        if (msgParameters.containerType == DataTypes.NO_DATA && expectedMsg.domainType() == DomainTypes.SOURCE) {
            DecodeIterator decIter = CodecFactory.createDecodeIterator();
            decIter.setBufferAndRWFVersion(actualMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
            checkDirectoryServiceList(decIter);
        }
    }


    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][] {
                /* Test with/without ExtendedHeader, PermData, MsgKey, SeqNum, ConflationInfo, DoNotCache, DoNotRipple, PostUserInfo, Discardable */

                /* Defaults */
                {1, DataTypes.FIELD_LIST, false, false, false, false, false, false, false, false, false, false, DomainTypes.MARKET_BY_TIME, 5},

                /* ExtendedHeader */
                {2, DataTypes.FIELD_LIST, true, false, false, false, false, false, false, false, false, false, DomainTypes.LOGIN, 1},

                /* PermData */
                {3, DataTypes.FIELD_LIST, false, true, false, false, false, false, false, false, false, false, DomainTypes.DICTIONARY, 3},

                /* MsgKey */
                {4, DataTypes.FIELD_LIST, false, false, true, false, false, false, false, false, false, false, DomainTypes.MARKET_BY_PRICE, 2},

                /* SeqNum */
                {5, DataTypes.FIELD_LIST, false, false, false, true, false, false, false, false, false, false, DomainTypes.LOGIN, 1},

                /* ConflationInfo */
                {6, DataTypes.FIELD_LIST, false, false, false, false, true, false, false, false, false, false, DomainTypes.MARKET_BY_PRICE, 2},

                /* ConflationInfo and no payload (tests parsing conflation info when it is the last member in the message) */
                {7, DataTypes.NO_DATA, false, false, false, false, true, false, false, false, false, false, DomainTypes.SOURCE, 4},

                /* DoNotCache */
                {8, DataTypes.FIELD_LIST, false, false, false, false, false, true, false, false, false, false, DomainTypes.YIELD_CURVE, 3},

                /* DoNotConflate */
                {9, DataTypes.FIELD_LIST, false, false, false, false, false, false, true, false, false, false, DomainTypes.MARKET_BY_PRICE, 6},

                /* DoNotRipple */
                {10, DataTypes.FIELD_LIST, false, false, false, false, false, false, false, true, false, false, DomainTypes.MARKET_BY_ORDER, 5},

                /* PostUserInfo */
                {11, DataTypes.FIELD_LIST, false, false, false, false, false, false, false, false, true, false, DomainTypes.MARKET_BY_PRICE, 4},

                /* Discardable */
                {12, DataTypes.FIELD_LIST, false, false, false, false, false, false, false, false, false, true, DomainTypes.MARKET_BY_PRICE, 3},

                /* Everything */
                {13, DataTypes.FIELD_LIST, true, true, true, true, true, true, true, true, true, true, DomainTypes.MARKET_BY_PRICE, 7},
                {14, DataTypes.NO_DATA, true, true, true, true, true, true, true, true, true, true, DomainTypes.MARKET_BY_PRICE, 2},
                {15, DataTypes.MAP, true, true, true, true, true, true, true, true, true, true, DomainTypes.LOGIN, 1},
                {16, DataTypes.NO_DATA, true, true, true, true, true, true, true, true, true, true, DomainTypes.SOURCE, 5}
        });
    }
}
