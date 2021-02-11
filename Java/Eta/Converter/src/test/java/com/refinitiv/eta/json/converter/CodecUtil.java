package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Double;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Float;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import static com.refinitiv.eta.codec.CodecReturnCodes.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class CodecUtil {

    private static int length = 5;
    public static int defaultArrayDataType = DataTypes.QOS;
    public static int mapKeyFieldId = 1;
    public static int defaultMapKeyType = DataTypes.STATE;

    public static boolean[] defaultVectorEntryHasPermData = { true, false, true };
    public static boolean[] defaultMapEntryHasPermData = { true, false, true };
    public static boolean[] defaultFilterEntryHasPermData = { true, false, true };

    public static int defaultSeriesCountHint = 3;
    public static int defaultFilterListCountHint = 3;

    public static int defaultMapContainerType = DataTypes.ELEMENT_LIST;
    public static int defaultSeriesContainerType = DataTypes.ELEMENT_LIST;
    public static int defaultFilterListContainerType = DataTypes.ELEMENT_LIST;
    public static int defaultVectorContainerType = DataTypes.SERIES;

    public static int[] defaultMapEntryActions = { MapEntryActions.UPDATE, MapEntryActions.ADD, MapEntryActions.DELETE };
    public static int[] defaultFilterListActions = { FilterEntryActions.CLEAR, FilterEntryActions.UPDATE, FilterEntryActions.SET };
    public static int[] defaultVectorActions = { VectorEntryActions.UPDATE, VectorEntryActions.INSERT, VectorEntryActions.SET };

    public static int[] defaultFilterListDataTypes = {DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST};
    public static int[] defaultFieldListTypes = { DataTypes.ASCII_STRING, DataTypes.REAL, DataTypes.DATE, DataTypes.TIME };
    public static int[] defaultElementListTypes = { DataTypes.INT, DataTypes.REAL, DataTypes.DATETIME, DataTypes.ARRAY };

    public static Buffer intName = CodecFactory.createBuffer();
    public static Buffer uintName = CodecFactory.createBuffer();
    public static Buffer doubleName = CodecFactory.createBuffer();
    public static Buffer realName = CodecFactory.createBuffer();
    public static Buffer floatName = CodecFactory.createBuffer();
    public static Buffer dateTimeName = CodecFactory.createBuffer();
    public static Buffer dateName = CodecFactory.createBuffer();
    public static Buffer timeName = CodecFactory.createBuffer();
    public static Buffer asciiName = CodecFactory.createBuffer();
    public static Buffer qosName = CodecFactory.createBuffer();
    public static Buffer stateName = CodecFactory.createBuffer();
    public static Buffer arrayName = CodecFactory.createBuffer();
    public static Buffer enumName = CodecFactory.createBuffer();
    public static Buffer elListName = CodecFactory.createBuffer();
    public static Buffer vectorName = CodecFactory.createBuffer();
    public static Buffer seriesName = CodecFactory.createBuffer();
    public static Buffer fieldListName = CodecFactory.createBuffer();
    public static Buffer mapName = CodecFactory.createBuffer();

    public static Int iv = CodecFactory.createInt();
    public static UInt uint = CodecFactory.createUInt();
    public static Real real = CodecFactory.createReal();
    public static Double dv = CodecFactory.createDouble();
    public static Float fv = CodecFactory.createFloat();
    public static DateTime dateTime = CodecFactory.createDateTime();
    public static Date date = CodecFactory.createDate();
    public static Time time = CodecFactory.createTime();
    public static Buffer ascii = CodecFactory.createBuffer();
    public static Qos qos = CodecFactory.createQos();
    public static State state = CodecFactory.createState();
    public static Enum enumer = CodecFactory.createEnum();

    public static final int intFid = 1;
    public static final int realFid = 7;
    public static final int dateFid = 16;
    public static final int timeFid = 18;
    public static final int asciiFid = 30140;
    public static final int elemListFid = 30127;
    public static final int vectorFid = 30141;
    public static final int rmtesFid = 30146;
    public static final int arrayFid = 30126;
    public static final int mapFid = 32469;

    public static Buffer permData = CodecFactory.createBuffer();
    public static Buffer opaqueBuf = CodecFactory.createBuffer();
    public static Buffer jsonBuf = CodecFactory.createBuffer();
    public static Buffer xmlBuf = CodecFactory.createBuffer();

    public static java.util.Map<Integer, String> dataTypeNameMap = new HashMap<>();
    public static java.util.Map<Integer, String> dataTypeBufferNameMap = new HashMap<>();
    public static java.util.Map<Integer, String> elDataTypeNameMap = new HashMap<>();
    public static java.util.Map<Integer, Integer> elDataTypeFidMap = new HashMap<>();
    public static java.util.Map<String, Integer> nameTypeMap = new HashMap<>();

    static {
        intName.data("int");
        uintName.data("uint");
        doubleName.data("double");
        floatName.data("float");
        realName.data("real");
        dateTimeName.data("datetime");
        dateName.data("date");
        timeName.data("time");
        asciiName.data("ascii");
        qosName.data("qos");
        stateName.data("state");
        arrayName.data("array");
        enumName.data("enum");
        elListName.data("elementList");
        vectorName.data("vector");
        seriesName.data("series");
        fieldListName.data("fieldList");
        mapName.data("map");

        byte[] binaryArray = { 3, 3, -67, 101, 98, -64};
        permData.data(ByteBuffer.wrap(binaryArray));
        opaqueBuf.data(ByteBuffer.wrap(binaryArray));
        jsonBuf.data("{\"Field1\":\"Value1\"}");
        xmlBuf.data("Xml data");

        iv.value(13);
        uint.value(1000000);
        dv.value(0.12345);
        real.value(12345, 2);
        fv.value(1.234f);

        date.year(2020);
        date.month(1);
        date.day(2);

        time.hour(0);
        time.minute(0);
        time.second(2);
        time.millisecond(0);
        time.microsecond(2);
        time.nanosecond(500);

        dateTime.hour(13);
        dateTime.minute(55);
        dateTime.second(18);
        dateTime.millisecond(15);
        dateTime.microsecond(1);
        dateTime.nanosecond(3);
        dateTime.year(2020);
        dateTime.month(11);
        dateTime.day(2);

        ascii.data("ascii string");

        qos.rate(QosRates.TIME_CONFLATED);
        qos.timeliness(QosTimeliness.DELAYED);

        Buffer text = CodecFactory.createBuffer();
        text.data("state text");
        state.text(text);
        state.dataState(DataStates.OK);
        state.code(StateCodes.FULL_VIEW_PROVIDED);
        state.streamState(StreamStates.OPEN);

        enumer.value(1);

        dataTypeNameMap.put(DataTypes.INT, "PROD_PERM");
        dataTypeNameMap.put(DataTypes.REAL, "TRDPRC_2");
        dataTypeNameMap.put(DataTypes.DATE, "TRADE_DATE");
        dataTypeNameMap.put(DataTypes.TIME, "TRDTIM_1");
        dataTypeNameMap.put(DataTypes.ELEMENT_LIST, "EX_MET_DAT");
        dataTypeNameMap.put(DataTypes.ASCII_STRING, "CASH_BASIS");
        dataTypeNameMap.put(DataTypes.VECTOR, "CASH_RATES");
        dataTypeNameMap.put(DataTypes.ARRAY, "TENORS");
        dataTypeNameMap.put(DataTypes.MAP, "PRE_MAP001");

        elDataTypeNameMap.put(DataTypes.INT, "int");
        elDataTypeNameMap.put(DataTypes.UINT, "uint");
        elDataTypeNameMap.put(DataTypes.DOUBLE, "double");
        elDataTypeNameMap.put(DataTypes.FLOAT, "float");
        elDataTypeNameMap.put(DataTypes.REAL, "real");
        elDataTypeNameMap.put(DataTypes.ARRAY, "array");
        elDataTypeNameMap.put(DataTypes.QOS, "qos");
        elDataTypeNameMap.put(DataTypes.STATE, "state");
        elDataTypeNameMap.put(DataTypes.ASCII_STRING, "ascii");
        elDataTypeNameMap.put(DataTypes.DATE, "date");
        elDataTypeNameMap.put(DataTypes.TIME, "time");
        elDataTypeNameMap.put(DataTypes.DATETIME, "datetime");
        elDataTypeNameMap.put(DataTypes.MAP, "map");
        elDataTypeNameMap.put(DataTypes.ELEMENT_LIST, "elementList");
        elDataTypeNameMap.put(DataTypes.FIELD_LIST, "fieldList");

        elDataTypeFidMap.put(DataTypes.INT, intFid);
        elDataTypeFidMap.put(DataTypes.REAL, realFid);
        elDataTypeFidMap.put(DataTypes.ARRAY, arrayFid);
        elDataTypeFidMap.put(DataTypes.ASCII_STRING, asciiFid);
        elDataTypeFidMap.put(DataTypes.DATE, dateFid);
        elDataTypeFidMap.put(DataTypes.TIME, timeFid);
        elDataTypeFidMap.put(DataTypes.VECTOR, vectorFid);
        elDataTypeFidMap.put(DataTypes.ELEMENT_LIST, elemListFid);
        elDataTypeFidMap.put(DataTypes.MAP, mapFid);

        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_INT, DataTypes.INT);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_UINT, DataTypes.UINT);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_REAL, DataTypes.REAL);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_ASCII_STRING, DataTypes.ASCII_STRING);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_DOUBLE, DataTypes.DOUBLE);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_FLOAT, DataTypes.FLOAT);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_DATE, DataTypes.DATE);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_TIME, DataTypes.TIME);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_QOS, DataTypes.QOS);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_STATE, DataTypes.STATE);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_DATE_TIME, DataTypes.DATETIME);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_ELEMENT_LIST, DataTypes.ELEMENT_LIST);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_FIELD_LIST, DataTypes.FIELD_LIST);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_FILTER_LIST, DataTypes.FILTER_LIST);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_VECTOR, DataTypes.VECTOR);
        nameTypeMap.put(ConstCharArrays.DATA_TYPE_STR_SERIES, DataTypes.SERIES);
    }

    public static void encodeSimpleArray(EncodeIterator encIter, int dataType) {
        Array array = CodecFactory.createArray();
        ArrayEntry arrEntry = CodecFactory.createArrayEntry();

        array.primitiveType(dataType);
        array.itemLength(0);
        array.encodeInit(encIter);
        for (int i = 0; i < length; i++) {
            arrEntry.clear();
            switch (dataType) {
                case DataTypes.INT:
                    arrEntry.encode(encIter, iv);
                    break;
                case DataTypes.UINT:
                    arrEntry.encode(encIter, uint);
                    break;
                case DataTypes.DOUBLE:
                    arrEntry.encode(encIter, dv);
                    break;
                case DataTypes.FLOAT:
                    arrEntry.encode(encIter, fv);
                    break;
                case DataTypes.REAL:
                    arrEntry.encode(encIter, real);
                    break;
                case DataTypes.ASCII_STRING:
                    arrEntry.encode(encIter, ascii);
                    break;
                case DataTypes.DATE:
                    arrEntry.encode(encIter, date);
                    break;
                case DataTypes.TIME:
                    arrEntry.encode(encIter, time);
                    break;
                case DataTypes.QOS:
                    arrEntry.encode(encIter, qos);
                    break;
                case DataTypes.STATE:
                    arrEntry.encode(encIter, state);
                    break;
                case DataTypes.DATETIME:
                    arrEntry.encode(encIter, dateTime);
                    break;
                default:
                    break;
            }
        }
        array.encodeComplete(encIter, true);
    }

    public static void encodeSimpleFieldList(EncodeIterator encIter, int[] dataTypes) {

        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        fieldList.applyHasStandardData();
        fieldList.encodeInit(encIter, null, 0);
        for (int i = 0; i < dataTypes.length; i++) {
            fieldEntry.clear();
            fieldEntry.dataType(dataTypes[i]);
            fieldEntry.fieldId(elDataTypeFidMap.get(dataTypes[i]));
            switch (dataTypes[i]) {
                case DataTypes.ARRAY:
                    fieldEntry.encodeInit(encIter, 0);
                    encodeSimpleArray(encIter, defaultArrayDataType);
                    fieldEntry.encodeComplete(encIter, true);
                    break;
                case DataTypes.INT:
                    fieldEntry.encode(encIter, iv);
                    break;
                case DataTypes.REAL:
                    fieldEntry.encode(encIter, real);
                    break;
                case DataTypes.DATE:
                    fieldEntry.encode(encIter, date);
                    break;
                case DataTypes.TIME:
                    fieldEntry.encode(encIter, time);
                    break;
                case DataTypes.ASCII_STRING:
                    fieldEntry.encode(encIter, ascii);
                    break;
                case DataTypes.ELEMENT_LIST:
                case DataTypes.VECTOR:
                case DataTypes.MAP:
                    fieldEntry.dataType(dataTypes[i]);
                    fieldEntry.fieldId(elDataTypeFidMap.get(dataTypes[i]));
                    Buffer local = CodecFactory.createBuffer();
                    local.data(ByteBuffer.allocate(10000));
                    EncodeIterator localIter = CodecFactory.createEncodeIterator();
                    localIter.setBufferAndRWFVersion(local, Codec.majorVersion(), Codec.minorVersion());
                    encodeDefaultContainer(localIter, dataTypes[i]);
                    fieldEntry.encodedData(local);
                    fieldEntry.encode(encIter);
                    break;
                default:
                    break;
            }
        }
        fieldList.encodeComplete(encIter, true);
    }

    public static void encodeSimpleVector(EncodeIterator encIter,
                                          int dataType,
                                          boolean hasSummary,
                                          boolean hasTotalCountHint,
                                          boolean supportsSorting,
                                          int[] actions,
                                          boolean[] hasPermData) {
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();

        vector.containerType(dataType);
        if (hasTotalCountHint) {
            vector.applyHasTotalCountHint();
            vector.totalCountHint(actions.length);
        }
        if (hasSummary)
            vector.applyHasSummaryData();
        if (supportsSorting)
            vector.applySupportsSorting();
        vector.encodeInit(encIter, 0, 0);
        if (hasSummary) {
            encodeDefaultContainer(encIter, dataType);
            vector.encodeSummaryDataComplete(encIter, true);
        }

        for (int i = 0; i < actions.length; i++) {
            vectorEntry.clear();
            vectorEntry.index(i);
            vectorEntry.action(actions[i]);
            if (hasPermData[i]) {
                vectorEntry.applyHasPermData();
                vectorEntry.permData(permData);
            }
            Buffer local = CodecFactory.createBuffer();
            local.data(ByteBuffer.allocate(5000));
            EncodeIterator localIter = CodecFactory.createEncodeIterator();
            localIter.setBufferAndRWFVersion(local, Codec.majorVersion(), Codec.minorVersion());
            encodeDefaultContainer(localIter, dataType);
            vectorEntry.encodedData(local);
            vectorEntry.encode(encIter);
        }
        vector.encodeComplete(encIter, true);
    }

    public static void encodeSimpleElementList(EncodeIterator encIter, int[] dataTypes) {
        ElementList elemList = CodecFactory.createElementList();
        ElementEntry elemEntry = CodecFactory.createElementEntry();

        elemList.clear();
        elemList.applyHasStandardData();

        elemList.encodeInit(encIter, null, 0);
        for (int i = 0; i < dataTypes.length; i++) {
            elemEntry.clear();
            switch (dataTypes[i]) {
                case DataTypes.INT:
                    elemEntry.dataType(DataTypes.INT);
                    elemEntry.name(intName);
                    elemEntry.encode(encIter, iv);
                    break;
                case DataTypes.UINT:
                    elemEntry.dataType(DataTypes.UINT);
                    elemEntry.name(uintName);
                    elemEntry.encode(encIter, uint);
                    break;
                case DataTypes.DATE:
                    elemEntry.dataType(DataTypes.DATE);
                    elemEntry.name(dateName);
                    elemEntry.encode(encIter, date);
                    break;
                case DataTypes.TIME:
                    elemEntry.dataType(DataTypes.TIME);
                    elemEntry.name(timeName);
                    elemEntry.encode(encIter, time);
                    break;
                case DataTypes.DATETIME:
                    elemEntry.dataType(DataTypes.DATETIME);
                    elemEntry.name(dateTimeName);
                    elemEntry.encode(encIter, dateTime);
                    break;
                case DataTypes.ASCII_STRING:
                    elemEntry.dataType(DataTypes.ASCII_STRING);
                    elemEntry.name(asciiName);
                    elemEntry.encode(encIter, ascii);
                    break;
                case DataTypes.REAL:
                    elemEntry.dataType(DataTypes.REAL);
                    elemEntry.name(realName);
                    elemEntry.encode(encIter, real);
                    break;
                case DataTypes.QOS:
                    elemEntry.dataType(DataTypes.QOS);
                    elemEntry.name(qosName);
                    elemEntry.encode(encIter, qos);
                    break;
                case DataTypes.STATE:
                    elemEntry.dataType(DataTypes.STATE);
                    elemEntry.name(stateName);
                    elemEntry.encode(encIter, state);
                    break;
                case DataTypes.DOUBLE:
                    elemEntry.dataType(DataTypes.DOUBLE);
                    elemEntry.name(doubleName);
                    elemEntry.encode(encIter, dv);
                    break;
                case DataTypes.FLOAT:
                    elemEntry.dataType(DataTypes.FLOAT);
                    elemEntry.name(floatName);
                    elemEntry.encode(encIter, fv);
                    break;
                case DataTypes.ARRAY:
                    elemEntry.dataType(DataTypes.ARRAY);
                    elemEntry.name(arrayName);
                    elemEntry.encodeInit(encIter, 0);
                    encodeSimpleArray(encIter, defaultArrayDataType);
                    elemEntry.encodeComplete(encIter, true);
                    break;
                case DataTypes.ELEMENT_LIST:
                case DataTypes.VECTOR:
                case DataTypes.SERIES:
                case DataTypes.MAP:
                case DataTypes.FIELD_LIST:
                    elemEntry.dataType(dataTypes[i]);
                    elemEntry.name().data(elDataTypeNameMap.get(dataTypes[i]));
                    elemEntry.encodeInit(encIter, 0);
                    encodeDefaultContainer(encIter, dataTypes[i]);
                    elemEntry.encodeComplete(encIter, true);
                    break;
                default:
                    break;
            }
        }
        elemList.encodeComplete(encIter, true);
    }

    public static void encodeSimpleMap(EncodeIterator encIter,
                                       int containerType,
                                       int[] entryActions,
                                       boolean[] permDataPresent,
                                       int keyType,
                                       boolean hasSummary,
                                       boolean hasKeyFieldId,
                                       boolean hasTotalHintCount) {
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();

        map.containerType(containerType);
        if (hasKeyFieldId) {
            map.applyHasKeyFieldId();
            map.keyFieldId(mapKeyFieldId);
        }
        if (hasTotalHintCount) {
            map.applyHasTotalCountHint();
            map.totalCountHint(entryActions.length);
        }
        map.keyPrimitiveType(keyType);
        if (hasSummary)
            map.applyHasSummaryData();
        map.encodeInit(encIter, 0, 0);
        if (hasSummary) {
            encodeDefaultContainer(encIter, containerType);
            map.encodeSummaryDataComplete(encIter, true);
        }
        for (int i = 0; i < entryActions.length; i++) {
            mapEntry.clear();
            mapEntry.action(entryActions[i]);
            if (permDataPresent[i]) {
                mapEntry.applyHasPermData();
                mapEntry.permData(permData);
            }
            if (entryActions[i] != MapEntryActions.DELETE) {
                encodeMapEntryInit(mapEntry, encIter, keyType);
                encodeDefaultContainer(encIter, containerType);
                mapEntry.encodeComplete(encIter, true);
            } else {
                encodeMapEntry(mapEntry, encIter, keyType);
            }
        }
        map.encodeComplete(encIter, true);
    }

    public static void encodeSimpleSeries(EncodeIterator encIter,
                                          int containerType,
                                          boolean hasCountHint,
                                          boolean hasSummary,
                                          int countHint,
                                          int length) {
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

        series.containerType(containerType);
        if (hasCountHint) {
            series.applyHasTotalCountHint();
            series.totalCountHint(countHint);
        }
        if (hasSummary)
            series.applyHasSummaryData();
        series.encodeInit(encIter, 0, 0);
        if (hasSummary) {
            encodeDefaultContainer(encIter, containerType);
            series.encodeSummaryDataComplete(encIter, true);
        }
        for (int i = 0; i < length; i++) {
            seriesEntry.clear();
            Buffer local = CodecFactory.createBuffer();
            local.data(ByteBuffer.allocate(10000));
            EncodeIterator localIter = CodecFactory.createEncodeIterator();
            localIter.setBufferAndRWFVersion(local, Codec.majorVersion(), Codec.minorVersion());
            encodeDefaultContainer(localIter, containerType);
            seriesEntry.encodedData(local);
            seriesEntry.encode(encIter);
        }
        series.encodeComplete(encIter, true);
    }

    public static void encodeSimpleFilterList(EncodeIterator encIter,
                                              int containerType,
                                              boolean hasCountHint,
                                              int[] filterActions,
                                              int[] dataTypes,
                                              boolean[] permDataPresent,
                                              int countHint) {
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();

        filterList.clear();
        filterList.containerType(containerType);
        if (hasCountHint) {
            filterList.applyHasTotalCountHint();
            filterList.totalCountHint(countHint);
        }
        filterList.encodeInit(encIter);
        for (int i = 0; i < filterActions.length; i++) {
            filterEntry.clear();
            filterEntry.id(i);
            filterEntry.action(filterActions[i]);
            if (dataTypes[i] != DataTypes.UNKNOWN) {
                filterEntry.applyHasContainerType();
                filterEntry.containerType(dataTypes[i]);
            }
            if (permDataPresent[i]) {
                filterEntry.applyHasPermData();
                filterEntry.permData(permData);
            }
            Buffer local = CodecFactory.createBuffer();
            local.data(ByteBuffer.allocate(10000));
            EncodeIterator localIter = CodecFactory.createEncodeIterator();
            localIter.setBufferAndRWFVersion(local, Codec.majorVersion(), Codec.minorVersion());
            encodeDefaultContainer(localIter, dataTypes[i]);
            filterEntry.encodedData(local);
            filterEntry.encode(encIter);
        }
        filterList.encodeComplete(encIter, true);
    }

    public static void encodeDefaultOpaque(EncodeIterator encIter) {

        Buffer dataBuf = CodecFactory.createBuffer();
        encIter.encodeNonRWFInit(dataBuf);
        dataBuf.data().put(opaqueBuf.data().array(), opaqueBuf.position(), opaqueBuf.length());
        encIter.encodeNonRWFComplete(dataBuf, true);
    }

    public static void encodeDefaultJson(EncodeIterator encIter) {

        Buffer dataBuf = CodecFactory.createBuffer();
        encIter.encodeNonRWFInit(dataBuf);
        dataBuf.data().put(jsonBuf.data().array(), jsonBuf.position(), jsonBuf.length());
        encIter.encodeNonRWFComplete(dataBuf, true);
    }

    public static void encodeDefaultXml(EncodeIterator encIter) {

        Buffer dataBuf = CodecFactory.createBuffer();
        encIter.encodeNonRWFInit(dataBuf);
        dataBuf.data().put(xmlBuf.data().array(), xmlBuf.position(), xmlBuf.length());
        encIter.encodeNonRWFComplete(dataBuf, true);
    }

    public static void encodeDefaultContainer(EncodeIterator encIter, int containerType) {
        switch (containerType) {
            case DataTypes.ELEMENT_LIST:
                encodeSimpleElementList(encIter, defaultElementListTypes);
                break;
            case DataTypes.VECTOR:
                encodeSimpleVector(encIter, defaultVectorContainerType, true, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
                break;
            case DataTypes.FIELD_LIST:
                encodeSimpleFieldList(encIter, defaultFieldListTypes);
                break;
            case DataTypes.SERIES:
                encodeSimpleSeries(encIter, defaultSeriesContainerType, true, true, defaultSeriesCountHint, defaultSeriesCountHint);
                break;
            case DataTypes.MAP:
                encodeSimpleMap(encIter, defaultMapContainerType, defaultMapEntryActions, defaultMapEntryHasPermData, defaultMapKeyType, true, true, true);
                break;
            case DataTypes.FILTER_LIST:
                encodeSimpleFilterList(encIter, defaultFilterListContainerType, true, defaultFilterListActions, defaultFilterListDataTypes, defaultFilterEntryHasPermData, defaultFilterListCountHint);
                break;
            case DataTypes.OPAQUE:
                encodeDefaultOpaque(encIter);
                break;
            case DataTypes.JSON:
                encodeDefaultJson(encIter);
                break;
            case DataTypes.XML:
                encodeDefaultXml(encIter);
                break;
            case DataTypes.MSG:
                JsonConverterTestUtils.encodeBasicMsg(encIter);
                break;
            default:
                break;
        }
    }

    public static void decodeAndCheckDefaultOpaque(DecodeIterator decIter) {
        Buffer dataBuf = CodecFactory.createBuffer();
        dataBuf.decode(decIter);
        assertEquals(dataBuf.toString(), opaqueBuf.toString());
    }
    public static void decodeAndCheckDefaultJson(DecodeIterator decIter) {
        Buffer dataBuf = CodecFactory.createBuffer();
        dataBuf.decode(decIter);
        assertEquals(dataBuf.toString(), jsonBuf.toString());
    }
    public static void decodeAndCheckDefaultXml(DecodeIterator decIter) {
        Buffer dataBuf = CodecFactory.createBuffer();
        dataBuf.decode(decIter);
        assertEquals(dataBuf.toString(), xmlBuf.toString());
    }
    public static void decodeAndCheckDefaultElementList(DecodeIterator decIter) {
        ElementList elList = CodecFactory.createElementList();
        ElementEntry elEntry = CodecFactory.createElementEntry();

        int ret = elList.decode(decIter, null);
        assertTrue(ret >= SUCCESS);
        if (ret != NO_DATA) {
            int i = 0;
            while (elEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
                assertEquals(elEntry.dataType(), defaultElementListTypes[i]);
                assertTrue(elEntry.name().toString().equals(elDataTypeNameMap.get(elEntry.dataType())));
                if (elEntry.encodedData().length() > 0) {
                    if (defaultElementListTypes[i] < DataTypes.SET_PRIMITIVE_MAX && defaultElementListTypes[i] != DataTypes.ARRAY)
                        decodeAndCheckDefaultPrimitiveType(decIter, defaultElementListTypes[i]);
                    else if (defaultElementListTypes[i] == DataTypes.ARRAY)
                        decodeAndCheckArray(decIter, defaultArrayDataType, length);
                    else
                        decodeAndCheckDefaultContainer(decIter, defaultElementListTypes[i]);
                }
                elEntry.clear();
                i++;
            }
        }
    }
    public static void decodeAndCheckDefaultVector(DecodeIterator decIter) {
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();

        assertEquals(vector.decode(decIter), SUCCESS);
        assertEquals(vector.containerType(), defaultVectorContainerType);
        assertTrue(vector.checkHasSummaryData());
        assertTrue(vector.checkHasTotalCountHint());
        assertEquals(vector.totalCountHint(), CodecUtil.defaultVectorActions.length);
        decodeAndCheckDefaultElementList(decIter);
        Set<Integer> actionsPresent = new HashSet<>();
        int j = 0;
        while (vectorEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
            actionsPresent.add(vectorEntry.action());
            decodeAndCheckDefaultElementList(decIter);
            assertEquals(defaultVectorEntryHasPermData[j], vectorEntry.checkHasPermData());
            if (vectorEntry.checkHasPermData())
                assertTrue(vectorEntry.permData().equals(permData));
            vectorEntry.clear();
            j++;
        }
        for (int i = 0; i < CodecUtil.defaultVectorActions.length; i++)
            assertTrue(actionsPresent.contains(CodecUtil.defaultVectorActions[i]));
    }
    public static void decodeAndCheckDefaultMap(DecodeIterator decIter) {
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();

        assertEquals(SUCCESS, map.decode(decIter));
        assertTrue(map.checkHasSummaryData());
        assertTrue(map.checkHasTotalCountHint());
        assertTrue(map.checkHasTotalCountHint());
        assertEquals(map.totalCountHint(), defaultMapEntryActions.length);
        assertEquals(map.keyPrimitiveType(), defaultMapKeyType);
        decodeAndCheckDefaultContainer(decIter, map.containerType()); //summary data
        int i = 0;
        while (mapEntry.decode(decIter, null) != END_OF_CONTAINER) {
            assertEquals(defaultMapEntryActions[i], mapEntry.action());
            decodeAndCheckDefaultPrimitiveType(mapEntry.encodedKey(), map.keyPrimitiveType());
            assertEquals(defaultMapEntryHasPermData[i], mapEntry.checkHasPermData());
            if (defaultMapEntryHasPermData[i]) {
                assertTrue(mapEntry.permData().equals(permData));
            }
            if (mapEntry.encodedData().length() > 0) {
                decodeAndCheckDefaultContainer(decIter, map.containerType());
            }
            i++;
            mapEntry.clear();
        }
    }
    public static void decodeAndCheckDefaultFilterList(DecodeIterator decIter) {
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();

        assertTrue(filterList.decode(decIter) >= SUCCESS);
        assertTrue(filterList.checkHasTotalCountHint());
        assertEquals(filterList.totalCountHint(), defaultFilterListCountHint);
        assertEquals(filterList.containerType(), defaultFilterListContainerType);
        int i = 0;
        while (filterEntry.decode(decIter) != END_OF_CONTAINER) {
            assertEquals(filterEntry.action(), defaultFilterListActions[i]);
            assertEquals(filterEntry.checkHasPermData(), defaultFilterEntryHasPermData[i]);
            if (filterEntry.checkHasPermData())
                assertTrue(filterEntry.permData().equals(permData));
            if (defaultFilterListDataTypes[i] != DataTypes.UNKNOWN) {
                assertTrue(filterEntry.checkHasContainerType());
                assertEquals(filterEntry.containerType(), defaultFilterListDataTypes[i]);
            }
            if (filterEntry.encodedData().length() > 0)
                decodeAndCheckDefaultContainer(decIter, defaultFilterListDataTypes[i] != DataTypes.UNKNOWN ? defaultFilterListDataTypes[i] : filterList.containerType());
            filterEntry.clear();
            i++;
        }
    }
    public static void decodeAndCheckDefaultFieldList(DecodeIterator decIter) {

        Set<Integer> dataPresent = new HashSet<>();

        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();

        int ret = fieldList.decode(decIter, null);
        assertEquals(SUCCESS, ret);
        ret = fieldEntry.decode(decIter);
        while (ret != CodecReturnCodes.END_OF_CONTAINER) {
            dataPresent.add(fieldEntry.fieldId());
            switch (fieldEntry.fieldId()) {
                case CodecUtil.arrayFid:
                    Array arr = CodecFactory.createArray();
                    ArrayEntry arrEntry = CodecFactory.createArrayEntry();
                    assertEquals(SUCCESS, arr.decode(decIter));
                    assertEquals(arr.primitiveType(), CodecUtil.defaultArrayDataType);
                    Qos qos = CodecFactory.createQos();
                    while (arrEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
                        qos.clear();
                        qos.decode(decIter);
                        assertTrue(qos.equals(CodecUtil.qos));
                    }
                    break;
                case CodecUtil.intFid:
                    Int intv = CodecFactory.createInt();
                    assertEquals(SUCCESS, intv.decode(decIter));
                    assertTrue(intv.equals(CodecUtil.iv));
                    break;
                case CodecUtil.realFid:
                    Real real = CodecFactory.createReal();
                    assertEquals(SUCCESS, real.decode(decIter));
                    break;
                case CodecUtil.timeFid:
                    Time time = CodecFactory.createTime();
                    assertEquals(SUCCESS, time.decode(decIter));
                    assertTrue(time.equals(CodecUtil.time));
                    break;
                case CodecUtil.dateFid:
                    Date date = CodecFactory.createDate();
                    assertEquals(SUCCESS, date.decode(decIter));
                    assertTrue(date.equals(CodecUtil.date));
                    break;
                case CodecUtil.asciiFid:
                    Buffer ascii = CodecFactory.createBuffer();
                    assertEquals(SUCCESS, ascii.decode(decIter));
                    assertTrue(ascii.equals(CodecUtil.ascii));
                    break;
                case CodecUtil.elemListFid:
                    CodecUtil.decodeAndCheckDefaultElementList(decIter);
                    break;
                case CodecUtil.vectorFid:
                    CodecUtil.decodeAndCheckDefaultVector(decIter);
                    break;
                case CodecUtil.mapFid:
                    CodecUtil.decodeAndCheckDefaultMap(decIter);
                    break;
                default:
                    break;
            }

            fieldEntry.clear();
            ret = fieldEntry.decode(decIter);
        }
    }
    public static void decodeAndCheckDefaultSeries(DecodeIterator decIter) {
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

        series.decode(decIter);
        assertEquals(series.containerType(), defaultSeriesContainerType);
        assertEquals(series.totalCountHint(), defaultSeriesCountHint);
        decodeAndCheckDefaultContainer(decIter, series.containerType());
        while (seriesEntry.decode(decIter) != END_OF_CONTAINER) {
            decodeAndCheckDefaultContainer(decIter, series.containerType());
            seriesEntry.clear();
        }
    }
    public static void decodeAndCheckBasicMessage(DecodeIterator decIter) {
        Msg message = CodecFactory.createMsg();
        assertEquals(SUCCESS, message.decode(decIter));
        assertEquals(MsgClasses.UPDATE, message.streamId());
        assertEquals(MsgClasses.UPDATE, message.msgClass());
        assertEquals(message.domainType(), DomainTypes.MARKET_PRICE);
        assertEquals(message.containerType(), DataTypes.FIELD_LIST);
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        assertEquals(SUCCESS, fieldList.decode(decIter, null));
        int ret = fieldEntry.decode(decIter);
        while (ret != END_OF_CONTAINER) {
            assertEquals(SUCCESS, ret);
            ret = fieldEntry.decode(decIter);
        }
    }

    public static void decodeAndCheckDefaultContainer(DecodeIterator decIter, int dataType) {
        switch (dataType) {
            case DataTypes.ELEMENT_LIST:
                decodeAndCheckDefaultElementList(decIter);
                break;
            case DataTypes.VECTOR:
                decodeAndCheckDefaultVector(decIter);
                break;
            case DataTypes.FIELD_LIST:
                decodeAndCheckDefaultFieldList(decIter);
                break;
            case DataTypes.FILTER_LIST:
                decodeAndCheckDefaultFilterList(decIter);
                break;
            case DataTypes.MAP:
                decodeAndCheckDefaultMap(decIter);
                break;
            case DataTypes.SERIES:
                decodeAndCheckDefaultSeries(decIter);
                break;
            case DataTypes.OPAQUE:
                decodeAndCheckDefaultOpaque(decIter);
                break;
            case DataTypes.JSON:
                decodeAndCheckDefaultJson(decIter);
                break;
            case DataTypes.XML:
                decodeAndCheckDefaultXml(decIter);
                break;
            case DataTypes.MSG:
                decodeAndCheckBasicMessage(decIter);
                break;
            default:
                break;
        }
    }
    public static void decodeAndCheckDefaultPrimitiveType(Buffer keyBuffer, int dataType) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(keyBuffer, Codec.majorVersion(), Codec.minorVersion());
        switch (dataType) {
            case DataTypes.INT:
                Int intval = CodecFactory.createInt();
                assertEquals(SUCCESS, intval.decode(decIter));
                assertTrue(intval.equals(iv));
                break;
            case DataTypes.UINT:
                UInt uintval = CodecFactory.createUInt();
                assertEquals(SUCCESS, uintval.decode(decIter));
                assertTrue(uintval.equals(uint));
                break;
            case DataTypes.DATE:
                Date dateval = CodecFactory.createDate();
                assertEquals(SUCCESS, dateval.decode(decIter));
                assertTrue(dateval.equals(date));
                break;
            case DataTypes.TIME:
                Time timeval = CodecFactory.createTime();
                assertEquals(SUCCESS, timeval.decode(decIter));
                assertTrue(timeval.equals(time));
                break;
            case DataTypes.DATETIME:
                DateTime datetimeval = CodecFactory.createDateTime();
                assertEquals(SUCCESS, datetimeval.decode(decIter));
                assertTrue(datetimeval.equals(dateTime));
                break;
            case DataTypes.QOS:
                Qos qosval = CodecFactory.createQos();
                assertEquals(SUCCESS, qosval.decode(decIter));
                assertTrue(qosval.equals(qos));
                break;
            case DataTypes.STATE:
                State stateval = CodecFactory.createState();
                assertEquals(SUCCESS, stateval.decode(decIter));
                assertTrue(stateval.equals(state));
                break;
            case DataTypes.DOUBLE:
                Double doubleval = CodecFactory.createDouble();
                assertEquals(SUCCESS, doubleval.decode(decIter));
                assertTrue(doubleval.equals(dv));
                break;
            case DataTypes.FLOAT:
                Float floatval = CodecFactory.createFloat();
                assertEquals(SUCCESS, floatval.decode(decIter));
                assertTrue(floatval.equals(fv));
                break;
            case DataTypes.REAL:
                Real realval = CodecFactory.createReal();
                assertEquals(SUCCESS, realval.decode(decIter));
                assertTrue(realval.equals(real));
                break;
            case DataTypes.ASCII_STRING:
                Buffer asciival = CodecFactory.createBuffer();
                assertEquals(SUCCESS, asciival.decode(decIter));
                assertTrue(asciival.equals(ascii));
                break;
            default:
                break;
        }
    }
    public static void decodeAndCheckDefaultPrimitiveType(DecodeIterator decIter, int dataType) {
        switch (dataType) {
            case DataTypes.INT:
                Int intval = CodecFactory.createInt();
                assertEquals(SUCCESS, intval.decode(decIter));
                assertTrue(intval.equals(iv));
                break;
            case DataTypes.UINT:
                UInt uintval = CodecFactory.createUInt();
                assertEquals(SUCCESS, uintval.decode(decIter));
                assertTrue(uintval.equals(uint));
                break;
            case DataTypes.DATE:
                Date dateval = CodecFactory.createDate();
                assertEquals(SUCCESS, dateval.decode(decIter));
                assertTrue(dateval.equals(date));
                break;
            case DataTypes.TIME:
                Time timeval = CodecFactory.createTime();
                assertEquals(SUCCESS, timeval.decode(decIter));
                assertTrue(timeval.equals(time));
                break;
            case DataTypes.DATETIME:
                DateTime datetimeval = CodecFactory.createDateTime();
                assertEquals(SUCCESS, datetimeval.decode(decIter));
                assertTrue(datetimeval.equals(dateTime));
                break;
            case DataTypes.QOS:
                Qos qosval = CodecFactory.createQos();
                assertEquals(SUCCESS, qosval.decode(decIter));
                assertTrue(qosval.equals(qos));
                break;
            case DataTypes.STATE:
                State stateval = CodecFactory.createState();
                assertEquals(SUCCESS, stateval.decode(decIter));
                assertTrue(stateval.equals(state));
                break;
            case DataTypes.DOUBLE:
                Double doubleval = CodecFactory.createDouble();
                assertEquals(SUCCESS, doubleval.decode(decIter));
                assertTrue(doubleval.equals(dv));
                break;
            case DataTypes.FLOAT:
                Float floatval = CodecFactory.createFloat();
                assertEquals(SUCCESS, floatval.decode(decIter));
                assertTrue(floatval.equals(fv));
                break;
            case DataTypes.REAL:
                Real realval = CodecFactory.createReal();
                assertEquals(SUCCESS, realval.decode(decIter));
                assertTrue(realval.equals(real));
                break;
            case DataTypes.ASCII_STRING:
                Buffer asciival = CodecFactory.createBuffer();
                assertEquals(SUCCESS, asciival.decode(decIter));
                System.out.println(asciival.toString());
                System.out.println(ascii.toString());
                assertTrue(asciival.toString().equals(ascii.toString()));
                break;
            default:
                break;
        }
    }


    //  Decode and check container types
    public static void decodeAndCheckArray(DecodeIterator decIter, int dataType, int length) {
        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        assertTrue(array.decode(decIter) >= SUCCESS);
        assertEquals(dataType, array.primitiveType());
        if (array.encodedData().length() > 0) {
            while (arrayEntry.decode(decIter) != END_OF_CONTAINER) {
                decodeAndCheckDefaultPrimitiveType(decIter, dataType);
                length--;
                arrayEntry.clear();
            }
        }
        assertEquals(length, 0);
    }

    public static void decodeAndCheckSeries(DecodeIterator decIter,
                                            int containerType,
                                            int countHint,
                                            int length,
                                            boolean hasSummary,
                                            boolean hasCountHint) {
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

        series.decode(decIter);
        assertEquals(series.containerType(), containerType);
        assertEquals(hasCountHint, series.checkHasTotalCountHint());
        if (series.checkHasTotalCountHint())
            assertEquals(series.totalCountHint(), countHint);
        assertEquals(hasSummary, series.checkHasSummaryData());
        if (series.checkHasSummaryData())
            decodeAndCheckDefaultContainer(decIter, series.containerType());
        while (seriesEntry.decode(decIter) != END_OF_CONTAINER) {
            decodeAndCheckDefaultContainer(decIter, series.containerType());
            length--;
            seriesEntry.clear();
        }
        assertEquals(0, length);
    }

    public static void decodeAndCheckFilterList(DecodeIterator decIter,
                                                boolean hasCountHint,
                                                int countHint,
                                                int containerType,
                                                int[] filterActions,
                                                int[] filterDataTypes,
                                                boolean[] hasPermData) {
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();

        assertTrue(filterList.decode(decIter) >= SUCCESS);
        assertTrue(filterList.checkHasTotalCountHint() == hasCountHint);
        if (filterList.checkHasTotalCountHint())
            assertEquals(filterList.totalCountHint(), countHint);
        int i = 0;
        while (filterEntry.decode(decIter) != END_OF_CONTAINER) {
            assertEquals(filterEntry.action(), filterActions[i]);
            assertEquals(filterEntry.checkHasPermData(), hasPermData[i]);
            if (filterEntry.checkHasPermData())
                assertTrue(filterEntry.permData().equals(permData));
            if (filterDataTypes[i] != DataTypes.UNKNOWN && filterEntry.action() != FilterEntryActions.CLEAR) {
                assertTrue(filterEntry.checkHasContainerType());
                assertEquals(filterEntry.containerType(), filterDataTypes[i]);
            }
            if (filterEntry.encodedData().length() > 0)
                decodeAndCheckDefaultContainer(decIter, filterDataTypes[i] != DataTypes.UNKNOWN ? filterDataTypes[i] : filterList.containerType());
            filterEntry.clear();
            i++;
        }
    }

    public static void decodeAndCheckElementList(DecodeIterator decIter, int[] elementTypes) {
        ElementList elList = CodecFactory.createElementList();
        ElementEntry elEntry = CodecFactory.createElementEntry();

        int ret = elList.decode(decIter, null);
        assertTrue(ret >= SUCCESS);
        if (ret != NO_DATA) {
            int i = 0;
            while (elEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
                assertEquals(elEntry.dataType(), elementTypes[i]);
                assertTrue(elEntry.name().toString().equals(elDataTypeNameMap.get(elEntry.dataType())));
                if (elEntry.encodedData().length() > 0) {
                    if (elementTypes[i] < DataTypes.SET_PRIMITIVE_MAX && elementTypes[i] != DataTypes.ARRAY)
                        decodeAndCheckDefaultPrimitiveType(decIter, elementTypes[i]);
                    else if (elementTypes[i] == DataTypes.ARRAY)
                        decodeAndCheckArray(decIter, defaultArrayDataType, length);
                    else
                        decodeAndCheckDefaultContainer(decIter, elementTypes[i]);
                }
                elEntry.clear();
                i++;
            }
        }
    }

    public static void decodeAndCheckMap(DecodeIterator decIter,
                                         int[] actions,
                                         boolean[] hasPermData,
                                         int mapKeyType,
                                         int containerType,
                                         boolean hasSummaryData,
                                         boolean hasTotalHintCount,
                                         boolean hasKeyFieldId) {
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();

        assertEquals(SUCCESS, map.decode(decIter));
        assertEquals(map.checkHasSummaryData(), hasSummaryData);

        assertEquals(map.checkHasTotalCountHint(), hasTotalHintCount);
        if (map.checkHasTotalCountHint())
            assertEquals(map.totalCountHint(), actions.length);
        assertEquals(map.containerType(), containerType);

        assertEquals(map.checkHasKeyFieldId(), hasKeyFieldId);
        if (map.checkHasKeyFieldId())
            assertEquals(map.keyFieldId(), 1);
        assertEquals(map.keyPrimitiveType(), mapKeyType);

        if (hasSummaryData)
            decodeAndCheckDefaultContainer(decIter, map.containerType()); //summary data

        int i = 0;
        while (mapEntry.decode(decIter, null) != END_OF_CONTAINER) {
            assertEquals(actions[i], mapEntry.action());
            decodeAndCheckDefaultPrimitiveType(mapEntry.encodedKey(), map.keyPrimitiveType());
            assertEquals(hasPermData[i], mapEntry.checkHasPermData());
            if (hasPermData[i]) {
                assertTrue(mapEntry.permData().equals(permData));
            }
            if (mapEntry.encodedData().length() > 0) {
                decodeAndCheckDefaultContainer(decIter, map.containerType());
            }
            i++;
            mapEntry.clear();
        }
    }

    public static void decodeAndCheckVector(DecodeIterator decIter,
                                            int dataType,
                                            boolean hasSummary,
                                            boolean hasTotalCountHint,
                                            boolean supportsSorting,
                                            int[] actions,
                                            boolean[] hasPermData) {
        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();

        assertEquals(vector.decode(decIter), SUCCESS);
        assertEquals(vector.containerType(), dataType);
        if (hasSummary)
            assertTrue(vector.checkHasSummaryData());
        if (hasTotalCountHint) {
            assertTrue(vector.checkHasTotalCountHint());
            assertEquals(vector.totalCountHint(), actions.length);
        }
        assertEquals(supportsSorting, vector.checkSupportsSorting());
        if (hasSummary)
            decodeAndCheckDefaultContainer(decIter, dataType);
        Set<Integer> actionsPresent = new HashSet<>();
        int j = 0;
        while (vectorEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
            actionsPresent.add(vectorEntry.action());
            if (vectorEntry.encodedData().length() > 0)
                decodeAndCheckDefaultContainer(decIter, dataType);
            assertEquals(hasPermData[j], vectorEntry.checkHasPermData());
            if (vectorEntry.checkHasPermData())
                assertTrue(vectorEntry.permData().equals(permData));
            vectorEntry.clear();
            j++;
        }
        for (int i = 0; i < actions.length; i++)
            assertTrue(actionsPresent.contains(actions[i]));
    }

    private static void encodeMapEntryInit(MapEntry mapEntry, EncodeIterator encIter, int keyType) {
        switch (keyType) {
            case DataTypes.INT:
                mapEntry.encodeInit(encIter, iv, 0);
                break;
            case DataTypes.UINT:
                mapEntry.encodeInit(encIter, uint, 0);
                break;
            case DataTypes.REAL:
                mapEntry.encodeInit(encIter, real, 0);
                break;
            case DataTypes.FLOAT:
                mapEntry.encodeInit(encIter, fv, 0);
                break;
            case DataTypes.DOUBLE:
                mapEntry.encodeInit(encIter, dv, 0);
                break;
            case DataTypes.ENUM:
                mapEntry.encodeInit(encIter, enumer, 0);
                break;
            case DataTypes.DATE:
                mapEntry.encodeInit(encIter, date, 0);
                break;
            case DataTypes.TIME:
                mapEntry.encodeInit(encIter, time, 0);
                break;
            case DataTypes.DATETIME:
                mapEntry.encodeInit(encIter, dateTime, 0);
                break;
            case DataTypes.QOS:
                mapEntry.encodeInit(encIter, qos, 0);
                break;
            case DataTypes.STATE:
                mapEntry.encodeInit(encIter, state, 0);
                break;
            default:
                break;
        }
    }
    private static void encodeMapEntry(MapEntry mapEntry, EncodeIterator encIter, int keyType) {
        switch (keyType) {
            case DataTypes.INT:
                mapEntry.encode(encIter, iv);
                break;
            case DataTypes.UINT:
                mapEntry.encode(encIter, uint);
                break;
            case DataTypes.REAL:
                mapEntry.encode(encIter, real);
                break;
            case DataTypes.FLOAT:
                mapEntry.encode(encIter, fv);
                break;
            case DataTypes.DOUBLE:
                mapEntry.encode(encIter, dv);
                break;
            case DataTypes.ENUM:
                mapEntry.encode(encIter, enumer);
                break;
            case DataTypes.DATE:
                mapEntry.encode(encIter, date);
                break;
            case DataTypes.TIME:
                mapEntry.encode(encIter, time);
                break;
            case DataTypes.DATETIME:
                mapEntry.encode(encIter, dateTime);
                break;
            case DataTypes.QOS:
                mapEntry.encode(encIter, qos);
                break;
            case DataTypes.STATE:
                mapEntry.encode(encIter, state);
                break;
            default:
                break;
        }
    }
}
