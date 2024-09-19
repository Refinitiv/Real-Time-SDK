/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class ContainersConverterTest {

    JsonConverterError convError;
    JsonAbstractConverter converter;

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        convError = ConverterFactory.createJsonConverterError();

        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setDictionary(dictionary)
                .build(convError);
    }

    @Test
    public void testElementListToJson() {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();

        ElementList container = CodecFactory.createElementList();
        ElementEntry entry = CodecFactory.createElementEntry();
        Buffer name = CodecFactory.createBuffer();

        encIter.clear();
        buffer.data(ByteBuffer.allocate(200));

        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        container.flags(ElementListFlags.HAS_STANDARD_DATA);
        container.encodeInit(encIter, null, 0);

        entry.clear();
        name.clear();
        name.data("real");
        entry.name(name);
        entry.dataType(DataTypes.REAL);
        Real real = CodecFactory.createReal();
        real.value(1234567, RealHints.EXPONENT_4);
        entry.encode(encIter, real);

        entry.clear();
        name.clear();
        name.data("time");
        entry.name(name);
        entry.dataType(DataTypes.TIME);
        Time time = CodecFactory.createTime();
        time.hour(5);
        time.minute(10);
        time.second(12);
        time.millisecond(123);
        time.microsecond(4);
        time.nanosecond(5);
        entry.encode(encIter, time);

        container.encodeComplete(encIter, true);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer output = new JsonBuffer();
        output.data = new byte[300];
        output.position = 1;

        assertTrue(new JsonElementListConverter(converter).encodeJson(decIter, output, true, null, convError));
        String correct = "\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), output.data[i + 1]);
    }

    @Test
    public void testFilterListToJson() {
        FilterList container = CodecFactory.createFilterList();
        FilterEntry entry = CodecFactory.createFilterEntry();
        Buffer permissionData = CodecFactory.createBuffer();
        permissionData.data("permissionData");

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(200));

        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        container.containerType(DataTypes.FIELD_LIST);
        container.applyHasTotalCountHint();
        container.totalCountHint(10);
        container.encodeInit(encIter);

        for (int i = 1; i < 4; i++) {
            entry.clear();
            entry.applyHasPermData();
            entry.permData(permissionData);
            entry.id(i);
            entry.action(i);
            entry.applyHasContainerType();
            entry.containerType(DataTypes.NO_DATA);
            entry.encodeInit(encIter, 0);
            entry.encodeComplete(encIter, true);
        }
        container.encodeComplete(encIter, true);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer output = new JsonBuffer();
        output.data = new byte[300];

        assertTrue(new JsonFilterListConverter(converter).encodeJson(decIter, output, true, null, convError));
        String correct = "\"FilterList\":{\"CountHint\":10,\"Entries\":[{\"ID\":1,\"Action\":\"Update\",\"PermData\":\"cGVybWlzc2lvbkRhdGE=\"},{\"ID\":2,\"Action\":\"Set\",\"PermData\":\"cGVybWlzc2lvbkRhdGE=\"},{\"ID\":3,\"Action\":\"Clear\",\"PermData\":\"cGVybWlzc2lvbkRhdGE=\"}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), output.data[i]);
    }

    @Test
    public void testFieldList() {

        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();

        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        container.flags(FieldListFlags.HAS_STANDARD_DATA);
        container.encodeInit(encIter, null, 0);

        entry.clear();
        entry.fieldId(4);
        entry.dataType(DataTypes.ENUM);
        Enum enumer = CodecFactory.createEnum();
        enumer.value(4);
        entry.encode(encIter, enumer);

        entry.clear();
        entry.fieldId(18);
        entry.dataType(DataTypes.TIME);
        Time time = CodecFactory.createTime();
        time.hour(23);
        time.minute(59);
        time.second(59);
        time.millisecond(999);
        entry.encode(encIter, time);

        container.encodeComplete(encIter, true);

        JsonBuffer output = new JsonBuffer();
        output.data = new byte[10];
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, output, true, null, convError));
        String correct = "\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\"}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), output.data[i]);

        //with ElementList as a field
        container.clear();
        entry.clear();
        decIter.clear();
        encIter.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(200));

        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        container.flags(FieldListFlags.HAS_STANDARD_DATA);
        container.encodeInit(encIter, null, 0);

        entry.clear();
        entry.fieldId(30127);
        entry.dataType(DataTypes.ELEMENT_LIST);

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry listEntry = CodecFactory.createElementEntry();
        EncodeIterator elIter = CodecFactory.createEncodeIterator();
        Buffer elBuf = CodecFactory.createBuffer();
        elBuf.data(ByteBuffer.allocate(200));
        elIter.setBufferAndRWFVersion(elBuf, Codec.majorVersion(), Codec.minorVersion());

        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
        elementList.encodeInit(elIter, null, 0);
        Buffer name = CodecFactory.createBuffer();

        listEntry.clear();
        name.clear();
        name.data("real");
        listEntry.name(name);
        listEntry.dataType(DataTypes.REAL);
        Real real = CodecFactory.createReal();
        real.value(1234567, RealHints.EXPONENT_4);
        listEntry.encode(elIter, real);

        listEntry.clear();
        name.clear();
        name.data("time");
        listEntry.name(name);
        listEntry.dataType(DataTypes.TIME);
        time.clear();
        time.hour(5);
        time.minute(10);
        time.second(12);
        time.millisecond(123);
        time.microsecond(4);
        time.nanosecond(5);
        listEntry.encode(elIter, time);

        elementList.encodeComplete(elIter, true);

        entry.encodedData(elBuf);
        entry.encode(encIter);

        entry.clear();
        entry.fieldId(18);
        entry.dataType(DataTypes.TIME);
        entry.encode(encIter, time);

        container.encodeComplete(encIter, true);

        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        output.data = new byte[10];
        output.position = 0;
        assertTrue(converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, output, true, null, convError));
        correct = "\"Fields\":{\"EX_MET_DAT\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}},\"TRDTIM_1\":\"05:10:12.123004005\"}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)output.data[i]);
    }

    @Test
    public void testVector() {
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(800));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        vector.applyHasSummaryData();
        vector.applyHasTotalCountHint();
        vector.totalCountHint(1);
        vector.containerType(DataTypes.ELEMENT_LIST);

        vector.encodeInit(encIter, 128, 0);

        encodeElementlist(encIter);

        vector.encodeSummaryDataComplete(encIter, true);

        vectorEntry.clear();
        vectorEntry.index(1);
        vectorEntry.flags(VectorEntryFlags.NONE);
        vectorEntry.action(VectorEntryActions.UPDATE);
        vectorEntry.encodeInit(encIter, 150);
        encodeElementlist(encIter);
        vectorEntry.encodeComplete(encIter, true);

        vector.encodeComplete(encIter, true);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer json = new JsonBuffer();
        json.data = new byte[500];

        converter.getContainerHandler(DataTypes.VECTOR).encodeJson(decIter, json, true, null, convError);
        String correct = "\"Vector\":{\"Summary\":{\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}},\"CountHint\":1," +
                "\"Entries\":[{\"Index\":1,\"Action\":\"Update\",\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testSeries() {
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(800));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        DecodeIterator decIter = CodecFactory.createDecodeIterator();

        series.applyHasSummaryData();
        series.applyHasTotalCountHint();
        series.totalCountHint(1);
        series.containerType(DataTypes.ELEMENT_LIST);
        series.encodeInit(encIter, 128, 0);
        encodeElementlist(encIter);
        series.encodeSummaryDataComplete(encIter, true);

        seriesEntry.encodeInit(encIter, 0);
        encodeElementlist(encIter);
        seriesEntry.encodeComplete(encIter, true);
        series.encodeComplete(encIter, true);

        JsonBuffer json = new JsonBuffer();
        json.data = new byte[500];

        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getContainerHandler(DataTypes.SERIES).encodeJson(decIter, json, true, null, convError));
        String correct = "\"Series\":{\"Summary\":{\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}},\"CountHint\":1," +
                "\"Entries\":[{\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testMapEncode() {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(500));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        map.applyHasTotalCountHint();
       // map.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_TOTAL_COUNT_HINT);
        map.containerType(DataTypes.FIELD_LIST);
        map.keyPrimitiveType(DataTypes.UINT);
        map.totalCountHint(3);
        map.encodeInit(encIter, 256, 0 );

        UInt entryKeyUInt = CodecFactory.createUInt();
        Buffer tmp = CodecFactory.createBuffer();

        tmp.data(ByteBuffer.allocate(50));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(13);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.ADD);
        mapEntry.encodedKey(tmp);
        Buffer tmp1 = CodecFactory.createBuffer();
        tmp1.data(ByteBuffer.allocate(200));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp1, Codec.majorVersion(), Codec.minorVersion());
        encodeFieldList(iter);
        mapEntry.encodedData(tmp1);
        mapEntry.encode(encIter);

        tmp.clear();
        tmp.data(ByteBuffer.allocate(50));
        tmp1.clear();
        tmp1.data(ByteBuffer.allocate(200));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(5);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.UPDATE);
        mapEntry.encodedKey(tmp);
        iter.clear();
        iter.setBufferAndRWFVersion(tmp1, Codec.majorVersion(), Codec.minorVersion());
        encodeFieldList(iter);
        mapEntry.encodedData(tmp1);
        mapEntry.encode(encIter);

        tmp.clear();
        tmp.data(ByteBuffer.allocate(50));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(7);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.DELETE);
        mapEntry.encodedKey(tmp);
        mapEntry.encode(encIter);

        map.encodeComplete(encIter, true);

        JsonBuffer json = new JsonBuffer();
        json.data = new byte[700];

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, json, true, null, convError));

        String correct = "\"Map\":{\"KeyType\":\"UInt\",\"CountHint\":3,\"Entries\":[{\"Action\":\"Add\",\"Key\":13," +
                "\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "{\"Action\":\"Update\",\"Key\":5,\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "{\"Action\":\"Delete\",\"Key\":7}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testOpaqueConverter() {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(500));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Buffer input = CodecFactory.createBuffer();
        input.data("This is a test string.");
        input.encode(encIter);

        JsonBuffer json = new JsonBuffer();
        json.data = new byte[700];

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getContainerHandler(DataTypes.OPAQUE).encodeJson(decIter, json, true, null, convError));

        String correct = "\"Opaque\":\"VGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg==\"";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testXMLConverter() {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(500));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Buffer input = CodecFactory.createBuffer();
        input.data("This is a test string.");
        input.encode(encIter);

        JsonBuffer json = new JsonBuffer();
        json.data = new byte[700];

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getContainerHandler(DataTypes.XML).encodeJson(decIter, json, true, null, convError));

        String correct = "\"Xml\":\"This is a test string.\"";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testJSONConverter() {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(500));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Buffer input = CodecFactory.createBuffer();
        input.data("{\"Field1\":\"Value1\",\"Field2\":123}");
        input.encode(encIter);

        JsonBuffer json = new JsonBuffer();
        json.data = new byte[700];

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(converter.getContainerHandler(DataTypes.JSON).encodeJson(decIter, json, true, null, convError));

        String correct = "\"Json\":{\"Field1\":\"Value1\",\"Field2\":123}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

   @Test
    public void testContainerWithSetDefDb() {
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(500));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        map.applyHasTotalCountHint();
        map.applyHasSetDefs();
        map.containerType(DataTypes.FIELD_LIST);
        map.keyPrimitiveType(DataTypes.UINT);
        map.totalCountHint(3);
        map.encodeInit(encIter, 256, 0 );

        LocalFieldSetDefDb setDefDb = getLocalFieldSetDb1();
        setDefDb.encode(encIter);
        map.encodeSetDefsComplete(encIter, true);

        UInt entryKeyUInt = CodecFactory.createUInt();
        Buffer tmp = CodecFactory.createBuffer();

        tmp.data(ByteBuffer.allocate(50));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(13);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.ADD);
        mapEntry.encodedKey(tmp);
        Buffer tmp1 = CodecFactory.createBuffer();
        tmp1.data(ByteBuffer.allocate(200));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp1, Codec.majorVersion(), Codec.minorVersion());
        encodeFieldListForSetDefs(iter);
        mapEntry.encodedData(tmp1);
        mapEntry.encode(encIter);

        tmp.clear();
        tmp.data(ByteBuffer.allocate(50));
        tmp1.clear();
        tmp1.data(ByteBuffer.allocate(200));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(5);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.UPDATE);
        mapEntry.encodedKey(tmp);
        iter.clear();
        iter.setBufferAndRWFVersion(tmp1, Codec.majorVersion(), Codec.minorVersion());
        encodeFieldListForSetDefs(iter);
        mapEntry.encodedData(tmp1);
        mapEntry.encode(encIter);

        tmp.clear();
        tmp.data(ByteBuffer.allocate(50));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(7);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.DELETE);
        mapEntry.encodedKey(tmp);
        mapEntry.encode(encIter);

        map.encodeComplete(encIter, true);

        JsonBuffer json = new JsonBuffer();
        json.data = new byte[700];

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, json, true, null, convError);
        String correct = "\"Map\":{\"KeyType\":\"UInt\",\"CountHint\":3,\"Entries\":[{\"Action\":\"Add\",\"Key\":13," +
                "\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "{\"Action\":\"Update\",\"Key\":5,\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "{\"Action\":\"Delete\",\"Key\":7}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    private LocalFieldSetDefDb getLocalFieldSetDb1() {
        LocalFieldSetDefDb fsetDb = CodecFactory.createLocalFieldSetDefDb();
        FieldSetDefEntry[] entries = new FieldSetDefEntry[3];

        entries[0] = CodecFactory.createFieldSetDefEntry();
        entries[0].fieldId(4);
        entries[0].dataType(DataTypes.ENUM);

        entries[1] = CodecFactory.createFieldSetDefEntry();
        entries[1].fieldId(18);
        entries[1].dataType(DataTypes.TIME);

        entries[2] = CodecFactory.createFieldSetDefEntry();
        entries[2].fieldId(16);
        entries[2].dataType(DataTypes.DATE);

        fsetDb.definitions()[0].setId(0);
        fsetDb.definitions()[0].count(3);
        fsetDb.definitions()[0].entries(entries);

        return fsetDb;
    }

    @Test
    public void testMapVectorElementListContainer() {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        int[] containerTypes = new int[] { DataTypes.MAP, DataTypes.VECTOR, DataTypes.ELEMENT_LIST };
        int current = 0;

        encodeContainer(encIter, containerTypes, current);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer json = new JsonBuffer();
        json.data = new byte[3000];

        assertTrue(converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, json, true, null, convError));
        String correct = "\"Map\":{\"KeyType\":\"UInt\",\"CountHint\":3,\"KeyFieldID\":13,\"Entries\":[{\"Action\":\"Add\",\"Key\":13,\"Vector\":{\"Summary\":" +
                "{\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}}," +
                "\"CountHint\":1,\"SupportSorting\":true,\"Entries\":[{\"Index\":1,\"Action\":\"Update\"," +
                "\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}}]}}," +
                "{\"Action\":\"Update\",\"Key\":5,\"Vector\":{\"Summary\":{\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}}," +
                "\"CountHint\":1,\"SupportSorting\":true,\"Entries\":[{\"Index\":1,\"Action\":\"Update\"," +
                "\"Elements\":{\"real\":{\"Type\":\"Real\",\"Data\":123.4567},\"time\":{\"Type\":\"Time\",\"Data\":\"05:10:12.123004005\"}}}]}},{\"Action\":\"Delete\",\"Key\":7}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testMapVectorOpaqueContainer() {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        int[] containerTypes = new int[] { DataTypes.MAP, DataTypes.VECTOR, DataTypes.OPAQUE };
        int current = 0;

        encodeContainer(encIter, containerTypes, current);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer json = new JsonBuffer();
        json.data = new byte[3000];

        assertTrue(converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, json, true, null, convError));
        String correct = "\"Map\":{\"KeyType\":\"UInt\",\"CountHint\":3,\"KeyFieldID\":13," +
                "\"Entries\":[{\"Action\":\"Add\",\"Key\":13," +
                "\"Vector\":{\"Summary\":{\"Opaque\":\"VGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg==\"}," +
                "\"CountHint\":1,\"SupportSorting\":true," +
                "\"Entries\":[{\"Index\":1,\"Action\":\"Update\",\"Opaque\":\"VGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg==\"}]}}," +
                "{\"Action\":\"Update\",\"Key\":5," +
                "\"Vector\":{\"Summary\":{\"Opaque\":\"VGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg==\"},\"CountHint\":1,\"SupportSorting\":true," +
                "\"Entries\":[{\"Index\":1,\"Action\":\"Update\",\"Opaque\":\"VGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg==\"}]}}," +
                "{\"Action\":\"Delete\",\"Key\":7}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    @Test
    public void testMapVectorSeriesFieldListContainer() {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(2000));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

        int[] containerTypes = new int[] { DataTypes.MAP, DataTypes.VECTOR, DataTypes.SERIES, DataTypes.FIELD_LIST };
        int current = 0;

        encodeContainer(encIter, containerTypes, current);

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer json = new JsonBuffer();
        json.data = new byte[3000];

        assertTrue(converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, json, true, null, convError));
        String correct = "\"Map\":{\"KeyType\":\"UInt\",\"CountHint\":3,\"KeyFieldID\":13,\"Entries\":[{\"Action\":\"Add\",\"Key\":13,\"Vector\":{\"Summary\":" +
                "{\"Series\":{\"Summary\":{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "\"CountHint\":1,\"Entries\":[{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}]}}," +
                "\"CountHint\":1,\"SupportSorting\":true,\"Entries\":[{\"Index\":1,\"Action\":\"Update\",\"Series\":{\"Summary\":" +
                "{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}},\"CountHint\":1," +
                "\"Entries\":[{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}]}}]}}," +
                "{\"Action\":\"Update\",\"Key\":5,\"Vector\":{\"Summary\":{\"Series\":{\"Summary\":{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "\"CountHint\":1,\"Entries\":[{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}]}},\"CountHint\":1,\"SupportSorting\":true," +
                "\"Entries\":[{\"Index\":1,\"Action\":\"Update\",\"Series\":{\"Summary\":{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}," +
                "\"CountHint\":1,\"Entries\":[{\"Fields\":{\"RDN_EXCHID\":4,\"TRDTIM_1\":\"23:59:59.999\",\"TRADE_DATE\":\"1999-12-23\"}}]}}]}},{\"Action\":\"Delete\",\"Key\":7}]}";
        for (int i = 0; i < correct.length(); i++)
            assertEquals(correct.charAt(i), (char)json.data[i]);
    }

    private void encodeElementlist(EncodeIterator encIter) {

        ElementList container = CodecFactory.createElementList();
        ElementEntry entry = CodecFactory.createElementEntry();
        Buffer name = CodecFactory.createBuffer();

        container.flags(ElementListFlags.HAS_STANDARD_DATA);
        container.encodeInit(encIter, null, 0);

        entry.clear();
        name.clear();
        name.data("real");
        entry.name(name);
        entry.dataType(DataTypes.REAL);
        Real real = CodecFactory.createReal();
        real.value(1234567, RealHints.EXPONENT_4);
        entry.encode(encIter, real);

        entry.clear();
        name.clear();
        name.data("time");
        entry.name(name);
        entry.dataType(DataTypes.TIME);
        Time time = CodecFactory.createTime();
        time.hour(5);
        time.minute(10);
        time.second(12);
        time.millisecond(123);
        time.microsecond(4);
        time.nanosecond(5);
        entry.encode(encIter, time);

        container.encodeComplete(encIter, true);
    }

    private void encodeFieldList(EncodeIterator encIter) {

        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        container.flags(FieldListFlags.HAS_STANDARD_DATA);
        container.encodeInit(encIter, null, 0);

        entry.clear();
        entry.fieldId(4);
        entry.dataType(DataTypes.ENUM);
        Enum enumer = CodecFactory.createEnum();
        enumer.value(4);
        entry.encode(encIter, enumer);

        entry.clear();
        entry.fieldId(18);
        entry.dataType(DataTypes.TIME);
        Time time = CodecFactory.createTime();
        time.hour(23);
        time.minute(59);
        time.second(59);
        time.millisecond(999);
        entry.encode(encIter, time);

        entry.clear();
        entry.fieldId(16);
        entry.dataType(DataTypes.DATE);
        Date date = CodecFactory.createDate();
        date.year(1999);
        date.month(12);
        date.day(23);
        entry.encode(encIter, date);

        container.encodeComplete(encIter, true);
    }

    private void encodeFieldListForSetDefs(EncodeIterator encIter) {

        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();
        container.applyHasSetData();
        container.applyHasStandardData();
        container.applyHasSetId();
        container.setId(0);
        container.encodeInit(encIter, getLocalFieldSetDb1(), 0);

        entry.clear();
        entry.fieldId(4);
        entry.dataType(DataTypes.ENUM);
        Enum enumer = CodecFactory.createEnum();
        enumer.value(4);
        entry.encode(encIter, enumer);

        entry.clear();
        Time time = CodecFactory.createTime();
        entry.fieldId(18);
        entry.dataType(DataTypes.TIME);
        time.hour(23);
        time.minute(59);
        time.second(59);
        time.millisecond(999);
        entry.encode(encIter, time);

        entry.clear();
        Date date = CodecFactory.createDate();
        entry.fieldId(16);
        entry.dataType(DataTypes.DATE);
        date.year(1999);
        date.month(12);
        date.day(23);
        entry.encode(encIter, date);

        container.encodeComplete(encIter, true);
    }

    private void encodeVector(EncodeIterator encIter, int[] containerTypes, int currType) {

        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();

        vector.applyHasSummaryData();
        vector.applySupportsSorting();
        vector.applyHasTotalCountHint();
        vector.totalCountHint(1);
        vector.containerType(containerTypes[currType + 1]);

        vector.encodeInit(encIter, 128, 0);

        encodeContainer(encIter, containerTypes, currType + 1);

        vector.encodeSummaryDataComplete(encIter, true);

        vectorEntry.clear();
        vectorEntry.applyHasPermData();
        Buffer pd = CodecFactory.createBuffer();
        pd.data("permission data string");
        vectorEntry.permData();
        vectorEntry.index(1);
        vectorEntry.flags(VectorEntryFlags.NONE);
        vectorEntry.action(VectorEntryActions.UPDATE);
        vectorEntry.encodeInit(encIter, 150);

        encodeContainer(encIter, containerTypes, currType + 1);

        vectorEntry.encodeComplete(encIter, true);
        vector.encodeComplete(encIter, true);
    }

    private void encodeSeries(EncodeIterator encIter, int[] containerTypes, int currType) {

        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

        series.applyHasSummaryData();
        series.applyHasTotalCountHint();
        series.totalCountHint(1);
        series.containerType(containerTypes[currType + 1]);
        series.encodeInit(encIter, 128, 0);
        encodeContainer(encIter, containerTypes, currType + 1);
        series.encodeSummaryDataComplete(encIter, true);

        seriesEntry.encodeInit(encIter, 0);
        encodeContainer(encIter, containerTypes, currType + 1);
        seriesEntry.encodeComplete(encIter, true);
        series.encodeComplete(encIter, true);
    }

    private void encodeMap(EncodeIterator encIter, int[] containerTypes, int currType) {

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        map.applyHasTotalCountHint();
        map.containerType(containerTypes[currType + 1]);
        map.keyPrimitiveType(DataTypes.UINT);
        map.applyHasKeyFieldId();
        map.keyFieldId(13);
        map.totalCountHint(3);
        map.encodeInit(encIter, 256, 0 );

        UInt entryKeyUInt = CodecFactory.createUInt();
        Buffer tmp = CodecFactory.createBuffer();

        tmp.data(ByteBuffer.allocate(50));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(13);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.ADD);
        mapEntry.encodedKey(tmp);
        Buffer tmp1 = CodecFactory.createBuffer();
        tmp1.data(ByteBuffer.allocate(200));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp1, Codec.majorVersion(), Codec.minorVersion());
        encodeContainer(iter, containerTypes, currType + 1);
        mapEntry.encodedData(tmp1);
        mapEntry.encode(encIter);

        tmp.clear();
        tmp.data(ByteBuffer.allocate(50));
        tmp1.clear();
        tmp1.data(ByteBuffer.allocate(200));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(5);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.UPDATE);
        mapEntry.encodedKey(tmp);
        iter.clear();
        iter.setBufferAndRWFVersion(tmp1, Codec.majorVersion(), Codec.minorVersion());
        encodeContainer(iter, containerTypes, currType + 1);
        mapEntry.encodedData(tmp1);
        mapEntry.encode(encIter);

        tmp.clear();
        tmp.data(ByteBuffer.allocate(50));
        iter.clear();
        iter.setBufferAndRWFVersion(tmp, Codec.majorVersion(), Codec.minorVersion());
        entryKeyUInt.value(7);
        entryKeyUInt.encode(iter);
        mapEntry.action(MapEntryActions.DELETE);
        mapEntry.encodedKey(tmp);
        mapEntry.encode(encIter);

        map.encodeComplete(encIter, true);
    }

    private void encodeOpaque(EncodeIterator encIter) {

        Buffer input = CodecFactory.createBuffer();
        input.data("This is a test string.");
        input.encode(encIter);
    }

    private void encodeFilterList(EncodeIterator encIter) {

        FilterList container = CodecFactory.createFilterList();
        FilterEntry entry = CodecFactory.createFilterEntry();
        Buffer permissionData = CodecFactory.createBuffer();
        permissionData.data("permissionData");

        container.containerType(DataTypes.FIELD_LIST);
        container.applyHasTotalCountHint();
        container.totalCountHint(10);
        container.encodeInit(encIter);

        for (int i = 1; i < 4; i++) {
            entry.clear();
            entry.applyHasPermData();
            entry.permData(permissionData);
            entry.id(i);
            entry.action(i);
            entry.applyHasContainerType();
            entry.containerType(DataTypes.NO_DATA);
            entry.encodeInit(encIter, 0);
            entry.encodeComplete(encIter, true);
        }
        container.encodeComplete(encIter, true);
    }

    private void encodeContainer(EncodeIterator encIter, int[] containerTypes, int currType) {

        switch (containerTypes[currType]) {
            case DataTypes.ELEMENT_LIST:
                encodeElementlist(encIter);
                break;
            case DataTypes.FIELD_LIST:
                encodeFieldList(encIter);
                break;
            case DataTypes.VECTOR:
                encodeVector(encIter, containerTypes, currType);
                break;
            case DataTypes.SERIES:
                encodeSeries(encIter, containerTypes, currType);
                break;
            case DataTypes.MAP:
                encodeMap(encIter, containerTypes, currType);
                break;
            case DataTypes.OPAQUE:
                encodeOpaque(encIter);
                break;
            case DataTypes.FILTER_LIST:
                encodeFilterList(encIter);
                break;
            default:
                break;
        }
    }
}
