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
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.Login;
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
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class RefreshMsgTest {

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
    private static final String blankStringConst = new String(new byte[] { 0x0 });

    public class MsgParameters {
        private int id;
        private boolean extendedHeader;
        private boolean permData;
        private boolean seqNum;
        private boolean partNum;
        private boolean solicited;
        private boolean refreshComplete;
        private boolean qos;
        private boolean clearCache;
        private boolean doNotCache;
        private boolean privateStream;
        private boolean postUserInfo;
        private boolean qualifiedStream;
        private int containerType;
        private int domainType;
        private int streamId;
        private int msgKeyUserNameType;
        private boolean hasUserNameType;
        private boolean hasName;
        private String msgKeyName;
        private boolean hasConnectionConfig;
    }


    private MsgParameters msgParameters = new MsgParameters();

    public RefreshMsgTest(int id,
                          boolean extendedHeader,
                          boolean permData,
                          boolean seqNum,
                          boolean partNum,
                          boolean solicited,
                          boolean refreshComplete,
                          boolean qos,
                          boolean clearCache,
                          boolean doNotCache,
                          boolean privateStream,
                          boolean postUserInfo,
                          boolean qualifiedStream,
                          int containerType,
                          int domainType,
                          int streamId,
                          boolean hasUserNameType,
                          int msgKeyUserNameType,
                          boolean hasName,
                          String msgKeyName,
                          boolean hasConnectionConfig) {
        msgParameters.id = id;
        msgParameters.extendedHeader = extendedHeader;
        msgParameters.permData = permData;
        msgParameters.seqNum = seqNum;
        msgParameters.partNum = partNum;
        msgParameters.solicited = solicited;
        msgParameters.refreshComplete = refreshComplete;
        msgParameters.qos = qos;
        msgParameters.clearCache = clearCache;
        msgParameters.doNotCache = doNotCache;
        msgParameters.privateStream = privateStream;
        msgParameters.postUserInfo = postUserInfo;
        msgParameters.qualifiedStream = qualifiedStream;
        msgParameters.containerType = containerType;
        msgParameters.domainType = domainType;
        msgParameters.streamId = streamId;
        msgParameters.msgKeyName = msgKeyName;
        msgParameters.msgKeyUserNameType = msgKeyUserNameType;
        msgParameters.hasUserNameType = hasUserNameType;
        msgParameters.hasName = hasName;
        msgParameters.hasConnectionConfig = hasConnectionConfig;
    }

    private Msg generateMsg() {
        RefreshMsg refreshMsg = (RefreshMsg) CodecFactory.createMsg();
        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        refreshMsg.encodedDataBody(emptyBuffer);

        refreshMsg.msgClass(MsgClasses.REFRESH);

        refreshMsg.streamId(msgParameters.streamId);
        refreshMsg.domainType(msgParameters.domainType);
        refreshMsg.containerType(msgParameters.containerType);

        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(MSGKEY_SVC_ID);

        if (msgParameters.hasName) {
            refreshMsg.msgKey().applyHasName();
            MSG_KEY_NAME_BUFFER.data(msgParameters.msgKeyName);
            refreshMsg.msgKey().name(MSG_KEY_NAME_BUFFER);
        }

        if (msgParameters.hasUserNameType) {
            if (msgParameters.msgKeyUserNameType == Login.UserIdTypes.AUTHN_TOKEN) {
                refreshMsg.msgKey().applyHasName();
                refreshMsg.msgKey().name().data(blankStringConst);
            }
            refreshMsg.msgKey().applyHasNameType();
            refreshMsg.msgKey().nameType(msgParameters.msgKeyUserNameType);
        }

        if (msgParameters.domainType == DomainTypes.LOGIN)
            encodeLoginMsgKeyAttrib(refreshMsg.msgKey());

        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.state().code(StateCodes.NONE);
        refreshMsg.state().text().data("Item Refresh Completed");

        Qos qos = CodecFactory.createQos();
        qos.timeliness(QosTimeliness.REALTIME);
        qos.rate(QosRates.TICK_BY_TICK);

        Qos worstQos = CodecFactory.createQos();
        worstQos.timeliness(QosTimeliness.DELAYED_UNKNOWN);
        worstQos.rate(QosRates.JIT_CONFLATED);

        if (msgParameters.extendedHeader) {
            refreshMsg.applyHasExtendedHdr();
            refreshMsg.extendedHeader(EXTENDED_HEADER_BUFFER);
        }

        if (msgParameters.permData) {
            refreshMsg.applyHasPermData();
            Buffer permissionData = CodecFactory.createBuffer();
            permissionData.data("RefreshMsg.permissionData");
            refreshMsg.permData(permissionData);
        }

        if (msgParameters.seqNum) {
            refreshMsg.applyHasSeqNum();
            refreshMsg.seqNum(101L);
        }

        if (msgParameters.partNum) {
            refreshMsg.applyHasPartNum();
            refreshMsg.partNum(10);
        }

        if (msgParameters.solicited) {
            refreshMsg.applySolicited();
        }

        if (msgParameters.refreshComplete) {
            refreshMsg.applyRefreshComplete();
        }

        if (msgParameters.qos) {
            refreshMsg.applyHasQos();
            qos.copy(refreshMsg.qos());
        }

        if (msgParameters.clearCache) {
            refreshMsg.applyClearCache();
        }

        if (msgParameters.doNotCache) {
            refreshMsg.applyDoNotCache();
        }

        if (msgParameters.privateStream) {
            refreshMsg.applyPrivateStream();
        }

        if (msgParameters.postUserInfo) {
            refreshMsg.applyHasPostUserInfo();
            refreshMsg.postUserInfo().userAddr(15);
            refreshMsg.postUserInfo().userId(30);
        }

        if (msgParameters.qualifiedStream) {
            refreshMsg.applyQualifiedStream();
        }

        encodeIter.setBufferAndRWFVersion(origMsgBuffer, Codec.majorVersion(), Codec.minorVersion());
        EncodeIterator iter = encodeIter;

        if (msgParameters.hasConnectionConfig && msgParameters.domainType == DomainTypes.LOGIN) {
            refreshMsg.containerType(ELEMENT_LIST);
            encodeConnectionConfig(refreshMsg);
            assertEquals(SUCCESS, refreshMsg.encode(iter));
        } else if (msgParameters.domainType == DomainTypes.SOURCE) {
            refreshMsg.containerType(DataTypes.MAP);
            encodeDirectoryServiceList(refreshMsg);
            assertEquals(SUCCESS, refreshMsg.encode(iter));
        } else if (msgParameters.domainType == DomainTypes.DICTIONARY){
            refreshMsg.containerType(DataTypes.SERIES);
            encodeDictionary(refreshMsg);
            assertEquals(SUCCESS, refreshMsg.encode(iter));
        } else {
            switch (msgParameters.containerType) {
                case FIELD_LIST:
                    assertEquals(CodecReturnCodes.ENCODE_CONTAINER, refreshMsg.encodeInit(iter, 0));
                    encodeSampleFieldList(refreshMsg);
                    assertEquals(SUCCESS, refreshMsg.encodeComplete(encodeIter, true));
                    break;
                case MAP:
                    assertEquals(CodecReturnCodes.ENCODE_CONTAINER, refreshMsg.encodeInit(iter, 0));
                    encodeMap(refreshMsg);
                    assertEquals(SUCCESS, refreshMsg.encodeComplete(encodeIter, true));
                case DataTypes.NO_DATA:
                    refreshMsg.encode(iter);
                    break;
            }
        }

        origMsgBuffer.data().flip();
        refreshMsg.encodedMsgBuffer().data(origMsgBuffer.data());

        return refreshMsg;
    }

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

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
                .setDictionary(dictionary)
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[]{JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(msgParameters.streamId);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
    }

    private void checkForRefreshMsgEqualsTo(RefreshMsg expectedMsg, RefreshMsg equalsToMsg) {
        checkMsgsAreEqual(expectedMsg, equalsToMsg);
        if (expectedMsg.domainType() == DomainTypes.LOGIN)
            checkMsgLoginKeyAttrib(equalsToMsg.msgKey());

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
        assertEquals(expectedMsg.checkSolicited(), equalsToMsg.checkSolicited());
        assertEquals(expectedMsg.checkRefreshComplete(), equalsToMsg.checkRefreshComplete());
        assertEquals(expectedMsg.checkHasQos(), equalsToMsg.checkHasQos());
        if (expectedMsg.checkHasQos()) {
            checkQoSsAreEqual(expectedMsg.qos(), equalsToMsg.qos());
        }
        assertEquals(expectedMsg.checkPrivateStream(), equalsToMsg.checkPrivateStream());
        assertEquals(expectedMsg.checkHasPostUserInfo(), equalsToMsg.checkHasPostUserInfo());
        if (expectedMsg.checkHasPostUserInfo()) {
            assertEquals(expectedMsg.postUserInfo().userId(), equalsToMsg.postUserInfo().userId());
            assertEquals(expectedMsg.postUserInfo().userAddr(), equalsToMsg.postUserInfo().userAddr());
        }
        assertEquals(expectedMsg.checkQualifiedStream(), equalsToMsg.checkQualifiedStream());

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        switch (equalsToMsg.domainType()) {
            case DomainTypes.SOURCE:
                decIter.setBufferAndRWFVersion(equalsToMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
                checkDirectoryServiceList(decIter);
                break;
            case DomainTypes.DICTIONARY:
                decIter.setBufferAndRWFVersion(equalsToMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
                checkDictionaryPayload(decIter);
                break;
            case DomainTypes.LOGIN:
                decIter.setBufferAndRWFVersion(equalsToMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
                checkConnectionInfo(decIter);
                break;
            default:
                break;
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

        assertEquals("check class", MsgClasses.REFRESH, resultMsg.msgClass());

        checkForRefreshMsgEqualsTo((RefreshMsg) origMsg, (RefreshMsg) resultMsg);
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][]{
                /* Test with/without ExtendedHeader, PermData, SeqNum, PartNum, Solicited, RefreshComplete, Qos, ClearCache,
                 * DoNotCache, PrivateStream, PostUserInfo, QualifiedStream, domainType, streamId, hasUserNameType, msgKeyUserNameType, hasName, msgKeyName, hasConnectionConfig*/

                /* Defaults */
                {0, false, false, true, true, false, true, false, false, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /*Extended Header*/
                {1, true, false, true, true, false, true, false, false, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_PRICE, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /*PermData*/
                {2, false, true, true, true, false, true, false, false, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.UNSPECIFIED, true, "Name", false},
                /*NO SeqNum*/
                {3, false, false, false, true, false, true, false, false, false, false, false, false, MAP, DomainTypes.MARKET_BY_TIME, 2, true, InstrumentNameTypes.RIC, true, "Name", true},
                /*NO PartNum*/
                {4, false, false, true, false, false, true, false, false, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_PRICE, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /*Solicited*/
                {5, false, false, true, true, true, true, false, false, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.UNSPECIFIED, true, "Name", false},
                /*Refresh is not completed*/
                {6, false, false, true, true, false, false, false, false, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /*Qos*/
                {7, false, false, true, true, false, true, true, false, false, false, false, false, FIELD_LIST, DomainTypes.MARKET_PRICE, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /*ClearCache*/
                {8, false, false, true, true, false, true, false, true, false, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.CONTRIBUTOR, true, "Name", false},
                /*DoNotCache*/
                {9, false, false, true, true, false, true, false, false, true, false, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_PRICE, 2, true, InstrumentNameTypes.RIC, true, "Name", true},
                /*PrivateStream*/
                {10, false, false, true, true, false, true, false, false, false, true, false, false, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /*PostUserInfo*/
                {11, false, false, true, true, false, true, false, false, false, false, true, false, DataTypes.NO_DATA, DomainTypes.YIELD_CURVE, 2, true, InstrumentNameTypes.CONTRIBUTOR, true, "Name", true},
                /*QualifiedStream*/
                {12, false, false, true, true, false, true, false, false, false, false, false, true, DataTypes.NO_DATA, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /* Defaults FIELD_LIST */
                {13, false, false, true, true, false, true, false, false, false, false, false, false, FIELD_LIST, DomainTypes.MARKET_BY_ORDER, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /* Login refresh */
                {14, true, true, true, true, true, true, true, true, true, true, true, true, DataTypes.NO_DATA, DomainTypes.LOGIN, 2, true, Login.UserIdTypes.AUTHN_TOKEN, true, "Name", true},
                /* Directory refresh */
                {15, true, true, true, true, true, true, true, true, true, true, true, true, FIELD_LIST, DomainTypes.SOURCE, 2, true, InstrumentNameTypes.RIC, true, "Name", false},
                /* Dictionary refresh */
                {16, true, true, true, true, true, true, true, true, true, true, true, true, MAP, DomainTypes.DICTIONARY, 2, true, InstrumentNameTypes.CONTRIBUTOR, true, "Name", true},
                /* Login refresh */
                {17, true, true, true, true, true, true, true, true, true, true, true, true, DataTypes.NO_DATA, DomainTypes.LOGIN, 2, true, Login.UserIdTypes.TOKEN, true, "Name", true},
                /* Backslash in Item Name on Msg Key refresh */
                {17, true, true, true, true, true, true, true, true, true, true, true, true, DataTypes.NO_DATA, DomainTypes.LOGIN, 2, true, Login.UserIdTypes.TOKEN, true, "\\Name", true},
        });
    }
}
