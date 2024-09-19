/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access.Tests
{
    public class EmaComplexTypeHandler
    {
        public static int length = 5;
        public static int defaultArrayDataType = DataType.DataTypes.QOS;
        public static int mapKeyFieldId = 1;
        public static int defaultMapKeyType = DataType.DataTypes.STATE;

        public static bool[] defaultVectorEntryHasPermData = { true, false, true };
        public static bool[] defaultMapEntryHasPermData = { true, false, true };
        public static bool[] defaultFilterEntryHasPermData = { true, false, true };

        public static int defaultSeriesCountHint = 3;
        public static int defaultFilterListCountHint = 3;

        public static int defaultMapContainerType = DataType.DataTypes.ELEMENT_LIST;
        public static int defaultSeriesContainerType = DataType.DataTypes.ELEMENT_LIST;
        public static int defaultFilterListContainerType = DataType.DataTypes.ELEMENT_LIST;
        public static int defaultVectorContainerType = DataType.DataTypes.SERIES;

        public static int[] defaultMapAction = { MapAction.UPDATE, MapAction.ADD, MapAction.DELETE };
        public static int[] defaultFilterListActions = { FilterAction.CLEAR, FilterAction.UPDATE, FilterAction.SET };
        public static int[] defaultVectorActions = { VectorAction.UPDATE, VectorAction.INSERT, VectorAction.SET };

        public static int[] defaultFilterListDataTypes = { DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST };
        public static int[] defaultFieldListTypes = { DataType.DataTypes.ASCII, DataType.DataTypes.REAL, DataType.DataTypes.DATE, DataType.DataTypes.TIME };
        public static int[] defaultElementListTypes = { DataType.DataTypes.INT, DataType.DataTypes.REAL, DataType.DataTypes.DATETIME, DataType.DataTypes.ARRAY };

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
        public static EmaBuffer permissionData = new EmaBuffer();
        public static EmaBuffer extendedHdrBuffer = new EmaBuffer();
        public static EmaBuffer groupIdBuffer = new EmaBuffer();

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

        public static IDictionary<int, string> dataTypeNameMap = new Dictionary<int, string>();
        public static IDictionary<int, string> dataTypeBufferNameMap = new Dictionary<int, string>();
        public static IDictionary<int, string> elDataTypeNameMap = new Dictionary<int, string>();
        public static IDictionary<int, int> elDataTypeFidMap = new Dictionary<int, int>();
        public static IDictionary<string, int> nameTypeMap = new Dictionary<String, int>();

        public static OmmArray ommArray = new OmmArray();
        public static ElementList ommElementList = new ElementList();
        public static FieldList ommFieldList = new FieldList();
        public static FilterList ommFilterList = new FilterList();
        public static Map ommMap = new Map();
        public static Vector ommVector = new Vector();
        public static Series ommSeries = new Series();

        public static EmaBuffer xmlEmaBuffer = new EmaBuffer();
        public static EmaBuffer opaqueEmaBuffer = new EmaBuffer();
        public static EmaBuffer ansiPageEmaBuffer = new EmaBuffer();

        public static byte[] xmlBytes = Encoding.GetEncoding("UTF-8").GetBytes("<tag> some XML stuff </tag>");
        public static byte[] opaqueBytes = Encoding.GetEncoding("UTF-8").GetBytes("Opaque string");
        public static byte[] ansiPageBytes = Encoding.GetEncoding("UTF-8").GetBytes("Ansi Page goes here");

        public static MsgParameters defaultAckMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.ACK,
            MsgDomainType = (int)DomainType.MARKET_PRICE,
            StreamId = 2,
            HasExtendedHeader = false,
            HasMsgKey = true,
            HasPayload = false,
            HasMsgKeyType = false,
            HasFilter = true,
            HasAttrib = false,
            HasIdentifier = true,
            HasNackCode = true,
            HasText = true,
            HasSeqNum = true,
            Solicited = false
        };

        public static MsgParameters defaultGenericMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.GENERIC,
            MsgDomainType = (int)DomainType.LOGIN,
            StreamId = 1,
            HasExtendedHeader = true,
            HasMsgKey = true,
            HasPayload = true,
            ContainerType = DataTypes.FIELD_LIST,
            HasQos = false,
            HasMsgKeyType = true,
            HasFilter = false,
            HasAttrib = true,
            AttribContainerType = DataTypes.VECTOR,
            HasIdentifier = true,
            MessageComplete = true,
            HasPartNum = false,
            HasSecondarySeqNum = true,
            HasSeqNum = true,
            Solicited = false,
            HasPermData = true,
            ProviderDriven = true
        };

        public static MsgParameters defaultPostMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.POST,
            MsgDomainType = (int)DomainType.MARKET_PRICE,
            StreamId = 3,
            HasExtendedHeader = true,
            HasMsgKey = false,
            HasPayload = true,
            ContainerType = DataTypes.FILTER_LIST,
            HasFilter = true,
            HasAttrib = false,
            HasIdentifier = true,
            MessageComplete = false,
            HasPartNum = true,
            HasPostId = true,
            HasSeqNum = true,
            Solicited = false,
            HasPermData = true,
            HasPostUserRights = true,
            SolicitAck = true
        };

        public static MsgParameters defaultStatusMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.STATUS,
            MsgDomainType = (int)DomainType.SOURCE,
            StreamId = 1,
            HasExtendedHeader = true,
            HasMsgKey = true,
            HasPayload = true,
            ContainerType = DataTypes.XML,
            HasQos = false,
            PrivateStream = true,
            HasMsgKeyType = true,
            HasFilter = false,
            HasAttrib = true,
            AttribContainerType = DataTypes.OPAQUE,
            HasIdentifier = true,
            HasState = true,
            ClearCache = true,
            HasGroupId = true,
            MessageComplete = false,
            HasPublisherId = true,
            HasPartNum = false,
            Solicited = false,
            HasPermData = true
        };

        public static MsgParameters defaultUpdateMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.UPDATE,
            MsgDomainType = (int)DomainType.SOURCE,
            StreamId = 1,
            HasExtendedHeader = false,
            HasMsgKey = true,
            HasPayload = true,
            ContainerType = DataTypes.ANSI_PAGE,
            HasQos = true,
            PrivateStream = true,
            HasMsgKeyType = true,
            HasFilter = false,
            HasAttrib = true,
            AttribContainerType = DataTypes.MAP,
            HasIdentifier = true,
            HasState = true,
            ClearCache = false,
            DoNotCache = true,
            HasGroupId = false,
            MessageComplete = false,
            HasPublisherId = true,
            HasPartNum = false,
            Solicited = false,
            HasSeqNum = true,
            DoNotConflate = true,
            DoNotRipple = true,
            HasPermData = true,
            HasConfInfo = true
        };

        public static MsgParameters defaultRefreshMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.REFRESH,
            MsgDomainType = (int)DomainType.SOURCE,
            StreamId = 1,
            HasExtendedHeader = true,
            HasMsgKey = true,
            HasPayload = true,
            ContainerType = DataTypes.MAP,
            HasQos = true,
            PrivateStream = true,
            HasMsgKeyType = true,
            HasFilter = false,
            HasAttrib = true,
            AttribContainerType = DataTypes.ELEMENT_LIST,
            HasIdentifier = true,
            HasState = true,
            ClearCache = true,
            DoNotCache = true,
            HasGroupId = true,
            MessageComplete = true,
            HasPublisherId = true,
            HasPartNum = true,
            Solicited = true,
            HasSeqNum = true
        };

        public static MsgParameters defaultRequestMsgParameters = new MsgParameters()
        {
            MsgClass = MsgClasses.REQUEST,
            MsgDomainType = (int)DomainType.MARKET_BY_ORDER,
            StreamId = 1,
            HasExtendedHeader = true,
            HasMsgKey = true,
            HasPayload = true,
            ContainerType = DataTypes.FIELD_LIST,
            Streaming = true,
            HasQos = true,
            Pause = true,
            NoRefresh = true,
            HasPriority = true,
            PrivateStream = true,
            HasMsgKeyType = true,
            HasFilter = true,
            HasAttrib = true,
            AttribContainerType = DataTypes.FILTER_LIST,
            HasIdentifier = true
        };

        internal static EmaObjectManager m_objectManager = new EmaObjectManager();

        public static DataDictionary m_dataDictionary = new DataDictionary();

        private static void LoadDictionary()
        {
            if (m_dataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def", out _) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary.");
                Assert.True(false);
            }

            if (m_dataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary", out _) < 0)
            {
                Console.WriteLine($"Unable to load enum dictionary.");
                Assert.True(false);
            }
        }

        static EmaComplexTypeHandler()
        {
            xmlEmaBuffer.CopyFrom(xmlBytes);
            opaqueEmaBuffer.CopyFrom(opaqueBytes);
            ansiPageEmaBuffer.CopyFrom(ansiPageBytes);
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
            permissionData.Append((byte)'p');
            extendedHdrBuffer.Append((byte)'e');
            extendedHdrBuffer.Append((byte)'x');
            extendedHdrBuffer.Append((byte)'t');
            extendedHdrBuffer.Append((byte)'h');
            extendedHdrBuffer.Append((byte)'d');
            extendedHdrBuffer.Append((byte)'r');

            groupIdBuffer.Append((byte)'1');

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

            dataTypeNameMap.Add(DataType.DataTypes.INT, "PROD_PERM");
            dataTypeNameMap.Add(DataType.DataTypes.REAL, "TRDPRC_2");
            dataTypeNameMap.Add(DataType.DataTypes.DATE, "TRADE_DATE");
            dataTypeNameMap.Add(DataType.DataTypes.TIME, "TRDTIM_1");
            dataTypeNameMap.Add(DataType.DataTypes.ELEMENT_LIST, "EX_MET_DAT");
            dataTypeNameMap.Add(DataType.DataTypes.ASCII, "CASH_BASIS");
            dataTypeNameMap.Add(DataType.DataTypes.VECTOR, "CASH_RATES");
            dataTypeNameMap.Add(DataType.DataTypes.ARRAY, "TENORS");
            dataTypeNameMap.Add(DataType.DataTypes.MAP, "PRE_MAP001");

            elDataTypeNameMap.Add(DataType.DataTypes.INT, "int");
            elDataTypeNameMap.Add(DataType.DataTypes.UINT, "uint");
            elDataTypeNameMap.Add(DataType.DataTypes.DOUBLE, "double");
            elDataTypeNameMap.Add(DataType.DataTypes.FLOAT, "float");
            elDataTypeNameMap.Add(DataType.DataTypes.REAL, "real");
            elDataTypeNameMap.Add(DataType.DataTypes.ARRAY, "array");
            elDataTypeNameMap.Add(DataType.DataTypes.QOS, "qos");
            elDataTypeNameMap.Add(DataType.DataTypes.STATE, "state");
            elDataTypeNameMap.Add(DataType.DataTypes.ASCII, "ascii");
            elDataTypeNameMap.Add(DataType.DataTypes.DATE, "date");
            elDataTypeNameMap.Add(DataType.DataTypes.TIME, "time");
            elDataTypeNameMap.Add(DataType.DataTypes.DATETIME, "datetime");
            elDataTypeNameMap.Add(DataType.DataTypes.ENUM, "enum");
            elDataTypeNameMap.Add(DataType.DataTypes.MAP, "map");
            elDataTypeNameMap.Add(DataType.DataTypes.ELEMENT_LIST, "elementList");
            elDataTypeNameMap.Add(DataType.DataTypes.FIELD_LIST, "fieldList");
            elDataTypeNameMap.Add(DataType.DataTypes.FILTER_LIST, "filterList");
            elDataTypeNameMap.Add(DataType.DataTypes.VECTOR, "vector");
            elDataTypeNameMap.Add(DataType.DataTypes.SERIES, "series");
            elDataTypeNameMap.Add(DataType.DataTypes.OPAQUE, "opaque");
            elDataTypeNameMap.Add(DataType.DataTypes.ANSI_PAGE, "ansi_page");
            elDataTypeNameMap.Add(DataType.DataTypes.XML, "xml");
            elDataTypeNameMap.Add(DataType.DataTypes.MSG, "msg");
            elDataTypeNameMap.Add(DataType.DataTypes.ACK_MSG, "ack");
            elDataTypeNameMap.Add(DataType.DataTypes.GENERIC_MSG, "gen");
            elDataTypeNameMap.Add(DataType.DataTypes.REFRESH_MSG, "ref");
            elDataTypeNameMap.Add(DataType.DataTypes.REQ_MSG, "req");
            elDataTypeNameMap.Add(DataType.DataTypes.POST_MSG, "post");
            elDataTypeNameMap.Add(DataType.DataTypes.STATUS_MSG, "status");
            elDataTypeNameMap.Add(DataType.DataTypes.UPDATE_MSG, "upd");

            elDataTypeFidMap.Add(DataType.DataTypes.INT, intFid);
            elDataTypeFidMap.Add(DataType.DataTypes.REAL, realFid);
            elDataTypeFidMap.Add(DataType.DataTypes.ARRAY, arrayFid);
            elDataTypeFidMap.Add(DataType.DataTypes.ASCII, asciiFid);
            elDataTypeFidMap.Add(DataType.DataTypes.DATE, dateFid);
            elDataTypeFidMap.Add(DataType.DataTypes.TIME, timeFid);
            elDataTypeFidMap.Add(DataType.DataTypes.VECTOR, vectorFid);
            elDataTypeFidMap.Add(DataType.DataTypes.ELEMENT_LIST, elemListFid);
            elDataTypeFidMap.Add(DataType.DataTypes.MAP, mapFid);

            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_INT, DataType.DataTypes.INT);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_UINT, DataType.DataTypes.UINT);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_REAL, DataType.DataTypes.REAL);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_ASCII_STRING, DataType.DataTypes.ASCII);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_DOUBLE, DataType.DataTypes.DOUBLE);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_FLOAT, DataType.DataTypes.FLOAT);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_DATE, DataType.DataTypes.DATE);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_TIME, DataType.DataTypes.TIME);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_QOS, DataType.DataTypes.QOS);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_STATE, DataType.DataTypes.STATE);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_DATE_TIME, DataType.DataTypes.DATETIME);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_ELEMENT_LIST, DataType.DataTypes.ELEMENT_LIST);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_FIELD_LIST, DataType.DataTypes.FIELD_LIST);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_FILTER_LIST, DataType.DataTypes.FILTER_LIST);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_VECTOR, DataType.DataTypes.VECTOR);
            nameTypeMap.Add(ConstCharArrays.DATA_TYPE_STR_SERIES, DataType.DataTypes.SERIES);

            LoadDictionary();
        }

        #region Encode containers & Array

        public static ComplexType? EncodeSummary(int dataType)
        {
            ComplexType? summary = m_objectManager.GetComplexTypeFromPool(dataType);
            switch (dataType)
            {
                case DataType.DataTypes.ELEMENT_LIST:
                    EncodeElementList((ElementList)summary!, defaultElementListTypes);
                    break;

                case DataType.DataTypes.VECTOR:
                    EncodeVector((Vector)summary!, defaultVectorContainerType, false, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
                    break;

                case DataType.DataTypes.FIELD_LIST:
                    EncodeFieldList((FieldList)summary!, defaultFieldListTypes);
                    break;

                case DataType.DataTypes.SERIES:
                    EncodeSeries((Series)summary!, defaultSeriesContainerType, true, false, defaultSeriesCountHint, defaultSeriesCountHint);
                    break;

                case DataType.DataTypes.MAP:
                    EncodeMap((Map)summary!, defaultMapContainerType, defaultMapAction, defaultMapEntryHasPermData, defaultMapKeyType, false, true, true);
                    break;

                case DataType.DataTypes.FILTER_LIST:
                    EncodeFilterList((FilterList)summary!, defaultFilterListContainerType, true, defaultFilterListActions, defaultFilterListDataTypes, defaultFilterEntryHasPermData, defaultFilterListCountHint);
                    break;

                case DataType.DataTypes.OPAQUE:
                    EncodeDefaultOpaque((OmmOpaque)summary!);
                    break;

                case DataType.DataTypes.XML:
                    EncodeDefaultXml((OmmXml)summary!);
                    break;

                case DataType.DataTypes.ANSI_PAGE:
                    EncodeDefaultAnsiPage((OmmAnsiPage)summary!);
                    break;

                default:
                    break;
            }

            return summary;
        }

        public static void EncodeArray(int dataType, OmmArray array)
        {
            for (int i = 0; i < length; i++)
            {
                switch (dataType)
                {
                    case DataType.DataTypes.INT:
                        array.AddInt(iv.ToLong());
                        break;

                    case DataType.DataTypes.UINT:
                        array.AddUInt((ulong)uintv.ToLong());
                        break;

                    case DataType.DataTypes.DOUBLE:
                        array.AddDouble(dv.ToDouble());
                        break;

                    case DataType.DataTypes.FLOAT:
                        array.AddFloat(fv.ToFloat());
                        break;

                    case DataType.DataTypes.REAL:
                        array.AddReal(real.ToLong(), real.Hint);
                        break;

                    case DataType.DataTypes.ASCII:
                        array.AddAscii(ascii.ToString());
                        break;

                    case DataType.DataTypes.DATE:
                        array.AddDate(date.Year(), date.Month(), date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        array.AddTime(time.Hour(), time.Minute(), time.Second());
                        break;

                    case DataType.DataTypes.QOS:
                        array.AddQos((uint)qos.Timeliness(), (uint)qos.Rate());
                        break;

                    case DataType.DataTypes.STATE:
                        array.AddState(state.StreamState(), state.DataState());
                        break;

                    case DataType.DataTypes.DATETIME:
                        array.AddDateTime(dateTime.Year(), dateTime.Month(), dateTime.Day(), dateTime.Hour(), dateTime.Minute());
                        break;

                    case DataType.DataTypes.ENUM:
                        array.AddEnum((ushort)enumer.ToInt());
                        break;

                    default:
                        break;
                }
            }
            array.Complete();
        }

        public static void EncodeFieldList(FieldList fieldList, int[] dataTypes, bool addPreencodedContainerEntries = false)
        {
            for (int i = 0; i < dataTypes.Length; i++)
            {
                switch (dataTypes[i])
                {
                    case DataType.DataTypes.ARRAY:
                        OmmArray array = m_objectManager.GetOmmArray();
                        fieldList.AddArray(elDataTypeFidMap[dataTypes[i]], array);
                        EncodeArray(defaultArrayDataType, array);
                        array.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.INT:
                        fieldList.AddInt(elDataTypeFidMap[dataTypes[i]], iv.ToLong());
                        break;

                    case DataType.DataTypes.REAL:
                        fieldList.AddReal(elDataTypeFidMap[dataTypes[i]], real.ToLong(), real.Hint);
                        break;

                    case DataType.DataTypes.DATE:
                        fieldList.AddDate(elDataTypeFidMap[dataTypes[i]], date.Year(), date.Month(), date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        fieldList.AddTime(elDataTypeFidMap[dataTypes[i]], time.Hour(), time.Minute(), time.Second());
                        break;

                    case DataType.DataTypes.ASCII:
                        fieldList.AddAscii(elDataTypeFidMap[dataTypes[i]], ascii.ToString());
                        break;

                    case DataType.DataTypes.ELEMENT_LIST:
                        ElementList elementList = m_objectManager.GetOmmElementList();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultElementList(elementList);
                            fieldList.AddElementList(elDataTypeFidMap[dataTypes[i]], elementList);
                        }
                        else
                        {
                            fieldList.AddElementList(elDataTypeFidMap[dataTypes[i]], elementList);
                            EncodeDefaultElementList(elementList);
                        }
                        elementList.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.VECTOR:
                        Vector vector = m_objectManager.GetOmmVector();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultVector(vector);
                            fieldList.AddVector(elDataTypeFidMap[dataTypes[i]], vector);
                        }
                        else
                        {
                            fieldList.AddVector(elDataTypeFidMap[dataTypes[i]], vector);
                            EncodeDefaultVector(vector);
                        }
                        vector.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.MAP:
                        Map map = m_objectManager.GetOmmMap();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMap(map);
                            fieldList.AddMap(elDataTypeFidMap[dataTypes[i]], map);
                        }
                        else
                        {
                            fieldList.AddMap(elDataTypeFidMap[dataTypes[i]], map);
                            EncodeDefaultMap(map);
                        }
                        map.ClearAndReturnToPool_All();
                        break;

                    default:
                        break;
                }
            }
            fieldList.Complete();
        }

        public static void EncodeVector(Vector vector,
                                              int dataType,
                                              bool hasSummary,
                                              bool hasTotalCountHint,
                                              bool supportsSorting,
                                              int[] actions,
                                              bool[] hasPermData,
                                              bool addPreencodedContainerEntries = false,
                                              bool entryHasNoData = false)
        {
            if (hasTotalCountHint)
                vector.TotalCountHint(actions.Length);
            if (hasSummary && !entryHasNoData)
            {
                var summary = EncodeSummary(dataType)!;
                vector.SummaryData(summary);
                summary.ClearAndReturnToPool_All();
            }
            if (supportsSorting)
                vector.Sortable(supportsSorting);

            for (uint i = 0; i < actions.Length; i++)
            {
                if (entryHasNoData)
                {
                    vector.Add(i, actions[i], hasPermData[i] ? permissionData : null);
                }
                else
                {
                    ComplexType? complexType = m_objectManager.GetComplexTypeFromPool(dataType);
                    if (addPreencodedContainerEntries)
                    {
                        EncodeDefaultContainer(complexType!, dataType);
                        vector.Add(i, actions[i], complexType!, hasPermData[i] ? permissionData : null);
                    }
                    else
                    {
                        vector.Add(i, actions[i], complexType!, hasPermData[i] ? permissionData : null);
                        EncodeDefaultContainer(complexType!, dataType);
                    }
                    complexType!.ClearAndReturnToPool_All();
                }
            }
            vector.Complete();
        }

        public static void EncodeElementList(ElementList elementList, int[] dataTypes, bool addPreencodedContainerEntries = false)
        {
            for (int i = 0; i < dataTypes.Length; i++)
            {
                switch (dataTypes[i])
                {
                    case DataType.DataTypes.INT:
                        elementList.AddInt(intName.ToString(), iv.ToLong());
                        break;

                    case DataType.DataTypes.UINT:
                        elementList.AddUInt(uintName.ToString(), (uint)uintv.ToLong());
                        break;

                    case DataType.DataTypes.DATE:
                        elementList.AddDate(dateName.ToString(), date.Year(), date.Month(), date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        elementList.AddTime(timeName.ToString(), time.Hour(), time.Minute(), time.Second(), time.Millisecond());
                        break;

                    case DataType.DataTypes.DATETIME:
                        elementList.AddDateTime(dateTimeName.ToString(), dateTime.Year(), dateTime.Month(), dateTime.Day(), dateTime.Hour());
                        break;

                    case DataType.DataTypes.ASCII:
                        elementList.AddAscii(asciiName.ToString(), ascii.ToString());
                        break;

                    case DataType.DataTypes.REAL:
                        elementList.AddReal(realName.ToString(), real.ToLong(), real.Hint);
                        break;

                    case DataType.DataTypes.QOS:
                        elementList.AddQos(qosName.ToString(), (uint)qos.Timeliness(), (uint)qos.Rate());
                        break;

                    case DataType.DataTypes.STATE:
                        elementList.AddState(stateName.ToString(), state.StreamState(), state.DataState());
                        break;

                    case DataType.DataTypes.DOUBLE:
                        elementList.AddDouble(doubleName.ToString(), dv.ToDouble());
                        break;

                    case DataType.DataTypes.FLOAT:
                        elementList.AddFloat(floatName.ToString(), fv.ToFloat());
                        break;

                    case DataType.DataTypes.ENUM:
                        elementList.AddEnum(enumName.ToString(), (ushort)enumer.ToInt());
                        break;

                    case DataType.DataTypes.ARRAY:
                        OmmArray array = m_objectManager.GetOmmArray();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeArray(defaultArrayDataType, array);
                            elementList.AddArray(arrayName.ToString(), array);
                        }
                        else
                        {
                            elementList.AddArray(arrayName.ToString(), array);
                            EncodeArray(defaultArrayDataType, array);
                        }
                        array.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.ELEMENT_LIST:
                        ElementList elementList_1 = m_objectManager.GetOmmElementList();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultElementList(elementList_1);
                            elementList.AddElementList(elDataTypeNameMap[dataTypes[i]].ToString(), elementList_1);
                        }
                        else
                        {
                            elementList.AddElementList(elDataTypeNameMap[dataTypes[i]].ToString(), elementList_1);
                            EncodeDefaultElementList(elementList_1);
                        }
                        elementList_1.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.VECTOR:
                        Vector vector = m_objectManager.GetOmmVector();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultVector(vector);
                            elementList.AddVector(elDataTypeNameMap[dataTypes[i]].ToString(), vector);
                        }
                        else
                        {
                            elementList.AddVector(elDataTypeNameMap[dataTypes[i]].ToString(), vector);
                            EncodeDefaultVector(vector);
                        }
                        vector.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.SERIES:
                        Series series = m_objectManager.GetOmmSeries();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultSeries(series);
                            elementList.AddSeries(elDataTypeNameMap[dataTypes[i]].ToString(), series);
                        }
                        else
                        {
                            elementList.AddSeries(elDataTypeNameMap[dataTypes[i]].ToString(), series);
                            EncodeDefaultSeries(series);
                        }
                        series.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.MAP:
                        Map map = m_objectManager.GetOmmMap();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMap(map);
                            elementList.AddMap(elDataTypeNameMap[dataTypes[i]].ToString(), map);
                        }
                        else
                        {
                            elementList.AddMap(elDataTypeNameMap[dataTypes[i]].ToString(), map);
                            EncodeDefaultMap(map);
                        }
                        map.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.FIELD_LIST:
                        FieldList fieldList = m_objectManager.GetOmmFieldList();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultFieldList(fieldList);
                            elementList.AddFieldList(elDataTypeNameMap[dataTypes[i]].ToString(), fieldList);
                        }
                        else
                        {
                            elementList.AddFieldList(elDataTypeNameMap[dataTypes[i]].ToString(), fieldList);
                            EncodeDefaultFieldList(fieldList);
                        }
                        fieldList.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.FILTER_LIST:
                        FilterList filterList = m_objectManager.GetOmmFilterList();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultFilterList(filterList);
                            elementList.AddFilterList(elDataTypeNameMap[dataTypes[i]].ToString(), filterList);
                        }
                        else
                        {
                            elementList.AddFilterList(elDataTypeNameMap[dataTypes[i]].ToString(), filterList);
                            EncodeDefaultFilterList(filterList);
                        }
                        filterList.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.OPAQUE:
                        OmmOpaque opaque = m_objectManager.GetOmmOpaque();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultOpaque(opaque);
                            elementList.AddOpaque(elDataTypeNameMap[dataTypes[i]], opaque);
                        }
                        else
                        {
                            elementList.AddOpaque(elDataTypeNameMap[dataTypes[i]], opaque);
                            EncodeDefaultOpaque(opaque);
                        }
                        opaque.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.XML:
                        OmmXml xml = m_objectManager.GetOmmXml();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultXml(xml);
                            elementList.AddXml(elDataTypeNameMap[dataTypes[i]], xml);
                        }
                        else
                        {
                            elementList.AddXml(elDataTypeNameMap[dataTypes[i]], xml);
                            EncodeDefaultXml(xml);
                        }
                        xml.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.ANSI_PAGE:
                        OmmAnsiPage ansiPage = m_objectManager.GetOmmAnsiPage();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultAnsiPage(ansiPage);
                            elementList.AddAnsiPage(elDataTypeNameMap[dataTypes[i]], ansiPage);
                        }
                        else
                        {
                            elementList.AddAnsiPage(elDataTypeNameMap[dataTypes[i]], ansiPage);
                            EncodeDefaultAnsiPage(ansiPage);
                        }
                        ansiPage.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.UPDATE_MSG:
                        UpdateMsg updateMsg = m_objectManager.GetOmmUpdateMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(updateMsg);
                            elementList.AddUpdateMsg(elDataTypeNameMap[dataTypes[i]], updateMsg);
                        }
                        else
                        {
                            elementList.AddUpdateMsg(elDataTypeNameMap[dataTypes[i]], updateMsg);
                            EncodeDefaultMessage(updateMsg);
                        }
                        updateMsg.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.STATUS_MSG:
                        StatusMsg statusMsg = m_objectManager.GetOmmStatusMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(statusMsg);
                            elementList.AddStatusMsg(elDataTypeNameMap[dataTypes[i]], statusMsg);
                        }
                        else
                        {
                            elementList.AddStatusMsg(elDataTypeNameMap[dataTypes[i]], statusMsg);
                            EncodeDefaultMessage(statusMsg);
                        }
                        statusMsg.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.REQ_MSG:
                        RequestMsg requestMsg = m_objectManager.GetOmmRequestMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(requestMsg);
                            elementList.AddRequestMsg(elDataTypeNameMap[dataTypes[i]], requestMsg);
                        }
                        else
                        {
                            elementList.AddRequestMsg(elDataTypeNameMap[dataTypes[i]], requestMsg);
                            EncodeDefaultMessage(requestMsg);
                        }
                        requestMsg.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.REFRESH_MSG:
                        RefreshMsg refreshMsg = m_objectManager.GetOmmRefreshMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(refreshMsg);
                            elementList.AddRefreshMsg(elDataTypeNameMap[dataTypes[i]], refreshMsg);
                        }
                        else
                        {
                            elementList.AddRefreshMsg(elDataTypeNameMap[dataTypes[i]], refreshMsg);
                            EncodeDefaultMessage(refreshMsg);
                        }
                        refreshMsg.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.POST_MSG:
                        PostMsg postMsg = m_objectManager.GetOmmPostMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(postMsg);
                            elementList.AddPostMsg(elDataTypeNameMap[dataTypes[i]], postMsg);
                        }
                        else
                        {
                            elementList.AddPostMsg(elDataTypeNameMap[dataTypes[i]], postMsg);
                            EncodeDefaultMessage(postMsg);
                        }
                        postMsg.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.GENERIC_MSG:
                        GenericMsg genericMsg = m_objectManager.GetOmmGenericMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(genericMsg);
                            elementList.AddGenericMsg(elDataTypeNameMap[dataTypes[i]], genericMsg);
                        }
                        else
                        {
                            elementList.AddGenericMsg(elDataTypeNameMap[dataTypes[i]], genericMsg);
                            EncodeDefaultMessage(genericMsg);
                        }
                        genericMsg.ClearAndReturnToPool_All();
                        break;

                    case DataType.DataTypes.ACK_MSG:
                        AckMsg ackMsg = m_objectManager.GetOmmAckMsg();
                        if (addPreencodedContainerEntries)
                        {
                            EncodeDefaultMessage(ackMsg);
                            elementList.AddAckMsg(elDataTypeNameMap[dataTypes[i]], ackMsg);
                        }
                        else
                        {
                            elementList.AddAckMsg(elDataTypeNameMap[dataTypes[i]], ackMsg);
                            EncodeDefaultMessage(ackMsg);
                        }
                        ackMsg.ClearAndReturnToPool_All();
                        break;

                    default:
                        break;
                }
            }
            elementList.Complete();
        }

        public static void EncodeMap(Map map,
                                           int containerType,
                                           int[] entryActions,
                                           bool[] permDataPresent,
                                           int keyType,
                                           bool hasSummary,
                                           bool hasKeyFieldId,
                                           bool hasTotalHintCount,
                                           bool addPreencodedContainerEntries = false)
        {
            if (hasKeyFieldId)
            {
                map.KeyFieldId(mapKeyFieldId);
            }
            if (hasTotalHintCount)
            {
                map.TotalCountHint(entryActions.Length);
            }
            if (hasSummary)
            {
                var summary = EncodeSummary(containerType)!;
                map.SummaryData(summary);
                summary.ClearAndReturnToPool_All();
            }

            for (int i = 0; i < entryActions.Length; i++)
            {
                ComplexType? complexType = m_objectManager.GetComplexTypeFromPool(containerType);
                if (addPreencodedContainerEntries)
                {
                    if (entryActions[i] != MapAction.DELETE)
                    {
                        EncodeDefaultContainer(complexType!, containerType);
                    }
                }
                switch (keyType)
                {
                    case DataType.DataTypes.INT:
                        map.AddKeyInt(1, entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.UINT:
                        map.AddKeyUInt(1, entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.DATE:
                        map.AddKeyDate(date.Year(), date.Month(), date.Day(), entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.TIME:
                        map.AddKeyTime(time.Hour(), time.Minute(), time.Second(), time.Millisecond(), 0, 0, entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.DATETIME:
                        map.AddKeyDateTime(dateTime.Year(), dateTime.Month(), dateTime.Day(), dateTime.Hour(), 0, 0, 0, 0, 0, entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.ASCII:
                        map.AddKeyAscii(ascii.ToString(), entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.REAL:
                        map.AddKeyReal(real.ToLong(), real.Hint, entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.QOS:
                        map.AddKeyQos((uint)qos.Timeliness(), (uint)qos.Rate(), entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.STATE:
                        map.AddKeyState(state.StreamState(), state.DataState(), 0, "", entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.DOUBLE:
                        map.AddKeyDouble(dv.ToDouble(), entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;

                    case DataType.DataTypes.FLOAT:
                        map.AddKeyFloat(fv.ToFloat(), entryActions[i], complexType!, permDataPresent[i] ? permissionData : null);
                        break;
                }
                if (!addPreencodedContainerEntries && entryActions[i] != MapAction.DELETE)
                {
                    EncodeDefaultContainer(complexType!, containerType);
                }
                complexType!.ClearAndReturnToPool_All();
            }
            map.Complete();
        }

        public static void EncodeSeries(Series series,
                                              int containerType,
                                              bool hasCountHint,
                                              bool hasSummary,
                                              int countHint,
                                              int length,
                                              bool addPreencodedContainerEntries = false)
        {
            if (hasCountHint)
            {
                series.TotalCountHint(countHint);
            }
            if (hasSummary)
            {
                var summary = EncodeSummary(containerType)!;
                series.SummaryData(summary);
                summary.ClearAndReturnToPool_All();
            }

            for (int i = 0; i < length; i++)
            {
                ComplexType? complexType = m_objectManager.GetComplexTypeFromPool(containerType);
                if (addPreencodedContainerEntries)
                {
                    EncodeDefaultContainer(complexType!, containerType);
                    series.AddEntry(complexType!);
                }
                else
                {
                    series.AddEntry(complexType!);
                    EncodeDefaultContainer(complexType!, containerType);
                }
                complexType!.ClearAndReturnToPool_All();
            }
            series.Complete();
        }

        public static void EncodeFilterList(FilterList filterList,
                                                  int containerType,
                                                  bool hasCountHint,
                                                  int[] filterActions,
                                                  int[] dataTypes,
                                                  bool[] permDataPresent,
                                                  int countHint,
                                                  bool addPreencodedContainerEntries = false,
                                                  bool[]? entryHasNoData = null)
        {
            if (hasCountHint)
            {
                filterList.TotalCountHint(countHint);
            }
            for (int i = 0; i < filterActions.Length; i++)
            {
                if (entryHasNoData != null && entryHasNoData[i])
                {
                    filterList.AddEntry(i, filterActions[i], permDataPresent[i] ? permissionData : null!);
                }
                else
                {
                    ComplexType? complexType = m_objectManager.GetComplexTypeFromPool(dataTypes[i]);
                    if (addPreencodedContainerEntries)
                    {
                        EncodeDefaultContainer(complexType!, dataTypes[i]);
                        filterList.AddEntry(i, filterActions[i], complexType!, permDataPresent[i] ? permissionData : null!);
                    }
                    else
                    {
                        filterList.AddEntry(i, filterActions[i], complexType!, permDataPresent[i] ? permissionData : null!);
                        if (filterActions[i] != FilterAction.CLEAR)
                        {
                            EncodeDefaultContainer(complexType!, dataTypes[i]);
                        }
                    }
                    complexType!.ClearAndReturnToPool_All();
                }
            }
            filterList.Complete();
        }

        #endregion Encode containers & Array

        #region Encode Messages

        public static void EncodeDefaultMessage(Msg msg)
        {
            switch (msg.DataType)
            {
                case Access.DataType.DataTypes.ACK_MSG:
                    EncodeAckMessage(defaultAckMsgParameters, (AckMsg)msg);
                    break;

                case Access.DataType.DataTypes.GENERIC_MSG:
                    EncodeGenericMessage(defaultGenericMsgParameters, (GenericMsg)msg);
                    break;

                case Access.DataType.DataTypes.POST_MSG:
                    EncodePostMessage(defaultPostMsgParameters, (PostMsg)msg);
                    break;

                case Access.DataType.DataTypes.REFRESH_MSG:
                    EncodeRefreshMessage(defaultRefreshMsgParameters, (RefreshMsg)msg);
                    break;

                case Access.DataType.DataTypes.REQ_MSG:
                    EncodeRequestMessage(defaultRequestMsgParameters, (RequestMsg)msg);
                    break;

                case Access.DataType.DataTypes.STATUS_MSG:
                    EncodeStatusMessage(defaultStatusMsgParameters, (StatusMsg)msg);
                    break;

                case Access.DataType.DataTypes.UPDATE_MSG:
                    EncodeUpdateMessage(defaultUpdateMsgParameters, (UpdateMsg)msg);
                    break;

                default:
                    break;
            }
        }

        public static void EncodeSimpleMessage(MsgParameters msgParameters, Msg msg)
        {
            switch (msg.DataType)
            {
                case Access.DataType.DataTypes.ACK_MSG:
                    EncodeAckMessage(msgParameters, (AckMsg)msg);
                    break;

                case Access.DataType.DataTypes.GENERIC_MSG:
                    EncodeGenericMessage(msgParameters, (GenericMsg)msg);
                    break;

                case Access.DataType.DataTypes.POST_MSG:
                    EncodePostMessage(msgParameters, (PostMsg)msg);
                    break;

                case Access.DataType.DataTypes.REFRESH_MSG:
                    EncodeRefreshMessage(msgParameters, (RefreshMsg)msg);
                    break;

                case Access.DataType.DataTypes.REQ_MSG:
                    EncodeRequestMessage(msgParameters, (RequestMsg)msg);
                    break;

                case Access.DataType.DataTypes.STATUS_MSG:
                    EncodeStatusMessage(msgParameters, (StatusMsg)msg);
                    break;

                case Access.DataType.DataTypes.UPDATE_MSG:
                    EncodeUpdateMessage(msgParameters, (UpdateMsg)msg);
                    break;

                default:
                    break;
            }
        }

        public static void EncodeRequestMessage(MsgParameters msgParameters, RequestMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(5);
            msg.DomainType(msgParameters.MsgDomainType);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("RequestMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            msg.InterestAfterRefresh(msgParameters.Streaming);
            msg.Pause(msgParameters.Pause);
            msg.PrivateStream(msgParameters.PrivateStream);
            if (msgParameters.HasIdentifier)
            {
                msg.Id(2);
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.HasPriority)
            {
                msg.Priority(1, 2);
            }
            if (msgParameters.HasQos)
            {
                msg.Qos(OmmQos.Rates.TICK_BY_TICK, OmmQos.Timelinesses.REALTIME);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        public static void EncodeRefreshMessage(MsgParameters msgParameters, RefreshMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(5);
            msg.DomainType(msgParameters.MsgDomainType);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("RefreshMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            if (msgParameters.PrivateStream)
            {
                msg.PrivateStream(true);
            }
            if (msgParameters.HasIdentifier)
            {
                msg.Id(2);
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.HasQos)
            {
                msg.Qos(OmmQos.Rates.TICK_BY_TICK, OmmQos.Timelinesses.REALTIME);
            }
            if (msgParameters.HasState)
            {
                msg.State(StreamStates.CLOSED_RECOVER, DataStates.SUSPECT);
            }
            if (msgParameters.ClearCache)
            {
                msg.ClearCache(true);
            }
            if (msgParameters.MessageComplete)
            {
                msg.Complete(true);
            }
            if (msgParameters.DoNotCache)
            {
                msg.DoNotCache(true);
            }
            if (msgParameters.Solicited)
            {
                msg.Solicited(true);
            }
            if (msgParameters.HasSeqNum)
            {
                msg.SeqNum(23);
            }
            if (msgParameters.HasPartNum)
            {
                msg.PartNum(51);
            }
            if (msgParameters.HasPublisherId)
            {
                msg.PublisherId(55, 77);
            }
            if (msgParameters.HasGroupId)
            {
                msg.ItemGroup(groupIdBuffer);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        public static void EncodeUpdateMessage(MsgParameters msgParameters, UpdateMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(3);
            msg.DomainType(msgParameters.MsgDomainType);
            msg.UpdateTypeNum(5);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasPermData)
            {
                msg.PermissionData(permissionData);
            }
            if (msgParameters.HasConfInfo)
            {
                msg.Conflated(3, 4);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("UpdateMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            if (msgParameters.HasIdentifier)
            {
                msg.Id(2);
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.DoNotCache)
            {
                msg.DoNotCache(true);
            }
            if (msgParameters.DoNotConflate)
            {
                msg.DoNotConflate(true);
            }
            if (msgParameters.DoNotRipple)
            {
                msg.DoNotRipple(true);
            }
            if (msgParameters.HasSeqNum)
            {
                msg.SeqNum(23);
            }
            if (msgParameters.HasPublisherId)
            {
                msg.PublisherId(55, 77);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        public static void EncodeStatusMessage(MsgParameters msgParameters, StatusMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(4);
            msg.DomainType(msgParameters.MsgDomainType);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasPermData)
            {
                msg.PermissionData(permissionData);
            }
            if (msgParameters.ClearCache)
            {
                msg.ClearCache(true);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("StatusMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            if (msgParameters.PrivateStream)
            {
                msg.PrivateStream(true);
            }
            if (msgParameters.HasState)
            {
                msg.State(StreamStates.CLOSED, DataStates.NO_CHANGE, StateCodes.INVALID_VIEW, "Something went wrong");
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.HasGroupId)
            {
                msg.ItemGroup(groupIdBuffer);
            }
            if (msgParameters.HasPublisherId)
            {
                msg.PublisherId(55, 77);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        public static void EncodeGenericMessage(MsgParameters msgParameters, GenericMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(4);
            msg.DomainType(msgParameters.MsgDomainType);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasPermData)
            {
                msg.PermissionData(permissionData);
            }
            if (msgParameters.HasSecondarySeqNum)
            {
                msg.SecondarySeqNum(22);
            }
            if (msgParameters.HasPartNum)
            {
                msg.PartNum(11);
            }
            if (msgParameters.HasSeqNum)
            {
                msg.SeqNum(11);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("GenericMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.ProviderDriven)
            {
                msg.ProviderDriven(true);
            }
            if (msgParameters.MessageComplete)
            {
                msg.Complete(true);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        public static void EncodePostMessage(MsgParameters msgParameters, PostMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(4);
            msg.DomainType(msgParameters.MsgDomainType);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasPermData)
            {
                msg.PermissionData(permissionData);
            }
            if (msgParameters.HasPostId)
            {
                msg.PostId(9);
            }
            if (msgParameters.HasPartNum)
            {
                msg.PartNum(11);
            }
            if (msgParameters.HasSeqNum)
            {
                msg.SeqNum(11);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("PostMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.HasIdentifier)
            {
                msg.Id(7);
            }
            if (msgParameters.HasPostUserRights)
            {
                msg.PostUserRights(25);
            }
            if (msgParameters.HasPublisherId)
            {
                msg.PublisherId(25, 52);
            }
            if (msgParameters.SolicitAck)
            {
                msg.SolicitAck(true);
            }
            if (msgParameters.MessageComplete)
            {
                msg.Complete(true);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        public static void EncodeAckMessage(MsgParameters msgParameters, AckMsg msg)
        {
            ComplexType? attrib = null;
            ComplexType? payload = null;

            ((MsgEncoder)msg.Encoder!).m_copyByteBuffer = msgParameters.SetCopyByteBuffersFlag;

            msg.StreamId(8);
            msg.DomainType(msgParameters.MsgDomainType);
            msg.AckId(123);
            if (msgParameters.HasExtendedHeader)
            {
                msg.ExtendedHeader(extendedHdrBuffer);
            }
            if (msgParameters.HasNackCode)
            {
                msg.NackCode(NackCode.ACCESS_DENIED);
            }
            if (msgParameters.HasSeqNum)
            {
                msg.SeqNum(11);
            }
            if (msgParameters.HasMsgKey)
            {
                msg.Name("AckMsg");
                msg.ServiceId(1);
            }
            if (msgParameters.HasMsgKeyType)
            {
                msg.NameType(InstrumentNameTypes.RIC);
            }
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            if (msgParameters.HasIdentifier)
            {
                msg.Id(7);
            }
            if (msgParameters.HasText)
            {
                msg.Text("Some text");
            }
            if (msgParameters.PrivateStream)
            {
                msg.PrivateStream(true);
            }
            if (msgParameters.HasAttrib)
            {
                attrib = EmaObjectManager.GetComplexTypeObject(msgParameters.AttribContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                    msg.Attrib(attrib!);
                }
                else
                {
                    msg.Attrib(attrib!);
                    EncodeDefaultContainer(attrib!, msgParameters.AttribContainerType);
                }
            }
            if (msgParameters.HasPayload)
            {
                payload = EmaObjectManager.GetComplexTypeObject(msgParameters.ContainerType);
                if (msgParameters.Preencoded)
                {
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                    msg.Payload(payload!);
                }
                else
                {
                    msg.Payload(payload!);
                    EncodeDefaultContainer(payload!, msgParameters.ContainerType);
                }
            }
            if (msgParameters.CompleteMsgEncoding) msg.EncodeComplete();

            attrib?.ClearAndReturnToPool_All();
            payload?.ClearAndReturnToPool_All();
        }

        #endregion Encode Messages

        #region Encode default contaniers

        public static void EncodeDefaultOpaque(OmmOpaque opaque)
        {
            opaque.SetBuffer(opaqueEmaBuffer);
        }

        public static void EncodeDefaultJson(EncodeIterator encIter)
        {
            Buffer dataBuf = new Buffer();
            encIter.EncodeNonRWFInit(dataBuf);
            dataBuf.Data().Put(jsonBuf.Data());
            encIter.EncodeNonRWFComplete(dataBuf, true);
        }

        public static void EncodeDefaultXml(OmmXml xml)
        {
            xml.SetBuffer(xmlEmaBuffer);
        }

        public static void EncodeDefaultAnsiPage(OmmAnsiPage ansiPage)
        {
            ansiPage.SetBuffer(ansiPageEmaBuffer);
        }

        public static void EncodeDefaultContainer(int dataType, ComplexType container)
        {
            switch (dataType)
            {
                case DataType.DataTypes.ELEMENT_LIST:
                    EncodeDefaultElementList((ElementList)container);
                    break;

                case DataType.DataTypes.VECTOR:
                    EncodeDefaultVector((Vector)container);
                    break;

                case DataType.DataTypes.MAP:
                    EncodeDefaultMap((Map)container);
                    break;

                case DataType.DataTypes.FIELD_LIST:
                    EncodeDefaultFieldList((FieldList)container);
                    break;

                case DataType.DataTypes.FILTER_LIST:
                    EncodeDefaultFilterList((FilterList)container);
                    break;

                case DataType.DataTypes.OPAQUE:
                    EncodeDefaultOpaque((OmmOpaque)container);
                    break;

                case DataType.DataTypes.XML:
                    EncodeDefaultXml((OmmXml)container);
                    break;

                case DataType.DataTypes.ANSI_PAGE:
                    EncodeDefaultAnsiPage((OmmAnsiPage)container);
                    break;

                default:
                    break;
            }
        }

        public static void EncodeDefaultElementList(ElementList elementList)
        {
            EncodeElementList(elementList, defaultElementListTypes);
        }

        public static void EncodeDefaultFieldList(FieldList fieldList)
        {
            EncodeFieldList(fieldList, defaultFieldListTypes);
        }

        public static void EncodeDefaultSeries(Series series)
        {
            EncodeSeries(series, defaultSeriesContainerType, true, false, defaultSeriesCountHint, defaultSeriesCountHint);
        }

        public static void EncodeDefaultMap(Map map)
        {
            EncodeMap(map, defaultMapContainerType, defaultMapAction, defaultMapEntryHasPermData, defaultMapKeyType, false, true, true);
        }

        public static void EncodeDefaultVector(Vector vector)
        {
            EncodeVector(vector, defaultVectorContainerType, false, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
        }

        public static void EncodeDefaultFilterList(FilterList filterList)
        {
            EncodeFilterList(filterList, defaultFilterListContainerType, true, defaultFilterListActions, defaultFilterListDataTypes, defaultFilterEntryHasPermData, defaultFilterListCountHint);
        }

        public static void EncodeDefaultContainer(ComplexType container, int containerType)
        {
            switch (containerType)
            {
                case DataType.DataTypes.ELEMENT_LIST:
                    EncodeElementList((ElementList)container, defaultElementListTypes);
                    break;

                case DataType.DataTypes.VECTOR:
                    EncodeVector((Vector)container, defaultVectorContainerType, false, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
                    break;

                case DataType.DataTypes.FIELD_LIST:
                    EncodeFieldList((FieldList)container, defaultFieldListTypes);
                    break;

                case DataType.DataTypes.SERIES:
                    EncodeSeries((Series)container, defaultSeriesContainerType, true, false, defaultSeriesCountHint, defaultSeriesCountHint);
                    break;

                case DataType.DataTypes.MAP:
                    EncodeMap((Map)container, defaultMapContainerType, defaultMapAction, defaultMapEntryHasPermData, defaultMapKeyType, false, true, true);
                    break;

                case DataType.DataTypes.FILTER_LIST:
                    EncodeFilterList((FilterList)container, defaultFilterListContainerType, true, defaultFilterListActions, defaultFilterListDataTypes, defaultFilterEntryHasPermData, defaultFilterListCountHint);
                    break;

                case DataType.DataTypes.OPAQUE:
                    EncodeDefaultOpaque((OmmOpaque)container);
                    break;

                case DataType.DataTypes.XML:
                    EncodeDefaultXml((OmmXml)container);
                    break;

                case DataType.DataTypes.ANSI_PAGE:
                    EncodeDefaultAnsiPage((OmmAnsiPage)container);
                    break;

                case DataType.DataTypes.ACK_MSG:
                case DataType.DataTypes.GENERIC_MSG:
                case DataType.DataTypes.POST_MSG:
                case DataType.DataTypes.REFRESH_MSG:
                case DataType.DataTypes.REQ_MSG:
                case DataType.DataTypes.UPDATE_MSG:
                case DataType.DataTypes.STATUS_MSG:
                    EncodeDefaultMessage((Msg)container);
                    break;

                default:
                    break;
            }
        }

        #endregion Encode default contaniers

        #region Decode and check containers

        public static void DecodeAndCheckElementList(ElementList elementList, int[] dataTypes)
        {
            int i = 0;
            foreach (var entry in elementList)
            {
                Assert.Equal(dataTypes[i], entry.LoadType);
                switch (entry.LoadType)
                {
                    case DataType.DataTypes.INT:
                        Assert.Equal(intName.ToString(), entry.Name);
                        Assert.Equal(iv.ToLong(), entry.IntValue());
                        break;

                    case DataType.DataTypes.UINT:
                        Assert.Equal(uintName.ToString(), entry.Name);
                        Assert.Equal((ulong)uintv.ToLong(), entry.UIntValue());
                        break;

                    case DataType.DataTypes.DATE:
                        Assert.Equal(dateName.ToString(), entry.Name);
                        var resultDate = entry.OmmDateValue();
                        Assert.Equal(resultDate.Year, date.Year());
                        Assert.Equal(resultDate.Month, date.Month());
                        Assert.Equal(resultDate.Day, date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        Assert.Equal(timeName.ToString(), entry.Name);
                        var resultTime = entry.OmmTimeValue();
                        Assert.Equal(resultTime.Hour, time.Hour());
                        Assert.Equal(resultTime.Minute, time.Minute());
                        Assert.Equal(resultTime.Second, time.Second());
                        Assert.Equal(resultTime.Millisecond, time.Millisecond());
                        break;

                    case DataType.DataTypes.DATETIME:
                        Assert.Equal(dateTimeName.ToString(), entry.Name);
                        var resultDateTime = entry.OmmDateTimeValue();
                        Assert.Equal(resultDateTime.Year, dateTime.Year());
                        Assert.Equal(resultDateTime.Month, dateTime.Month());
                        Assert.Equal(resultDateTime.Day, dateTime.Day());
                        Assert.Equal(resultDateTime.Hour, dateTime.Hour());
                        break;

                    case DataType.DataTypes.ASCII:
                        Assert.Equal(asciiName.ToString(), entry.Name);
                        Assert.Equal(ascii.ToString(), entry.OmmAsciiValue().ToString());
                        break;

                    case DataType.DataTypes.REAL:
                        Assert.Equal(realName.ToString(), entry.Name);
                        Assert.Equal(real.ToDouble(), entry.OmmRealValue().AsDouble());
                        break;

                    case DataType.DataTypes.QOS:
                        Assert.Equal(qosName.ToString(), entry.Name);
                        Assert.Equal((uint)qos.Timeliness(), entry.OmmQosValue().Timeliness);
                        Assert.Equal((uint)qos.Rate(), entry.OmmQosValue().Rate);
                        break;

                    case DataType.DataTypes.STATE:
                        Assert.Equal(stateName.ToString(), entry.Name);
                        var resultState = entry.OmmStateValue();
                        Assert.Equal(state.StreamState(), resultState.StreamState);
                        Assert.Equal(state.DataState(), resultState.DataState);
                        break;

                    case DataType.DataTypes.DOUBLE:
                        Assert.Equal(doubleName.ToString(), entry.Name);
                        var resultDouble = entry.OmmDoubleValue();
                        Assert.Equal(dv.ToDouble(), entry.DoubleValue());
                        break;

                    case DataType.DataTypes.FLOAT:
                        Assert.Equal(floatName.ToString(), entry.Name);
                        var resultFloat = entry.OmmFloatValue();
                        Assert.Equal(fv.ToFloat(), resultFloat.Value);
                        break;

                    case DataType.DataTypes.ARRAY:
                        Assert.Equal(arrayName.ToString(), entry.Name);
                        DecodeAndCheckArray(entry.OmmArrayValue(), defaultArrayDataType);
                        break;

                    case DataType.DataTypes.ELEMENT_LIST:
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        DecodeAndCheckElementList(entry.ElementList(), defaultElementListTypes);
                        break;

                    case DataType.DataTypes.VECTOR:
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        DecodeAndCheckVector(entry.Vector(),
                            defaultVectorContainerType,
                            false, true, true,
                            defaultVectorActions,
                            defaultVectorEntryHasPermData);
                        break;

                    case DataType.DataTypes.SERIES:
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        DecodeAndCheckDefaultContainer(entry.Series());
                        break;

                    case DataType.DataTypes.MAP:
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        DecodeAndCheckMap(entry.Map(),
                            defaultMapContainerType,
                            defaultMapAction,
                            defaultMapEntryHasPermData,
                            defaultMapKeyType,
                            false, true, true);
                        break;

                    case DataType.DataTypes.FIELD_LIST:
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        DecodeAndCheckFieldList(entry.FieldList(), defaultFieldListTypes);
                        break;

                    case DataType.DataTypes.FILTER_LIST:
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        DecodeAndCheckDefaultContainer(entry.FilterList());
                        break;

                    case DataType.DataTypes.OPAQUE:
                    case DataType.DataTypes.XML:
                    case DataType.DataTypes.ANSI_PAGE:
                        DecodeAndCheckDefaultContainer((ComplexType)entry.Load!);
                        Assert.Equal(elDataTypeNameMap[dataTypes[i]], entry.Name);
                        break;

                    case DataType.DataTypes.MSG:
                    case DataType.DataTypes.REFRESH_MSG:
                    case DataType.DataTypes.REQ_MSG:
                    case DataType.DataTypes.UPDATE_MSG:
                    case DataType.DataTypes.ACK_MSG:
                    case DataType.DataTypes.GENERIC_MSG:
                    case DataType.DataTypes.POST_MSG:
                    case DataType.DataTypes.STATUS_MSG:
                        DecodeAndCheckDefaultMessage((Msg)entry.Load!);
                        break;

                    default:
                        break;
                }
                i++;
            }
        }

        public static void DecodeAndCheckFieldList(FieldList fieldList, int[] dataTypes)
        {
            int i = 0;
            foreach (var entry in fieldList)
            {
                var dictionaryEntry = m_dataDictionary.Entry(entry.FieldId);
                if (dictionaryEntry != null)
                {
                    Assert.Equal(dictionaryEntry.GetAcronym().ToString(), entry.Name);
                    Assert.Equal(dictionaryEntry.GetRippleToField(), entry.RippleTo());
                }
                Assert.Equal(elDataTypeFidMap[dataTypes[i]], entry.FieldId);
                switch (dataTypes[i])
                {
                    case DataType.DataTypes.ARRAY:
                        DecodeAndCheckArray(entry.OmmArrayValue(), defaultArrayDataType);
                        break;

                    case DataType.DataTypes.INT:
                        Assert.Equal(iv.ToLong(), entry.IntValue());
                        break;

                    case DataType.DataTypes.REAL:
                        Assert.Equal(real.ToDouble(), entry.OmmRealValue().AsDouble());
                        break;

                    case DataType.DataTypes.DATE:
                        var resultDate = entry.OmmDateValue();
                        Assert.Equal(resultDate.Year, date.Year());
                        Assert.Equal(resultDate.Month, date.Month());
                        Assert.Equal(resultDate.Day, date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        var resultTime = entry.OmmTimeValue();
                        Assert.Equal(resultTime.Hour, time.Hour());
                        Assert.Equal(resultTime.Minute, time.Minute());
                        Assert.Equal(resultTime.Second, time.Second());
                        Assert.Equal(resultTime.Millisecond, time.Millisecond());
                        break;

                    case DataType.DataTypes.ASCII:
                        Assert.Equal(ascii.ToString(), entry.OmmAsciiValue().ToString());
                        break;

                    case DataType.DataTypes.ELEMENT_LIST:
                        DecodeAndCheckElementList(entry.ElementList(), defaultElementListTypes);
                        break;

                    case DataType.DataTypes.VECTOR:
                        DecodeAndCheckDefaultContainer(entry.Vector());
                        break;

                    case DataType.DataTypes.MAP:
                        DecodeAndCheckDefaultContainer(entry.Map());
                        break;

                    default:
                        break;
                }
                i++;
            }
        }

        public static void DecodeAndCheckVector(Vector vector,
                                              int dataType,
                                              bool hasSummary,
                                              bool hasTotalCountHint,
                                              bool supportsSorting,
                                              int[] actions,
                                              bool[] hasPermData,
                                              bool entryHasNoData = false)
        {
            Assert.Equal(hasTotalCountHint, vector.HasTotalCountHint);
            if (hasTotalCountHint)
            {
                Assert.Equal(actions.Length, vector.TotalCountHint());
            }
            Assert.Equal(hasSummary && !entryHasNoData, vector.HasSummaryData);
            if (hasSummary && !entryHasNoData)
            {
                var summary = vector.SummaryData();
                DecodeAndCheckDefaultContainer(summary.Data);
            }
            Assert.Equal(supportsSorting, vector.Sortable());

            int i = 0;
            HashSet<int> foundActions = new HashSet<int>();
            foreach (var entry in vector)
            {
                Assert.Equal(actions[i], entry.Action);
                foundActions.Add(entry.Action);
                Assert.Equal(hasPermData[i], entry.HasPermissionData);
                if (entry.HasPermissionData)
                {
                    Assert.True(permissionData.Equals(entry.PermissionData));
                }
                if (entryHasNoData)
                {
                    Assert.Equal(Access.DataType.DataTypes.NO_DATA, entry.LoadType);
                }
                else
                {
                    if (actions[i] != VectorAction.CLEAR && actions[i] != VectorAction.DELETE)
                    {
                        Assert.Equal(dataType, entry.LoadType);
                        DecodeAndCheckDefaultContainer((ComplexType)entry.Load!);
                    }
                }
                i++;
            }
            Assert.Equal(actions.Length, i);
            foreach (var action in actions) Assert.Contains(action, foundActions);
        }

        public static void DecodeAndCheckMap(Map map,
                                           int containerType,
                                           int[] entryActions,
                                           bool[] permDataPresent,
                                           int keyType,
                                           bool hasSummary,
                                           bool hasKeyFieldId,
                                           bool hasTotalHintCount)
        {
            Assert.Equal(hasKeyFieldId, map.HasKeyFieldId);
            if (hasKeyFieldId)
            {
                Assert.Equal(mapKeyFieldId, map.KeyFieldId());
            }
            Assert.Equal(hasTotalHintCount, map.HasTotalCountHint);
            if (hasTotalHintCount)
            {
                Assert.Equal(entryActions.Length, map.TotalCountHint());
            }
            if (hasSummary)
            {
                var summary = map.SummaryData();
                DecodeAndCheckDefaultContainer(summary.Data);
            }
            Assert.Equal(keyType, map.KeyType());

            int i = 0;
            HashSet<int> foundActions = new HashSet<int>();
            foreach (var entry in map)
            {
                Assert.Equal(entryActions[i], entry.Action);
                foundActions.Add(entry.Action);
                if (entry.Action != MapAction.DELETE)
                {
                    Assert.Equal(containerType, entry.LoadType);
                }

                Assert.Equal(permDataPresent[i], entry.HasPermissionData);
                if (permDataPresent[i])
                {
                    Assert.True(permissionData.Equals(entry.PermissionData));
                }
                var key = entry.Key;
                switch (keyType)
                {
                    case DataType.DataTypes.INT:
                        Assert.Equal(1, key.Int());
                        break;

                    case DataType.DataTypes.UINT:
                        Assert.Equal((ulong)1, key.UInt());
                        break;

                    case DataType.DataTypes.DATE:
                        var resultDate = key.Date();
                        Assert.Equal(resultDate.Year, date.Year());
                        Assert.Equal(resultDate.Month, date.Month());
                        Assert.Equal(resultDate.Day, date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        var resultTime = key.Time();
                        Assert.Equal(resultTime.Hour, time.Hour());
                        Assert.Equal(resultTime.Minute, time.Minute());
                        Assert.Equal(resultTime.Second, time.Second());
                        Assert.Equal(resultTime.Millisecond, time.Millisecond());
                        break;

                    case DataType.DataTypes.DATETIME:
                        var resultDateTime = key.DateTime();
                        Assert.Equal(resultDateTime.Year, dateTime.Year());
                        Assert.Equal(resultDateTime.Month, dateTime.Month());
                        Assert.Equal(resultDateTime.Day, dateTime.Day());
                        Assert.Equal(resultDateTime.Hour, dateTime.Hour());
                        break;

                    case DataType.DataTypes.ASCII:
                        Assert.Equal(ascii.ToString(), key.Ascii().ToString());
                        break;

                    case DataType.DataTypes.REAL:
                        Assert.Equal(real.ToDouble(), key.Real().AsDouble());
                        break;

                    case DataType.DataTypes.QOS:
                        var resultQos = key.Qos();
                        Assert.Equal((uint)qos.Timeliness(), resultQos.Timeliness);
                        Assert.Equal((uint)qos.Rate(), resultQos.Rate);
                        break;

                    case DataType.DataTypes.STATE:
                        var resultState = key.State();
                        Assert.Equal(state.StreamState(), resultState.StreamState);
                        Assert.Equal(state.DataState(), resultState.DataState);
                        break;

                    case DataType.DataTypes.DOUBLE:
                        Assert.Equal(dv.ToDouble(), key.Double());
                        break;

                    case DataType.DataTypes.FLOAT:
                        Assert.Equal(fv.ToFloat(), key.Float());
                        break;
                }
                if (entry.Action != MapAction.DELETE)
                {
                    DecodeAndCheckDefaultContainer((ComplexType)entry.Load!);
                }
                i++;
            }
            Assert.Equal(entryActions.Length, i);
            foreach (var action in entryActions) Assert.Contains(action, foundActions);
        }

        public static void DecodeAndCheckSeries(Series series,
                                              int containerType,
                                              bool hasCountHint,
                                              bool hasSummary,
                                              int countHint,
                                              int length)
        {
            Assert.Equal(hasCountHint, series.HasTotalCountHint);
            if (hasCountHint)
            {
                Assert.Equal(countHint, series.TotalCountHint());
            }
            if (hasSummary)
            {
                var summary = series.SummaryData();
                DecodeAndCheckDefaultContainer(summary.Data);
            }
            int i = 0;
            foreach (var entry in series)
            {
                Assert.Equal(containerType, entry.LoadType);
                DecodeAndCheckDefaultContainer((ComplexType)entry.Load!);
                i++;
            }
            Assert.Equal(i, length);
        }

        public static void DecodeAndCheckFilterList(FilterList filterList,
                                                  int containerType,
                                                  bool hasCountHint,
                                                  int[] filterActions,
                                                  int[] dataTypes,
                                                  bool[] permDataPresent,
                                                  int countHint,
                                                  bool[]? noDataEntries = null)
        {
            Assert.Equal(hasCountHint, filterList.HasTotalCountHint);
            if (hasCountHint)
            {
                Assert.Equal(countHint, filterList.TotalCountHint());
            }
            int i = 0;
            HashSet<int> foundActions = new HashSet<int>();
            foreach (var entry in filterList)
            {
                Assert.Equal(filterActions[i], entry.Action);
                Assert.Equal(permDataPresent[i], entry.HasPermissionData);
                if (permDataPresent[i])
                {
                    Assert.True(permissionData.Equals(entry.PermissionData));
                }
                foundActions.Add(entry.Action);
                if (noDataEntries != null && noDataEntries[i])
                {
                    Assert.Equal(Access.DataType.DataTypes.NO_DATA, entry.LoadType);
                }
                else
                {
                    if (entry.Action != FilterAction.CLEAR)
                    {
                        Assert.Equal(dataTypes[i], entry.LoadType);
                        DecodeAndCheckDefaultContainer((ComplexType)entry.Load!);
                    }
                }
                i++;
            }
            Assert.Equal(filterActions.Length, i);
            foreach (var action in filterActions) Assert.Contains(action, foundActions);
        }

        #endregion Decode and check containers

        #region Decode And Check Messages

        public static void DecodeAndCheckRequestMessage(MsgParameters msgParameters, RequestMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(5, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.True(msg.HasName);
                Assert.Equal("RequestMsg", msg.Name());
                Assert.True(msg.HasServiceId);
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            if (msgParameters.Streaming)
            {
                Assert.True(msg.InterestAfterRefresh());
            }
            else
            {
                Assert.False(msg.InterestAfterRefresh());
            }
            if (msgParameters.Pause)
            {
                Assert.True(msg.Pause());
            }
            else
            {
                Assert.False(msg.Pause());
            }
            if (msgParameters.PrivateStream)
            {
                Assert.True(msg.PrivateStream());
            }
            else
            {
                Assert.False(msg.PrivateStream());
            }
            if (msgParameters.HasIdentifier)
            {
                Assert.True(msg.HasId);
                msg.Id(2);
            }
            else
            {
                Assert.False(msg.HasId);
            }
            if (msgParameters.HasFilter)
            {
                Assert.True(msg.HasFilter);
                msg.Filter(7);
            }
            else
            {
                Assert.False(msg.HasFilter);
            }
            if (msgParameters.HasPriority)
            {
                Assert.True(msg.HasPriority);
                Assert.Equal(1, msg.PriorityClass());
                Assert.Equal(2, msg.PriorityCount());
            }
            else
            {
                Assert.False(msg.HasPriority);
            }

            if (msgParameters.HasQos)
            {
                Assert.True(msg.HasQos);
                Assert.Equal(OmmQos.Rates.TICK_BY_TICK, msg.QosRate());
                Assert.Equal(OmmQos.Timelinesses.REALTIME, msg.QosTimeliness());
            }
            else
            {
                Assert.False(msg.HasQos);
            }

            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckRefreshMessage(MsgParameters msgParameters, RefreshMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(5, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.True(msg.HasName);
                Assert.Equal("RefreshMsg", msg.Name());
                Assert.True(msg.HasServiceId);
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.True(msg.HasNameType);
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            if (msgParameters.PrivateStream)
            {
                Assert.True(msg.PrivateStream());
            }
            else
            {
                Assert.False(msg.PrivateStream());
            }
            if (msgParameters.HasIdentifier)
            {
                Assert.True(msg.HasId);
                msg.Id(2);
            }
            else
            {
                Assert.False(msg.HasId);
            }
            if (msgParameters.HasFilter)
            {
                Assert.True(msg.HasFilter);
                msg.Filter(7);
            }
            else
            {
                Assert.False(msg.HasFilter);
            }
            if (msgParameters.HasQos)
            {
                Assert.True(msg.HasQos);
                Assert.Equal(OmmQos.Rates.TICK_BY_TICK, msg.Qos().Rate);
                Assert.Equal(OmmQos.Timelinesses.REALTIME, msg.Qos().Timeliness);
            }
            else
            {
                Assert.False(msg.HasQos);
            }
            if (msgParameters.HasState)
            {
                Assert.Equal(StreamStates.CLOSED_RECOVER, msg.State().StreamState);
                Assert.Equal(DataStates.SUSPECT, msg.State().DataState);
            }
            Assert.Equal(msgParameters.ClearCache, msg.ClearCache());
            Assert.Equal(msgParameters.MessageComplete, msg.Complete());
            Assert.Equal(msgParameters.DoNotCache, msg.DoNotCache());
            Assert.Equal(msgParameters.Solicited, msg.Solicited());
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(23, msg.SeqNum());
            }
            Assert.Equal(msgParameters.HasPartNum, msg.HasPartNum);
            if (msgParameters.HasPartNum)
            {
                Assert.Equal(51, msg.PartNum());
            }
            Assert.Equal(msgParameters.HasPublisherId, msg.HasPublisherId);
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(55, msg.PublisherIdUserId());
                Assert.Equal(77, msg.PublisherIdUserAddress());
            }
            if (msgParameters.HasGroupId)
            {
                Assert.Equal(groupIdBuffer, msg.ItemGroup());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckUpdateMessage(MsgParameters msgParameters, UpdateMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(3, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(5, msg.UpdateTypeNum());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.HasConfInfo, msg.HasConflated);
            if (msgParameters.HasConfInfo)
            {
                Assert.Equal(3, msg.ConflatedCount());
                Assert.Equal(4, msg.ConflatedTime());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("UpdateMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasIdentifier, msg.HasId);
            if (msgParameters.HasIdentifier)
            {
                Assert.Equal(2, msg.Id());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.DoNotCache, msg.DoNotCache());
            Assert.Equal(msgParameters.DoNotConflate, msg.DoNotConflate());
            Assert.Equal(msgParameters.DoNotRipple, msg.DoNotRipple());
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(23, msg.SeqNum());
            }
            Assert.Equal(msgParameters.HasPublisherId, msg.HasPublisherId);
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(55, msg.PublisherIdUserId());
                Assert.Equal(77, msg.PublisherIdUserAddress());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckStatusMessage(MsgParameters msgParameters, StatusMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(4, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasPermData, msg.HasPermissionData);
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.ClearCache, msg.ClearCache());
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("StatusMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.PrivateStream, msg.PrivateStream());
            Assert.Equal(msgParameters.HasState, msg.HasState);
            if (msgParameters.HasState)
            {
                Assert.Equal(OmmState.StreamStates.CLOSED, msg.State().StreamState);
                Assert.Equal(OmmState.DataStates.NO_CHANGE, msg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.INVALID_VIEW, msg.State().StatusCode);
                Assert.Equal("Something went wrong", msg.State().StatusText);
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            Assert.Equal(msgParameters.HasGroupId, msg.HasItemGroup);
            if (msgParameters.HasGroupId)
            {
                Assert.Equal(groupIdBuffer, msg.ItemGroup());
            }
            Assert.Equal(msgParameters.HasPublisherId, msg.HasPublisherId);
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(55, msg.PublisherIdUserId());
                Assert.Equal(77, msg.PublisherIdUserAddress());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckGenericMessage(MsgParameters msgParameters, GenericMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(4, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasPermData, msg.HasPermissionData);
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.HasSecondarySeqNum, msg.HasSecondarySeqNum);
            if (msgParameters.HasSecondarySeqNum)
            {
                Assert.Equal(22, msg.SecondarySeqNum());
            }
            Assert.Equal(msgParameters.HasPartNum, msg.HasPartNum);
            if (msgParameters.HasPartNum)
            {
                Assert.Equal(11, msg.PartNum());
            }
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(11, msg.SeqNum());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("GenericMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            Assert.Equal(msgParameters.HasMsgKeyType && msgParameters.HasMsgKey, msg.HasNameType);
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.ProviderDriven, msg.ProviderDriven());
            Assert.Equal(msgParameters.MessageComplete, msg.Complete());
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckPostMessage(MsgParameters msgParameters, PostMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(4, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasPermData, msg.HasPermissionData);
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.HasIdentifier, msg.HasId);
            if (msgParameters.HasIdentifier)
            {
                Assert.Equal(7, msg.Id());
            }
            Assert.Equal(msgParameters.HasPartNum, msg.HasPartNum);
            if (msgParameters.HasPartNum)
            {
                Assert.Equal(11, msg.PartNum());
            }
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(11, msg.SeqNum());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("PostMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            Assert.Equal(msgParameters.HasMsgKeyType && msgParameters.HasMsgKey, msg.HasNameType);
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.HasPostUserRights, msg.HasPostUserRights);
            if (msgParameters.HasPostUserRights)
            {
                Assert.Equal(25, msg.PostUserRights());
            }
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(25, msg.PublisherIdUserId());
                Assert.Equal(52, msg.PublisherIdUserAddress());
            }
            Assert.Equal(msgParameters.SolicitAck, msg.SolicitAck());
            Assert.Equal(msgParameters.MessageComplete, msg.Complete());
            Assert.Equal(msgParameters.HasPostId, msg.HasPostId);
            if (msgParameters.HasPostId)
            {
                Assert.Equal(9, msg.PostId());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckAckMessage(MsgParameters msgParameters, AckMsg msg, Buffer body, DataDictionary dataDictionary)
        {
            Assert.Equal(CodecReturnCode.SUCCESS, msg.Decode(body, Codec.MajorVersion(), Codec.MinorVersion(), dataDictionary, null));
            Assert.Equal(8, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(123, msg.AckId());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasNackCode, msg.HasNackCode);
            if (msgParameters.HasNackCode)
            {
                Assert.Equal(NackCode.ACCESS_DENIED, msg.NackCode());
            }
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(11, msg.SeqNum());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("AckMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.HasIdentifier, msg.HasId);
            if (msgParameters.HasIdentifier)
            {
                Assert.Equal(7, msg.Id());
            }
            Assert.Equal(msgParameters.HasText, msg.HasText);
            if (msgParameters.HasText)
            {
                Assert.Equal("Some text", msg.Text());
            }
            Assert.Equal(msgParameters.PrivateStream, msg.PrivateStream());
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        #endregion Decode And Check Messages

        public static void DecodeAndCheckRequestMessage(MsgParameters msgParameters, RequestMsg msg)
        {
            Assert.Equal(5, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.True(msg.HasName);
                Assert.Equal("RequestMsg", msg.Name());
                Assert.True(msg.HasServiceId);
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            if (msgParameters.Streaming)
            {
                Assert.True(msg.InterestAfterRefresh());
            }
            else
            {
                Assert.False(msg.InterestAfterRefresh());
            }
            if (msgParameters.Pause)
            {
                Assert.True(msg.Pause());
            }
            else
            {
                Assert.False(msg.Pause());
            }
            if (msgParameters.PrivateStream)
            {
                Assert.True(msg.PrivateStream());
            }
            else
            {
                Assert.False(msg.PrivateStream());
            }
            if (msgParameters.HasIdentifier)
            {
                Assert.True(msg.HasId);
                msg.Id(2);
            }
            else
            {
                Assert.False(msg.HasId);
            }
            if (msgParameters.HasFilter)
            {
                Assert.True(msg.HasFilter);
                msg.Filter(7);
            }
            else
            {
                Assert.False(msg.HasFilter);
            }
            if (msgParameters.HasPriority)
            {
                Assert.True(msg.HasPriority);
                Assert.Equal(1, msg.PriorityClass());
                Assert.Equal(2, msg.PriorityCount());
            }
            else
            {
                Assert.False(msg.HasPriority);
            }

            if (msgParameters.HasQos)
            {
                Assert.True(msg.HasQos);
                Assert.Equal(OmmQos.Rates.TICK_BY_TICK, msg.QosRate());
                Assert.Equal(OmmQos.Timelinesses.REALTIME, msg.QosTimeliness());
            }
            else
            {
                Assert.False(msg.HasQos);
            }

            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckRefreshMessage(MsgParameters msgParameters, RefreshMsg msg)
        {
            Assert.Equal(5, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.True(msg.HasName);
                Assert.Equal("RefreshMsg", msg.Name());
                Assert.True(msg.HasServiceId);
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.True(msg.HasNameType);
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            if (msgParameters.PrivateStream)
            {
                Assert.True(msg.PrivateStream());
            }
            else
            {
                Assert.False(msg.PrivateStream());
            }
            if (msgParameters.HasIdentifier)
            {
                Assert.True(msg.HasId);
                msg.Id(2);
            }
            else
            {
                Assert.False(msg.HasId);
            }
            if (msgParameters.HasFilter)
            {
                Assert.True(msg.HasFilter);
                msg.Filter(7);
            }
            else
            {
                Assert.False(msg.HasFilter);
            }
            if (msgParameters.HasQos)
            {
                Assert.True(msg.HasQos);
                Assert.Equal(OmmQos.Rates.TICK_BY_TICK, msg.Qos().Rate);
                Assert.Equal(OmmQos.Timelinesses.REALTIME, msg.Qos().Timeliness);
            }
            else
            {
                Assert.False(msg.HasQos);
            }
            if (msgParameters.HasState)
            {
                Assert.Equal(StreamStates.CLOSED_RECOVER, msg.State().StreamState);
                Assert.Equal(DataStates.SUSPECT, msg.State().DataState);
            }
            Assert.Equal(msgParameters.ClearCache, msg.ClearCache());
            Assert.Equal(msgParameters.MessageComplete, msg.Complete());
            Assert.Equal(msgParameters.DoNotCache, msg.DoNotCache());
            Assert.Equal(msgParameters.Solicited, msg.Solicited());
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(23, msg.SeqNum());
            }
            Assert.Equal(msgParameters.HasPartNum, msg.HasPartNum);
            if (msgParameters.HasPartNum)
            {
                Assert.Equal(51, msg.PartNum());
            }
            Assert.Equal(msgParameters.HasPublisherId, msg.HasPublisherId);
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(55, msg.PublisherIdUserId());
                Assert.Equal(77, msg.PublisherIdUserAddress());
            }
            if (msgParameters.HasGroupId)
            {
                Assert.Equal(groupIdBuffer, msg.ItemGroup());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckUpdateMessage(MsgParameters msgParameters, UpdateMsg msg)
        {
            Assert.Equal(3, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(5, msg.UpdateTypeNum());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.HasConfInfo, msg.HasConflated);
            if (msgParameters.HasConfInfo)
            {
                Assert.Equal(3, msg.ConflatedCount());
                Assert.Equal(4, msg.ConflatedTime());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("UpdateMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasIdentifier, msg.HasId);
            if (msgParameters.HasIdentifier)
            {
                Assert.Equal(2, msg.Id());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.DoNotCache, msg.DoNotCache());
            Assert.Equal(msgParameters.DoNotConflate, msg.DoNotConflate());
            Assert.Equal(msgParameters.DoNotRipple, msg.DoNotRipple());
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(23, msg.SeqNum());
            }
            Assert.Equal(msgParameters.HasPublisherId, msg.HasPublisherId);
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(55, msg.PublisherIdUserId());
                Assert.Equal(77, msg.PublisherIdUserAddress());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckStatusMessage(MsgParameters msgParameters, StatusMsg msg)
        {
            Assert.Equal(4, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasPermData, msg.HasPermissionData);
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.ClearCache, msg.ClearCache());
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("StatusMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.PrivateStream, msg.PrivateStream());
            Assert.Equal(msgParameters.HasState, msg.HasState);
            if (msgParameters.HasState)
            {
                Assert.Equal(OmmState.StreamStates.CLOSED, msg.State().StreamState);
                Assert.Equal(OmmState.DataStates.NO_CHANGE, msg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.INVALID_VIEW, msg.State().StatusCode);
                Assert.Equal("Something went wrong", msg.State().StatusText);
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                msg.Filter(7);
            }
            Assert.Equal(msgParameters.HasGroupId, msg.HasItemGroup);
            if (msgParameters.HasGroupId)
            {
                Assert.Equal(groupIdBuffer, msg.ItemGroup());
            }
            Assert.Equal(msgParameters.HasPublisherId, msg.HasPublisherId);
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(55, msg.PublisherIdUserId());
                Assert.Equal(77, msg.PublisherIdUserAddress());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckGenericMessage(MsgParameters msgParameters, GenericMsg msg)
        {
            Assert.Equal(4, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasPermData, msg.HasPermissionData);
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.HasSecondarySeqNum, msg.HasSecondarySeqNum);
            if (msgParameters.HasSecondarySeqNum)
            {
                Assert.Equal(22, msg.SecondarySeqNum());
            }
            Assert.Equal(msgParameters.HasPartNum, msg.HasPartNum);
            if (msgParameters.HasPartNum)
            {
                Assert.Equal(11, msg.PartNum());
            }
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(11, msg.SeqNum());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("GenericMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            Assert.Equal(msgParameters.HasMsgKeyType && msgParameters.HasMsgKey, msg.HasNameType);
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.ProviderDriven, msg.ProviderDriven());
            Assert.Equal(msgParameters.MessageComplete, msg.Complete());
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckPostMessage(MsgParameters msgParameters, PostMsg msg)
        {
            Assert.Equal(4, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasPermData, msg.HasPermissionData);
            if (msgParameters.HasPermData)
            {
                Assert.Equal(permissionData, msg.PermissionData());
            }
            Assert.Equal(msgParameters.HasIdentifier, msg.HasId);
            if (msgParameters.HasIdentifier)
            {
                Assert.Equal(7, msg.Id());
            }
            Assert.Equal(msgParameters.HasPartNum, msg.HasPartNum);
            if (msgParameters.HasPartNum)
            {
                Assert.Equal(11, msg.PartNum());
            }
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(11, msg.SeqNum());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("PostMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            Assert.Equal(msgParameters.HasMsgKeyType && msgParameters.HasMsgKey, msg.HasNameType);
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.HasPostUserRights, msg.HasPostUserRights);
            if (msgParameters.HasPostUserRights)
            {
                Assert.Equal(25, msg.PostUserRights());
            }
            if (msgParameters.HasPublisherId)
            {
                Assert.Equal(25, msg.PublisherIdUserId());
                Assert.Equal(52, msg.PublisherIdUserAddress());
            }
            Assert.Equal(msgParameters.SolicitAck, msg.SolicitAck());
            Assert.Equal(msgParameters.MessageComplete, msg.Complete());
            Assert.Equal(msgParameters.HasPostId, msg.HasPostId);
            if (msgParameters.HasPostId)
            {
                Assert.Equal(9, msg.PostId());
            }
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckAckMessage(MsgParameters msgParameters, AckMsg msg)
        {
            Assert.Equal(8, msg.StreamId());
            Assert.Equal(msgParameters.MsgDomainType, msg.DomainType());
            Assert.Equal(123, msg.AckId());
            Assert.Equal(msgParameters.HasExtendedHeader, msg.HasExtendedHeader);
            if (msgParameters.HasExtendedHeader)
            {
                Assert.Equal(extendedHdrBuffer, msg.ExtendedHeader());
            }
            Assert.Equal(msgParameters.HasNackCode, msg.HasNackCode);
            if (msgParameters.HasNackCode)
            {
                Assert.Equal(NackCode.ACCESS_DENIED, msg.NackCode());
            }
            Assert.Equal(msgParameters.HasSeqNum, msg.HasSeqNum);
            if (msgParameters.HasSeqNum)
            {
                Assert.Equal(11, msg.SeqNum());
            }
            if (msgParameters.HasMsgKey)
            {
                Assert.Equal("AckMsg", msg.Name());
                Assert.Equal(1, msg.ServiceId());
            }
            if (msgParameters.HasMsgKeyType && msgParameters.HasMsgKey)
            {
                Assert.Equal(InstrumentNameTypes.RIC, msg.NameType());
            }
            Assert.Equal(msgParameters.HasFilter, msg.HasFilter);
            if (msgParameters.HasFilter)
            {
                Assert.Equal(7, msg.Filter());
            }
            Assert.Equal(msgParameters.HasIdentifier, msg.HasId);
            if (msgParameters.HasIdentifier)
            {
                Assert.Equal(7, msg.Id());
            }
            Assert.Equal(msgParameters.HasText, msg.HasText);
            if (msgParameters.HasText)
            {
                Assert.Equal("Some text", msg.Text());
            }
            Assert.Equal(msgParameters.PrivateStream, msg.PrivateStream());
            if (msgParameters.HasAttrib)
            {
                Assert.True(msg.HasAttrib);
                ComplexTypeData attrib = msg.Attrib();
                Assert.Equal(msgParameters.AttribContainerType, attrib.DataType);
                DecodeAndCheckDefaultContainer(attrib.Data);
            }
            else
            {
                Assert.False(msg.HasAttrib);
            }
            if (msgParameters.HasPayload)
            {
                ComplexTypeData payload = msg.Payload();
                Assert.Equal(msgParameters.ContainerType, payload.DataType);
                DecodeAndCheckDefaultContainer(payload.Data);
            }
        }

        public static void DecodeAndCheckDefaultMessage(Msg msg)
        {
            switch (msg.DataType)
            {
                case DataType.DataTypes.ACK_MSG:
                    DecodeAndCheckAckMessage(defaultAckMsgParameters, (AckMsg)msg);
                    break;

                case DataType.DataTypes.GENERIC_MSG:
                    DecodeAndCheckGenericMessage(defaultGenericMsgParameters, (GenericMsg)msg);
                    break;

                case DataType.DataTypes.POST_MSG:
                    DecodeAndCheckPostMessage(defaultPostMsgParameters, (PostMsg)msg);
                    break;

                case DataType.DataTypes.REFRESH_MSG:
                    DecodeAndCheckRefreshMessage(defaultRefreshMsgParameters, (RefreshMsg)msg);
                    break;

                case DataType.DataTypes.REQ_MSG:
                    DecodeAndCheckRequestMessage(defaultRequestMsgParameters, (RequestMsg)msg);
                    break;

                case DataType.DataTypes.UPDATE_MSG:
                    DecodeAndCheckUpdateMessage(defaultUpdateMsgParameters, (UpdateMsg)msg);
                    break;

                case DataType.DataTypes.STATUS_MSG:
                    DecodeAndCheckStatusMessage(defaultStatusMsgParameters, (StatusMsg)msg);
                    break;

                default:
                    break;
            }
        }

        public static void DecodeAndCheckDefaultOpaque(OmmOpaque container)
        {
            var buffer = container.Buffer;
            for (int i = 0; i < buffer.Length; i++)
            {
                Assert.Equal(opaqueBytes[i], buffer[i]);
            }
        }

        public static void DecodeAndCheckDefaultAnsiPage(OmmAnsiPage container)
        {
            var buffer = container.Buffer;
            for (int i = 0; i < buffer.Length; i++)
            {
                Assert.Equal(ansiPageBytes[i], buffer[i]);
            }
        }

        public static void DecodeAndCheckDefaultXml(OmmXml container)
        {
            var buffer = container.Value;
            for (int i = 0; i < buffer.Length; i++)
            {
                Assert.Equal(xmlBytes[i], buffer[i]);
            }
        }

        public static void DecodeAndCheckDefaultContainer(ComplexType container)
        {
            switch (container.DataType)
            {
                case DataType.DataTypes.ELEMENT_LIST:
                    DecodeAndCheckElementList((ElementList)container, defaultElementListTypes);
                    break;

                case DataType.DataTypes.VECTOR:
                    DecodeAndCheckVector((Vector)container, defaultVectorContainerType, false, true, true, defaultVectorActions, defaultVectorEntryHasPermData);
                    break;

                case DataType.DataTypes.FIELD_LIST:
                    DecodeAndCheckFieldList((FieldList)container, defaultFieldListTypes);
                    break;

                case DataType.DataTypes.FILTER_LIST:
                    DecodeAndCheckFilterList((FilterList)container,
                        defaultFilterListContainerType,
                        true,
                        defaultFilterListActions,
                        defaultFilterListDataTypes,
                        defaultFilterEntryHasPermData,
                        defaultFilterListCountHint);
                    break;

                case DataType.DataTypes.MAP:
                    DecodeAndCheckMap((Map)container,
                        defaultMapContainerType,
                        defaultMapAction,
                        defaultMapEntryHasPermData,
                        defaultMapKeyType,
                        false, true, true);
                    break;

                case DataType.DataTypes.SERIES:
                    DecodeAndCheckSeries((Series)container, defaultSeriesContainerType, true, false, defaultSeriesCountHint, defaultSeriesCountHint);
                    break;

                case DataType.DataTypes.OPAQUE:
                    DecodeAndCheckDefaultOpaque((OmmOpaque)container);
                    break;

                case DataType.DataTypes.XML:
                    DecodeAndCheckDefaultXml((OmmXml)container);
                    break;

                case DataType.DataTypes.ACK_MSG:
                case DataType.DataTypes.GENERIC_MSG:
                case DataType.DataTypes.POST_MSG:
                case DataType.DataTypes.REFRESH_MSG:
                case DataType.DataTypes.REQ_MSG:
                case DataType.DataTypes.UPDATE_MSG:
                case DataType.DataTypes.STATUS_MSG:
                    DecodeAndCheckDefaultMessage((Msg)container);
                    break;

                default:
                    break;
            }
        }

        private static DecodeIterator primitiveTypeDecodeIterator = new DecodeIterator();
        private static Int ptIntVal = new Int();
        private static UInt ptUintVal = new UInt();
        private static Date ptDateVal = new Date();
        private static Time ptTimeVal = new Time();
        private static DateTime ptDateTimeVal = new DateTime();
        private static Qos ptQosVal = new Qos();
        private static State ptStateVal = new State();
        private static Double ptDoubleVal = new Double();
        private static Float ptFloatVal = new Float();
        private static Real ptRealVal = new Real();
        private static Buffer ptAsciiVal = new Buffer();

        public static void DecodeAndCheckDefaultPrimitiveType(Buffer keyBuffer, int dataType)
        {
            primitiveTypeDecodeIterator.Clear();
            primitiveTypeDecodeIterator.SetBufferAndRWFVersion(keyBuffer, Codec.MajorVersion(), Codec.MinorVersion());
            switch (dataType)
            {
                case DataType.DataTypes.INT:
                    ptIntVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptIntVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptIntVal.Equals(iv));
                    break;

                case DataType.DataTypes.UINT:
                    ptUintVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptUintVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptUintVal.Equals(uintv));
                    break;

                case DataType.DataTypes.DATE:
                    ptDateVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptDateVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptDateVal.Equals(date));
                    break;

                case DataType.DataTypes.TIME:
                    ptTimeVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptTimeVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptTimeVal.Equals(time));
                    break;

                case DataType.DataTypes.DATETIME:
                    ptDateTimeVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptDateTimeVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptDateTimeVal.Equals(dateTime));
                    break;

                case DataType.DataTypes.QOS:
                    ptQosVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptQosVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptQosVal.Equals(qos));
                    break;

                case DataType.DataTypes.STATE:
                    ptStateVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptStateVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptStateVal.Equals(state));
                    break;

                case DataType.DataTypes.DOUBLE:
                    ptDoubleVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptDoubleVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptDoubleVal.Equals(dv));
                    break;

                case DataType.DataTypes.FLOAT:
                    ptFloatVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptFloatVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptFloatVal.Equals(fv));
                    break;

                case DataType.DataTypes.REAL:
                    ptRealVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptRealVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptRealVal.Equals(real));
                    break;

                case DataType.DataTypes.ASCII:
                    ptAsciiVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptAsciiVal.Decode(primitiveTypeDecodeIterator));
                    Assert.True(ptAsciiVal.Equals(ascii));
                    break;

                default:
                    break;
            }
        }

        public static void DecodeAndCheckDefaultPrimitiveType(DecodeIterator decIter, int dataType)
        {
            switch (dataType)
            {
                case DataType.DataTypes.INT:
                    ptIntVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptIntVal.Decode(decIter));
                    Assert.True(ptIntVal.Equals(iv));
                    break;

                case DataType.DataTypes.UINT:
                    ptUintVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptUintVal.Decode(decIter));
                    Assert.True(ptUintVal.Equals(uintv));
                    break;

                case DataType.DataTypes.DATE:
                    ptDateVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptDateVal.Decode(decIter));
                    Assert.True(ptDateVal.Equals(date));
                    break;

                case DataType.DataTypes.TIME:
                    ptTimeVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptTimeVal.Decode(decIter));
                    Assert.True(ptTimeVal.Equals(time));
                    break;

                case DataType.DataTypes.DATETIME:
                    ptDateTimeVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptDateTimeVal.Decode(decIter));
                    Assert.True(ptDateTimeVal.Equals(dateTime));
                    break;

                case DataType.DataTypes.QOS:
                    ptQosVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptQosVal.Decode(decIter));
                    Assert.True(ptQosVal.Equals(qos));
                    break;

                case DataType.DataTypes.STATE:
                    ptStateVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptStateVal.Decode(decIter));
                    Assert.True(ptStateVal.Equals(state));
                    break;

                case DataType.DataTypes.DOUBLE:
                    ptDoubleVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptDoubleVal.Decode(decIter));
                    Assert.True(ptDoubleVal.Equals(dv));
                    break;

                case DataType.DataTypes.FLOAT:
                    ptFloatVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptFloatVal.Decode(decIter));
                    Assert.True(ptFloatVal.Equals(fv));
                    break;

                case DataType.DataTypes.REAL:
                    ptRealVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptRealVal.Decode(decIter));
                    Assert.True(ptRealVal.Equals(real));
                    break;

                case DataType.DataTypes.ASCII:
                    ptAsciiVal.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, ptAsciiVal.Decode(decIter));
                    Assert.Equal(ptAsciiVal.ToString(), ascii.ToString());
                    break;

                default:
                    break;
            }
        }

        public static void DecodeAndCheckArray(OmmArray array, int dataType)
        {
            int i = 0;
            foreach (var entry in array)
            {
                i++;
                switch (dataType)
                {
                    case DataType.DataTypes.INT:
                        Assert.Equal(iv.ToLong(), entry.OmmIntValue().Value);
                        break;

                    case DataType.DataTypes.UINT:
                        Assert.Equal((ulong)uintv.ToLong(), entry.OmmUIntValue().Value);
                        break;

                    case DataType.DataTypes.DOUBLE:
                        Assert.Equal(dv.ToDouble(), entry.OmmDoubleValue().Value);
                        break;

                    case DataType.DataTypes.FLOAT:
                        Assert.Equal(fv.ToFloat(), entry.OmmFloatValue().Value);
                        break;

                    case DataType.DataTypes.REAL:
                        Assert.Equal(real.ToDouble(), entry.OmmRealValue().AsDouble());
                        break;

                    case DataType.DataTypes.ASCII:
                        Assert.Equal(ascii.ToString(), entry.OmmAsciiValue().ToString());
                        break;

                    case DataType.DataTypes.DATE:
                        var resultDate = entry.OmmDateValue();
                        Assert.Equal(resultDate.Year, date.Year());
                        Assert.Equal(resultDate.Month, date.Month());
                        Assert.Equal(resultDate.Day, date.Day());
                        break;

                    case DataType.DataTypes.TIME:
                        var resultTime = entry.OmmTimeValue();
                        Assert.Equal(resultTime.Hour, time.Hour());
                        Assert.Equal(resultTime.Minute, time.Minute());
                        Assert.Equal(resultTime.Second, time.Second());
                        Assert.Equal(resultTime.Millisecond, time.Millisecond());
                        break;

                    case DataType.DataTypes.QOS:
                        //Assert.Equal(qos.Timeliness() != QosTimeliness.DELAYED ? (uint)qos.Timeliness() : (uint)qos.TimeInfo(), entry.OmmQosValue.Timeliness);
                        Assert.Equal(qos.Timeliness(), (int)entry.OmmQosValue().Timeliness);
                        //Assert.Equal(qos.Rate() != QosRates.TIME_CONFLATED ? (uint)qos.Rate() : (uint)qos.RateInfo(), entry.OmmQosValue.Rate);
                        Assert.Equal(qos.Rate(), (int)entry.OmmQosValue().Rate);
                        break;

                    case DataType.DataTypes.STATE:
                        var resultState = entry.OmmStateValue();
                        Assert.Equal(state.StreamState(), resultState.StreamState);
                        Assert.Equal(state.DataState(), resultState.DataState);
                        break;

                    case DataType.DataTypes.DATETIME:
                        var resultDateTime = entry.OmmDateTimeValue();
                        Assert.Equal(resultDateTime.Year, dateTime.Year());
                        Assert.Equal(resultDateTime.Month, dateTime.Month());
                        Assert.Equal(resultDateTime.Day, dateTime.Day());
                        Assert.Equal(resultDateTime.Hour, dateTime.Hour());
                        break;

                    case DataType.DataTypes.ENUM:
                        Assert.Equal(enumer.ToInt(), entry.OmmEnumValue().Value);
                        break;

                    default:
                        break;
                }
            }
            Assert.Equal(length, i);
        }

        public static void EncodeElementListWithCodeValues(ElementList elementList, int[] dataTypes)
        {
            for (int i = 0; i < dataTypes.Length; i++)
            {
                switch (dataTypes[i])
                {
                    case DataType.DataTypes.INT:
                        elementList.AddCodeInt(intName.ToString());
                        break;

                    case DataType.DataTypes.UINT:
                        elementList.AddCodeUInt(uintName.ToString());
                        break;

                    case DataType.DataTypes.DATE:
                        elementList.AddCodeDate(dateName.ToString());
                        break;

                    case DataType.DataTypes.TIME:
                        elementList.AddCodeTime(timeName.ToString());
                        break;

                    case DataType.DataTypes.DATETIME:
                        elementList.AddCodeDateTime(dateTimeName.ToString());
                        break;

                    case DataType.DataTypes.ASCII:
                        elementList.AddCodeAscii(asciiName.ToString());
                        break;

                    case DataType.DataTypes.REAL:
                        elementList.AddCodeReal(realName.ToString());
                        break;

                    case DataType.DataTypes.QOS:
                        elementList.AddCodeQos(qosName.ToString());
                        break;

                    case DataType.DataTypes.RMTES:
                        elementList.AddCodeRmtes("rmtes");
                        break;

                    case DataType.DataTypes.BUFFER:
                        elementList.AddCodeBuffer("buffer");
                        break;

                    case DataType.DataTypes.STATE:
                        elementList.AddCodeState(stateName.ToString());
                        break;

                    case DataType.DataTypes.DOUBLE:
                        elementList.AddCodeDouble(doubleName.ToString());
                        break;

                    case DataType.DataTypes.FLOAT:
                        elementList.AddCodeFloat(floatName.ToString());
                        break;

                    case DataType.DataTypes.ENUM:
                        elementList.AddCodeEnum(enumName.ToString());
                        break;
                    case DataType.DataTypes.ARRAY:
                        elementList.AddCodeArray(enumName.ToString());
                        break;

                    default:
                        break;
                }
            }
            elementList.Complete();
        }

        public static void DecodeAndCheckElementListWithCodeValues(ElementList elementList)
        {
            foreach (var entry in elementList)
            {
                switch (entry.LoadType)
                {
                    case DataType.DataTypes.INT:
                        Assert.Equal(intName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.UINT:
                        Assert.Equal(uintName.ToString(), entry.Name);
                        break;

                    case DataType.DataTypes.DATE:
                        Assert.Equal(dateName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.TIME:
                        Assert.Equal(timeName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.DATETIME:
                        Assert.Equal(dateTimeName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.ASCII:
                        Assert.Equal(asciiName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.REAL:
                        Assert.Equal(realName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.QOS:
                        Assert.Equal(qosName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.RMTES:
                        Assert.Equal("rmtes", entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.BUFFER:
                        Assert.Equal("buffer", entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.STATE:
                        Assert.Equal(stateName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.DOUBLE:
                        Assert.Equal(doubleName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.FLOAT:
                        Assert.Equal(floatName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;

                    case DataType.DataTypes.ENUM:
                        Assert.Equal(enumName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.ARRAY:
                        Assert.Equal(enumName.ToString(), entry.Name);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    default:
                        break;
                }
            }
        }

        public static void EncodeFieldListWithCodeValues(FieldList fieldList, int[] dataTypes)
        {
            for (int i = 0; i < dataTypes.Length; i++)
            {
                switch (dataTypes[i])
                {
                    case DataType.DataTypes.INT:
                        fieldList.AddCodeInt(5170);
                        break;
                    case DataType.DataTypes.UINT:
                        fieldList.AddCodeUInt(1);
                        break;
                    case DataType.DataTypes.DATE:
                        fieldList.AddCodeDate(16);
                        break;
                    case DataType.DataTypes.TIME:
                        fieldList.AddCodeTime(18);
                        break;
                    case DataType.DataTypes.DATETIME:
                        fieldList.AddCodeDateTime(-1);
                        break;
                    case DataType.DataTypes.ASCII:
                        fieldList.AddCodeAscii(235);
                        break;
                    case DataType.DataTypes.REAL:
                        fieldList.AddCodeReal(6);
                        break;
                    case DataType.DataTypes.QOS:
                        fieldList.AddCodeQos(-2);
                        break;
                    case DataType.DataTypes.RMTES:
                        fieldList.AddCodeRmtes(3);
                        break;
                    case DataType.DataTypes.BUFFER:
                        fieldList.AddCodeBuffer(457);
                        break;
                    case DataType.DataTypes.STATE:
                        fieldList.AddCodeState(-3);
                        break;
                    case DataType.DataTypes.DOUBLE:
                        fieldList.AddCodeDouble(-4);
                        break;
                    case DataType.DataTypes.FLOAT:
                        fieldList.AddCodeFloat(-5);
                        break;
                    case DataType.DataTypes.ENUM:
                        fieldList.AddCodeEnum(4);
                        break;
                    case DataType.DataTypes.ARRAY:
                        fieldList.AddCodeArray(30015);
                        break;
                    default:
                        break;
                }
            }
            fieldList.Complete();
        }
        public static void DecodeAndCheckFieldListWithCodeValues(FieldList fieldList)
        {
            foreach (var entry in fieldList)
            {
                switch (entry.LoadType)
                {
                    case DataType.DataTypes.INT:
                        Assert.Equal(5170, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.UINT:
                        Assert.Equal(1, entry.FieldId);
                        break;
                    case DataType.DataTypes.DATE:
                        Assert.Equal(16, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.TIME:
                        Assert.Equal(18, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.DATETIME:
                        Assert.Equal(-1, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.ASCII:
                        Assert.Equal(235, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.REAL:
                        Assert.Equal(6, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.QOS:
                        Assert.Equal(-2, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.RMTES:
                        Assert.Equal(3, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.BUFFER:
                        Assert.Equal(457, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.STATE:
                        Assert.Equal(-3, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.DOUBLE:
                        Assert.Equal(-4, entry.FieldId);
                        break;
                    case DataType.DataTypes.FLOAT:
                        Assert.Equal(-5, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.ENUM:
                        Assert.Equal(4, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    case DataType.DataTypes.ARRAY:
                        Assert.Equal(30015, entry.FieldId);
                        Assert.Equal(Data.DataCode.BLANK, entry.Code);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    public class MsgParameters
    {
        public Boolean HasMsgKey { get; set; }
        public Boolean HasExtendedHeader { get; set; }
        public Boolean HasPermData { get; set; }
        public int StreamId { get; set; }
        public int ContainerType { get; set; }
        public int MsgDomainType { get; set; }
        public int MsgClass { get; set; }
        public int PayloadType { get; set; }
        public Boolean DoNotCache { get; set; }
        public Boolean DoNotConflate { get; set; }
        public Boolean HasSeqNum { get; set; }
        public Boolean HasPartNum { get; set; }
        public Boolean HasPublisherId { get; set; }
        public int SeqNum { get; set; }
        public Boolean HasState { get; set; }
        public int NackCode { get; set; }
        public Boolean HasText { get; set; }
        public Boolean Ack { get; set; }
        public Boolean PostComplete { get; set; }
        public Boolean HasPostUserRights { get; set; }
        public Boolean RefreshComplete { get; set; }
        public Boolean Streaming { get; set; }
        public Boolean HasQos { get; set; }
        public Boolean Pause { get; set; }
        public Boolean NoRefresh { get; set; }
        public Boolean HasPriority { get; set; }
        public Boolean PrivateStream { get; set; }
        public Boolean HasMsgKeyType { get; set; }
        public Boolean HasMsgKeyInUpdates { get; set; }
        public Boolean HasConfInfoInUpdates { get; set; }
        public Boolean HasWorstQos { get; set; }
        public Boolean QualifiedStream { get; set; }
        public Boolean HasConfInfo { get; set; }
        public Boolean DoNotRipple { get; set; }
        public Boolean HasPostUserInfo { get; set; }
        public Boolean Discardable { get; set; }
        public int SecondarySeqNum { get; set; }
        public Boolean ClearCache { get; set; }
        public Boolean Solicited { get; set; }
        public Boolean MessageComplete { get; set; }
        public Boolean HasGroupId { get; set; }
        public Boolean HasPostId { get; set; }
        public Boolean HasIdentifier { get; set; }
        public Boolean HasFilter { get; set; }
        public Boolean HasAttrib { get; set; }
        public int AttribContainerType { get; set; }
        public Boolean HasPayload { get; set; }
        public Boolean HasSecondarySeqNum { get; set; }
        public Boolean ProviderDriven { get; set; }
        public Boolean SolicitAck { get; set; }
        public Boolean HasNackCode { get; set; }
        public bool Preencoded { get; set; }
        public bool CompleteMsgEncoding { get; set; } = true;
        public bool SetCopyByteBuffersFlag { get; set; }

        public void Clear()
        {
            HasMsgKey = false;
            HasExtendedHeader = false;
            HasPermData = false;
            StreamId = 0;
            ContainerType = 0;
            MsgDomainType = 0;
            MsgClass = 0;
            PayloadType = 0;
            DoNotCache = false;
            DoNotConflate = false;
            HasSeqNum = false;
            HasPartNum = false;
            HasPublisherId = false;
            SeqNum = 0;
            HasState = false;
            NackCode = 0;
            HasText = false;
            Ack = false;
            PostComplete = false;
            HasPostUserRights = false;
            RefreshComplete = false;
            Streaming = false;
            HasQos = false;
            Pause = false;
            NoRefresh = false;
            HasPriority = false;
            PrivateStream = false;
            HasMsgKeyType = false;
            HasMsgKeyInUpdates = false;
            HasConfInfoInUpdates = false;
            HasWorstQos = false;
            QualifiedStream = false;
            HasConfInfo = false;
            DoNotRipple = false;
            HasPostUserInfo = false;
            Discardable = false;
            SecondarySeqNum = 0;
            ClearCache = false;
            Solicited = false;
            MessageComplete = false;
            HasGroupId = false;
            HasPostId = false;
            HasIdentifier = false;
            HasFilter = false;
            HasAttrib = false;
            AttribContainerType = 0;
            HasPayload = false;
            HasSecondarySeqNum = false;
            ProviderDriven = false;
            SolicitAck = false;
            HasNackCode = false;
            Preencoded = true;
            CompleteMsgEncoding = true;
            SetCopyByteBuffersFlag = false;
        }
    }
}
