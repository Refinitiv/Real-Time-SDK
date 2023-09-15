/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Concurrent;


namespace LSEG.Ema.Access
{
    internal class EmaObjectManager
    {
        public const int INITIAL_POOL_SIZE = 100;

        #region Primitive type Pools

        private ConcurrentBag<OmmInt> m_ommIntPool = new ConcurrentBag<OmmInt>();
        private ConcurrentBag<OmmUInt> m_ommUIntPool = new ConcurrentBag<OmmUInt>();
        private ConcurrentBag<OmmFloat> m_ommFloatPool = new ConcurrentBag<OmmFloat>();
        private ConcurrentBag<OmmDouble> m_ommDoublePool = new ConcurrentBag<OmmDouble>();
        private ConcurrentBag<OmmReal> m_ommRealPool = new ConcurrentBag<OmmReal>();
        private ConcurrentBag<OmmDate> m_ommDatePool = new ConcurrentBag<OmmDate>();
        private ConcurrentBag<OmmTime> m_ommTimePool = new ConcurrentBag<OmmTime>();
        private ConcurrentBag<OmmDateTime> m_ommDateTimePool = new ConcurrentBag<OmmDateTime>();
        private ConcurrentBag<OmmQos> m_ommQosPool = new ConcurrentBag<OmmQos>();
        private ConcurrentBag<OmmState> m_ommStatePool = new ConcurrentBag<OmmState>();
        private ConcurrentBag<OmmEnum> m_ommEnumPool = new ConcurrentBag<OmmEnum>();
        private ConcurrentBag<OmmArray> m_ommArrayPool = new ConcurrentBag<OmmArray>();
        private ConcurrentBag<OmmBuffer> m_ommBufferPool = new ConcurrentBag<OmmBuffer>();
        private ConcurrentBag<OmmAscii> m_ommAsciiPool = new ConcurrentBag<OmmAscii>();
        private ConcurrentBag<OmmUtf8> m_ommUtf8Pool = new ConcurrentBag<OmmUtf8>();
        private ConcurrentBag<OmmRmtes> m_ommRmtesPool = new ConcurrentBag<OmmRmtes>();

        #endregion

        #region Complex Type Pools

        private ConcurrentBag<OmmOpaque> m_ommOpaquePool = new ConcurrentBag<OmmOpaque>();
        private ConcurrentBag<OmmXml> m_ommXmlPool = new ConcurrentBag<OmmXml>();
        private ConcurrentBag<FieldList> m_ommFieldListPool = new ConcurrentBag<FieldList>();
        private ConcurrentBag<ElementList> m_ommElementListPool = new ConcurrentBag<ElementList>();
        private ConcurrentBag<OmmAnsiPage> m_ommAnsiPagePool = new ConcurrentBag<OmmAnsiPage>();
        private ConcurrentBag<FilterList> m_ommFilterListPool = new ConcurrentBag<FilterList>();
        private ConcurrentBag<Vector> m_ommVectorPool = new ConcurrentBag<Vector>();
        private ConcurrentBag<Map> m_ommMapPool = new ConcurrentBag<Map>();
        private ConcurrentBag<Series> m_ommSeriesPool = new ConcurrentBag<Series>();
        private ConcurrentBag<NoData> m_ommNoDataPool = new ConcurrentBag<NoData>();

        #endregion

        #region Message Type Pools

        private ConcurrentBag<AckMsg> m_ommAckMsgPool = new ConcurrentBag<AckMsg>();
        private ConcurrentBag<GenericMsg> m_ommGenericMsgPool = new ConcurrentBag<GenericMsg>();
        private ConcurrentBag<PostMsg> m_ommPostMsgPool = new ConcurrentBag<PostMsg>();
        private ConcurrentBag<UpdateMsg> m_ommUpdateMsgPool = new ConcurrentBag<UpdateMsg>();
        private ConcurrentBag<RequestMsg> m_ommRequestMsgPool = new ConcurrentBag<RequestMsg>();
        private ConcurrentBag<RefreshMsg> m_ommRefreshMsgPool = new ConcurrentBag<RefreshMsg>();
        private ConcurrentBag<StatusMsg> m_ommStatusMsgPool = new ConcurrentBag<StatusMsg>();
        private ConcurrentBag<Msg> m_ommMsgPool = new ConcurrentBag<Msg>();

        #endregion

        private ConcurrentBag<OmmError> m_ommErrorPool = new ConcurrentBag<OmmError>();

        #region Enumerator Pools

        private ConcurrentBag<FieldListEnumerator> m_ommFieldListEnumeratorPool = new ConcurrentBag<FieldListEnumerator>();
        private ConcurrentBag<ElementListEnumerator> m_ommElementListEnumeratorPool = new ConcurrentBag<ElementListEnumerator>();
        private ConcurrentBag<FilterListEnumerator> m_ommFilterListEnumeratorPool = new ConcurrentBag<FilterListEnumerator>();
        private ConcurrentBag<SeriesEnumerator> m_ommSeriesEnumeratorPool = new ConcurrentBag<SeriesEnumerator>();
        private ConcurrentBag<MapEnumerator> m_ommMapEnumeratorPool = new ConcurrentBag<MapEnumerator>();
        private ConcurrentBag<VectorEnumerator> m_ommVectorEnumeratorPool = new ConcurrentBag<VectorEnumerator>();

        private ConcurrentBag<FieldListErrorEnumerator> m_ommFieldListErrorEnumeratorPool = new ConcurrentBag<FieldListErrorEnumerator>();
        private ConcurrentBag<ElementListErrorEnumerator> m_ommElementListErrorEnumeratorPool = new ConcurrentBag<ElementListErrorEnumerator>();
        private ConcurrentBag<FilterListErrorEnumerator> m_ommFilterListErrorEnumeratorPool = new ConcurrentBag<FilterListErrorEnumerator>();
        private ConcurrentBag<SeriesErrorEnumerator> m_ommSeriesErrorEnumeratorPool = new ConcurrentBag<SeriesErrorEnumerator>();
        private ConcurrentBag<MapErrorEnumerator> m_ommMapErrorEnumeratorPool = new ConcurrentBag<MapErrorEnumerator>();
        private ConcurrentBag<VectorErrorEnumerator> m_ommVectorErrorEnumeratorPool = new ConcurrentBag<VectorErrorEnumerator>();

        #endregion

        private Action<Data>?[] m_returnPrimitiveTypeToPool;
        private Action<Data>?[] m_returnContainerTypeToPool;
        private Action<Data>?[] m_returnMsgTypeToPool;

        private Func<Data>?[] m_getFromPrimitiveTypePool;
        private Func<Data>?[] m_getFromContainerTypePool;
        private Func<Data>?[] m_getFromMsgTypePool;

        private Action<Decoder>?[] m_returnEnumeratorToPool;
        private Action<Decoder>?[] m_returnErrorEnumeratorToPool;

        private const int CONTAINER_TYPE_START = Access.DataType.DataTypes.NO_DATA;
        private const int MSG_TYPE_START = Access.DataType.DataTypes.REQ_MSG;

        internal EmaObjectManager(int initialPoolSize = INITIAL_POOL_SIZE)
        {          
            InitEnumeratorPool(m_ommElementListEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommFilterListEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommSeriesEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommMapEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommVectorEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommFieldListEnumeratorPool, initialPoolSize);
            
            InitEnumeratorPool(m_ommElementListErrorEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommFilterListErrorEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommSeriesErrorEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommMapErrorEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommVectorErrorEnumeratorPool, initialPoolSize);
            InitEnumeratorPool(m_ommFieldListErrorEnumeratorPool, initialPoolSize);

            InitContainerPool(m_ommAckMsgPool, initialPoolSize);
            InitContainerPool(m_ommFieldListPool, initialPoolSize);
            InitContainerPool(m_ommElementListPool, initialPoolSize);
            InitContainerPool(m_ommFilterListPool, initialPoolSize);
            InitContainerPool(m_ommVectorPool, initialPoolSize);
            InitContainerPool(m_ommMapPool, initialPoolSize);
            InitContainerPool(m_ommSeriesPool, initialPoolSize);
            InitContainerPool(m_ommNoDataPool, initialPoolSize);
            InitMsgPool(m_ommGenericMsgPool, initialPoolSize);
            InitMsgPool(m_ommPostMsgPool, initialPoolSize);
            InitMsgPool(m_ommUpdateMsgPool, initialPoolSize);
            InitMsgPool(m_ommRequestMsgPool, initialPoolSize);
            InitMsgPool(m_ommRefreshMsgPool, initialPoolSize);
            InitMsgPool(m_ommStatusMsgPool, initialPoolSize);
            InitMsgPool(m_ommMsgPool, initialPoolSize);

            InitPrimitiveTypePool(m_ommErrorPool, initialPoolSize, () => new OmmError());

            InitPrimitiveTypePool(m_ommArrayPool, initialPoolSize, () => new OmmArray());
            InitPrimitiveTypePool(m_ommIntPool, initialPoolSize, () => new OmmInt());
            InitPrimitiveTypePool(m_ommUIntPool, initialPoolSize, () => new OmmUInt());
            InitPrimitiveTypePool(m_ommFloatPool, initialPoolSize, () => new OmmFloat());
            InitPrimitiveTypePool(m_ommDoublePool, initialPoolSize, () => new OmmDouble());
            InitPrimitiveTypePool(m_ommRealPool, initialPoolSize, () => new OmmReal());
            InitPrimitiveTypePool(m_ommDatePool, initialPoolSize, () => new OmmDate());
            InitPrimitiveTypePool(m_ommTimePool, initialPoolSize, () => new OmmTime());
            InitPrimitiveTypePool(m_ommDateTimePool, initialPoolSize, () => new OmmDateTime());
            InitPrimitiveTypePool(m_ommStatePool, initialPoolSize, () => new OmmState());
            InitPrimitiveTypePool(m_ommQosPool, initialPoolSize, () => new OmmQos());
            InitPrimitiveTypePool(m_ommXmlPool, initialPoolSize, () => new OmmXml());
            InitPrimitiveTypePool(m_ommOpaquePool, initialPoolSize, () => new OmmOpaque());
            InitPrimitiveTypePool(m_ommAnsiPagePool, initialPoolSize, () => new OmmAnsiPage());
            InitPrimitiveTypePool(m_ommAsciiPool, initialPoolSize, () => new OmmAscii());
            InitPrimitiveTypePool(m_ommEnumPool, initialPoolSize, () => new OmmEnum());
            InitPrimitiveTypePool(m_ommBufferPool, initialPoolSize, () => new OmmBuffer());
            InitPrimitiveTypePool(m_ommUtf8Pool, initialPoolSize, () => new OmmUtf8());
            InitPrimitiveTypePool(m_ommRmtesPool, initialPoolSize, () => new OmmRmtes());

            m_returnPrimitiveTypeToPool = new Action<Data>?[]
            {
                null, null, null, // 0 - 2
                value => { m_ommIntPool.Add((OmmInt)value); value.m_inPool = true; },               // Int = 3
                value => { m_ommUIntPool.Add((OmmUInt)value); value.m_inPool = true; },             // UInt = 4
                value => { m_ommFloatPool.Add((OmmFloat)value); value.m_inPool = true; },           // Float = 5
                value => { m_ommDoublePool.Add((OmmDouble)value); value.m_inPool = true; },         // Double = 6
                null,
                value => { m_ommRealPool.Add((OmmReal)value); value.m_inPool = true; },             // Real = 8
                value => { m_ommDatePool.Add((OmmDate)value); value.m_inPool = true; },             // Date = 9
                value => { m_ommTimePool.Add((OmmTime)value); value.m_inPool = true; },             // Time = 10
                value => { m_ommDateTimePool.Add((OmmDateTime)value); value.m_inPool = true; },     // DateTime = 11
                value => { m_ommQosPool.Add((OmmQos)value); value.m_inPool = true; },               // Qos = 12
                value => { m_ommStatePool.Add((OmmState)value); value.m_inPool = true; },           // State = 13
                value => { m_ommEnumPool.Add((OmmEnum)value); value.m_inPool = true; },             // Enum = 14
                value => { m_ommArrayPool.Add((OmmArray)value); value.m_inPool = true; },           // Array = 15
                value => { m_ommBufferPool.Add((OmmBuffer)value); value.m_inPool = true; },         // Buffer = 16
                value => { m_ommAsciiPool.Add((OmmAscii)value); value.m_inPool = true; },           // Ascii = 17
                value => { m_ommUtf8Pool.Add((OmmUtf8)value); value.m_inPool = true; },             // Utf8 = 18
                value => { m_ommRmtesPool.Add((OmmRmtes)value); value.m_inPool = true; },           // RMTES = 19
            };
            m_returnContainerTypeToPool = new Action<Data>?[]
            {
                value => { m_ommNoDataPool.Add((NoData) value); value.m_inPool = true; },            // NoData = 128
                null,
                value => { m_ommOpaquePool.Add((OmmOpaque) value); value.m_inPool = true; },         // Opaque = 130
                value => { m_ommXmlPool.Add((OmmXml) value); value.m_inPool = true; },               // Xml = 131
                value => { m_ommFieldListPool.Add((FieldList) value); value.m_inPool = true; },      // FieldList = 132
                value => { m_ommElementListPool.Add((ElementList) value); value.m_inPool = true; },  // ElementList = 133
                value => { m_ommAnsiPagePool.Add((OmmAnsiPage) value); value.m_inPool = true; },     // AnsiPage = 134
                value => { m_ommFilterListPool.Add((FilterList) value); value.m_inPool = true; },    // FilterList = 135
                value => { m_ommVectorPool.Add((Vector) value); value.m_inPool = true; },            // Vector = 136
                value => { m_ommMapPool.Add((Map) value); value.m_inPool = true; },                  // Map = 137
                value => { m_ommSeriesPool.Add((Series) value); value.m_inPool = true; },            // Series = 138,
                null,
                null,
                value => { m_ommMsgPool.Add((Msg) value); value.m_inPool = true; },                  // Msg = 141
            };
            m_returnMsgTypeToPool = new Action<Data>?[]
            {
                value => { m_ommRequestMsgPool.Add((RequestMsg) value); value.m_inPool = true; },    // RequestMsg = 256
                value => { m_ommRefreshMsgPool.Add((RefreshMsg) value); value.m_inPool = true; },    // RefreshMsg = 257
                value => { m_ommUpdateMsgPool.Add((UpdateMsg) value); value.m_inPool = true; },      // UpdateMsg = 258
                value => { m_ommStatusMsgPool.Add((StatusMsg) value); value.m_inPool = true; },      // StatusMsg = 259
                value => { m_ommPostMsgPool.Add((PostMsg) value); value.m_inPool = true; },          // PostMsg = 260
                value => { m_ommAckMsgPool.Add((AckMsg) value); value.m_inPool = true; },            // AckMsg = 261
                value => { m_ommGenericMsgPool.Add((GenericMsg) value); value.m_inPool = true; },    // GenericMsg = 262
            };
            m_returnEnumeratorToPool = new Action<Decoder>?[]
            {
                value => { m_ommFieldListEnumeratorPool.Add((FieldListEnumerator)value); value.m_inPool = true; },      // FieldList = 132
                value => { m_ommElementListEnumeratorPool.Add((ElementListEnumerator)value); value.m_inPool = true; },  // ElementList = 133
                null,                                                                                                   // AnsiPage = 134
                value => { m_ommFilterListEnumeratorPool.Add((FilterListEnumerator)value); value.m_inPool = true; },    // FilterList = 135
                value => { m_ommVectorEnumeratorPool.Add((VectorEnumerator)value); value.m_inPool = true; },            // Vector = 136
                value => { m_ommMapEnumeratorPool.Add((MapEnumerator)value); value.m_inPool = true; },                  // Map = 137
                value => { m_ommSeriesEnumeratorPool.Add((SeriesEnumerator)value); value.m_inPool = true; }             // Series = 138
            };
            m_returnErrorEnumeratorToPool = new Action<Decoder>?[]
            {
                value => { m_ommFieldListErrorEnumeratorPool.Add((FieldListErrorEnumerator) value); value.m_inPool = true; },      // FieldList = 132
                value => { m_ommElementListErrorEnumeratorPool.Add((ElementListErrorEnumerator) value); value.m_inPool = true; },  // ElementList = 133
                null,                                                                                                               // AnsiPage = 134
                value => { m_ommFilterListErrorEnumeratorPool.Add((FilterListErrorEnumerator) value); value.m_inPool = true; },    // FilterList = 135
                value => { m_ommVectorErrorEnumeratorPool.Add((VectorErrorEnumerator)value); value.m_inPool = true; },            // Vector = 136
                value => { m_ommMapErrorEnumeratorPool.Add((MapErrorEnumerator)value); value.m_inPool = true; },                   // Map = 137
                value => { m_ommSeriesErrorEnumeratorPool.Add((SeriesErrorEnumerator)value); value.m_inPool = true; }               // Series = 138
            };

            m_getFromPrimitiveTypePool = new Func<Data>?[]
            {
                null, null, null,       // 0 - 2
                GetOmmInt,                 // Int = 3
                GetOmmUInt,                // UInt = 4
                GetOmmFloat,               // Float = 5
                GetOmmDouble,              // Double = 6
                null,
                GetOmmReal,                // Real = 8
                GetOmmDate,                // Date = 9
                GetOmmTime,                // Time = 10
                GetOmmDateTime,            // DateTime = 11
                GetOmmQos,                 // Qos = 12
                GetOmmState,               // State = 13
                GetOmmEnum,                // Enum = 14
                GetOmmArray,               // Array = 15
                GetOmmBuffer,              // Buffer = 16
                GetOmmAscii,               // Ascii = 17
                GetOmmUtf8,                // Utf8 = 18
                GetOmmRmtes,               // RMTES = 19
            };
            m_getFromContainerTypePool = new Func<Data>?[]
            {
                GetOmmNoData,              // NoData = 128
                null,
                GetOmmOpaque,              // Opaque = 130
                GetOmmXml,                 // Xml = 131
                GetOmmFieldList,           // FieldList = 132
                GetOmmElementList,         // ElementList = 133
                GetOmmAnsiPage,            // AnsiPage = 134
                GetOmmFilterList,          // FilterList = 135
                GetOmmVector,              // Vector = 136
                GetOmmMap,                 // Map = 137
                GetOmmSeries,              // Series = 138,
                null,
                null,
                GetOmmMsg,                 // Msg = 141
            };
            m_getFromMsgTypePool = new Func<Data>?[]
            {
                GetOmmRequestMsg,          // RequestMsg = 256
                GetOmmRefreshMsg,          // RefreshMsg = 257
                GetOmmUpdateMsg,           // UpdateMsg = 258
                GetOmmStatusMsg,           // StatusMsg = 259
                GetOmmPostMsg,             // PostMsg = 260
                GetOmmAckMsg,              // AckMsg = 261
                GetOmmGenericMsg,          // GenericMsg = 262
            };
        }

        public static ComplexType? GetComplexTypeObject(int dataType)
        {
            switch (dataType)
            {
                case DataType.DataTypes.OPAQUE:
                    return new OmmOpaque();
                case DataType.DataTypes.XML:
                    return new OmmXml();
                case DataType.DataTypes.FIELD_LIST:
                    return new FieldList();
                case DataType.DataTypes.ELEMENT_LIST:
                    return new ElementList();
                case DataType.DataTypes.ANSI_PAGE:
                    return new OmmAnsiPage();
                case DataType.DataTypes.FILTER_LIST:
                    return new FilterList();
                case DataType.DataTypes.VECTOR:
                    return new Vector();
                case DataType.DataTypes.MAP:
                    return new Map();
                case DataType.DataTypes.SERIES:
                    return new Series();
                case DataType.DataTypes.ACK_MSG:
                    return new AckMsg();
                case DataType.DataTypes.GENERIC_MSG:
                    return new GenericMsg();
                case DataType.DataTypes.REFRESH_MSG:
                    return new RefreshMsg();
                case DataType.DataTypes.REQ_MSG:
                    return new RequestMsg();
                case DataType.DataTypes.UPDATE_MSG:
                    return new UpdateMsg();
                case DataType.DataTypes.STATUS_MSG:
                    return new StatusMsg();
                case DataType.DataTypes.POST_MSG:
                    return new PostMsg();
                case DataType.DataTypes.NO_DATA:
                    return new NoData();
                default:
                    return null;
            }
        }

        public static Data? GetDataObject(int dataType)
        {
            switch (dataType)
            {
                case DataType.DataTypes.INT:
                    return new OmmInt();
                case DataType.DataTypes.UINT:
                    return new OmmUInt();
                case DataType.DataTypes.FLOAT:
                    return new OmmFloat();
                case DataType.DataTypes.DOUBLE:
                    return new OmmDouble();
                case DataType.DataTypes.REAL:
                    return new OmmReal();
                case DataType.DataTypes.DATE:
                    return new OmmDate();
                case DataType.DataTypes.TIME:
                    return new OmmTime();
                case DataType.DataTypes.DATETIME:
                    return new OmmDateTime();
                case DataType.DataTypes.QOS:
                    return new OmmQos();
                case DataType.DataTypes.STATE:
                    return new OmmState();
                case DataType.DataTypes.ENUM:
                    return new OmmEnum();
                case DataType.DataTypes.ARRAY:
                    return new OmmArray();
                case DataType.DataTypes.BUFFER:
                    return new OmmBuffer();
                case DataType.DataTypes.ASCII:
                    return new OmmAscii();
                case DataType.DataTypes.UTF8:
                    return new OmmUtf8();
                case DataType.DataTypes.RMTES:
                    return new OmmRmtes();
                case DataType.DataTypes.OPAQUE:
                    return new OmmOpaque();
                case DataType.DataTypes.XML:
                    return new OmmXml();
                case DataType.DataTypes.FIELD_LIST:
                    return new FieldList();
                case DataType.DataTypes.ELEMENT_LIST:
                    return new ElementList();
                case DataType.DataTypes.ANSI_PAGE:
                    return new OmmAnsiPage();
                case DataType.DataTypes.FILTER_LIST:
                    return new FilterList();
                case DataType.DataTypes.VECTOR:
                    return new Vector();
                case DataType.DataTypes.MAP:
                    return new Map();
                case DataType.DataTypes.SERIES:
                    return new Series();
                case DataType.DataTypes.ACK_MSG:
                    return new AckMsg();
                case DataType.DataTypes.GENERIC_MSG:
                    return new GenericMsg();
                case DataType.DataTypes.REFRESH_MSG:
                    return new RefreshMsg();
                case DataType.DataTypes.REQ_MSG:
                    return new RequestMsg();
                case DataType.DataTypes.UPDATE_MSG:
                    return new UpdateMsg();
                case DataType.DataTypes.STATUS_MSG:
                    return new StatusMsg();
                case DataType.DataTypes.POST_MSG:
                    return new PostMsg();
                case DataType.DataTypes.NO_DATA:
                    return new NoData();
                default:
                    return null;
            }
        }

        public Data? GetDataObjectFromPool(int dataType)
        {
            if (dataType <= Access.DataType.DataTypes.RMTES)
            {
                return m_getFromPrimitiveTypePool[dataType]!();
            }
            if (dataType >= Access.DataType.DataTypes.NO_DATA && dataType <= Access.DataType.DataTypes.MSG)
            {
                return m_getFromContainerTypePool[dataType - CONTAINER_TYPE_START]!();
            }
            if (dataType >= Access.DataType.DataTypes.REQ_MSG && dataType <= Access.DataType.DataTypes.GENERIC_MSG)
            {
                return m_getFromMsgTypePool[dataType - MSG_TYPE_START]!();
            }
            if (dataType == Access.DataType.DataTypes.ERROR)
            {
                return GetOmmError();
            }

            return null;
        }

        public ComplexType? GetComplexTypeFromPool(int dataType)
        {
            if (dataType >= Access.DataType.DataTypes.NO_DATA && dataType <= Access.DataType.DataTypes.MSG)
            {
                return (ComplexType)m_getFromContainerTypePool[dataType - CONTAINER_TYPE_START]!();
            }
            if (dataType >= Access.DataType.DataTypes.REQ_MSG && dataType <= Access.DataType.DataTypes.GENERIC_MSG)
            {
                return (ComplexType)m_getFromMsgTypePool[dataType - MSG_TYPE_START]!();
            }

            return null;
        }

        public void ReturnToPool(Data value)
        {
            int dataType = value.DataType;
            if (dataType <= Access.DataType.DataTypes.RMTES)
            {
                m_returnPrimitiveTypeToPool[dataType]!(value);
                return;
            }
            if (dataType >= Access.DataType.DataTypes.NO_DATA && dataType <= Access.DataType.DataTypes.MSG)
            {
                m_returnContainerTypeToPool[dataType - CONTAINER_TYPE_START]!(value);
                return;
            }
            if (dataType >= Access.DataType.DataTypes.REQ_MSG && dataType <= Access.DataType.DataTypes.GENERIC_MSG)
            {
                m_returnMsgTypeToPool[dataType - MSG_TYPE_START]!(value);
                return;
            }
            if (dataType == Access.DataType.DataTypes.ERROR)
            {
                m_ommErrorPool.Add((OmmError)value);
                return;
            }
        }

        public void ReturnToPool(int dataType, Decoder enumerator, bool isErrorEnumerator = false)
        {
            if (dataType >= Access.DataType.DataTypes.FIELD_LIST && dataType <= Access.DataType.DataTypes.SERIES)
            {
                if (!isErrorEnumerator)
                {
                    m_returnEnumeratorToPool[dataType - Access.DataType.DataTypes.FIELD_LIST]!(enumerator);
                }
                else
                {
                    m_returnErrorEnumeratorToPool[dataType - Access.DataType.DataTypes.FIELD_LIST]!(enumerator);
                }
                return;
            }
        }

        #region Methods for getting OMM objects

        public OmmInt GetOmmInt()
        {
            OmmInt? result;
            m_ommIntPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmInt();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmUInt GetOmmUInt()
        {
            OmmUInt? result;
            m_ommUIntPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmUInt();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmFloat GetOmmFloat()
        {
            OmmFloat? result;
            m_ommFloatPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmFloat();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmDouble GetOmmDouble()
        {
            OmmDouble? result;
            m_ommDoublePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmDouble();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmReal GetOmmReal()
        {
            OmmReal? result;
            m_ommRealPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmReal();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmDate GetOmmDate()
        {
            OmmDate? result;
            m_ommDatePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmDate();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmTime GetOmmTime()
        {
            OmmTime? result;
            m_ommTimePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmTime();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmDateTime GetOmmDateTime()
        {
            OmmDateTime? result;
            m_ommDateTimePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmDateTime();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmQos GetOmmQos()
        {
            OmmQos? result;
            m_ommQosPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmQos();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmState GetOmmState()
        {
            OmmState? result;
            m_ommStatePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmState();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmEnum GetOmmEnum()
        {
            OmmEnum? result;
            m_ommEnumPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmEnum();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmArray GetOmmArray()
        {
            OmmArray? result;
            m_ommArrayPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmArray();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmBuffer GetOmmBuffer()
        {
            OmmBuffer? result;
            m_ommBufferPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmBuffer();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmAscii GetOmmAscii()
        {
            OmmAscii? result;
            m_ommAsciiPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmAscii();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmUtf8 GetOmmUtf8()
        {
            OmmUtf8? result;
            m_ommUtf8Pool.TryTake(out result);
            if (result == null)
            {
                result = new OmmUtf8();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmRmtes GetOmmRmtes()
        {
            OmmRmtes? result;
            m_ommRmtesPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmRmtes();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmOpaque GetOmmOpaque()
        {
            OmmOpaque? result;
            m_ommOpaquePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmOpaque();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmXml GetOmmXml()
        {
            OmmXml? result;
            m_ommXmlPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmXml();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmAnsiPage GetOmmAnsiPage()
        {
            OmmAnsiPage? result;
            m_ommAnsiPagePool.TryTake(out result);
            if (result == null)
            {
                result = new OmmAnsiPage();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public FieldList GetOmmFieldList()
        {
            FieldList? result;
            m_ommFieldListPool.TryTake(out result);
            if (result == null)
            {
                result = new FieldList();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public ElementList GetOmmElementList()
        {
            ElementList? result;
            m_ommElementListPool.TryTake(out result);
            if (result == null)
            {
                result = new ElementList();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public FilterList GetOmmFilterList()
        {
            FilterList? result;
            m_ommFilterListPool.TryTake(out result);
            if (result == null)
            {
                result = new FilterList();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public Vector GetOmmVector()
        {
            Vector? result;
            m_ommVectorPool.TryTake(out result);
            if (result == null)
            {
                result = new Vector();
                result.m_ownedByPool = true;
            }
            result.SetObjectManager(this);
            result.m_inPool = false;
            return result;
        }

        public Series GetOmmSeries()
        {
            Series? result;
            m_ommSeriesPool.TryTake(out result);
            if (result == null)
            {
                result = new Series();
                result.m_ownedByPool = true;
            }
            result.SetObjectManager(this);
            result.m_inPool = false;
            return result;
        }

        public Map GetOmmMap()
        {
            Map? result;
            m_ommMapPool.TryTake(out result);
            if (result == null)
            {
                result = new Map();
                result.m_ownedByPool = true;
            }
            result.SetObjectManager(this);
            result.m_inPool = false;
            return result;
        }

        public Msg GetOmmMsg()
        {
            Msg? result;
            m_ommMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new Msg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public AckMsg GetOmmAckMsg()
        {
            AckMsg? result;
            m_ommAckMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new AckMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public PostMsg GetOmmPostMsg()
        {
            PostMsg? result;
            m_ommPostMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new PostMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public RequestMsg GetOmmRequestMsg()
        {
            RequestMsg? result;
            m_ommRequestMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new RequestMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public RefreshMsg GetOmmRefreshMsg()
        {
            RefreshMsg? result;
            m_ommRefreshMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new RefreshMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public StatusMsg GetOmmStatusMsg()
        {
            StatusMsg? result;
            m_ommStatusMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new StatusMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public GenericMsg GetOmmGenericMsg()
        {
            GenericMsg? result;
            m_ommGenericMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new GenericMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public UpdateMsg GetOmmUpdateMsg()
        {
            UpdateMsg? result;
            m_ommUpdateMsgPool.TryTake(out result);
            if (result == null)
            {
                result = new UpdateMsg();
                result.m_ownedByPool = true;
                result.SetObjectManager(this);
            }
            else
            {
                result.SetObjectManager(this);
            }

            result.m_inPool = false;
            return result;
        }

        public NoData GetOmmNoData()
        {
            NoData? result;
            m_ommNoDataPool.TryTake(out result);
            if (result == null)
            {
                result = new NoData();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public OmmError GetOmmError()
        {
            OmmError? result;
            m_ommErrorPool.TryTake(out result);
            if (result == null)
            {
                result = new OmmError();
                result.m_ownedByPool = true;
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        #endregion

        #region Methods for getting Enumerators

        public ElementListEnumerator GetElementListEnumerator()
        {
            ElementListEnumerator? result;
            m_ommElementListEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new ElementListEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public FieldListEnumerator GetFieldListEnumerator()
        {
            FieldListEnumerator? result;
            m_ommFieldListEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new FieldListEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public FilterListEnumerator GetFilterListEnumerator()
        {
            FilterListEnumerator? result;
            m_ommFilterListEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new FilterListEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public MapEnumerator GetMapEnumerator()
        {
            MapEnumerator? result;
            m_ommMapEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new MapEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public VectorEnumerator GetVectorEnumerator()
        {
            VectorEnumerator? result;
            m_ommVectorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new VectorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public SeriesEnumerator GetSeriesEnumerator()
        {
            SeriesEnumerator? result;
            m_ommSeriesEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new SeriesEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public ElementListErrorEnumerator GetElementListErrorEnumerator()
        {
            ElementListErrorEnumerator? result;
            m_ommElementListErrorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new ElementListErrorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public FieldListErrorEnumerator GetFieldListErrorEnumerator()
        {
            FieldListErrorEnumerator? result;
            m_ommFieldListErrorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new FieldListErrorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public FilterListErrorEnumerator GetFilterListErrorEnumerator()
        {
            FilterListErrorEnumerator? result;
            m_ommFilterListErrorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new FilterListErrorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public MapErrorEnumerator GetMapErrorEnumerator()
        {
            MapErrorEnumerator? result;
            m_ommMapErrorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new MapErrorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public VectorErrorEnumerator GetVectorErrorEnumerator()
        {
            VectorErrorEnumerator? result;
            m_ommVectorErrorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new VectorErrorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        public SeriesErrorEnumerator GetSeriesErrorEnumerator()
        {
            SeriesErrorEnumerator? result;
            m_ommSeriesErrorEnumeratorPool.TryTake(out result);
            if (result == null)
            {
                result = new SeriesErrorEnumerator();
                result.m_objectManager = this;
            }
            result.m_inPool = false;
            return result;
        }

        #endregion

        #region Private initialization methods

        private void InitContainerPool<T>(ConcurrentBag<T> pool, int poolSize) where T : Data, new()
        {
            for (int i = 0; i < poolSize; i++)
            {
                var val = new T();
                val.m_objectManager = this;
                val.m_inPool = true;
                val.m_ownedByPool = true;
                pool.Add(val);
            }
        }

        private void InitMsgPool<T>(ConcurrentBag<T> pool, int poolSize) where T : Msg, new()
        {
            for (int i = 0; i < poolSize; i++)
            {
                var val = new T();
                val.SetObjectManager(this);
                val.m_inPool = true;
                val.m_ownedByPool = true;
                pool.Add(val);
            }
        }

        private void InitEnumeratorPool<T>(ConcurrentBag<T> pool, int poolSize) where T : Decoder, new()
        {
            for (int i = 0; i < poolSize; i++)
            {
                var val = new T();
                val.m_objectManager = this;
                val.m_inPool = true;
                pool.Add(val);
            }
        }

        private void InitPrimitiveTypePool<T>(ConcurrentBag<T> pool, int poolSize, Func<T> constructor) where T : Data
        {
            for (int i = 0; i < poolSize; i++)
            {
                var val = constructor();
                val.m_objectManager = this;
                val.m_inPool = true;
                val.m_ownedByPool = true;
                pool.Add(val);
            }
        }

        #endregion

        #region Internal methods for checking manager pools

        internal int GetDataObjectPoolSize(int dataType)
        {
            switch (dataType)
            {
                case DataType.DataTypes.INT:
                    return m_ommIntPool.Count;
                case DataType.DataTypes.UINT:
                    return m_ommUIntPool.Count;
                case DataType.DataTypes.FLOAT:
                    return m_ommFloatPool.Count;
                case DataType.DataTypes.DOUBLE:
                    return m_ommDoublePool.Count;
                case DataType.DataTypes.REAL:
                    return m_ommRealPool.Count;
                case DataType.DataTypes.DATE:
                    return m_ommDatePool.Count;
                case DataType.DataTypes.TIME:
                    return m_ommTimePool.Count;
                case DataType.DataTypes.DATETIME:
                    return m_ommDateTimePool.Count;
                case DataType.DataTypes.QOS:
                    return m_ommQosPool.Count;
                case DataType.DataTypes.STATE:
                    return m_ommStatePool.Count;
                case DataType.DataTypes.ENUM:
                    return m_ommEnumPool.Count;
                case DataType.DataTypes.ARRAY:
                    return m_ommArrayPool.Count;
                case DataType.DataTypes.BUFFER:
                    return m_ommBufferPool.Count;
                case DataType.DataTypes.ASCII:
                    return m_ommAsciiPool.Count;
                case DataType.DataTypes.UTF8:
                    return m_ommUtf8Pool.Count;
                case DataType.DataTypes.RMTES:
                    return m_ommRmtesPool.Count;
                case DataType.DataTypes.OPAQUE:
                    return m_ommOpaquePool.Count;
                case DataType.DataTypes.XML:
                    return m_ommXmlPool.Count;
                case DataType.DataTypes.FIELD_LIST:
                    return m_ommFieldListPool.Count;
                case DataType.DataTypes.ELEMENT_LIST:
                    return m_ommElementListPool.Count;
                case DataType.DataTypes.ANSI_PAGE:
                    return m_ommAnsiPagePool.Count;
                case DataType.DataTypes.FILTER_LIST:
                    return m_ommFilterListPool.Count;
                case DataType.DataTypes.VECTOR:
                    return m_ommVectorPool.Count;
                case DataType.DataTypes.MAP:
                    return m_ommMapPool.Count;
                case DataType.DataTypes.SERIES:
                    return m_ommSeriesPool.Count;
                case DataType.DataTypes.ACK_MSG:
                    return m_ommAckMsgPool.Count;
                case DataType.DataTypes.GENERIC_MSG:
                    return m_ommGenericMsgPool.Count;
                case DataType.DataTypes.REFRESH_MSG:
                    return m_ommRefreshMsgPool.Count;
                case DataType.DataTypes.REQ_MSG:
                    return m_ommRequestMsgPool.Count;
                case DataType.DataTypes.UPDATE_MSG:
                    return m_ommUpdateMsgPool.Count;
                case DataType.DataTypes.STATUS_MSG:
                    return m_ommStatusMsgPool.Count;
                case DataType.DataTypes.POST_MSG:
                    return m_ommPostMsgPool.Count;
                case DataType.DataTypes.NO_DATA:
                    return m_ommNoDataPool.Count;
                default:
                    return 0;
            }
        }

        #endregion
    }
}
