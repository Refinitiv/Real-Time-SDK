/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.Login;

import java.io.IOException;
import java.nio.ByteBuffer;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.codec.DataTypes.*;
import static org.junit.Assert.*;

public class JsonConverterTestUtils {
    public static final int TEST_MSG_BUFFER_SIZE = 4096;

    public static final byte RMTES_UPTICK[] = {(byte) 0xde};
    public static final byte UTF8_UPTICK[] = {(byte) 0xe2, (byte) 0x87, (byte) 0xa7};

    public static final Buffer MSG_KEY_NAME_BUFFER = CodecFactory.createBuffer();
    public static final Buffer REQ_MSG_KEY_NAME_BUFFER = CodecFactory.createBuffer();
    public static final Buffer STATE_TEXT_BUFFER = CodecFactory.createBuffer();
    public static final Buffer EXTENDED_HEADER_BUFFER = CodecFactory.createBuffer();
    public static final Buffer PERM_DATA_BUFFER = CodecFactory.createBuffer();
    public static final Buffer ACK_TEXT_BUFFER = CodecFactory.createBuffer();

    public static final int MSGKEY_SVC_ID = 555;
    public static final int MSGKEY_IDENTIFIER = 1205;
    public static final int REQ_MSGKEY_SVC_ID = 555; //is not used in ETAJ
    public static final String IP_ADDRESS_STR = "127.0.0.1";
    public static final int IP_ADDRESS_UINT = 0x7f000001;
    public static final int USER_ID = 8;
    public static final int SEQ_NUM = 88;
    public static final int SECONDARY_SEQ_NUM = 16;
    public static final int PART_NUM = 5;


    public static final Buffer GROUP_ID_BUFFER = CodecFactory.createBuffer();
    public static final Buffer JSON_BUFFER = CodecFactory.createBuffer();
    public static final Buffer OPAQUE_BUFFER = CodecFactory.createBuffer();
    public static final Buffer ASCII_STRING_BUFFER = CodecFactory.createBuffer();
    public static final Buffer UTF8_STRING_BUFFER = CodecFactory.createBuffer();
    public static final Buffer RMTES_STRING_BUFFER = CodecFactory.createBuffer();
    public static final Buffer RMTES_STRING_AS_UTF8_BUFFER = CodecFactory.createBuffer();
    public static final Buffer XML_BUFFER = CodecFactory.createBuffer();
    public static final Buffer SERVICE_NAME_BUFFER = CodecFactory.createBuffer();

    public static int ACTION_COUNT = 3;
    public static final String[] MAP_DATA_ARRAY = new String[]{"AAPL.O", "TEST_UNIT_1", "UNIQUE"};

    private static final ObjectMapper mapper = new ObjectMapper();

    static {
        MSG_KEY_NAME_BUFFER.data("TINY");
        REQ_MSG_KEY_NAME_BUFFER.data("ROLL");
        STATE_TEXT_BUFFER.data("All is well");
        EXTENDED_HEADER_BUFFER.data("ExtendedHeader");
        PERM_DATA_BUFFER.data("PerMission");
        ACK_TEXT_BUFFER.data("Acknowledged");

        GROUP_ID_BUFFER.data("SomeGroupId");
        JSON_BUFFER.data("{\"SomeKey\":true}");
        OPAQUE_BUFFER.data("OpaqueData");
        ASCII_STRING_BUFFER.data("Ascii");
        UTF8_STRING_BUFFER.data(ByteBuffer.wrap(UTF8_UPTICK));
        RMTES_STRING_BUFFER.data(ByteBuffer.wrap(RMTES_UPTICK));
        RMTES_STRING_AS_UTF8_BUFFER.data(ByteBuffer.wrap(UTF8_UPTICK));
        XML_BUFFER.data("<html><head>Hello World!</head></html>");
        SERVICE_NAME_BUFFER.data("DUCK_FEED");
    }

    public static void checkQoSsAreEqual(Qos expectedQos, Qos actualQos) {
        assertEquals(expectedQos.isBlank(), actualQos.isBlank());
        assertEquals(expectedQos.isDynamic(), actualQos.isDynamic());
        assertTrue(expectedQos.equals(actualQos));
    }

    public static void checkMsgsAreEqual(Msg expectedMsg, Msg actualMsg) {
        assertEquals(expectedMsg.msgClass(), actualMsg.msgClass());
        assertEquals(expectedMsg.streamId(), actualMsg.streamId());
        assertEquals(expectedMsg.domainType(), actualMsg.domainType());
        if (expectedMsg.extendedHeader() != null)
            assertTrue(expectedMsg.extendedHeader().equals(actualMsg.extendedHeader()));
        else
            assertTrue(actualMsg.extendedHeader() == null);

        if (expectedMsg.msgKey() == null)
            assertTrue(actualMsg.msgKey() == null);
        else
            assertTrue(actualMsg.msgKey() != null);

        if (expectedMsg.msgKey() != null) {
            if (expectedMsg.msgKey().checkHasName() && expectedMsg.msgKey().checkHasNameType() && expectedMsg.msgKey().nameType() != Login.UserIdTypes.AUTHN_TOKEN)
                assertTrue(expectedMsg.msgKey().name().equals(actualMsg.msgKey().name()));

            if (expectedMsg.msgKey().checkHasNameType())
                assertEquals(expectedMsg.msgKey().nameType(), actualMsg.msgKey().nameType());
            else if (actualMsg.msgKey().checkHasNameType())
                assertEquals(1, actualMsg.msgKey().nameType());

            assertEquals(expectedMsg.msgKey().checkHasServiceId(), actualMsg.msgKey().checkHasServiceId());
            if (expectedMsg.msgKey().checkHasServiceId())
                assertEquals(expectedMsg.msgKey().serviceId(), actualMsg.msgKey().serviceId());

            assertEquals(expectedMsg.msgKey().checkHasAttrib(), actualMsg.msgKey().checkHasAttrib());
            if (expectedMsg.msgKey().checkHasAttrib())
                assertEquals(expectedMsg.msgKey().attribContainerType(), actualMsg.msgKey().attribContainerType());

            assertEquals(expectedMsg.msgKey().checkHasFilter(), actualMsg.msgKey().checkHasFilter());
            assertEquals(expectedMsg.msgKey().checkHasIdentifier(), actualMsg.msgKey().checkHasIdentifier());
            if (expectedMsg.msgKey().checkHasIdentifier())
                assertEquals(expectedMsg.msgKey().identifier(), actualMsg.msgKey().identifier());
            assertEquals(expectedMsg.msgKey().filter(), actualMsg.msgKey().filter());
        } else
            assertTrue(actualMsg.msgKey() == null);

        if (expectedMsg.extendedHeader() != null)
            assertTrue(expectedMsg.extendedHeader().equals(actualMsg.extendedHeader()));

    }

    static void encodeSampleFieldList(Msg msg) {
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer fieldListBuf = CodecFactory.createBuffer();
        fieldListBuf.data(ByteBuffer.allocate(100));
        iter.setBufferAndRWFVersion(fieldListBuf, Codec.majorVersion(), Codec.minorVersion());

        FieldList fieldList = CodecFactory.createFieldList();
        fieldList.flags(ElementListFlags.HAS_STANDARD_DATA);
        assertEquals(SUCCESS, fieldList.encodeInit(iter, null, 0));

        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();
        //BID
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.REAL);
        real.value(30396, RealHints.FRACTION_256);
        assertEquals(SUCCESS, fieldEntry.encode(iter, real));

        //ASK
        fieldEntry.fieldId(25);
        real.value(3906, RealHints.EXPONENT2);
        assertEquals(SUCCESS, fieldEntry.encode(iter, real));

        assertEquals(SUCCESS, fieldList.encodeComplete(iter, true));

        msg.encodedDataBody(fieldListBuf);
    }

    static void writeBasicMsg(Msg outerMsg) {
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer msgBuf = CodecFactory.createBuffer();
        msgBuf.data(ByteBuffer.allocate(500));
        iter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
        encodeBasicMsg(iter);
        outerMsg.encodedDataBody(msgBuf);
    }

    static void encodeStatusMsgKeyAttrib(MsgKey msgKey) {

        msgKey.applyHasAttrib();
        msgKey.attribContainerType(DataTypes.ELEMENT_LIST);

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer tmp = CodecFactory.createBuffer();
        UInt tmpUInt = CodecFactory.createUInt();
        Buffer buf = CodecFactory.createBuffer();
        tmp.data(ByteBuffer.allocate(300));
        encodeIter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());

        elementList.applyHasStandardData();
        elementList.encodeInit(encodeIter, null, 0);

        element.dataType(DataTypes.UINT);
        element.name(ElementNames.AUTHN_ERROR_CODE);
        tmpUInt.value(5);
        element.encode(encodeIter, tmpUInt);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.AUTHN_ERROR_TEXT);
        buf.data("Error text");
        element.encode(encodeIter, buf);

        elementList.encodeComplete(encodeIter, true);

        msgKey.encodedAttrib(tmp);
    }

    static void encodeLoginMsgKeyAttrib(MsgKey msgKey) {

        msgKey.applyHasAttrib();
        msgKey.attribContainerType(DataTypes.ELEMENT_LIST);

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer tmp = CodecFactory.createBuffer();
        UInt tmpUInt = CodecFactory.createUInt();
        tmp.data(ByteBuffer.allocate(300));
        encodeIter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());

        elementList.applyHasStandardData();
        elementList.encodeInit(encodeIter, null, 0);

        element.dataType(DataTypes.UINT);
        element.name(ElementNames.AUTHN_TT_REISSUE);
        tmpUInt.value(5);
        element.encode(encodeIter, tmpUInt);

        element.dataType(DataTypes.UINT);
        element.name(ElementNames.AUTHN_ERROR_CODE);
        tmpUInt.value(10);
        element.encode(encodeIter, tmpUInt);

        elementList.encodeComplete(encodeIter, true);

        msgKey.encodedAttrib(tmp);
    }

    static void encodeConnectionConfig(RefreshMsg refreshMsg) {

        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer tmp = CodecFactory.createBuffer();
        tmp.data(ByteBuffer.allocate(300));
        encodeIter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        UInt tmpUInt = CodecFactory.createUInt();
        Buffer tmpBuffer = CodecFactory.createBuffer();

        elementList.applyHasStandardData();
        elementList.encodeInit(encodeIter, null, 0);
        elementEntry.clear();
        elementEntry.dataType(DataTypes.VECTOR);
        elementEntry.name(ElementNames.CONNECTION_CONFIG);

        elementEntry.encodeInit(encodeIter, 0);

        // Encode Server Entries
        vector.clear();
        vector.containerType(DataTypes.ELEMENT_LIST);
        vector.flags(VectorFlags.HAS_SUMMARY_DATA);
        vector.encodeInit(encodeIter, 0, 0);

        // encode numStandbyServers in summary data.
        elementList.clear();
        elementList.applyHasStandardData();

        elementList.encodeInit(encodeIter, null, 0);
        elementEntry.clear();

        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.NUM_STANDBY_SERVERS);
        tmpUInt.value(3);
        elementEntry.encode(encodeIter, tmpUInt);
        elementList.encodeComplete(encodeIter, true);
        vector.encodeSummaryDataComplete(encodeIter, true);

        vectorEntry.clear();
        vectorEntry.index(1);
        vectorEntry.action(VectorEntryActions.SET);
        vectorEntry.encodeInit(encodeIter, 0);

        // Encode Element List describing server
        elementList.clear();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);

        elementList.encodeInit(encodeIter, null, 0);
        elementEntry.clear();

        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(ElementNames.HOSTNAME);
        tmpBuffer.clear();
        tmpBuffer.data("HostName");
        elementEntry.encode(encodeIter, tmpBuffer);

        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.PORT);
        tmpUInt.value(1234);
        elementEntry.encode(encodeIter, tmpUInt);

        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.LOAD_FACT);
        tmpUInt.value(5);
        elementEntry.encode(encodeIter, tmpUInt);

        elementList.encodeComplete(encodeIter, true);
        vectorEntry.encodeComplete(encodeIter, true);

        // Complete
        vector.encodeComplete(encodeIter, true);
        elementEntry.encodeComplete(encodeIter, true);
        elementList.encodeComplete(encodeIter, true);

        refreshMsg.encodedDataBody(tmp);
    }

    static void encodeDirectoryServiceList(Msg msg) {

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer tmp = CodecFactory.createBuffer();
        tmp.data(ByteBuffer.allocate(300));
        encIter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());

        Map map = CodecFactory.createMap();
        MapEntry mEntry = CodecFactory.createMapEntry();
        UInt tmpUInt = CodecFactory.createUInt();
        UInt tmpUInt2 = CodecFactory.createUInt();
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();

        map.flags(MapEntryFlags.NONE);
        map.keyPrimitiveType(DataTypes.UINT);
        map.containerType(DataTypes.FILTER_LIST);
        map.encodeInit(encIter, 0, 0);

        mEntry.flags(MapEntryFlags.NONE);
        mEntry.action(MapEntryActions.ADD);
        tmpUInt.value(1);
        mEntry.encodeInit(encIter, tmpUInt, 0);

        filterList.flags(FilterEntryFlags.NONE);
        filterList.containerType(DataTypes.ELEMENT_LIST);
        filterList.encodeInit(encIter);

        filterEntry.clear();
        filterEntry.flags(FilterEntryFlags.NONE);
        filterEntry.id(1);
        filterEntry.action(FilterEntryActions.UPDATE);
        filterEntry.containerType(ELEMENT_LIST);
        filterEntry.encodeInit(encIter, 0);

        elementList.clear();
        elementList.applyHasStandardData();
        elementList.encodeInit(encIter, null, 0);

        element.clear();
        tmpUInt.clear();
        element.name(ElementNames.LOAD_FACT);
        element.dataType(DataTypes.UINT);
        tmpUInt2.value(2);
        element.encode(encIter, tmpUInt2);

        elementList.encodeComplete(encIter, true);

        filterEntry.encodeComplete(encIter, true);
        filterList.encodeComplete(encIter, true);

        mEntry.encodeComplete(encIter, true);
        map.encodeComplete(encIter, true);

        msg.encodedDataBody(tmp);
    }

    static void encodeDictionary(RefreshMsg msg) {
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer tmp = CodecFactory.createBuffer();
        tmp.data(ByteBuffer.allocate(1000));
        encodeIter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        /* Basically, dictionary refresh message contains a Series of ElementLists  */
        CodecUtil.encodeSimpleSeries(encodeIter, ELEMENT_LIST, true, true, 3, 3);

        msg.encodedDataBody(tmp);
    }

    static void checkDirectoryServiceList(DecodeIterator decIter) {

        Map map = CodecFactory.createMap();
        MapEntry mEntry = CodecFactory.createMapEntry();
        UInt tmpUInt = CodecFactory.createUInt();
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry element = CodecFactory.createElementEntry();

        assertEquals(SUCCESS, map.decode(decIter));
        assertEquals(DataTypes.UINT, map.keyPrimitiveType());
        assertEquals(FILTER_LIST, map.containerType());
        assertEquals(SUCCESS, mEntry.decode(decIter, tmpUInt));
        assertEquals(1, tmpUInt.toLong());
        assertEquals(SUCCESS, filterList.decode(decIter));
        assertEquals(SUCCESS, filterEntry.decode(decIter));
        assertEquals(1, filterEntry.id());
        assertEquals(FilterEntryActions.UPDATE, filterEntry.action());
        assertEquals(ELEMENT_LIST, filterEntry.containerType());
        assertEquals(SUCCESS, elementList.decode(decIter, null));
        assertEquals(SUCCESS, element.decode(decIter));
        assertTrue(element.name().equals(ElementNames.LOAD_FACT));
        assertTrue(element.dataType() == UINT);
        assertEquals(SUCCESS, tmpUInt.decode(decIter));
        assertEquals(2, tmpUInt.toLong());
    }

    static void checkDictionaryPayload(DecodeIterator decIter) {
        CodecUtil.decodeAndCheckSeries(decIter, ELEMENT_LIST, 3, 3, true, true);
    }

    static void checkMsgStatusKeyAttrib(MsgKey msgKey) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(msgKey.encodedAttrib(), Codec.majorVersion(), Codec.minorVersion());
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Buffer buf = CodecFactory.createBuffer();
        UInt uint = CodecFactory.createUInt();
        elementList.decode(decIter, null);
        int entriesCount = 0;
        while (elementEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
            if (elementEntry.name().equals(ElementNames.AUTHN_ERROR_CODE)) {
                assertTrue(elementEntry.dataType() == DataTypes.UINT);
                uint.decode(decIter);
                assertTrue(uint.toLong() == 5);
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.AUTHN_ERROR_TEXT)) {
                assertTrue(elementEntry.dataType() == ASCII_STRING);
                buf.decode(decIter);
                assertTrue(buf.toString().equals("Error text"));
                entriesCount++;
            }
        }
        assertEquals(2, entriesCount);
    }

    static void checkMsgLoginKeyAttrib(MsgKey msgKey) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(msgKey.encodedAttrib(), Codec.majorVersion(), Codec.minorVersion());
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        UInt uint = CodecFactory.createUInt();
        elementList.decode(decIter, null);
        int entriesCount = 0;
        while (elementEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
            if (elementEntry.name().equals(ElementNames.AUTHN_TT_REISSUE)) {
                assertTrue(elementEntry.dataType() == DataTypes.UINT);
                uint.decode(decIter);
                assertTrue(uint.toLong() == 5);
                entriesCount++;
            } else if (elementEntry.name().equals(ElementNames.AUTHN_ERROR_CODE)) {
                assertTrue(elementEntry.dataType() == DataTypes.UINT);
                uint.decode(decIter);
                assertTrue(uint.toLong() == 10);
                entriesCount++;
            }
        }
        assertEquals(2, entriesCount);
    }

    static void checkConnectionInfo(DecodeIterator decIter) {

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        UInt tmpUInt = CodecFactory.createUInt();
        Buffer tmpBuffer = CodecFactory.createBuffer();

        assertEquals(SUCCESS, elementList.decode(decIter, null));
        assertEquals(SUCCESS, elementEntry.decode(decIter));
        assertEquals(VECTOR, elementEntry.dataType());
        assertTrue(elementEntry.name().equals(ElementNames.CONNECTION_CONFIG));
        assertEquals(SUCCESS, vector.decode(decIter));
        assertEquals(ELEMENT_LIST, vector.containerType());

        elementList.clear();
        assertEquals(SUCCESS, elementList.decode(decIter, null));
        elementEntry.clear();
        assertEquals(SUCCESS, elementEntry.decode(decIter));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertTrue(elementEntry.name().equals(ElementNames.NUM_STANDBY_SERVERS));
        assertEquals(SUCCESS, tmpUInt.decode(decIter));
        assertEquals(3, tmpUInt.toLong());
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(decIter));

        vectorEntry.clear();
        assertEquals(SUCCESS, vectorEntry.decode(decIter));
        assertEquals(1, vectorEntry.index());
        assertEquals(VectorEntryActions.SET, vectorEntry.action());

        elementEntry.clear();
        elementList.clear();
        assertEquals(SUCCESS, elementList.decode(decIter, null));
        int count = 0;
        while (elementEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
            if (elementEntry.name().equals(ElementNames.HOSTNAME)) {
                assertEquals(ASCII_STRING, elementEntry.dataType());
                assertEquals(SUCCESS, tmpBuffer.decode(decIter));
                count++;
            } else if (elementEntry.name().equals(ElementNames.PORT)) {
                assertEquals(UINT, elementEntry.dataType());
                assertEquals(SUCCESS, tmpUInt.decode(decIter));
                count++;
            } if (elementEntry.name().equals(ElementNames.LOAD_FACT)) {
                assertEquals(UINT, elementEntry.dataType());
                assertEquals(SUCCESS, tmpUInt.decode(decIter));
                count++;
            }
        }
        assertEquals(3, count);
    }

    static void encodeBasicMsg(EncodeIterator encIter) {

        Msg msg = CodecFactory.createMsg();
        msg.streamId(MsgClasses.UPDATE);
        msg.msgClass(MsgClasses.UPDATE);
        msg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.FIELD_LIST);
        encodeSampleFieldList(msg);
        assertEquals(SUCCESS, msg.encode(encIter));
    }

    static void encodeMap(Msg msg) {
        EncodeIterator iter = CodecFactory.createEncodeIterator();

        Map msgMap = CodecFactory.createMap();
        MapEntry msgMapEntry = CodecFactory.createMapEntry();

        Buffer dataBuffer = CodecFactory.createBuffer();

        Buffer msgBuf = CodecFactory.createBuffer();
        msgBuf.data(ByteBuffer.allocate(2048));
        /* encode message */
        iter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());

        msgMap.flags(0);
        msgMap.containerType(DataTypes.FIELD_LIST);
        msgMap.keyPrimitiveType(DataTypes.ASCII_STRING);
        assertEquals(SUCCESS, msgMap.encodeInit(iter, 0, 0));
        for (int i = 0; i < ACTION_COUNT; i++) {
            msgMapEntry.clear();
            msgMapEntry.flags(MapEntryFlags.NONE);
            msgMapEntry.action(i + 1);
            for (String data : MAP_DATA_ARRAY)
            dataBuffer.data(data);
            assertEquals(SUCCESS, msgMapEntry.encode(iter, dataBuffer));
        }
        assertEquals(SUCCESS, msgMap.encodeComplete(iter, true));
        msg.encodedDataBody(msgBuf);
    }

    static void checkJsonErrorMsg(Buffer errorBuffer) throws IOException {
        JsonNode errorNode = mapper.readTree(errorBuffer.data().array(), errorBuffer.position(), errorBuffer.position() + errorBuffer.length());
        assertFalse(errorNode.isMissingNode());
        assertFalse(errorNode.path(ConstCharArrays.JSON_ID).isMissingNode());
        assertFalse(errorNode.path(ConstCharArrays.JSON_TYPE).isMissingNode());
        assertEquals(errorNode.path(ConstCharArrays.JSON_TYPE).asText(), ConstCharArrays.JSON_ERROR);
        JsonNode debug = errorNode.path(ConstCharArrays.JSON_DEBUG);
        assertFalse(debug.isMissingNode());
        assertFalse(debug.path(ConstCharArrays.JSON_FILE).isMissingNode());
        assertFalse(debug.path(ConstCharArrays.JSON_LINE).isMissingNode());
        assertFalse(debug.path(ConstCharArrays.JSON_MESSAGE).isMissingNode());
    }
}
