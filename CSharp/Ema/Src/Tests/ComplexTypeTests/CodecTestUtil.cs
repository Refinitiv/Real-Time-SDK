/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access.Tests;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using System;
using System.Collections.Generic;
using System.Xml;
using Array = LSEG.Eta.Codec.Array;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.Tests
{

    class CodecTestUtil
    {
        public static int length = 5;
        public static int defaultArrayDataType = DataTypes.QOS;
        public static int mapKeyFieldId = 1;
        public static int defaultMapKeyType = DataTypes.STATE;

        public static Boolean[] defaultVectorEntryHasPermData = { true, false, true };
        public static Boolean[] defaultMapEntryHasPermData = { true, false, true };
        public static Boolean[] defaultFilterEntryHasPermData = { true, false, true };

        public static int defaultSeriesCountHint = 3;
        public static int defaultFilterListCountHint = 3;

        public static int defaultMapContainerType = DataTypes.ELEMENT_LIST;
        public static int defaultSeriesContainerType = DataTypes.ELEMENT_LIST;
        public static int defaultFilterListContainerType = DataTypes.ELEMENT_LIST;
        public static int defaultVectorContainerType = DataTypes.SERIES;

        public static MapEntryActions[] defaultMapEntryActions = { MapEntryActions.UPDATE, MapEntryActions.ADD, MapEntryActions.DELETE };
        public static FilterEntryActions[] defaultFilterListActions = { FilterEntryActions.CLEAR, FilterEntryActions.UPDATE, FilterEntryActions.SET };
        public static VectorEntryActions[] defaultVectorActions = { VectorEntryActions.UPDATE, VectorEntryActions.INSERT, VectorEntryActions.SET };

        public static int[] defaultFilterListDataTypes = { DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST };
        public static int[] defaultFieldListTypes = { DataTypes.ASCII_STRING, DataTypes.REAL, DataTypes.DATE, DataTypes.TIME };
        public static int[] defaultElementListTypes = { DataTypes.INT, DataTypes.REAL, DataTypes.DATETIME, DataTypes.ARRAY };

        public static Buffer intName = new Buffer();
        public static Buffer uintName = new Buffer();
        public static Buffer doubleName = new Buffer();
        public static Buffer realName = new Buffer();
        public static Buffer floatName = new Buffer();
        public static Buffer dateTimeName = new Buffer();
        public static Buffer dateName = new Buffer();
        public static Buffer timeName = new Buffer();
        public static Buffer asciiName = new Buffer();
        public static Buffer qosName = new Buffer();
        public static Buffer stateName = new Buffer();
        public static Buffer arrayName = new Buffer();
        public static Buffer enumName = new Buffer();
        public static Buffer elListName = new Buffer();
        public static Buffer vectorName = new Buffer();
        public static Buffer seriesName = new Buffer();
        public static Buffer fieldListName = new Buffer();
        public static Buffer mapName = new Buffer();

        public static Int iv = new Int();
        public static UInt uintv = new UInt();
        public static Real real = new Real();
        public static Double dv = new Double();
        public static Float fv = new Float();
        public static DateTime dateTime = new DateTime();
        public static Date date = new Date();
        public static Time time = new Time();
        public static Buffer ascii = new Buffer();
        public static Qos qos = new Qos();
        public static State state = new State();
        public static Enum enumer = new Enum();

        public const int intFid = 1;
        public const int realFid = 7;
        public const int dateFid = 16;
        public const int timeFid = 18;
        public const int asciiFid = 30140;
        public const int elemListFid = 30127;
        public const int vectorFid = 30141;
        public const int rmtesFid = 30146;
        public const int arrayFid = 30126;
        public const int mapFid = 32469;

        public static Buffer permData = new Buffer();
        public static Buffer opaqueBuf = new Buffer();
        public static Buffer jsonBuf = new Buffer();
        public static Buffer xmlBuf = new Buffer();

        public static Buffer extendedHeaderBuffer = new Buffer();
        public static Buffer msgKeyBuffer = new Buffer();

        public static int msgKeyServiceId = 1;
        public static int defaultPartNum = 5;
        public static int defaultUserId = 25;
        public static string defaultUserAddr = "127.0.0.1";
        public static int defaultPostUserRights = 10;
        public static Buffer defaultGroupId = new Buffer();
        public static long defaultAckId = 10L;
        public static long defaultPostId = 12;
        public static string jsonValueInXml = "{\"Field1\":\"Value1\"}";

        public static IDictionary<int, String> dataTypeNameMap = new Dictionary<int, String>();
        public static IDictionary<int, String> dataTypeBufferNameMap = new Dictionary<int, String>();
        public static IDictionary<int, String> elDataTypeNameMap = new Dictionary<int, String>();
        public static IDictionary<int, int> elDataTypeFidMap = new Dictionary<int, int>();
        public static IDictionary<String, int> nameTypeMap = new Dictionary<String, int>();

        static CodecTestUtil()
        {
            intName.Data("int");
            uintName.Data("uint");
            doubleName.Data("double");
            floatName.Data("float");
            realName.Data("real");
            dateTimeName.Data("datetime");
            dateName.Data("date");
            timeName.Data("time");
            asciiName.Data("ascii");
            qosName.Data("qos");
            stateName.Data("state");
            arrayName.Data("array");
            enumName.Data("enum");
            elListName.Data("elementList");
            vectorName.Data("vector");
            seriesName.Data("series");
            fieldListName.Data("fieldList");
            mapName.Data("map");

            permData.Data("Permission Data");
            opaqueBuf.Data("Opaque Data");
            jsonBuf.Data("{\"Field1\":\"Value1\"}");
            xmlBuf.Data("Xml data");

            iv.Value(13);
            uintv.Value(1000000);
            dv.Value(0.12345);
            real.Value(12345, 2);
            fv.Value(1.234f);

            date.Year(2020);
            date.Month(1);
            date.Day(2);

            time.Hour(0);
            time.Minute(0);
            time.Second(2);
            time.Millisecond(0);
            time.Microsecond(2);
            time.Nanosecond(500);

            dateTime.Hour(13);
            dateTime.Minute(55);
            dateTime.Second(18);
            dateTime.Millisecond(15);
            dateTime.Microsecond(1);
            dateTime.Nanosecond(3);
            dateTime.Year(2020);
            dateTime.Month(11);
            dateTime.Day(2);

            ascii.Data("ascii string");

            qos.Rate(QosRates.TIME_CONFLATED);
            qos.Timeliness(QosTimeliness.DELAYED);

            Buffer text = new Buffer();
            text.Data("state text");
            state.Text(text);
            state.DataState(DataStates.OK);
            state.Code(StateCodes.FULL_VIEW_PROVIDED);
            state.StreamState(StreamStates.OPEN);

            extendedHeaderBuffer.Data("Extended Buffer");
            msgKeyBuffer.Data("MsgKeyName");
            defaultGroupId.Data("13");

            enumer.Value(1);

            dataTypeNameMap.Add(DataTypes.INT, "PROD_PERM");
            dataTypeNameMap.Add(DataTypes.REAL, "TRDPRC_2");
            dataTypeNameMap.Add(DataTypes.DATE, "TRADE_DATE");
            dataTypeNameMap.Add(DataTypes.TIME, "TRDTIM_1");
            dataTypeNameMap.Add(DataTypes.ELEMENT_LIST, "EX_MET_DAT");
            dataTypeNameMap.Add(DataTypes.ASCII_STRING, "CASH_BASIS");
            dataTypeNameMap.Add(DataTypes.VECTOR, "CASH_RATES");
            dataTypeNameMap.Add(DataTypes.ARRAY, "TENORS");
            dataTypeNameMap.Add(DataTypes.MAP, "PRE_MAP001");

            elDataTypeNameMap.Add(DataTypes.INT, "int");
            elDataTypeNameMap.Add(DataTypes.UINT, "uint");
            elDataTypeNameMap.Add(DataTypes.DOUBLE, "double");
            elDataTypeNameMap.Add(DataTypes.FLOAT, "float");
            elDataTypeNameMap.Add(DataTypes.REAL, "real");
            elDataTypeNameMap.Add(DataTypes.ARRAY, "array");
            elDataTypeNameMap.Add(DataTypes.QOS, "qos");
            elDataTypeNameMap.Add(DataTypes.STATE, "state");
            elDataTypeNameMap.Add(DataTypes.ASCII_STRING, "ascii");
            elDataTypeNameMap.Add(DataTypes.DATE, "date");
            elDataTypeNameMap.Add(DataTypes.TIME, "time");
            elDataTypeNameMap.Add(DataTypes.DATETIME, "datetime");
            elDataTypeNameMap.Add(DataTypes.MAP, "map");
            elDataTypeNameMap.Add(DataTypes.ELEMENT_LIST, "elementList");
            elDataTypeNameMap.Add(DataTypes.FIELD_LIST, "fieldList");
            elDataTypeNameMap.Add(DataTypes.FILTER_LIST, "filterList");
            elDataTypeNameMap.Add(DataTypes.VECTOR, "vector");
            elDataTypeNameMap.Add(DataTypes.SERIES, "series");
            elDataTypeNameMap.Add(DataTypes.OPAQUE, "opaque");
            elDataTypeNameMap.Add(DataTypes.JSON, "json");
            elDataTypeNameMap.Add(DataTypes.MSG, "msg");

            elDataTypeFidMap.Add(DataTypes.INT, intFid);
            elDataTypeFidMap.Add(DataTypes.REAL, realFid);
            elDataTypeFidMap.Add(DataTypes.ARRAY, arrayFid);
            elDataTypeFidMap.Add(DataTypes.ASCII_STRING, asciiFid);
            elDataTypeFidMap.Add(DataTypes.DATE, dateFid);
            elDataTypeFidMap.Add(DataTypes.TIME, timeFid);
            elDataTypeFidMap.Add(DataTypes.VECTOR, vectorFid);
            elDataTypeFidMap.Add(DataTypes.ELEMENT_LIST, elemListFid);
            elDataTypeFidMap.Add(DataTypes.MAP, mapFid);

            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_INT, DataTypes.INT);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_UINT, DataTypes.UINT);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_REAL, DataTypes.REAL);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_ASCII_STRING, DataTypes.ASCII_STRING);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_DOUBLE, DataTypes.DOUBLE);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_FLOAT, DataTypes.FLOAT);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_DATE, DataTypes.DATE);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_TIME, DataTypes.TIME);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_QOS, DataTypes.QOS);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_STATE, DataTypes.STATE);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_DATE_TIME, DataTypes.DATETIME);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_ELEMENT_LIST, DataTypes.ELEMENT_LIST);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_FIELD_LIST, DataTypes.FIELD_LIST);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_FILTER_LIST, DataTypes.FILTER_LIST);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_VECTOR, DataTypes.VECTOR);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_SERIES, DataTypes.SERIES);
        }

        public static void EncodeSimpleArray(EncodeIterator encIter, int dataType)
        {
            Codec.Array array = new Codec.Array();
            ArrayEntry arrEntry = new ArrayEntry();

            array.PrimitiveType = dataType;
            array.ItemLength = 0;
            array.EncodeInit(encIter);
            for (int i = 0; i < length; i++)
            {
                arrEntry.Clear();
                switch (dataType)
                {
                    case DataTypes.INT:
                        arrEntry.Encode(encIter, iv);
                        break;
                    case DataTypes.UINT:
                        arrEntry.Encode(encIter, uintv);
                        break;
                    case DataTypes.DOUBLE:
                        arrEntry.Encode(encIter, dv);
                        break;
                    case DataTypes.FLOAT:
                        arrEntry.Encode(encIter, fv);
                        break;
                    case DataTypes.REAL:
                        arrEntry.Encode(encIter, real);
                        break;
                    case DataTypes.ASCII_STRING:
                        arrEntry.Encode(encIter, ascii);
                        break;
                    case DataTypes.DATE:
                        arrEntry.Encode(encIter, date);
                        break;
                    case DataTypes.TIME:
                        arrEntry.Encode(encIter, time);
                        break;
                    case DataTypes.QOS:
                        arrEntry.Encode(encIter, qos);
                        break;
                    case DataTypes.STATE:
                        arrEntry.Encode(encIter, state);
                        break;
                    case DataTypes.DATETIME:
                        arrEntry.Encode(encIter, dateTime);
                        break;
                    case DataTypes.ENUM:
                        arrEntry.Encode(encIter, enumer);
                        break;
                    default:
                        break;
                }
            }
            array.EncodeComplete(encIter, true);
        }

        public static void EncodeSimpleFieldList(EncodeIterator encIter, int[] dataTypes)
        {
            FieldList fieldList = new FieldList();
            FieldEntry fieldEntry = new FieldEntry();

            fieldList.Clear();
            fieldList.ApplyHasStandardData();
            fieldList.EncodeInit(encIter, null, 0);
            for (int i = 0; i < dataTypes.Length; i++)
            {
                fieldEntry.Clear();
                fieldEntry.DataType = dataTypes[i];
                fieldEntry.FieldId = elDataTypeFidMap[dataTypes[i]];
                switch (dataTypes[i])
                {
                    case DataTypes.ARRAY:
                        fieldEntry.EncodeInit(encIter, 0);
                        EncodeSimpleArray(encIter, defaultArrayDataType);
                        fieldEntry.EncodeComplete(encIter, true);
                        break;
                    case DataTypes.INT:
                        fieldEntry.Encode(encIter, iv);
                        break;
                    case DataTypes.REAL:
                        fieldEntry.Encode(encIter, real);
                        break;
                    case DataTypes.DATE:
                        fieldEntry.Encode(encIter, date);
                        break;
                    case DataTypes.TIME:
                        fieldEntry.Encode(encIter, time);
                        break;
                    case DataTypes.ASCII_STRING:
                        fieldEntry.Encode(encIter, ascii);
                        break;
                    case DataTypes.ELEMENT_LIST:
                    case DataTypes.VECTOR:
                    case DataTypes.MAP:
                        fieldEntry.DataType = dataTypes[i];
                        fieldEntry.FieldId = elDataTypeFidMap[dataTypes[i]];
                        Buffer local = new Buffer();
                        local.Data(new ByteBuffer(10000));
                        EncodeIterator localIter = new EncodeIterator();
                        localIter.SetBufferAndRWFVersion(local, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        EncodeDefaultContainer(localIter, dataTypes[i]);
                        fieldEntry.EncodedData = local;
                        fieldEntry.Encode(encIter);
                        break;
                    default:
                        break;
                }
            }
            fieldList.EncodeComplete(encIter, true);
        }

        public static void EncodeSimpleVector(EncodeIterator encIter,
                                              int dataType,
                                              Boolean hasSummary,
                                              Boolean hasTotalCountHint,
                                              Boolean supportsSorting,
                                              VectorEntryActions[] actions,
                                              Boolean[] hasPermData)
        {
            Vector vector = new Vector();
            VectorEntry vectorEntry = new VectorEntry();

            vector.ContainerType = dataType;
            if (hasTotalCountHint)
            {
                vector.ApplyHasTotalCountHint();
                vector.TotalCountHint = actions.Length;
            }
            if (hasSummary)
                vector.ApplyHasSummaryData();
            if (supportsSorting)
                vector.ApplySupportsSorting();
            vector.EncodeInit(encIter, 0, 0);
            if (hasSummary)
            {
                EncodeDefaultContainer(encIter, dataType);
                vector.EncodeSummaryDataComplete(encIter, true);
            }

            for (uint i = 0; i < actions.Length; i++)
            {
                vectorEntry.Clear();
                vectorEntry.Index = i;
                vectorEntry.Action = actions[i];
                if (hasPermData[i])
                {
                    vectorEntry.ApplyHasPermData();
                    vectorEntry.PermData = permData;
                }
                Buffer local = new Buffer();
                local.Data(new ByteBuffer(10000));
                EncodeIterator localIter = new EncodeIterator();
                localIter.SetBufferAndRWFVersion(local, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                EncodeDefaultContainer(localIter, dataType);
                vectorEntry.EncodedData = local;
                vectorEntry.Encode(encIter);
            }
            vector.EncodeComplete(encIter, true);
        }

        public static void EncodeSimpleElementList(EncodeIterator encIter, int[] dataTypes)
        {
            ElementList elemList = new ElementList();
            ElementEntry elemEntry = new ElementEntry();

            elemList.Clear();
            elemList.ApplyHasStandardData();

            elemList.EncodeInit(encIter, null, 0);
            for (int i = 0; i < dataTypes.Length; i++)
            {
                elemEntry.Clear();
                switch (dataTypes[i])
                {
                    case DataTypes.INT:
                        elemEntry.DataType = DataTypes.INT;
                        elemEntry.Name = intName;
                        elemEntry.Encode(encIter, iv);
                        break;
                    case DataTypes.UINT:
                        elemEntry.DataType = DataTypes.UINT;
                        elemEntry.Name = uintName;
                        elemEntry.Encode(encIter, uintv);
                        break;
                    case DataTypes.DATE:
                        elemEntry.DataType = DataTypes.DATE;
                        elemEntry.Name = dateName;
                        elemEntry.Encode(encIter, date);
                        break;
                    case DataTypes.TIME:
                        elemEntry.DataType = DataTypes.TIME;
                        elemEntry.Name = timeName;
                        elemEntry.Encode(encIter, time);
                        break;
                    case DataTypes.DATETIME:
                        elemEntry.DataType = DataTypes.DATETIME;
                        elemEntry.Name = dateTimeName;
                        elemEntry.Encode(encIter, dateTime);
                        break;
                    case DataTypes.ASCII_STRING:
                        elemEntry.DataType = DataTypes.ASCII_STRING;
                        elemEntry.Name = asciiName;
                        elemEntry.Encode(encIter, ascii);
                        break;
                    case DataTypes.REAL:
                        elemEntry.DataType = DataTypes.REAL;
                        elemEntry.Name = realName;
                        elemEntry.Encode(encIter, real);
                        break;
                    case DataTypes.QOS:
                        elemEntry.DataType = DataTypes.QOS;
                        elemEntry.Name = qosName;
                        elemEntry.Encode(encIter, qos);
                        break;
                    case DataTypes.STATE:
                        elemEntry.DataType = DataTypes.STATE;
                        elemEntry.Name = stateName;
                        elemEntry.Encode(encIter, state);
                        break;
                    case DataTypes.DOUBLE:
                        elemEntry.DataType = DataTypes.DOUBLE;
                        elemEntry.Name = doubleName;
                        elemEntry.Encode(encIter, dv);
                        break;
                    case DataTypes.FLOAT:
                        elemEntry.DataType = DataTypes.FLOAT;
                        elemEntry.Name = floatName;
                        elemEntry.Encode(encIter, fv);
                        break;
                    case DataTypes.ARRAY:
                        elemEntry.DataType = DataTypes.ARRAY;
                        elemEntry.Name = arrayName;
                        elemEntry.EncodeInit(encIter, 0);
                        EncodeSimpleArray(encIter, defaultArrayDataType);
                        elemEntry.EncodeComplete(encIter, true);
                        break;
                    case DataTypes.ELEMENT_LIST:
                    case DataTypes.VECTOR:
                    case DataTypes.SERIES:
                    case DataTypes.MAP:
                    case DataTypes.FIELD_LIST:
                    case DataTypes.OPAQUE:
                    case DataTypes.JSON:
                    case DataTypes.MSG:
                        elemEntry.DataType = dataTypes[i];
                        elemEntry.Name.Data(elDataTypeNameMap[dataTypes[i]]);
                        elemEntry.EncodeInit(encIter, 0);
                        EncodeDefaultContainer(encIter, dataTypes[i]);
                        elemEntry.EncodeComplete(encIter, true);
                        break;
                    default:
                        break;
                }
            }
            elemList.EncodeComplete(encIter, true);
        }

        public static void EncodeSimpleMap(EncodeIterator encIter,
                                           int containerType,
                                           MapEntryActions[] entryActions,
                                           Boolean[] permDataPresent,
                                           int keyType,
                                           Boolean hasSummary,
                                           Boolean hasKeyFieldId,
                                           Boolean hasTotalHintCount)
        {
            Map map = new Map();
            MapEntry mapEntry = new MapEntry();

            map.ContainerType = containerType;
            if (hasKeyFieldId)
            {
                map.ApplyHasKeyFieldId();
                map.KeyFieldId = mapKeyFieldId;
            }
            if (hasTotalHintCount)
            {
                map.ApplyHasTotalCountHint();
                map.TotalCountHint = entryActions.Length;
            }
            map.KeyPrimitiveType = keyType;
            if (hasSummary)
                map.ApplyHasSummaryData();
            map.EncodeInit(encIter, 0, 0);
            if (hasSummary)
            {
                EncodeDefaultContainer(encIter, containerType);
                map.EncodeSummaryDataComplete(encIter, true);
            }
            for (int i = 0; i < entryActions.Length; i++)
            {
                mapEntry.Clear();
                mapEntry.Action = entryActions[i];
                if (permDataPresent[i])
                {
                    mapEntry.ApplyHasPermData();
                    mapEntry.PermData = permData;
                }
                if (entryActions[i] != MapEntryActions.DELETE)
                {
                    EncodeMapEntryInit(mapEntry, encIter, keyType);
                    EncodeDefaultContainer(encIter, containerType);
                    mapEntry.EncodeComplete(encIter, true);
                }
                else
                {
                    EncodeMapEntry(mapEntry, encIter, keyType);
                }
            }
            map.EncodeComplete(encIter, true);
        }

        public static void EncodeSimpleSeries(EncodeIterator encIter,
                                              int containerType,
                                              Boolean hasCountHint,
                                              Boolean hasSummary,
                                              int countHint,
                                              int length)
        {
            Series series = new Series();
            SeriesEntry seriesEntry = new SeriesEntry();

            series.ContainerType = containerType;
            if (hasCountHint)
            {
                series.ApplyHasTotalCountHint();
                series.TotalCountHint = countHint;
            }

            if (hasSummary)
                series.ApplyHasSummaryData();

            series.EncodeInit(encIter, 0, 0);
            if (hasSummary)
            {
                EncodeDefaultContainer(encIter, containerType);
                series.EncodeSummaryDataComplete(encIter, true);
            }
            for (int i = 0; i < length; i++)
            {
                seriesEntry.Clear();
                Buffer local = new Buffer();
                local.Data(new ByteBuffer(10000));
                EncodeIterator localIter = new EncodeIterator();
                localIter.SetBufferAndRWFVersion(local, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                EncodeDefaultContainer(localIter, containerType);
                seriesEntry.EncodedData = local;
                seriesEntry.Encode(encIter);
            }
            series.EncodeComplete(encIter, true);
        }

        public static void EncodeSimpleFilterList(EncodeIterator encIter,
                                                  int containerType,
                                                  Boolean hasCountHint,
                                                  FilterEntryActions[] filterActions,
                                                  int[] dataTypes,
                                                  Boolean[] permDataPresent,
                                                  int countHint)
        {
            FilterList filterList = new FilterList();
            FilterEntry filterEntry = new FilterEntry();

            filterList.Clear();
            filterList.ContainerType = containerType;
            if (hasCountHint)
            {
                filterList.ApplyHasTotalCountHint();
                filterList.TotalCountHint = countHint;
            }
            filterList.EncodeInit(encIter);
            for (int i = 0; i < filterActions.Length; i++)
            {
                filterEntry.Clear();
                filterEntry.Id = i;
                filterEntry.Action = filterActions[i];
                if (dataTypes[i] != DataTypes.UNKNOWN)
                {
                    filterEntry.ApplyHasContainerType();
                    filterEntry.ContainerType = dataTypes[i];
                }
                if (permDataPresent[i])
                {
                    filterEntry.ApplyHasPermData();
                    filterEntry.PermData = permData;
                }
                Buffer local = new Buffer();
                local.Data(new ByteBuffer(10000));
                EncodeIterator localIter = new EncodeIterator();
                localIter.SetBufferAndRWFVersion(local, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                EncodeDefaultContainer(localIter, dataTypes[i]);
                filterEntry.EncodedData = local;
                filterEntry.Encode(encIter);
            }
            filterList.EncodeComplete(encIter, true);
        }

        public static void EncodeDefaultOpaque(EncodeIterator encIter)
        {

            Buffer dataBuf = new Buffer();
            encIter.EncodeNonRWFInit(dataBuf);
            dataBuf.Data().Put(opaqueBuf.Data());
            encIter.EncodeNonRWFComplete(dataBuf, true);
        }

        public static void EncodeDefaultJson(EncodeIterator encIter)
        {

            Buffer dataBuf = new Buffer();
            encIter.EncodeNonRWFInit(dataBuf);
            dataBuf.Data().Put(jsonBuf.Data());
            encIter.EncodeNonRWFComplete(dataBuf, true);
        }

        public static void EncodeDefaultXml(EncodeIterator encIter)
        {

            Buffer dataBuf = new Buffer();
            encIter.EncodeNonRWFInit(dataBuf);
            dataBuf.Data().Put(xmlBuf.Data());
            encIter.EncodeNonRWFComplete(dataBuf, true);
        }

        public static void EncodeDefaultContainer(EncodeIterator encIter, int containerType)
        {
            switch (containerType)
            {
                case DataTypes.ELEMENT_LIST:
                    EncodeSimpleElementList(encIter, defaultElementListTypes);
                    break;
                case DataTypes.VECTOR:
                    EncodeSimpleVector(encIter, defaultVectorContainerType, true, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
                    break;
                case DataTypes.FIELD_LIST:
                    EncodeSimpleFieldList(encIter, defaultFieldListTypes);
                    break;
                case DataTypes.SERIES:
                    EncodeSimpleSeries(encIter, defaultSeriesContainerType, true, true, defaultSeriesCountHint, defaultSeriesCountHint);
                    break;
                case DataTypes.MAP:
                    EncodeSimpleMap(encIter, defaultMapContainerType, defaultMapEntryActions, defaultMapEntryHasPermData, defaultMapKeyType, true, true, true);
                    break;
                case DataTypes.FILTER_LIST:
                    EncodeSimpleFilterList(encIter, defaultFilterListContainerType, true, defaultFilterListActions, defaultFilterListDataTypes, defaultFilterEntryHasPermData, defaultFilterListCountHint);
                    break;
                case DataTypes.OPAQUE:
                    EncodeDefaultOpaque(encIter);
                    break;
                case DataTypes.JSON:
                    EncodeDefaultJson(encIter);
                    break;
                case DataTypes.XML:
                    EncodeDefaultXml(encIter);
                    break;
                case DataTypes.MSG:
                    EncodeBasicMsg(encIter);
                    break;
                default:
                    break;
            }
        }

        public static void DecodeAndCheckDefaultOpaque(DecodeIterator decIter)
        {
            Buffer dataBuf = new Buffer();
            dataBuf.Decode(decIter);
            Assert.Equal(dataBuf.ToString(), opaqueBuf.ToString());
        }
        public static void DecodeAndCheckDefaultJson(DecodeIterator decIter)
        {
            Buffer dataBuf = new Buffer();
            dataBuf.Decode(decIter);
            Assert.Equal(dataBuf.ToString(), jsonBuf.ToString());
        }
        public static void DecodeAndCheckDefaultXml(DecodeIterator decIter)
        {
            Buffer dataBuf = new Buffer();
            dataBuf.Decode(decIter);
            Assert.Equal(dataBuf.ToString(), xmlBuf.ToString());
        }
        public static void DecodeAndCheckDefaultElementList(DecodeIterator decIter)
        {
            ElementList elList = new ElementList();
            ElementEntry elEntry = new ElementEntry();

            CodecReturnCode ret = elList.Decode(decIter, null);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            if (ret != CodecReturnCode.NO_DATA)
            {
                int i = 0;
                while (elEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
                {
                    Assert.Equal(elEntry.DataType, defaultElementListTypes[i]);
                    Assert.Equal(elEntry.Name.ToString(), elDataTypeNameMap[elEntry.DataType]);
                    if (elEntry.EncodedData.Length > 0)
                    {
                        if (defaultElementListTypes[i] < DataTypes.SET_PRIMITIVE_MAX && defaultElementListTypes[i] != DataTypes.ARRAY)
                            DecodeAndCheckDefaultPrimitiveType(decIter, defaultElementListTypes[i]);
                        else if (defaultElementListTypes[i] == DataTypes.ARRAY)
                            DecodeAndCheckArray(decIter, defaultArrayDataType, length);
                        else
                            DecodeAndCheckDefaultContainer(decIter, defaultElementListTypes[i]);
                    }
                    elEntry.Clear();
                    i++;
                }
            }
        }
        public static void DecodeAndCheckDefaultVector(DecodeIterator decIter)
        {
            Vector vector = new Vector();
            VectorEntry vectorEntry = new VectorEntry();

            Assert.Equal(CodecReturnCode.SUCCESS, vector.Decode(decIter));
            Assert.Equal(vector.ContainerType, defaultVectorContainerType);
            Assert.True(vector.CheckHasSummaryData());
            Assert.True(vector.CheckHasTotalCountHint());
            Assert.Equal(vector.TotalCountHint, defaultVectorActions.Length);
            DecodeAndCheckDefaultElementList(decIter);
            HashSet<VectorEntryActions> actionsPresent = new HashSet<VectorEntryActions>();
            int j = 0;
            while (vectorEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
            {
                actionsPresent.Add(vectorEntry.Action);
                DecodeAndCheckDefaultElementList(decIter);
                Assert.Equal(defaultVectorEntryHasPermData[j], vectorEntry.CheckHasPermData());
                if (vectorEntry.CheckHasPermData())
                    Assert.True(vectorEntry.PermData.Equals(permData));
                vectorEntry.Clear();
                j++;
            }
            for (int i = 0; i < defaultVectorActions.Length; i++)
                Assert.Contains(defaultVectorActions[i], actionsPresent);
        }
        public static void DecodeAndCheckDefaultMap(DecodeIterator decIter)
        {
            Map map = new Map();
            MapEntry mapEntry = new MapEntry();

            Assert.Equal(CodecReturnCode.SUCCESS, map.Decode(decIter));
            Assert.True(map.CheckHasSummaryData());
            Assert.True(map.CheckHasTotalCountHint());
            Assert.True(map.CheckHasTotalCountHint());
            Assert.Equal(map.TotalCountHint, defaultMapEntryActions.Length);
            Assert.Equal(map.KeyPrimitiveType, defaultMapKeyType);
            DecodeAndCheckDefaultContainer(decIter, map.ContainerType); //summary data
            int i = 0;
            while (mapEntry.Decode(decIter, null) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(defaultMapEntryActions[i], mapEntry.Action);
                DecodeAndCheckDefaultPrimitiveType(mapEntry.EncodedKey, map.KeyPrimitiveType);
                Assert.Equal(defaultMapEntryHasPermData[i], mapEntry.CheckHasPermData());
                if (defaultMapEntryHasPermData[i])
                {
                    Assert.True(mapEntry.PermData.Equals(permData));
                }
                if (mapEntry.EncodedData.Length > 0)
                {
                    DecodeAndCheckDefaultContainer(decIter, map.ContainerType);
                }
                i++;
                mapEntry.Clear();
            }
        }
        public static void DecodeAndCheckDefaultFilterList(DecodeIterator decIter)
        {
            FilterList filterList = new FilterList();
            FilterEntry filterEntry = new FilterEntry();

            Assert.True(filterList.Decode(decIter) >= CodecReturnCode.SUCCESS);
            Assert.True(filterList.CheckHasTotalCountHint());
            Assert.Equal(filterList.TotalCountHint, defaultFilterListCountHint);
            Assert.Equal(filterList.ContainerType, defaultFilterListContainerType);
            int i = 0;
            while (filterEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(filterEntry.Action, defaultFilterListActions[i]);
                Assert.Equal(filterEntry.CheckHasPermData(), defaultFilterEntryHasPermData[i]);
                if (filterEntry.CheckHasPermData())
                    Assert.True(filterEntry.PermData.Equals(permData));
                if (defaultFilterListDataTypes[i] != DataTypes.UNKNOWN)
                {
                    Assert.True(filterEntry.CheckHasContainerType());
                    Assert.Equal(filterEntry.ContainerType, defaultFilterListDataTypes[i]);
                }
                if (filterEntry.EncodedData.Length > 0)
                    DecodeAndCheckDefaultContainer(decIter, defaultFilterListDataTypes[i] != DataTypes.UNKNOWN ? defaultFilterListDataTypes[i] : filterList.ContainerType);
                filterEntry.Clear();
                i++;
            }
        }
        public static void DecodeAndCheckDefaultFieldList(DecodeIterator decIter)
        {

            HashSet<int> dataPresent = new HashSet<int>();

            FieldList fieldList = new FieldList();
            FieldEntry fieldEntry = new FieldEntry();

            CodecReturnCode ret = fieldList.Decode(decIter, null);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = fieldEntry.Decode(decIter);
            while (ret != CodecReturnCode.END_OF_CONTAINER)
            {
                dataPresent.Add(fieldEntry.FieldId);
                switch (fieldEntry.FieldId)
                {
                    case arrayFid:
                        Array arr = new Array();
                        ArrayEntry arrEntry = new ArrayEntry();
                        Assert.Equal(CodecReturnCode.SUCCESS, arr.Decode(decIter));
                        Assert.Equal(arr.PrimitiveType, defaultArrayDataType);
                        Qos qos = new Qos();
                        while (arrEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            qos.Clear();
                            qos.Decode(decIter);
                            Assert.True(qos.Equals(qos));
                        }
                        break;
                    case intFid:
                        Int intv = new Int();
                        Assert.Equal(CodecReturnCode.SUCCESS, intv.Decode(decIter));
                        Assert.True(intv.Equals(iv));
                        break;
                    case realFid:
                        Real real = new Real();
                        Assert.Equal(CodecReturnCode.SUCCESS, real.Decode(decIter));
                        break;
                    case timeFid:
                        Time time = new Time();
                        Assert.Equal(CodecReturnCode.SUCCESS, time.Decode(decIter));
                        Assert.True(time.Equals(time));
                        break;
                    case dateFid:
                        Date date = new Date();
                        Assert.Equal(CodecReturnCode.SUCCESS, date.Decode(decIter));
                        Assert.True(date.Equals(date));
                        break;
                    case asciiFid:
                        Buffer ascii = new Buffer();
                        Assert.Equal(CodecReturnCode.SUCCESS, ascii.Decode(decIter));
                        Assert.True(ascii.Equals(ascii));
                        break;
                    case elemListFid:
                        DecodeAndCheckDefaultElementList(decIter);
                        break;
                    case vectorFid:
                        DecodeAndCheckDefaultVector(decIter);
                        break;
                    case mapFid:
                        DecodeAndCheckDefaultMap(decIter);
                        break;
                    default:
                        break;
                }

                fieldEntry.Clear();
                ret = fieldEntry.Decode(decIter);
            }
        }
        public static void DecodeAndCheckDefaultSeries(DecodeIterator decIter)
        {
            Series series = new Series();
            SeriesEntry seriesEntry = new SeriesEntry();

            series.Decode(decIter);
            Assert.Equal(series.ContainerType, defaultSeriesContainerType);
            Assert.Equal(series.TotalCountHint, defaultSeriesCountHint);
            DecodeAndCheckDefaultContainer(decIter, series.ContainerType);
            while (seriesEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
            {
                DecodeAndCheckDefaultContainer(decIter, series.ContainerType);
                seriesEntry.Clear();
            }
        }
        public static void DecodeAndCheckBasicMessage(DecodeIterator decIter)
        {
            Msg message = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, message.Decode(decIter));
            Assert.Equal(MsgClasses.UPDATE, message.StreamId);
            Assert.Equal(MsgClasses.UPDATE, message.MsgClass);
            Assert.True(message.DomainType == (byte)DomainType.MARKET_PRICE);
        }

        public static void DecodeAndCheckDefaultContainer(DecodeIterator decIter, int dataType)
        {
            switch (dataType)
            {
                case DataTypes.ELEMENT_LIST:
                    DecodeAndCheckDefaultElementList(decIter);
                    break;
                case DataTypes.VECTOR:
                    DecodeAndCheckDefaultVector(decIter);
                    break;
                case DataTypes.FIELD_LIST:
                    DecodeAndCheckDefaultFieldList(decIter);
                    break;
                case DataTypes.FILTER_LIST:
                    DecodeAndCheckDefaultFilterList(decIter);
                    break;
                case DataTypes.MAP:
                    DecodeAndCheckDefaultMap(decIter);
                    break;
                case DataTypes.SERIES:
                    DecodeAndCheckDefaultSeries(decIter);
                    break;
                case DataTypes.OPAQUE:
                    DecodeAndCheckDefaultOpaque(decIter);
                    break;
                case DataTypes.JSON:
                    DecodeAndCheckDefaultJson(decIter);
                    break;
                case DataTypes.XML:
                    DecodeAndCheckDefaultXml(decIter);
                    break;
                case DataTypes.MSG:
                    DecodeAndCheckBasicMessage(decIter);
                    break;
                default:
                    break;
            }
        }
        public static void DecodeAndCheckDefaultPrimitiveType(Buffer keyBuffer, int dataType)
        {
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(keyBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            switch (dataType)
            {
                case DataTypes.INT:
                    Int intval = new Int();
                    Assert.Equal(CodecReturnCode.SUCCESS, intval.Decode(decIter));
                    Assert.True(intval.Equals(iv));
                    break;
                case DataTypes.UINT:
                    UInt uintval = new UInt();
                    Assert.Equal(CodecReturnCode.SUCCESS, uintval.Decode(decIter));
                    Assert.True(uintval.Equals(uintv));
                    break;
                case DataTypes.DATE:
                    Date dateval = new Date();
                    Assert.Equal(CodecReturnCode.SUCCESS, dateval.Decode(decIter));
                    Assert.True(dateval.Equals(date));
                    break;
                case DataTypes.TIME:
                    Time timeval = new Time();
                    Assert.Equal(CodecReturnCode.SUCCESS, timeval.Decode(decIter));
                    Assert.True(timeval.Equals(time));
                    break;
                case DataTypes.DATETIME:
                    DateTime datetimeval = new DateTime();
                    Assert.Equal(CodecReturnCode.SUCCESS, datetimeval.Decode(decIter));
                    Assert.True(datetimeval.Equals(dateTime));
                    break;
                case DataTypes.QOS:
                    Qos qosval = new Qos();
                    Assert.Equal(CodecReturnCode.SUCCESS, qosval.Decode(decIter));
                    Assert.True(qosval.Equals(qos));
                    break;
                case DataTypes.STATE:
                    State stateval = new State();
                    Assert.Equal(CodecReturnCode.SUCCESS, stateval.Decode(decIter));
                    Assert.True(stateval.Equals(state));
                    break;
                case DataTypes.DOUBLE:
                    Double doubleval = new Double();
                    Assert.Equal(CodecReturnCode.SUCCESS, doubleval.Decode(decIter));
                    Assert.True(doubleval.Equals(dv));
                    break;
                case DataTypes.FLOAT:
                    Float floatval = new Float();
                    Assert.Equal(CodecReturnCode.SUCCESS, floatval.Decode(decIter));
                    Assert.True(floatval.Equals(fv));
                    break;
                case DataTypes.REAL:
                    Real realval = new Real();
                    Assert.Equal(CodecReturnCode.SUCCESS, realval.Decode(decIter));
                    Assert.True(realval.Equals(real));
                    break;
                case DataTypes.ASCII_STRING:
                    Buffer asciival = new Buffer();
                    Assert.Equal(CodecReturnCode.SUCCESS, asciival.Decode(decIter));
                    Assert.True(asciival.Equals(ascii));
                    break;
                default:
                    break;
            }
        }
        public static void DecodeAndCheckDefaultPrimitiveType(DecodeIterator decIter, int dataType)
        {
            switch (dataType)
            {
                case DataTypes.INT:
                    Int intval = new Int();
                    Assert.Equal(CodecReturnCode.SUCCESS, intval.Decode(decIter));
                    Assert.True(intval.Equals(iv));
                    break;
                case DataTypes.UINT:
                    UInt uintval = new UInt();
                    Assert.Equal(CodecReturnCode.SUCCESS, uintval.Decode(decIter));
                    Assert.True(uintval.Equals(uintv));
                    break;
                case DataTypes.DATE:
                    Date dateval = new Date();
                    Assert.Equal(CodecReturnCode.SUCCESS, dateval.Decode(decIter));
                    Assert.True(dateval.Equals(date));
                    break;
                case DataTypes.TIME:
                    Time timeval = new Time();
                    Assert.Equal(CodecReturnCode.SUCCESS, timeval.Decode(decIter));
                    Assert.True(timeval.Equals(time));
                    break;
                case DataTypes.DATETIME:
                    DateTime datetimeval = new DateTime();
                    Assert.Equal(CodecReturnCode.SUCCESS, datetimeval.Decode(decIter));
                    Assert.True(datetimeval.Equals(dateTime));
                    break;
                case DataTypes.QOS:
                    Qos qosval = new Qos();
                    Assert.Equal(CodecReturnCode.SUCCESS, qosval.Decode(decIter));
                    Assert.True(qosval.Equals(qos));
                    break;
                case DataTypes.STATE:
                    State stateval = new State();
                    Assert.Equal(CodecReturnCode.SUCCESS, stateval.Decode(decIter));
                    Assert.True(stateval.Equals(state));
                    break;
                case DataTypes.DOUBLE:
                    Double doubleval = new Double();
                    Assert.Equal(CodecReturnCode.SUCCESS, doubleval.Decode(decIter));
                    Assert.True(doubleval.Equals(dv));
                    break;
                case DataTypes.FLOAT:
                    Float floatval = new Float();
                    Assert.Equal(CodecReturnCode.SUCCESS, floatval.Decode(decIter));
                    Assert.True(floatval.Equals(fv));
                    break;
                case DataTypes.REAL:
                    Real realval = new Real();
                    Assert.Equal(CodecReturnCode.SUCCESS, realval.Decode(decIter));
                    Assert.True(realval.Equals(real));
                    break;
                case DataTypes.ASCII_STRING:
                    Buffer asciival = new Buffer();
                    Assert.Equal(CodecReturnCode.SUCCESS, asciival.Decode(decIter));
                    Assert.Equal(asciival.ToString(), ascii.ToString());
                    break;
                default:
                    break;
            }
        }


        //  Decode and check container types
        public static void DecodeAndCheckArray(DecodeIterator decIter, int dataType, int length)
        {
            Array array = new Array();
            ArrayEntry arrayEntry = new ArrayEntry();

            Assert.True(array.Decode(decIter) >= CodecReturnCode.SUCCESS);
            Assert.Equal(dataType, array.PrimitiveType);
            if (array.EncodedData().Length > 0)
            {
                while (arrayEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
                {
                    DecodeAndCheckDefaultPrimitiveType(decIter, dataType);
                    length--;
                    arrayEntry.Clear();
                }
            }
            Assert.Equal(0, length);
        }

        public static void DecodeAndCheckSeries(DecodeIterator decIter,
                                                int containerType,
                                                int countHint,
                                                int length,
                                                Boolean hasSummary,
                                                Boolean hasCountHint)
        {
            Series series = new Series();
            SeriesEntry seriesEntry = new SeriesEntry();

            series.Decode(decIter);
            Assert.Equal(series.ContainerType, containerType);
            Assert.Equal(hasCountHint, series.CheckHasTotalCountHint());
            if (series.CheckHasTotalCountHint())
                Assert.Equal(series.TotalCountHint, countHint);
            Assert.Equal(hasSummary, series.CheckHasSummaryData());
            if (series.CheckHasSummaryData())
                DecodeAndCheckDefaultContainer(decIter, series.ContainerType);
            while (seriesEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
            {
                DecodeAndCheckDefaultContainer(decIter, series.ContainerType);
                length--;
                seriesEntry.Clear();
            }
            Assert.Equal(0, length);
        }

        public static void DecodeAndCheckFilterList(DecodeIterator decIter,
                                                    Boolean hasCountHint,
                                                    int countHint,
                                                    int containerType,
                                                    FilterEntryActions[] filterActions,
                                                    int[] filterDataTypes,
                                                    Boolean[] hasPermData)
        {
            FilterList filterList = new FilterList();
            FilterEntry filterEntry = new FilterEntry();

            Assert.True(filterList.Decode(decIter) >= CodecReturnCode.SUCCESS);
            Assert.True(filterList.CheckHasTotalCountHint() == hasCountHint);
            if (filterList.CheckHasTotalCountHint())
                Assert.Equal(filterList.TotalCountHint, countHint);
            int i = 0;
            while (filterEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(filterEntry.Action, filterActions[i]);
                Assert.Equal(filterEntry.CheckHasPermData(), hasPermData[i]);
                if (filterEntry.CheckHasPermData())
                    Assert.True(filterEntry.PermData.Equals(permData));
                if (filterDataTypes[i] != DataTypes.UNKNOWN && filterEntry.Action != FilterEntryActions.CLEAR)
                {
                    Assert.True(filterEntry.CheckHasContainerType());
                    Assert.Equal(filterEntry.ContainerType, filterDataTypes[i]);
                }
                if (filterEntry.EncodedData.Length > 0)
                    DecodeAndCheckDefaultContainer(decIter, filterDataTypes[i] != DataTypes.UNKNOWN ? filterDataTypes[i] : filterList.ContainerType);
                filterEntry.Clear();
                i++;
            }
        }

        public static void DecodeAndCheckElementList(DecodeIterator decIter, int[] elementTypes)
        {
            ElementList elList = new ElementList();
            ElementEntry elEntry = new ElementEntry();

            CodecReturnCode ret = elList.Decode(decIter, null);
            Assert.True(ret >= CodecReturnCode.SUCCESS);
            if (ret != CodecReturnCode.NO_DATA)
            {
                int i = 0;
                while (elEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
                {
                    Assert.Equal(elEntry.DataType, elementTypes[i]);
                    Assert.Equal(elEntry.Name.ToString(), elDataTypeNameMap[elEntry.DataType]);
                    if (elEntry.EncodedData.Length > 0)
                    {
                        if (elementTypes[i] < DataTypes.SET_PRIMITIVE_MAX && elementTypes[i] != DataTypes.ARRAY)
                            DecodeAndCheckDefaultPrimitiveType(decIter, elementTypes[i]);
                        else if (elementTypes[i] == DataTypes.ARRAY)
                            DecodeAndCheckArray(decIter, defaultArrayDataType, length);
                        else
                            DecodeAndCheckDefaultContainer(decIter, elementTypes[i]);
                    }
                    elEntry.Clear();
                    i++;
                }
            }
        }

        public static void DecodeAndCheckMap(DecodeIterator decIter,
                                             MapEntryActions[] actions,
                                             Boolean[] hasPermData,
                                             int mapKeyType,
                                             int containerType,
                                             Boolean hasSummaryData,
                                             Boolean hasTotalHintCount,
                                             Boolean hasKeyFieldId)
        {
            Map map = new Map();
            MapEntry mapEntry = new MapEntry();

            Assert.Equal(CodecReturnCode.SUCCESS, map.Decode(decIter));
            Assert.Equal(map.CheckHasSummaryData(), hasSummaryData);

            Assert.Equal(map.CheckHasTotalCountHint(), hasTotalHintCount);
            if (map.CheckHasTotalCountHint())
                Assert.Equal(map.TotalCountHint, actions.Length);
            Assert.Equal(map.ContainerType, containerType);

            Assert.Equal(map.CheckHasKeyFieldId(), hasKeyFieldId);
            if (map.CheckHasKeyFieldId())
                Assert.Equal(1, map.KeyFieldId);
            Assert.Equal(map.KeyPrimitiveType, mapKeyType);

            if (hasSummaryData)
                DecodeAndCheckDefaultContainer(decIter, map.ContainerType); //summary data

            int i = 0;
            while (mapEntry.Decode(decIter, null) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(actions[i], mapEntry.Action);
                DecodeAndCheckDefaultPrimitiveType(mapEntry.EncodedKey, map.KeyPrimitiveType);
                Assert.Equal(hasPermData[i], mapEntry.CheckHasPermData());
                if (hasPermData[i])
                {
                    Assert.True(mapEntry.PermData.Equals(permData));
                }
                if (mapEntry.EncodedData.Length > 0)
                {
                    DecodeAndCheckDefaultContainer(decIter, map.ContainerType);
                }
                i++;
                mapEntry.Clear();
            }
        }

        public static void DecodeAndCheckVector(DecodeIterator decIter,
                                                int dataType,
                                                Boolean hasSummary,
                                                Boolean hasTotalCountHint,
                                                Boolean supportsSorting,
                                                VectorEntryActions[] actions,
                                                Boolean[] hasPermData)
        {
            Vector vector = new Vector();
            VectorEntry vectorEntry = new VectorEntry();

            Assert.Equal(CodecReturnCode.SUCCESS, vector.Decode(decIter));
            Assert.Equal(vector.ContainerType, dataType);
            if (hasSummary)
                Assert.True(vector.CheckHasSummaryData());
            if (hasTotalCountHint)
            {
                Assert.True(vector.CheckHasTotalCountHint());
                Assert.Equal(vector.TotalCountHint, actions.Length);
            }
            Assert.Equal(supportsSorting, vector.CheckSupportsSorting());
            if (hasSummary)
                DecodeAndCheckDefaultContainer(decIter, dataType);
            HashSet<VectorEntryActions> actionsPresent = new HashSet<VectorEntryActions>();
            int j = 0;
            while (vectorEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER)
            {
                actionsPresent.Add(vectorEntry.Action);
                if (vectorEntry.EncodedData.Length > 0)
                    DecodeAndCheckDefaultContainer(decIter, dataType);
                Assert.Equal(hasPermData[j], vectorEntry.CheckHasPermData());
                if (vectorEntry.CheckHasPermData())
                    Assert.True(vectorEntry.PermData.Equals(permData));
                vectorEntry.Clear();
                j++;
            }
            for (int i = 0; i < actions.Length; i++)
                Assert.Contains(actions[i], actionsPresent);
        }

        private static void EncodeMapEntryInit(MapEntry mapEntry, EncodeIterator encIter, int keyType)
        {
            switch (keyType)
            {
                case DataTypes.INT:
                    mapEntry.EncodeInit(encIter, iv, 0);
                    break;
                case DataTypes.UINT:
                    mapEntry.EncodeInit(encIter, uintv, 0);
                    break;
                case DataTypes.REAL:
                    mapEntry.EncodeInit(encIter, real, 0);
                    break;
                case DataTypes.FLOAT:
                    mapEntry.EncodeInit(encIter, fv, 0);
                    break;
                case DataTypes.DOUBLE:
                    mapEntry.EncodeInit(encIter, dv, 0);
                    break;
                case DataTypes.ENUM:
                    mapEntry.EncodeInit(encIter, enumer, 0);
                    break;
                case DataTypes.DATE:
                    mapEntry.EncodeInit(encIter, date, 0);
                    break;
                case DataTypes.TIME:
                    mapEntry.EncodeInit(encIter, time, 0);
                    break;
                case DataTypes.DATETIME:
                    mapEntry.EncodeInit(encIter, dateTime, 0);
                    break;
                case DataTypes.QOS:
                    mapEntry.EncodeInit(encIter, qos, 0);
                    break;
                case DataTypes.STATE:
                    mapEntry.EncodeInit(encIter, state, 0);
                    break;
                default:
                    break;
            }
        }
        private static void EncodeMapEntry(MapEntry mapEntry, EncodeIterator encIter, int keyType)
        {
            switch (keyType)
            {
                case DataTypes.INT:
                    mapEntry.Encode(encIter, iv);
                    break;
                case DataTypes.UINT:
                    mapEntry.Encode(encIter, uintv);
                    break;
                case DataTypes.REAL:
                    mapEntry.Encode(encIter, real);
                    break;
                case DataTypes.FLOAT:
                    mapEntry.Encode(encIter, fv);
                    break;
                case DataTypes.DOUBLE:
                    mapEntry.Encode(encIter, dv);
                    break;
                case DataTypes.ENUM:
                    mapEntry.Encode(encIter, enumer);
                    break;
                case DataTypes.DATE:
                    mapEntry.Encode(encIter, date);
                    break;
                case DataTypes.TIME:
                    mapEntry.Encode(encIter, time);
                    break;
                case DataTypes.DATETIME:
                    mapEntry.Encode(encIter, dateTime);
                    break;
                case DataTypes.QOS:
                    mapEntry.Encode(encIter, qos);
                    break;
                case DataTypes.STATE:
                    mapEntry.Encode(encIter, state);
                    break;
                default:
                    break;
            }
        }

        private static void EncodeBasicMsg(EncodeIterator encIter)
        {
            Msg msg = new Msg();
            msg.StreamId = MsgClasses.UPDATE;
            msg.MsgClass = MsgClasses.UPDATE;
            msg.DomainType = (byte)DomainType.MARKET_PRICE;
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
        }

        public static void DecodeMsgToXMLAndCheck(Buffer msg, MsgParameters msgParameters)
        {
            Msg message = new Msg();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(msg, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            String xml = message.DecodeToXml(decIter);
            var document = new XmlDocument();
            document.LoadXml(xml);
            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var msgNode = comments.NextSibling;
            CheckMsgClass(msgParameters.MsgClass, msgNode!.Name);
            
            Assert.True(msgNode.Attributes!.Count > 0);
            
            var domainType = msgNode.Attributes["domainType"];
            Assert.NotNull(domainType);
            Assert.Equal(domainType!.Value, DomainTypeToString(msgParameters.MsgDomainType));

            var payloadType = msgNode.Attributes["containerType"];
            Assert.NotNull(payloadType);
            Assert.Equal(payloadType!.Value, PayloadTypeToStirng(msgParameters.PayloadType));

            Assert.NotNull(msgNode.Attributes["dataSize"]);

            var flagsNode = msgNode.Attributes["flags"];
            if (flagsNode != null)
            {
                var flags = flagsNode.Value;

                Assert.Equal(msgParameters.HasExtendedHeader, flags.Contains("HAS_EXTENDED_HEADER"));
                Assert.Equal(msgParameters.HasPriority, flags.Contains("HAS_PRIORITY"));
                Assert.Equal(msgParameters.HasPriority, msgNode.Attributes["priorityCount"] != null);
                Assert.Equal(msgParameters.HasPriority, msgNode.Attributes["priorityClass"] != null);
                Assert.Equal(msgParameters.Streaming, flags.Contains("STREAMING"));
                Assert.Equal(msgParameters.HasQos, flags.Contains("HAS_QOS"));
                Assert.Equal(msgParameters.HasQos, msgNode.Attributes["qos"] != null);
                Assert.Equal(msgParameters.HasState || msgParameters.MsgClass == MsgClasses.REFRESH, msgNode.Attributes["state"] != null);
                Assert.Equal(msgParameters.SeqNum != 0, flags.Contains("HAS_SEQ_NUM"));
                Assert.Equal(msgParameters.SeqNum != 0, msgNode.Attributes["seqNum"] != null && msgNode.Attributes["seqNum"]!.Value.Equals(msgParameters.SeqNum.ToString()));
                Assert.Equal(msgParameters.PostComplete, flags.Contains("POST_COMPLETE"));
                Assert.Equal(msgParameters.Ack, flags.Contains("ACK"));
                Assert.Equal(msgParameters.HasPermData, flags.Contains("HAS_PERM_DATA"));
                Assert.Equal(msgParameters.HasPermData, msgNode.Attributes["permData"] != null);
                Assert.Equal(msgParameters.DoNotCache, flags.Contains("DO_NOT_CACHE"));
                Assert.Equal(msgParameters.DoNotConflate, flags.Contains("DO_NOT_CONFLATE"));
                Assert.Equal(msgParameters.DoNotRipple, flags.Contains("DO_NOT_RIPPLE"));
                Assert.Equal(msgParameters.RefreshComplete, flags.Contains("REFRESH_COMPLETE"));
                Assert.Equal(msgParameters.HasMsgKeyInUpdates, flags.Contains("MSG_KEY_IN_UPDATES"));
                Assert.Equal(msgParameters.HasConfInfoInUpdates, flags.Contains("CONF_INFO_IN_UPDATES"));
                Assert.Equal(msgParameters.HasWorstQos, flags.Contains("HAS_WORST_QOS"));
                Assert.Equal(msgParameters.HasWorstQos, msgNode.Attributes["worstQos"] != null);
                Assert.Equal(msgParameters.QualifiedStream, flags.Contains("QUALIFIED_STREAM"));
                Assert.Equal(msgParameters.HasConfInfo, flags.Contains("HAS_CONF_INFO"));
                Assert.Equal(msgParameters.HasPostUserInfo, flags.Contains("HAS_POST_USER_INFO"));
                Assert.Equal(msgParameters.HasPostUserInfo || msgParameters.MsgClass == MsgClasses.POST, msgNode.Attributes["postUserId"] != null && msgNode.Attributes["postUserId"]!.Value.Equals(defaultUserId.ToString()));
                Assert.Equal(msgParameters.HasPostUserInfo || msgParameters.MsgClass == MsgClasses.POST, msgNode.Attributes["postUserAddr"] != null && msgNode.Attributes["postUserAddr"]!.Value.Equals(defaultUserAddr.ToString()));
                Assert.Equal(msgParameters.Discardable, flags.Contains("DISCARDABLE"));
                Assert.Equal(msgParameters.ClearCache, flags.Contains("CLEAR_CACHE"));
                Assert.Equal(msgParameters.Solicited, flags.Contains("SOLICITED"));
                Assert.Equal(msgParameters.MessageComplete, flags.Contains("MESSAGE_COMPLETE"));
                Assert.Equal(msgParameters.HasGroupId && msgParameters.MsgClass != MsgClasses.REFRESH, flags.Contains("HAS_GROUP_ID"));
                Assert.Equal(msgParameters.HasGroupId || msgParameters.MsgClass == MsgClasses.REFRESH, msgNode.Attributes["groupId"] != null);
                Assert.Equal(msgParameters.MsgClass == MsgClasses.ACK, msgNode.Attributes["ackId"] != null && defaultAckId.ToString().Equals(msgNode.Attributes["ackId"]!.Value));
                Assert.Equal(msgParameters.HasPartNum, flags.Contains("HAS_PART_NUM"));
                Assert.Equal(msgParameters.HasPartNum, msgNode.Attributes["partNum"] != null);
                Assert.Equal(msgParameters.SecondarySeqNum != 0, flags.Contains("HAS_SECONDARY_SEQ_NUM"));
                Assert.Equal(msgParameters.HasPartNum, msgNode.Attributes["partNum"] != null);
                Assert.Equal(msgParameters.HasPostId, flags.Contains("HAS_POST_ID"));
                Assert.Equal(msgParameters.HasPostId, msgNode.Attributes["postId"] != null && msgNode.Attributes["postId"]!.Value.Equals(defaultPostId.ToString()));
            }

            var nodesEnumer = msgNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            if (msgParameters.HasMsgKey || msgParameters.MsgClass == MsgClasses.REQUEST)
            {
                Assert.True(hasNext);
                var node = (XmlNode)nodesEnumer.Current;
                Assert.Equal("key", node.Name);
                Assert.True(node.Attributes!.Count > 0);
                var keyName = node.Attributes["name"];
                Assert.NotNull(keyName);
                Assert.Equal(msgKeyBuffer.ToString(), keyName!.Value);
                hasNext = nodesEnumer.MoveNext();
            }
            if (msgParameters.HasExtendedHeader)
            {
                Assert.True(hasNext);
                var node = (XmlNode)nodesEnumer.Current;
                Assert.Equal("extendedHeader", node.Name);
                hasNext = nodesEnumer.MoveNext();
            }
            if (msgParameters.PayloadType != DataTypes.NO_DATA)
            {
                Assert.True(hasNext);
                var node = (XmlNode)nodesEnumer.Current;
                Assert.Equal("dataBody", node.Name);
                var body = node.FirstChild;
                Assert.Equal(body!.Name, DataBodyPayload(msgParameters.PayloadType));
            }
            
        }

        private static void CheckMsgClass(int msgClass, String name)
        {
            switch (name)
            {
                case "REQUEST":
                    Assert.Equal(MsgClasses.REQUEST, msgClass);
                    break;
                case "REFRESH":
                    Assert.Equal(MsgClasses.REFRESH, msgClass);
                    break;
                case "CLOSE":
                    Assert.Equal(MsgClasses.CLOSE, msgClass);
                    break;
                case "ACK":
                    Assert.Equal(MsgClasses.ACK, msgClass);
                    break;
                case "GENERIC":
                    Assert.Equal(MsgClasses.GENERIC, msgClass);
                    break;
                case "POST":
                    Assert.Equal(MsgClasses.POST, msgClass);
                    break;
                case "STATUS":
                    Assert.Equal(MsgClasses.STATUS, msgClass);
                    break;
                case "UPDATE":
                    Assert.Equal(MsgClasses.UPDATE, msgClass);
                    break;
                default:
                    Assert.True(false);
                    break;
            }
        }

        private static string DomainTypeToString(int domainType)
        {
            switch (domainType)
            {
                case (int)DomainType.LOGIN:
                    return "LOGIN";
                case (int)DomainType.DICTIONARY:
                    return "DICTIONARY";
                case (int)DomainType.SOURCE:
                    return "SOURCE";
                case (int)DomainType.SERVICE_PROVIDER_STATUS:
                    return "SERVICE_PROVIDER_STATUS";
                case (int)DomainType.MARKET_BY_ORDER:
                    return "MARKET_BY_ORDER";
                case (int)DomainType.MARKET_BY_PRICE:
                    return "MARKET_BY_PRICE";
                case (int)DomainType.MARKET_BY_TIME:
                    return "MARKET_BY_TIME";
                case (int)DomainType.MARKET_PRICE:
                    return "MARKET_PRICE";
                case (int)DomainType.YIELD_CURVE:
                    return "YIELD_CURVE";
                default:
                    return "";
            }
        }

        private static string PayloadTypeToStirng(int payloadType)
        {
            switch (payloadType)
            {
                case DataTypes.FIELD_LIST:
                    return "FIELD_LIST";
                case DataTypes.FILTER_LIST:
                    return "FILTER_LIST";
                case DataTypes.ELEMENT_LIST:
                    return "ELEMENT_LIST";
                case DataTypes.SERIES:
                    return "SERIES";
                case DataTypes.MAP:
                    return "MAP";
                case DataTypes.VECTOR:
                    return "VECTOR";
                case DataTypes.MSG:
                    return "MSG";
                case DataTypes.NO_DATA:
                    return "NO_DATA";
                default:
                    return "";
            }
        }

        private static string DataBodyPayload(int payloadType)
        {
            switch (payloadType)
            {
                case DataTypes.FIELD_LIST:
                    return "fieldList";
                case DataTypes.FILTER_LIST:
                    return "filterList";
                case DataTypes.ELEMENT_LIST:
                    return "elementList";
                case DataTypes.SERIES:
                    return "series";
                case DataTypes.MAP:
                    return "map";
                case DataTypes.VECTOR:
                    return "vector";
                case DataTypes.MSG:
                    return "UPDATE";
                default:
                    return "";
            }
        }

        public static void EncodeRefreshMsg(EncodeIterator encIter,
                                            MsgParameters msgParameters)
        {
            Msg msg = new Msg()
            {
                MsgClass = msgParameters.MsgClass,
                StreamId = msgParameters.StreamId,
                ContainerType = msgParameters.PayloadType,
                DomainType = msgParameters.MsgDomainType
            };

            State state = new State();
            state.DataState(DataStates.OK);
            state.StreamState(StreamStates.OPEN);
            msg.State = state;

            msg.GroupId = defaultGroupId;

            EncodeSimpleMessage(encIter, msgParameters, msg);
        }


        public static void EncodeAckMsg(EncodeIterator encIter,
                                            MsgParameters msgParameters)
        {
            Msg msg = new Msg()
            {
                MsgClass = msgParameters.MsgClass,
                StreamId = msgParameters.StreamId,
                ContainerType = msgParameters.PayloadType,
                DomainType = msgParameters.MsgDomainType
            };

            msg.AckId = defaultAckId;

            EncodeSimpleMessage(encIter, msgParameters, msg);
        }

        public static void EncodePostMsg(EncodeIterator encIter,
                                            MsgParameters msgParameters)
        {
            Msg msg = new Msg()
            {
                MsgClass = msgParameters.MsgClass,
                StreamId = msgParameters.StreamId,
                ContainerType = msgParameters.PayloadType,
                DomainType = msgParameters.MsgDomainType
            };

            if (msgParameters.HasPostId)
            {
                msg.ApplyHasPostId();
                msg.PostId = defaultPostId;
            }

            msg.PostUserInfo.UserAddrFromString(defaultUserAddr);
            msg.PostUserInfo.UserId = defaultUserId;

            EncodeSimpleMessage(encIter, msgParameters, msg);
        }

        public static void EncodeMessage(EncodeIterator encIter,
                                            MsgParameters msgParameters)
        {
            Msg msg = new Msg()
            {
                MsgClass = msgParameters.MsgClass,
                StreamId = msgParameters.StreamId,
                ContainerType = msgParameters.PayloadType,
                DomainType = msgParameters.MsgDomainType
            };

            EncodeSimpleMessage(encIter, msgParameters, msg);
        }

        public static void EncodeSimpleMessage(EncodeIterator encIter,
                                               MsgParameters msgParameters, Msg msg)
        {
            
            if (msgParameters.HasExtendedHeader)
            {
                msg.ApplyHasExtendedHdr();
                msg.ExtendedHeader = extendedHeaderBuffer;
            }
            if (msgParameters.HasPermData)
            {
                msg.ApplyHasPermData();
                msg.PermData.Data("PermData");
            }
            if (msgParameters.HasMsgKey)
            {
                msg.ApplyHasMsgKey();
                msg.MsgKey.ApplyHasName();
                msg.MsgKey.Name = msgKeyBuffer;
                msg.MsgKey.ServiceId = msgKeyServiceId;
                if (msgParameters.HasMsgKeyType)
                {
                    msg.MsgKey.ApplyHasNameType();
                    msg.MsgKey.NameType = InstrumentNameTypes.RIC;
                }
            }           
            if (msgParameters.DoNotCache)
            {
                msg.ApplyDoNotCache();
            }
            if (msgParameters.DoNotConflate)
            {
                msg.ApplyDoNotConflate();
            }
            if (msgParameters.DoNotRipple)
            {
                msg.ApplyDoNotRipple();
            }
            if (msgParameters.SeqNum != 0)
            {
                msg.ApplyHasSeqNum();
                msg.SeqNum = msgParameters.SeqNum;
            }  
            if (msgParameters.HasState)
            {
                msg.ApplyHasState();
                State state = new State();
                state.DataState(DataStates.OK);
                state.StreamState(StreamStates.OPEN);
                msg.State = state;
            }
            if (msgParameters.NackCode != 0)
            {
                msg.ApplyHasNakCode();
                msg.NakCode = msgParameters.NackCode;
            }           
            if (msgParameters.HasText)
            {
                msg.Text = ascii;
            }
            if (msgParameters.HasQos)
            {
                msg.ApplyHasQos();
                msg.Qos.Timeliness(qos.Timeliness());
                msg.Qos.Rate(qos.Rate());
            }
            if (msgParameters.HasWorstQos)
            {
                msg.ApplyHasWorstQos();
                msg.WorstQos.Timeliness(qos.Timeliness());
                msg.WorstQos.Rate(qos.Rate());
            }
            if (msgParameters.RefreshComplete)
            {
                msg.ApplyRefreshComplete();
            }
            if (msgParameters.NoRefresh)
            {
                msg.ApplyNoRefresh();
            }
            if (msgParameters.PostComplete)
            {
                msg.ApplyPostComplete();
            }
            if (msgParameters.Streaming)
            {
                msg.ApplyStreaming();
            }
            if (msgParameters.PrivateStream)
            {
                msg.ApplyPrivateStream();
            }
            if (msgParameters.HasPriority)
            {
                msg.ApplyHasPriority();
                msg.Priority.Count = 1;
                msg.Priority.PriorityClass = 1;
            }
            if (msgParameters.Ack)
            {
                msg.ApplyAck();
            }
            if (msgParameters.HasPartNum)
            {
                msg.ApplyHasPartNum();
                msg.PartNum = defaultPartNum;
            }
            if (msgParameters.HasPostUserInfo)
            {
                msg.ApplyHasPostUserInfo();
                msg.PostUserInfo.UserAddrFromString(defaultUserAddr);
                msg.PostUserInfo.UserId = defaultUserId;
            }
            if (msgParameters.HasPostUserRights)
            {
                msg.ApplyHasPostUserRights();
                msg.PostUserRights = defaultPostUserRights;
            }
            if (msgParameters.QualifiedStream)
            {
                msg.ApplyQualifiedStream();
            }
            if (msgParameters.SecondarySeqNum != 0)
            {
                msg.ApplyHasSecondarySeqNum();
                msg.SecondarySeqNum = msgParameters.SecondarySeqNum;
            }
            if (msgParameters.Solicited)
            {
                msg.ApplySolicited();
            }
            if (msgParameters.ClearCache)
            {
                msg.ApplyClearCache();
            }
            if (msgParameters.MessageComplete)
            {
                msg.ApplyMessageComplete();
            }
            if (msgParameters.Discardable)
            {
                msg.ApplyDiscardable();
            }
            if (msgParameters.HasGroupId)
            {
                msg.ApplyHasGroupId();
                msg.GroupId = defaultGroupId;
            }
            if (msgParameters.HasConfInfo)
            {
                msg.ApplyHasConfInfo();
                msg.ConflationCount = 1;
                msg.ConflationTime = 1;
            }
            if (msgParameters.HasMsgKeyInUpdates)
            {
                msg.ApplyMsgKeyInUpdates();
            }
            if (msgParameters.HasConfInfoInUpdates)
            {
                msg.ApplyConfInfoInUpdates();
            }

            EncodeIterator payloadIter = new EncodeIterator();
            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(500));
            payloadIter.Clear();
            payloadIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            EncodeDefaultContainer(payloadIter, msgParameters.PayloadType);
            msg.EncodedDataBody = buffer;
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encIter));
        }  
        

        public static void DecodeXMLArrayAndCheck(Buffer arrayBuf, int dataType)
        {
            Array array = new Array();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(arrayBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            String xml = array.DecodeToXml(decIter);
            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLArrayAndCheck(bodyNode!, dataType);
        }

        public static void DecodeXMLArrayAndCheck(XmlNode arrayNode, int dataType)
        {
            Assert.Equal("array", arrayNode.Name);
            Assert.True(arrayNode.Attributes!.Count == 2);
            Assert.NotNull(arrayNode.Attributes["itemLength"]);
            Assert.NotNull(arrayNode.Attributes["primitiveType"]);
            Assert.Equal(arrayNode.Attributes["primitiveType"]!.Value, PrimitiveTypeToString(dataType));
            var nodesEnumer = arrayNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            for (int i = 0; i < length; i++)
            {
                Assert.True(hasNext);
                var child = (XmlNode)nodesEnumer.Current;
                Assert.Equal("arrayEntry", child.Name);
                Assert.NotNull(child.Attributes!["data"]);
                CheckDefaultPrimitiveTypeXmlDump(child.Attributes!["data"]!.Value, dataType);
                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLElementListAndCheck(Buffer elementListBuf, int[] dataTypes)
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../Src/Tests/RDMFieldDictionary", out error));
            ElementList elementList = new ElementList();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(elementListBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            String xml = elementList.DecodeToXml(decIter, dictionary);

            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLElementListAndCheck(bodyNode!, dataTypes);
        }

        public static void DecodeXMLElementListAndCheck(XmlNode elementListNode, int[] dataTypes)
        {
            Assert.Equal("elementList", elementListNode.Name);

            var nodesEnumer = elementListNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            foreach (var type in dataTypes)
            {
                Assert.True(hasNext);
                var entry = (XmlNode)nodesEnumer.Current;
                Assert.Equal("elementEntry", entry.Name);
                Assert.NotNull(entry.Attributes!["dataType"]);
                Assert.Equal(entry.Attributes["dataType"]!.Value, PrimitiveTypeToString(type));
                if (type <= DataTypes.SET_PRIMITIVE_MAX && type != DataTypes.ARRAY)
                {
                    Assert.NotNull(entry.Attributes["data"]);
                    CheckDefaultPrimitiveTypeXmlDump(entry.Attributes["data"]!.Value, type);
                }
                else
                {
                    Assert.True(entry.ChildNodes.Count > 0);
                    DecodeXmlDefaultContainerAndCheck(entry.ChildNodes.Item(0)!, type);
                }

                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLFieldListAndCheck(Buffer filedListBuf, int[] dataTypes)
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../Src/Tests/RDMFieldDictionary", out error));
            FieldList fieldList = new FieldList();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(filedListBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            String xml = fieldList.DecodeToXml(decIter, dictionary);

            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLFieldListAndCheck(bodyNode!, dataTypes);
        }

        public static void DecodeXMLFieldListAndCheck(XmlNode fieldListNode, int[] dataTypes)
        {
            Assert.Equal("fieldList", fieldListNode.Name);

            var nodesEnumer = fieldListNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            foreach (var type in dataTypes)
            {
                Assert.True(hasNext);
                var entry = (XmlNode)nodesEnumer.Current;
                Assert.Equal("fieldEntry", entry.Name);
                Assert.NotNull(entry.Attributes!["dataType"]);
                Assert.Equal(entry.Attributes["dataType"]!.Value, PrimitiveTypeToString(type));
                Assert.NotNull(entry.Attributes["fieldId"]);
                Assert.Equal(entry.Attributes["fieldId"]!.Value, elDataTypeFidMap[type].ToString());
                if (type <= DataTypes.SET_PRIMITIVE_MAX && type != DataTypes.ARRAY)
                {
                    Assert.NotNull(entry.Attributes["data"]);
                }
                else
                {
                    Assert.True(entry.ChildNodes.Count > 0);
                    DecodeXmlDefaultContainerAndCheck(entry.ChildNodes.Item(0)!, type);
                }

                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLVectorAndCheck(Buffer vectorBuf, int dataType,
                                              Boolean hasSummary,
                                              Boolean hasTotalCountHint,
                                              Boolean supportsSorting,
                                              VectorEntryActions[] actions,
                                              Boolean[] hasPermData)
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../Src/Tests/RDMFieldDictionary", out error));
            Vector vector = new Vector();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(vectorBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            String xml = vector.DecodeToXml(decIter, dictionary);

            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLVectorAndCheck(bodyNode!, dataType, hasSummary, hasTotalCountHint, supportsSorting, actions, hasPermData);
        }

        public static void DecodeXMLVectorAndCheck(XmlNode vectorNode, int dataType,
                                              Boolean hasSummary,
                                              Boolean hasTotalCountHint,
                                              Boolean supportsSorting,
                                              VectorEntryActions[] actions,
                                              Boolean[] hasPermData)
        {
            Assert.Equal("vector", vectorNode.Name);
            Assert.True(vectorNode.Attributes!.Count > 2);
            Assert.NotNull(vectorNode.Attributes["containerType"]);
            Assert.Equal(vectorNode.Attributes["containerType"]!.Value, PrimitiveTypeToString(dataType));
            Assert.NotNull(vectorNode.Attributes["flags"]);
            Assert.Equal(hasSummary, vectorNode.Attributes["flags"]!.Value.Contains("HAS_SUMMARY_DATA"));
            Assert.Equal(hasTotalCountHint, vectorNode.Attributes["flags"]!.Value.Contains("HAS_TOTAL_COUNT_HINT"));
            Assert.True(vectorNode.Attributes["countHint"] != null);
            Assert.Contains("HAS_PER_ENTRY_PERM_DATA", vectorNode.Attributes["flags"]!.Value);
            Assert.Equal(supportsSorting, vectorNode.Attributes["flags"]!.Value.Contains("SUPPORTS_SORTING"));

            var nodesEnumer = vectorNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            int nodesNum = actions.Length + (hasSummary ? 1 : 0);
            for (int i = 0; i < nodesNum; i++)
            {
                Assert.True(hasNext);
                var child = (XmlNode)nodesEnumer.Current;
                Assert.Equal(i == 0 && hasSummary ? "summaryData" : "vectorEntry", child.Name);
                if (i == 0 && hasSummary)
                {
                    DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, dataType);
                } else
                {
                    int num = i - (hasSummary ? 1 : 0);
                    Assert.NotNull(child.Attributes!["index"]);
                    Assert.Equal(hasPermData[num], child.Attributes["flags"]!.Value.Contains("HAS_PERM_DATA"));
                    Assert.Equal(hasPermData[num], child.Attributes["permData"] != null);
                    Assert.NotNull(child.Attributes["action"]);
                    Assert.Equal(child.Attributes["action"]!.Value, VectorActionToString(actions[num]));
                    if (actions[num] != VectorEntryActions.CLEAR && actions[num] != VectorEntryActions.DELETE)
                    {
                        DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, dataType);
                    } else
                    {
                        Assert.True(child.ChildNodes.Count == 0);
                    }
                }
                
                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLMapAndCheck(Buffer mapBuf, int containerType,
                                           MapEntryActions[] actions,
                                           Boolean[] hasPermData,
                                           int keyType,
                                           Boolean hasSummary,
                                           Boolean hasKeyFieldId,
                                           Boolean hasTotalHintCount)
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../Src/Tests/RDMFieldDictionary", out error));
            Map map = new Map();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(mapBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            String xml = map.DecodeToXml(decIter, dictionary);

            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLMapAndCheck(bodyNode!, containerType, actions, hasPermData, keyType, hasSummary, hasKeyFieldId, hasTotalHintCount);
        }

        public static void DecodeXMLMapAndCheck(XmlNode mapNode, int containerType,
                                           MapEntryActions[] actions,
                                           Boolean[] hasPermData,
                                           int keyType,
                                           Boolean hasSummary,
                                           Boolean hasKeyFieldId,
                                           Boolean hasTotalHintCount)
        {
            Assert.Equal("map", mapNode.Name);
            Assert.True(mapNode.Attributes!.Count > 0);
            Assert.NotNull(mapNode.Attributes["containerType"]);
            Assert.Equal(mapNode.Attributes["containerType"]!.Value, PrimitiveTypeToString(containerType));
            Assert.NotNull(mapNode.Attributes["flags"]);
            Assert.Equal(hasSummary, mapNode.Attributes["flags"]!.Value.Contains("HAS_SUMMARY_DATA"));
            Assert.Contains("HAS_PER_ENTRY_PERM_DATA", mapNode.Attributes["flags"]!.Value);
            Assert.Equal(hasTotalHintCount, mapNode.Attributes["flags"]!.Value.Contains("HAS_TOTAL_COUNT_HINT"));
            Assert.Equal(hasTotalHintCount, mapNode.Attributes["countHint"] != null && mapNode.Attributes["countHint"]!.Value.Equals("3"));
            Assert.NotNull(mapNode.Attributes["keyPrimitiveType"]);
            Assert.Equal(mapNode.Attributes["keyPrimitiveType"]!.Value, PrimitiveTypeToString(keyType));
            Assert.Equal(hasKeyFieldId, mapNode.Attributes["flags"]!.Value.Contains("HAS_KEY_FIELD_ID"));
            Assert.Equal(hasKeyFieldId, mapNode.Attributes["keyFieldId"] != null && mapNode.Attributes["keyFieldId"]!.Value.Equals("1"));

            var nodesEnumer = mapNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            int nodesNum = actions.Length + (hasSummary ? 1 : 0);
            for (int i = 0; i < nodesNum; i++)
            {
                Assert.True(hasNext);
                var child = (XmlNode)nodesEnumer.Current;
                Assert.Equal(i == 0 && hasSummary ? "summaryData" : "mapEntry", child.Name);
                if (i == 0 && hasSummary)
                {
                    DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, containerType);
                }
                else
                {
                    int num = i - (hasSummary ? 1 : 0);
                    Assert.Equal(hasPermData[num], child.Attributes!["flags"]!.Value.Contains("HAS_PERM_DATA"));
                    Assert.Equal(hasPermData[num], child.Attributes["permData"] != null);
                    Assert.NotNull(child.Attributes["action"]);
                    Assert.Equal(child.Attributes["action"]!.Value, MapActionToString(actions[num]));
                    Assert.NotNull(child.Attributes["key"]);
                    CheckDefaultPrimitiveTypeXmlDump(child.Attributes["key"]!.Value, keyType);
                    if (actions[num] != MapEntryActions.DELETE)
                    {
                        DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, containerType);
                    }
                    else
                    {
                        Assert.True(child.ChildNodes.Count == 0);
                    }
                }

                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLSeriesAndCheck(Buffer vectorBuf, int containerType,
                                              Boolean hasCountHint,
                                              Boolean hasSummary,
                                              int countHint,
                                              int length)
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../Src/Tests/RDMFieldDictionary", out error));
            Series series = new Series();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(vectorBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            String xml = series.DecodeToXml(decIter, dictionary);

            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLSeriesAndCheck(bodyNode!, containerType, hasCountHint, hasSummary, countHint, length);
        }

        public static void DecodeXMLSeriesAndCheck(XmlNode seriesNode, int containerType,
                                              Boolean hasCountHint,
                                              Boolean hasSummary,
                                              int countHint,
                                              int length)
        {
            Assert.Equal("series", seriesNode.Name);
            Assert.True(seriesNode.Attributes!.Count > 2);
            Assert.NotNull(seriesNode.Attributes["containerType"]);
            Assert.Equal(seriesNode.Attributes["containerType"]!.Value, PrimitiveTypeToString(containerType));
            Assert.NotNull(seriesNode.Attributes["flags"]);
            Assert.Equal(hasSummary, seriesNode.Attributes["flags"]!.Value.Contains("HAS_SUMMARY_DATA"));
            Assert.Equal(hasCountHint, seriesNode.Attributes["flags"]!.Value.Contains("HAS_TOTAL_COUNT_HINT"));
            Assert.Equal(hasCountHint, seriesNode.Attributes["countHint"] != null && seriesNode.Attributes["countHint"]!.Value.Equals(countHint.ToString()));

            var nodesEnumer = seriesNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            int nodesNum = length + (hasSummary ? 1 : 0);
            for (int i = 0; i < nodesNum; i++)
            {
                Assert.True(hasNext);
                var child = (XmlNode)nodesEnumer.Current;
                Assert.Equal(i == 0 && hasSummary ? "summaryData" : "seriesEntry", child.Name);
                if (i == 0 && hasSummary)
                {
                    DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, containerType);
                }
                else
                {
                    DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, containerType);
                }

                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLFilterListAndCheck(Buffer filterListBuf, int containerType,
                                                  Boolean hasCountHint,
                                                  FilterEntryActions[] filterActions,
                                                  int[] dataTypes,
                                                  Boolean[] permDataPresent,
                                                  int countHint)
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../../etc/RDMFieldDictionary", out error));
            FilterList filterList = new FilterList();
            DecodeIterator decIter = new DecodeIterator();
            decIter.SetBufferAndRWFVersion(filterListBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            String xml = filterList.DecodeToXml(decIter, dictionary);

            var document = new XmlDocument();
            document.LoadXml(xml);

            var comments = document.FirstChild;
            Assert.Equal(XmlNodeType.Comment, comments!.NodeType);
            var bodyNode = comments.NextSibling;
            DecodeXMLFilterListAndCheck(bodyNode!, containerType, hasCountHint, filterActions, dataTypes, permDataPresent, countHint);
        }

        public static void DecodeXMLFilterListAndCheck(XmlNode filterListNode, int containerType,
                                                  Boolean hasCountHint,
                                                  FilterEntryActions[] filterActions,
                                                  int[] dataTypes,
                                                  Boolean[] permDataPresent,
                                                  int countHint)
        {
            Assert.Equal("filterList", filterListNode.Name);
            Assert.True(filterListNode.Attributes!.Count > 0);
            Assert.NotNull(filterListNode.Attributes["containerType"]);
            Assert.Equal(filterListNode.Attributes["containerType"]!.Value, PrimitiveTypeToString(containerType));
            Assert.NotNull(filterListNode.Attributes["flags"]);
            Assert.Equal(hasCountHint, filterListNode.Attributes["flags"]!.Value.Contains("HAS_TOTAL_COUNT_HINT"));
            Assert.Equal(hasCountHint, filterListNode.Attributes["countHint"] != null && filterListNode.Attributes["countHint"]!.Value.Equals(countHint.ToString()));

            var nodesEnumer = filterListNode.ChildNodes.GetEnumerator();
            Boolean hasNext = nodesEnumer.MoveNext();
            for (int i = 0; i < filterActions.Length; i++)
            {
                Assert.True(hasNext);
                var child = (XmlNode)nodesEnumer.Current;
                Assert.Equal("filterEntry", child.Name);
                Assert.Equal(permDataPresent[i], child.Attributes!["permData"] != null);
                Assert.Equal(child.Attributes["action"]!.Value, FilterActionToString(filterActions[i]));
                Assert.True(child.Attributes["id"] != null);
                Assert.Equal(child.Attributes["id"]!.Value, i.ToString());
                if (filterActions[i] != FilterEntryActions.CLEAR)
                {
                    DecodeXmlDefaultContainerAndCheck(child.ChildNodes.Item(0)!, dataTypes[i]);
                }   else
                {
                    Assert.True(child.ChildNodes.Count == 0);
                }           

                hasNext = nodesEnumer.MoveNext();
            }
        }

        public static void DecodeXMLBasicMsgAndCheck(XmlNode msgNode)
        {
            Assert.Equal("UPDATE", msgNode.Name);
            Assert.Equal("MARKET_PRICE", msgNode.Attributes!["domainType"]!.Value);
            Assert.Equal("4", msgNode.Attributes["streamId"]!.Value);
            Assert.Equal("NO_DATA", msgNode.Attributes["containerType"]!.Value);
            Assert.Equal("0", msgNode.Attributes["dataSize"]!.Value);
            Assert.NotNull(msgNode.Attributes["flags"]);
            Assert.Equal(6, msgNode.Attributes.Count);
        }

        public static void DecodeXMLBasicJsonAndCheck(XmlNode jsonNode)
        {
            Assert.Equal("json", jsonNode.Name);
            Assert.Equal(jsonValueInXml, jsonNode.Attributes!["data"]!.Value);
            Assert.Equal(1, jsonNode.Attributes.Count);
        }

        public static void DecodeXMLBasicOpaqueAndCheck(XmlNode opaqueNode)
        {
            Assert.Equal("opaque", opaqueNode.Name);
            Assert.NotNull(opaqueNode.Attributes!["data"]);
            Assert.Equal(1, opaqueNode.Attributes.Count);
        }

        private static void DecodeXmlDefaultContainerAndCheck(XmlNode container, int dataType)
        {
            switch (dataType)
            {
                case DataTypes.ARRAY:
                    DecodeXMLArrayAndCheck(container, defaultArrayDataType);
                    break;
                case DataTypes.ELEMENT_LIST:
                    DecodeXMLElementListAndCheck(container, defaultElementListTypes);
                    break;
                case DataTypes.FIELD_LIST:
                    DecodeXMLFieldListAndCheck(container, defaultFieldListTypes);
                    break;
                case DataTypes.VECTOR:
                    DecodeXMLVectorAndCheck(container, defaultVectorContainerType, true, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
                    break;
                case DataTypes.MAP:
                    DecodeXMLMapAndCheck(container, defaultMapContainerType, defaultMapEntryActions, defaultMapEntryHasPermData, defaultMapKeyType, true, true, true);
                    break;
                case DataTypes.SERIES:
                    DecodeXMLSeriesAndCheck(container, defaultSeriesContainerType, true, true, defaultSeriesCountHint, defaultSeriesCountHint);
                    break;
                case DataTypes.FILTER_LIST:
                    DecodeXMLFilterListAndCheck(container, defaultFilterListContainerType, true, defaultFilterListActions, defaultFilterListDataTypes, defaultFilterEntryHasPermData, defaultFilterListCountHint);
                    break;
                case DataTypes.MSG:
                    DecodeXMLBasicMsgAndCheck(container);
                    break;
                case DataTypes.OPAQUE:
                    DecodeXMLBasicOpaqueAndCheck(container);
                    break;
                case DataTypes.JSON:
                    DecodeXMLBasicJsonAndCheck(container);
                    break;
                default:
                    break;
            }
        }

        private static string VectorActionToString(VectorEntryActions action)
        {
            switch (action)
            {
                case VectorEntryActions.CLEAR:
                    return "CLEAR";
                case VectorEntryActions.DELETE:
                    return "DELETE";
                case VectorEntryActions.UPDATE:
                    return "UPDATE";
                case VectorEntryActions.SET:
                    return "SET";
                case VectorEntryActions.INSERT:
                    return "INSERT";
                default:
                    return "";
            }
        }

        private static string MapActionToString(MapEntryActions action)
        {
            switch (action)
            {
                case MapEntryActions.ADD:
                    return "ADD";
                case MapEntryActions.DELETE:
                    return "DELETE";
                case MapEntryActions.UPDATE:
                    return "UPDATE";
                default:
                    return "";
            }
        }

        private static string FilterActionToString(FilterEntryActions action)
        {
            switch (action)
            {
                case FilterEntryActions.CLEAR:
                    return "CLEAR";
                case FilterEntryActions.SET:
                    return "SET";
                case FilterEntryActions.UPDATE:
                    return "UPDATE";
                default:
                    return "";
            }
        }

        private static string PrimitiveTypeToString(int primitiveType)
        {
            switch (primitiveType)
            {
                case DataTypes.QOS:
                    return "QOS";
                case DataTypes.UINT:
                    return "UINT";
                case DataTypes.REAL:
                    return "REAL";
                case DataTypes.INT:
                    return "INT";
                case DataTypes.INT_1:
                    return "INT_1";
                case DataTypes.FLOAT:
                    return "FLOAT";
                case DataTypes.RMTES_STRING:
                    return "RMTES_STRING";
                case DataTypes.ASCII_STRING:
                    return "ASCII_STRING";
                case DataTypes.STATE:
                    return "STATE";
                case DataTypes.TIME:
                    return "TIME";
                case DataTypes.DATE:
                    return "DATE";
                case DataTypes.DATETIME:
                    return "DATETIME";
                case DataTypes.DOUBLE:
                    return "DOUBLE";
                case DataTypes.ENUM:
                    return "ENUM";
                case DataTypes.ARRAY:
                    return "ARRAY";
                case DataTypes.MAP:
                    return "MAP";
                case DataTypes.VECTOR:
                    return "VECTOR";
                case DataTypes.SERIES:
                    return "SERIES";
                case DataTypes.ELEMENT_LIST:
                    return "ELEMENT_LIST";
                case DataTypes.FILTER_LIST:
                    return "FILTER_LIST";
                case DataTypes.FIELD_LIST:
                    return "FIELD_LIST";
                case DataTypes.OPAQUE:
                    return "OPAQUE";
                case DataTypes.JSON:
                    return "JSON";
                case DataTypes.MSG:
                    return "MSG";
                default:
                    return "";
            }
        }

        public static void CheckQosXmlDump(String qosStr, Qos qos)
        {
            Assert.Equal(qos.ToString(), qosStr);
        }

        public static void CheckStateXmlDump(String stateStr, State state)
        {
            Assert.Equal(state.ToString().Replace("\"", "\'"), stateStr);
        }

        public static void CheckBufferXmlDump(String bufStr, Buffer buffer)
        {
            Assert.Equal(bufStr, buffer.ToString());
        }

        public static void CheckBufferToHexXmlDump(String bufStr, Buffer buffer)
        {
            Assert.Equal(bufStr, buffer.ToHexString());
        }

        public static void CheckTimeXmlDump(String timeStr, Time time)
        {
            Assert.Equal(timeStr, time.ToString());
        }

        public static void CheckDateXmlDump(String dateStr, Date date)
        {
            Assert.Equal(dateStr, date.ToString());
        }

        public static void CheckDateTimeXmlDump(String dateTimeStr, DateTime dateTime)
        {
            Assert.Equal(dateTimeStr, dateTime.ToString());
        }

        public static void CheckRealXmlDump(String realStr, Real real)
        {
            Assert.Equal(realStr, real.ToString());
        }

        public static void CheckDoubleXmlDump(String doubleStr, Double doubleVal)
        {
            Assert.Equal(doubleStr, doubleVal.ToString());
        }

        public static void CheckDoubleXmlDump(String doubleStr, Float doubleVal)
        {
            Assert.Equal(doubleStr, doubleVal.ToString());
        }

        public static void CheckIntXmlDump(String intStr, Int intVal)
        {
            Assert.Equal(intStr, intVal.ToString());
        }

        public static void CheckUIntXmlDump(String intStr, UInt intVal)
        {
            Assert.Equal(intStr, intVal.ToString());
        }

        public static void CheckEnumXmlDump(String enumStr, Enum enumer)
        {
            Assert.Equal(enumStr, enumer.ToString());
        }

        public static void CheckDefaultPrimitiveTypeXmlDump(String dump, int type)
        {
            switch (type)
            {
                case DataTypes.UINT:
                    CheckUIntXmlDump(dump, uintv);
                    break;
                case DataTypes.DOUBLE:
                    CheckDoubleXmlDump(dump, dv);
                    break;
                case DataTypes.FLOAT:
                    CheckDoubleXmlDump(dump, fv);
                    break;
                case DataTypes.REAL:
                    CheckRealXmlDump(dump, real);
                    break;
                case DataTypes.ASCII_STRING:
                    CheckBufferXmlDump(dump, ascii);
                    break;
                case DataTypes.TIME:
                    CheckTimeXmlDump(dump, time);
                    break;
                case DataTypes.DATE:
                    CheckDateXmlDump(dump, date);
                    break;
                case DataTypes.DATETIME:
                    CheckDateTimeXmlDump(dump, dateTime);
                    break;
                case DataTypes.STATE:
                    CheckStateXmlDump(dump, state);
                    break;
                case DataTypes.QOS:
                    CheckQosXmlDump(dump, qos);
                    break;
                default:
                    break;
            }
        }
    }
}
