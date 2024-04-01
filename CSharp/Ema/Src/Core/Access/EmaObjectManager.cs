/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;

namespace LSEG.Ema.Access
{
    internal class EmaObjectManager
    {    
        #region Inner classes

        internal interface IEmaPool<out T>
        {
            public T Get();

            public void FreePool();
        }

        internal class EmaPool<T> : IEmaPool<T>
        {
            private bool m_global = false;
            private object? m_lock;

            internal T[] pool;
            internal int limit;
            internal int current = 0;
            internal object _lock = new object();
            internal Func<T> _constructor;
            internal int initialSize;
            internal int currentDecreaseCeiling;
            internal Action<T>? m_freeObject;
            internal Action<T>? m_pinObject;

            public int Count { get => limit - current; }

            public EmaPool(int size, Func<T> constructor, Action<T>? pinObject = null, Action<T>? freeObject = null, bool global = false)
            {
                m_freeObject = freeObject;
                m_pinObject = pinObject;

                pool = new T[size];
                for (int i = 0; i < size; i++)
                {
                    pool[i] = constructor();
                    if (m_pinObject != null) m_pinObject(pool[i]);
                }
                    
                limit = size;
                _constructor = constructor;
                initialSize = size;

                m_global = global;
                if (m_global) m_lock = new object();
            }

            public T Get()
            {
                T res;

                if (!m_global)
                {
                    
                    if (current < limit)
                    {
                        res = pool[current++];
                    }
                    else
                    {
                        res = _constructor();
                    }
                }
                else
                {
                    Monitor.Enter(m_lock!);
                    if (current < limit)
                    {
                        res = pool[current++];
                    }
                    else
                    {
                        res = _constructor();
                    }
                    Monitor.Exit(m_lock!);
                }

                return res;
            }

            public void ReturnToPool(T obj)
            {
                if (!m_global)
                {
                    if (current == 0)
                        return;
                    current--;
                    pool[current] = obj;
                }
                else
                {
                    Monitor.Enter(m_lock!);
                    if (current == 0)
                        return;
                    current--;
                    pool[current] = obj;
                    Monitor.Exit(m_lock!);
                }               
            }

            public void FreePool()
            {
                if (m_freeObject != null)
                {
                    for (int i = 0; i < pool.Length; i++) m_freeObject(pool[i]);
                }
            }
        }

        public class DataArray
        {
            internal bool InPool;
            internal Data[] Array;
            internal bool OwnedByPool;
            internal GCHandle m_handle;

            public DataArray(EmaObjectManager manager)
            {
                Array = new Data[20];
                Array[DataType.DataTypes.INT] = new OmmInt();
                Array[DataType.DataTypes.INT].m_objectManager = manager;
                Array[DataType.DataTypes.UINT] = new OmmUInt();
                Array[DataType.DataTypes.UINT].m_objectManager = manager;
                Array[DataType.DataTypes.FLOAT] = new OmmFloat();
                Array[DataType.DataTypes.FLOAT].m_objectManager = manager;
                Array[DataType.DataTypes.DOUBLE] = new OmmDouble();
                Array[DataType.DataTypes.DOUBLE].m_objectManager = manager;
                Array[DataType.DataTypes.REAL] = new OmmReal();
                Array[DataType.DataTypes.REAL].m_objectManager = manager;
                Array[DataType.DataTypes.DATE] = new OmmDate();
                Array[DataType.DataTypes.DATE].m_objectManager = manager;
                Array[DataType.DataTypes.TIME] = new OmmTime();
                Array[DataType.DataTypes.TIME].m_objectManager = manager;
                Array[DataType.DataTypes.DATETIME] = new OmmDateTime();
                Array[DataType.DataTypes.DATETIME].m_objectManager = manager;
                Array[DataType.DataTypes.QOS] = new OmmQos();
                Array[DataType.DataTypes.QOS].m_objectManager= manager;
                Array[DataType.DataTypes.STATE] = new OmmState();
                Array[DataType.DataTypes.ENUM] = new OmmEnum();
                Array[DataType.DataTypes.ARRAY] = new OmmArray();
                Array[DataType.DataTypes.ARRAY].m_objectManager = manager;
                Array[DataType.DataTypes.BUFFER] = new OmmBuffer();
                Array[DataType.DataTypes.ASCII] = new OmmAscii();
                Array[DataType.DataTypes.UTF8] = new OmmUtf8();
                Array[DataType.DataTypes.RMTES] = new OmmRmtes();

                InPool = true;
                OwnedByPool = true;
                m_handle = GCHandle.Alloc(Array);
            }

            public DataArray()
            {
                Array = new Data[20];
                Array[DataType.DataTypes.INT] = new OmmInt();
                Array[DataType.DataTypes.UINT] = new OmmUInt();
                Array[DataType.DataTypes.FLOAT] = new OmmFloat();
                Array[DataType.DataTypes.DOUBLE] = new OmmDouble();
                Array[DataType.DataTypes.REAL] = new OmmReal();
                Array[DataType.DataTypes.DATE] = new OmmDate();
                Array[DataType.DataTypes.TIME] = new OmmTime();
                Array[DataType.DataTypes.DATETIME] = new OmmDateTime();
                Array[DataType.DataTypes.QOS] = new OmmQos();
                Array[DataType.DataTypes.STATE] = new OmmState();
                Array[DataType.DataTypes.ENUM] = new OmmEnum();
                Array[DataType.DataTypes.ARRAY] = new OmmArray();
                Array[DataType.DataTypes.BUFFER] = new OmmBuffer();
                Array[DataType.DataTypes.ASCII] = new OmmAscii();
                Array[DataType.DataTypes.UTF8] = new OmmUtf8();
                Array[DataType.DataTypes.RMTES] = new OmmRmtes();

                OwnedByPool = false;
            }
        }

        public class ComplexTypeArray
        {
            internal bool InPool;
            internal ComplexType[] Array;
            internal bool OwnedByPool;
            internal GCHandle m_handle;

            public ComplexTypeArray(EmaObjectManager manager)
            {
                Array = new ComplexType[DataType.DataTypes.SERIES - DataType.DataTypes.OPAQUE + 2];
                Array[DataType.DataTypes.OPAQUE - DataType.DataTypes.OPAQUE] = new OmmOpaque();
                Array[DataType.DataTypes.OPAQUE - DataType.DataTypes.OPAQUE].m_objectManager = manager;
                Array[DataType.DataTypes.XML - DataType.DataTypes.OPAQUE] = new OmmXml();
                Array[DataType.DataTypes.XML - DataType.DataTypes.OPAQUE].m_objectManager = manager;
                Array[DataType.DataTypes.FIELD_LIST - DataType.DataTypes.OPAQUE] = new FieldList();
                Array[DataType.DataTypes.FIELD_LIST - DataType.DataTypes.OPAQUE].m_objectManager = manager;
                Array[DataType.DataTypes.ELEMENT_LIST - DataType.DataTypes.OPAQUE] = new ElementList();
                Array[DataType.DataTypes.ELEMENT_LIST - DataType.DataTypes.OPAQUE].m_objectManager = manager;
                Array[DataType.DataTypes.ANSI_PAGE - DataType.DataTypes.OPAQUE] = new OmmAnsiPage();
                Array[DataType.DataTypes.ANSI_PAGE - DataType.DataTypes.OPAQUE].m_objectManager = manager;
                Array[DataType.DataTypes.FILTER_LIST - DataType.DataTypes.OPAQUE] = new FilterList();
                Array[DataType.DataTypes.FILTER_LIST - DataType.DataTypes.OPAQUE].m_objectManager = manager;
                Array[DataType.DataTypes.VECTOR - DataType.DataTypes.OPAQUE] = new Vector();
                ((Vector)Array[DataType.DataTypes.VECTOR - DataType.DataTypes.OPAQUE]).SetObjectManager(manager);
                Array[DataType.DataTypes.MAP - DataType.DataTypes.OPAQUE] = new Map();
                ((Map)Array[DataType.DataTypes.MAP - DataType.DataTypes.OPAQUE]).SetObjectManager(manager);
                Array[DataType.DataTypes.SERIES - DataType.DataTypes.OPAQUE] = new Series();
                ((Series)Array[DataType.DataTypes.SERIES - DataType.DataTypes.OPAQUE]).SetObjectManager(manager);
                m_handle = GCHandle.Alloc(Array);

                OwnedByPool = true;
            }

            public ComplexTypeArray()
            {
                Array = new ComplexType[DataType.DataTypes.SERIES - DataType.DataTypes.OPAQUE + 2];
                Array[DataType.DataTypes.OPAQUE - DataType.DataTypes.OPAQUE] = new OmmOpaque();
                Array[DataType.DataTypes.XML - DataType.DataTypes.OPAQUE] = new OmmXml();
                Array[DataType.DataTypes.FIELD_LIST - DataType.DataTypes.OPAQUE] = new FieldList();
                Array[DataType.DataTypes.ELEMENT_LIST - DataType.DataTypes.OPAQUE] = new ElementList();
                Array[DataType.DataTypes.ANSI_PAGE - DataType.DataTypes.OPAQUE] = new OmmAnsiPage();
                Array[DataType.DataTypes.FILTER_LIST - DataType.DataTypes.OPAQUE] = new FilterList();
                Array[DataType.DataTypes.VECTOR - DataType.DataTypes.OPAQUE] = new Vector();
                Array[DataType.DataTypes.MAP - DataType.DataTypes.OPAQUE] = new Map();
                Array[DataType.DataTypes.SERIES - DataType.DataTypes.OPAQUE] = new Series();

                OwnedByPool = false;
            }
        }

        public class MsgTypeArray
        {
            internal bool InPool;
            internal Msg[] Array;
            internal bool OwnedByPool;
            internal GCHandle m_handle;

            public MsgTypeArray(EmaObjectManager manager)
            {
                Array = new Msg[DataType.DataTypes.GENERIC_MSG - DataType.DataTypes.REQ_MSG + 2];
                Array[DataType.DataTypes.REQ_MSG - DataType.DataTypes.REQ_MSG] = new RequestMsg(manager);
                Array[DataType.DataTypes.REFRESH_MSG - DataType.DataTypes.REQ_MSG] = new RefreshMsg(manager);
                Array[DataType.DataTypes.UPDATE_MSG - DataType.DataTypes.REQ_MSG] = new UpdateMsg(manager);
                Array[DataType.DataTypes.STATUS_MSG - DataType.DataTypes.REQ_MSG] = new StatusMsg(manager);
                Array[DataType.DataTypes.POST_MSG - DataType.DataTypes.REQ_MSG] = new PostMsg(manager);
                Array[DataType.DataTypes.ACK_MSG - DataType.DataTypes.REQ_MSG] = new AckMsg(manager);
                Array[DataType.DataTypes.GENERIC_MSG - DataType.DataTypes.REQ_MSG] = new GenericMsg(manager);
                m_handle = GCHandle.Alloc(Array);

                OwnedByPool = true;
                InPool = true;
            }

            public MsgTypeArray()
            {
                Array = new Msg[DataType.DataTypes.GENERIC_MSG - DataType.DataTypes.REQ_MSG + 2];
                Array[DataType.DataTypes.REQ_MSG - DataType.DataTypes.REQ_MSG] = new RequestMsg();
                Array[DataType.DataTypes.REFRESH_MSG - DataType.DataTypes.REQ_MSG] = new RefreshMsg();
                Array[DataType.DataTypes.UPDATE_MSG - DataType.DataTypes.REQ_MSG] = new UpdateMsg();
                Array[DataType.DataTypes.STATUS_MSG - DataType.DataTypes.REQ_MSG] = new StatusMsg();
                Array[DataType.DataTypes.POST_MSG - DataType.DataTypes.REQ_MSG] = new PostMsg();
                Array[DataType.DataTypes.ACK_MSG - DataType.DataTypes.REQ_MSG] = new AckMsg();
                Array[DataType.DataTypes.GENERIC_MSG - DataType.DataTypes.REQ_MSG] = new GenericMsg();

                OwnedByPool = false;
                InPool = true;
            }
        }

        internal class EmaPrimitiveDataPool
        {
            internal DataArray[] pool;
            internal int limit;
            internal int current = 0;
            internal object _lock = new object();
            internal EmaObjectManager _manager;

            private bool m_global;

            internal int Count { get => limit - current; }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal DataArray CreateElement(EmaObjectManager manager)
            {
                DataArray dataArray = new DataArray(manager);
                dataArray.InPool = true;
                return dataArray;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public DataArray Get()
            {
                DataArray res;

                if (!m_global)
                {
                    if (current < limit)
                        res = pool[current++];
                    else
                        res = new DataArray(_manager);
                }
                else
                {
                    Monitor.Enter(_lock);
                    if (current < limit)
                        res = pool[current++];
                    else
                        res = new DataArray(_manager);
                    Monitor.Exit(_lock);
                }
                
                return res;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void ReturnToPool(DataArray obj)
            {
                obj.InPool = true;
                if (!m_global)
                {
                    current--;
                    if (current == 0)
                        return;
                    pool[current] = obj;
                }
                else
                {
                    Monitor.Enter(_lock);
                    current--;
                    if (current == 0)
                        return;
                    pool[current] = obj;
                    Monitor.Exit(_lock);
                }                
            }

            internal EmaPrimitiveDataPool(EmaObjectManager manager, int size, bool global = false)
            {
                _manager = manager;
                pool = new DataArray[size];
                for (int i = 0; i < size; i++)
                    pool[i] = CreateElement(_manager);
                limit = size;

                m_global = global;
            }
        }

        internal class EmaComplexTypePool
        {
            internal ComplexTypeArray[] pool;
            internal int limit;
            internal int current = 0;
            internal object _lock = new object();
            internal EmaObjectManager _manager;

            private bool m_global;

            internal int Count { get => limit - current; }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal ComplexTypeArray CreateElement(EmaObjectManager manager)
            {
                ComplexTypeArray dataArray = new ComplexTypeArray(manager);
                return dataArray;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public ComplexTypeArray Get()
            {
                ComplexTypeArray res;

                if (!m_global)
                {
                    if (current < limit)
                        res = pool[current++];
                    else
                        res = new ComplexTypeArray(_manager);
                }
                else
                {
                    Monitor.Enter(_lock);
                    if (current < limit)
                        res = pool[current++];
                    else
                        res = new ComplexTypeArray(_manager);
                    Monitor.Exit(_lock);
                }
                
                return res;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void ReturnToPool(ComplexTypeArray obj)
            {
                if (!m_global)
                {
                    current--;
                    if (current == 0)
                        return;
                    pool[current] = obj;
                }
                else
                {
                    Monitor.Enter(_lock);
                    current--;
                    if (current == 0)
                        return;
                    pool[current] = obj;
                    Monitor.Exit(_lock);
                }
            }

            internal EmaComplexTypePool(EmaObjectManager manager, int size, bool global = false)
            {
                _manager = manager;
                pool = new ComplexTypeArray[size];
                for (int i = 0; i < size; i++)
                    pool[i] = CreateElement(_manager);
                limit = size;

                m_global = global;
            }
        }

        internal class EmaMsgTypePool
        {
            internal MsgTypeArray[] pool;
            internal int limit;
            internal int current = 0;
            internal object _lock = new object();
            internal EmaObjectManager _manager;

            private bool m_global;

            internal int Count { get => limit - current; }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal MsgTypeArray CreateElement(EmaObjectManager manager)
            {
                MsgTypeArray dataArray = new MsgTypeArray(manager);
                dataArray.InPool = true;
                return dataArray;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public MsgTypeArray Get()
            {
                if (!m_global)
                {
                    if (current < limit)
                        return pool[current++];
                    else
                        return new MsgTypeArray(_manager);
                }
                else
                {
                    MsgTypeArray res;
                    Monitor.Enter(_lock);
                    
                    if (current < limit)
                        res = pool[current++];
                    else
                        res = new MsgTypeArray(_manager);
                    
                    Monitor.Exit(_lock);

                    return res;
                }
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void ReturnToPool(MsgTypeArray obj)
            {
                if (!m_global)
                {
                    obj.InPool = true;
                    current--;
                    if (current == 0)
                        return;
                    pool[current] = obj;
                }
                else
                {
                    Monitor.Enter(_lock);
                    obj.InPool = true;
                    current--;
                    if (current == 0)
                        return;
                    pool[current] = obj;
                    Monitor.Exit(_lock);
                }
                
            }

            internal EmaMsgTypePool(EmaObjectManager manager, int size, bool global = false)
            {
                _manager = manager;
                pool = new MsgTypeArray[size];
                for (int i = 0; i < size; i++)
                    pool[i] = CreateElement(_manager);
                limit = size;

                m_global = global;
            }
        }

        #endregion

        public const int INITIAL_POOL_SIZE = 25;
        public const int INITIAL_ADMIN_DOMAIN_ITEM_POOL_SIZE = 2;

        #region Primitive type Pools

        private EmaPool<OmmInt> m_ommIntPool;
        private EmaPool<OmmUInt> m_ommUIntPool;
        private EmaPool<OmmFloat> m_ommFloatPool;
        private EmaPool<OmmDouble> m_ommDoublePool;
        private EmaPool<OmmReal> m_ommRealPool;
        private EmaPool<OmmDate> m_ommDatePool;
        private EmaPool<OmmTime> m_ommTimePool;
        private EmaPool<OmmDateTime> m_ommDateTimePool;
        private EmaPool<OmmQos> m_ommQosPool;
        private EmaPool<OmmState> m_ommStatePool;
        private EmaPool<OmmEnum> m_ommEnumPool;
        private EmaPool<OmmArray> m_ommArrayPool;
        private EmaPool<OmmBuffer> m_ommBufferPool;
        private EmaPool<OmmAscii> m_ommAsciiPool;
        private EmaPool<OmmUtf8> m_ommUtf8Pool;
        private EmaPool<OmmRmtes> m_ommRmtesPool;

        #endregion

        #region Complex Type Pools

        private EmaPool<OmmOpaque> m_ommOpaquePool;
        private EmaPool<OmmXml> m_ommXmlPool;
        private EmaPool<FieldList> m_ommFieldListPool;
        private EmaPool<ElementList> m_ommElementListPool;
        private EmaPool<OmmAnsiPage> m_ommAnsiPagePool;
        private EmaPool<FilterList> m_ommFilterListPool;
        private EmaPool<Vector> m_ommVectorPool;
        private EmaPool<Map> m_ommMapPool;
        private EmaPool<Series> m_ommSeriesPool;
        private EmaPool<NoData> m_ommNoDataPool;

        #endregion

        #region Message Type Pools

        private EmaPool<AckMsg> m_ommAckMsgPool;
        private EmaPool<GenericMsg> m_ommGenericMsgPool;
        private EmaPool<PostMsg> m_ommPostMsgPool;
        private EmaPool<UpdateMsg> m_ommUpdateMsgPool;
        private EmaPool<RequestMsg> m_ommRequestMsgPool;
        private EmaPool<RefreshMsg> m_ommRefreshMsgPool;
        private EmaPool<StatusMsg> m_ommStatusMsgPool;

        #endregion

        private EmaPool<OmmError> m_ommErrorPool;

        #region Enumerator Pools

        private EmaPool<FieldListEnumerator> m_ommFieldListEnumeratorPool;
        private EmaPool<ElementListEnumerator> m_ommElementListEnumeratorPool;
        private EmaPool<FilterListEnumerator> m_ommFilterListEnumeratorPool;
        private EmaPool<SeriesEnumerator> m_ommSeriesEnumeratorPool;
        private EmaPool<MapEnumerator> m_ommMapEnumeratorPool;
        private EmaPool<VectorEnumerator> m_ommVectorEnumeratorPool;

        private EmaPool<FieldListErrorEnumerator> m_ommFieldListErrorEnumeratorPool;
        private EmaPool<ElementListErrorEnumerator> m_ommElementListErrorEnumeratorPool;
        private EmaPool<FilterListErrorEnumerator> m_ommFilterListErrorEnumeratorPool;
        private EmaPool<SeriesErrorEnumerator> m_ommSeriesErrorEnumeratorPool;
        private EmaPool<MapErrorEnumerator> m_ommMapErrorEnumeratorPool;
        private EmaPool<VectorErrorEnumerator> m_ommVectorErrorEnumeratorPool;

        #endregion

        internal VaPool m_singleItemPool = new VaPool(false);
        internal VaPool m_loginItemPool = new VaPool(false);
        internal VaPool m_dictionaryItemPool = new VaPool(false);
        internal VaPool m_directoryItemPool = new VaPool(false);
        internal VaPool m_batchItemPool = new VaPool(false);

        private IEmaPool<Data>[] pools = new IEmaPool<Data>[Access.DataType.DataTypes.ERROR + 1];

        internal EmaPrimitiveDataPool primitivePool;
        internal EmaComplexTypePool complexTypePool;
        internal EmaMsgTypePool msgTypePool;

        private bool m_global;

        internal EmaObjectManager(int initialPoolSize = INITIAL_POOL_SIZE, bool global = false)
        {
            m_global = global;

            m_ommFieldListEnumeratorPool = new EmaPool<FieldListEnumerator>(initialPoolSize, () => { var res = new FieldListEnumerator(); res.m_objectManager = this; return res; });
            m_ommElementListEnumeratorPool = new EmaPool<ElementListEnumerator>(initialPoolSize, () => { var res = new ElementListEnumerator(); res.m_objectManager = this; return res; });
            m_ommFilterListEnumeratorPool = new EmaPool<FilterListEnumerator>(initialPoolSize, () => { var res = new FilterListEnumerator(); res.m_objectManager = this; return res; });
            m_ommSeriesEnumeratorPool = new EmaPool<SeriesEnumerator>(initialPoolSize, () => { var res = new SeriesEnumerator(); res.m_objectManager = this; return res; });
            m_ommMapEnumeratorPool = new EmaPool<MapEnumerator>(initialPoolSize, () => { var res = new MapEnumerator(this); return res; });
            m_ommVectorEnumeratorPool = new EmaPool<VectorEnumerator>(initialPoolSize, () => { var res = new VectorEnumerator(); res.m_objectManager = this; return res; });

            m_ommFieldListErrorEnumeratorPool = new EmaPool<FieldListErrorEnumerator>(initialPoolSize, () => { var res = new FieldListErrorEnumerator(); res.m_objectManager = this; return res; });
            m_ommElementListErrorEnumeratorPool = new EmaPool<ElementListErrorEnumerator>(initialPoolSize, () => { var res = new ElementListErrorEnumerator(); res.m_objectManager = this; return res; });
            m_ommFilterListErrorEnumeratorPool = new EmaPool<FilterListErrorEnumerator>(initialPoolSize, () => { var res = new FilterListErrorEnumerator(); res.m_objectManager = this; return res; });
            m_ommSeriesErrorEnumeratorPool = new EmaPool<SeriesErrorEnumerator>(initialPoolSize, () => { var res = new SeriesErrorEnumerator(); res.m_objectManager = this; return res; });
            m_ommMapErrorEnumeratorPool = new EmaPool<MapErrorEnumerator>(initialPoolSize, () => { var res = new MapErrorEnumerator(); res.m_objectManager = this; return res; });
            m_ommVectorErrorEnumeratorPool = new EmaPool<VectorErrorEnumerator>(initialPoolSize, () => { var res = new VectorErrorEnumerator(); res.m_objectManager = this; return res; });

            m_ommOpaquePool = new EmaPool<OmmOpaque>(initialPoolSize, () => { var res = new OmmOpaque(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommXmlPool = new EmaPool<OmmXml>(initialPoolSize, () => { var res = new OmmXml(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommFieldListPool = new EmaPool<FieldList>(initialPoolSize, () => { var res = new FieldList(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommElementListPool = new EmaPool<ElementList>(initialPoolSize, () => { var res = new ElementList(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommAnsiPagePool = new EmaPool<OmmAnsiPage>(initialPoolSize, () => { var res = new OmmAnsiPage(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommFilterListPool = new EmaPool<FilterList>(initialPoolSize, () => { var res = new FilterList(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommVectorPool = new EmaPool<Vector>(initialPoolSize, () => { var res = new Vector(); res.m_ownedByPool = true;  res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommMapPool = new EmaPool<Map>(initialPoolSize, () => { var res = new Map(); res.m_ownedByPool = true;  res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommSeriesPool = new EmaPool<Series>(initialPoolSize, () => { var res = new Series(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);

            m_ommAckMsgPool = new EmaPool<AckMsg>(initialPoolSize, () => { var res = new AckMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommGenericMsgPool = new EmaPool<GenericMsg>(initialPoolSize, () => { var res = new GenericMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommPostMsgPool = new EmaPool<PostMsg>(initialPoolSize, () => { var res = new PostMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommUpdateMsgPool = new EmaPool<UpdateMsg>(initialPoolSize, () => { var res = new UpdateMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommRequestMsgPool = new EmaPool<RequestMsg>(initialPoolSize, () => { var res = new RequestMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommRefreshMsgPool = new EmaPool<RefreshMsg>(initialPoolSize, () => { var res = new RefreshMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommStatusMsgPool = new EmaPool<StatusMsg>(initialPoolSize, () => { var res = new StatusMsg(); res.m_ownedByPool = true; res.SetObjectManager(this); return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);

            m_ommErrorPool = new EmaPool<OmmError>(initialPoolSize, () => { var res = new OmmError(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommNoDataPool = new EmaPool<NoData>(initialPoolSize, () => { var res = new NoData(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);

            m_ommIntPool = new EmaPool<OmmInt>(initialPoolSize, () => { var res = new OmmInt(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommUIntPool = new EmaPool<OmmUInt>(initialPoolSize, () => { var res = new OmmUInt(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommFloatPool = new EmaPool<OmmFloat>(initialPoolSize, () => { var res = new OmmFloat(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommDoublePool = new EmaPool<OmmDouble>(initialPoolSize, () => { var res = new OmmDouble(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommRealPool = new EmaPool<OmmReal>(initialPoolSize, () => { var res = new OmmReal(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommDatePool = new EmaPool<OmmDate>(initialPoolSize, () => { var res = new OmmDate(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommTimePool = new EmaPool<OmmTime>(initialPoolSize, () => { var res = new OmmTime(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommDateTimePool = new EmaPool<OmmDateTime>(initialPoolSize, () => { var res = new OmmDateTime(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommQosPool = new EmaPool<OmmQos>(initialPoolSize, () => { var res = new OmmQos(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommStatePool = new EmaPool<OmmState>(initialPoolSize, () => { var res = new OmmState(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommEnumPool = new EmaPool<OmmEnum>(initialPoolSize, () => { var res = new OmmEnum(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommArrayPool = new EmaPool<OmmArray>(initialPoolSize, () => { var res = new OmmArray(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommBufferPool = new EmaPool<OmmBuffer>(initialPoolSize, () => { var res = new OmmBuffer(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommAsciiPool = new EmaPool<OmmAscii>(initialPoolSize, () => { var res = new OmmAscii(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommUtf8Pool = new EmaPool<OmmUtf8>(initialPoolSize, () => { var res = new OmmUtf8(); res.m_ownedByPool = true; res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);
            m_ommRmtesPool = new EmaPool<OmmRmtes>(initialPoolSize, () => { var res = new OmmRmtes(); res.m_ownedByPool = true;  res.m_objectManager = this; return res; },
                o => o.m_handle = GCHandle.Alloc(o), o => { if (o.m_handle.IsAllocated) o.m_handle.Free(); }, global);

            pools[DataType.DataTypes.INT] = m_ommIntPool;
            pools[DataType.DataTypes.UINT] = m_ommUIntPool;
            pools[DataType.DataTypes.FLOAT] = m_ommFloatPool;
            pools[DataType.DataTypes.DOUBLE] = m_ommDoublePool;
            pools[DataType.DataTypes.REAL] = m_ommRealPool;
            pools[DataType.DataTypes.DATE] = m_ommDatePool;
            pools[DataType.DataTypes.TIME] = m_ommTimePool;
            pools[DataType.DataTypes.DATETIME] = m_ommDateTimePool;
            pools[DataType.DataTypes.QOS] = m_ommQosPool;
            pools[DataType.DataTypes.STATE] = m_ommStatePool;
            pools[DataType.DataTypes.ENUM] = m_ommEnumPool;
            pools[DataType.DataTypes.ARRAY] = m_ommArrayPool;
            pools[DataType.DataTypes.BUFFER] = m_ommBufferPool;
            pools[DataType.DataTypes.ASCII] = m_ommAsciiPool;
            pools[DataType.DataTypes.UTF8] = m_ommUtf8Pool;
            pools[DataType.DataTypes.RMTES] = m_ommRmtesPool;
            pools[DataType.DataTypes.NO_DATA] = m_ommNoDataPool;
            pools[DataType.DataTypes.OPAQUE] = m_ommOpaquePool;
            pools[DataType.DataTypes.XML] = m_ommXmlPool;
            pools[DataType.DataTypes.FIELD_LIST] = m_ommFieldListPool;
            pools[DataType.DataTypes.ELEMENT_LIST] = m_ommElementListPool;
            pools[DataType.DataTypes.ANSI_PAGE] = m_ommAnsiPagePool;
            pools[DataType.DataTypes.FILTER_LIST] = m_ommFilterListPool;
            pools[DataType.DataTypes.VECTOR] = m_ommVectorPool;
            pools[DataType.DataTypes.MAP] = m_ommMapPool;
            pools[DataType.DataTypes.SERIES] = m_ommSeriesPool;
            pools[DataType.DataTypes.REQ_MSG] = m_ommRequestMsgPool;
            pools[DataType.DataTypes.REFRESH_MSG] = m_ommRefreshMsgPool;
            pools[DataType.DataTypes.UPDATE_MSG] = m_ommUpdateMsgPool;
            pools[DataType.DataTypes.STATUS_MSG] = m_ommStatusMsgPool;
            pools[DataType.DataTypes.POST_MSG] = m_ommPostMsgPool;
            pools[DataType.DataTypes.ACK_MSG] = m_ommAckMsgPool;
            pools[DataType.DataTypes.GENERIC_MSG] = m_ommGenericMsgPool;
            pools[DataType.DataTypes.ERROR] = m_ommErrorPool;

            primitivePool = new EmaPrimitiveDataPool(this, initialPoolSize, global);
            complexTypePool = new EmaComplexTypePool(this, initialPoolSize, global);
            msgTypePool = new EmaMsgTypePool(this, initialPoolSize, global);
        }

        internal Action? FreeSingleItemPool;

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void GrowSingleItemPool<T>(int count)
        {
            for (int i = 0; i < count; i++)
            {
                var singleItem = new SingleItem<T>();
                m_singleItemPool.Add(singleItem);
                singleItem.m_handle = GCHandle.Alloc(singleItem);
            }

            if (FreeSingleItemPool == null)
            {
                FreeSingleItemPool = () =>
                {
                    SingleItem<T>? item;
                    while ((item = (SingleItem<T>?)m_singleItemPool.Poll()) != null) { item.m_handle.Free(); }
                };
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void GrowAdminDomainItemPools<T>()
        {
            for (int i = 0; i < INITIAL_ADMIN_DOMAIN_ITEM_POOL_SIZE; i++)
            {
                var loginItem = new LoginItem<T>();
                m_loginItemPool.Add(loginItem);
            }

            for (int i = 0; i < INITIAL_ADMIN_DOMAIN_ITEM_POOL_SIZE; i++)
            {
                var directoryItem = new DirectoryItem<T>();
                m_directoryItemPool.Add(directoryItem);
            }

            for (int i = 0; i < INITIAL_ADMIN_DOMAIN_ITEM_POOL_SIZE; i++)
            {
                var batchItem = new BatchItem<T>();
                m_batchItemPool.Add(batchItem);
            }

            for (int i = 0; i < INITIAL_ADMIN_DOMAIN_ITEM_POOL_SIZE; i++)
            {
                var dictionaryItem = new DictionaryItem<T>();
                m_dictionaryItemPool.Add(dictionaryItem);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Data? GetDataObjectFromPool(int dataType)
        {
            var res = pools[dataType].Get();
            res.m_inPool = false;
            return res;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ComplexType? GetComplexTypeFromPool(int dataType)
        {
            var res = (ComplexType)pools[dataType].Get();
            res.m_inPool = false;
            return res;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ComplexTypeArray GetComplexTypeArrayFromPool()
        {
            return complexTypePool.Get();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnComplexTypeArrayToPool(ComplexTypeArray obj)
        {
            complexTypePool.ReturnToPool(obj);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public DataArray GetPrimitiveDataArrayFromPool()
        {
            return primitivePool.Get();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnPrimitiveDataArrayToPool(DataArray obj)
        {
            primitivePool.ReturnToPool(obj);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public MsgTypeArray GetMsgTypeArrayFromPool()
        {
            return msgTypePool.Get();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnMsgTypeArrayToPool(MsgTypeArray obj)
        {
            msgTypePool.ReturnToPool(obj);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(NoData value)
        {
            m_ommNoDataPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmState value)
        {
            m_ommStatePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmQos value)
        {
            m_ommQosPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmError value)
        {
            m_ommErrorPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmEnum value)
        {
            m_ommEnumPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmAnsiPage value)
        {
            m_ommAnsiPagePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmInt value)
        {
            m_ommIntPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmUInt value)
        {
            m_ommUIntPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmFloat value)
        {
            m_ommFloatPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmDouble value)
        {
            m_ommDoublePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmReal value)
        {
            m_ommRealPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmAscii value)
        {
            m_ommAsciiPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmDate value)
        {
            m_ommDatePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmTime value)
        {
            m_ommTimePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmDateTime value)
        {
            m_ommDateTimePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmRmtes value)
        {
            m_ommRmtesPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmBuffer value)
        {
            m_ommBufferPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmArray value)
        {
            m_ommArrayPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmUtf8 value)
        {
            m_ommUtf8Pool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmXml value)
        {
            m_ommXmlPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(OmmOpaque value)
        {
            m_ommOpaquePool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(FieldList value)
        {
            m_ommFieldListPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(ElementList value)
        {
            m_ommElementListPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(FilterList value)
        {
            m_ommFilterListPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(Vector value)
        {
            m_ommVectorPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(Map value)
        {
            m_ommMapPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(Series value)
        {
            m_ommSeriesPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(AckMsg value)
        {
            m_ommAckMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(PostMsg value)
        {
            m_ommPostMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(RefreshMsg value)
        {
            m_ommRefreshMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(RequestMsg value)
        {
            m_ommRequestMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(StatusMsg value)
        {
            m_ommStatusMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(GenericMsg value)
        {
            m_ommGenericMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToPool(UpdateMsg value)
        {
            m_ommUpdateMsgPool.ReturnToPool(value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(FieldListEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommFieldListEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(FieldListErrorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommFieldListErrorEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(FilterListEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommFilterListEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(FilterListErrorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommFilterListErrorEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(ElementListEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommElementListEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(ElementListErrorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommElementListErrorEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(MapEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommMapEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(SeriesEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommSeriesEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(VectorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommVectorEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(MapErrorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommMapErrorEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(SeriesErrorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommSeriesErrorEnumeratorPool.ReturnToPool(enumerator);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReturnToEnumeratorPool(VectorErrorEnumerator enumerator)
        {
            enumerator.m_inPool = true;
            m_ommVectorErrorEnumeratorPool.ReturnToPool(enumerator);
        }

        #region Methods for getting OMM objects

        public OmmInt GetOmmInt()
        {
            var result = m_ommIntPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmUInt GetOmmUInt()
        {
            var result = m_ommUIntPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmFloat GetOmmFloat()
        {
            var result = m_ommFloatPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmDouble GetOmmDouble()
        {
            var result = m_ommDoublePool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmReal GetOmmReal()
        {
            var result = m_ommRealPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmDate GetOmmDate()
        {
            var result = m_ommDatePool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmTime GetOmmTime()
        {
            var result = m_ommTimePool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmDateTime GetOmmDateTime()
        {
            var result = m_ommDateTimePool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmQos GetOmmQos()
        {
            var result = m_ommQosPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmState GetOmmState()
        {
            var result = m_ommStatePool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmEnum GetOmmEnum()
        {
            var result = m_ommEnumPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmArray GetOmmArray()
        {
            var result = m_ommArrayPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmBuffer GetOmmBuffer()
        {
            var result = m_ommBufferPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmAscii GetOmmAscii()
        {
            var result = m_ommAsciiPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmUtf8 GetOmmUtf8()
        {
            var result = m_ommUtf8Pool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmRmtes GetOmmRmtes()
        {
            var result = m_ommRmtesPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmOpaque GetOmmOpaque()
        {
            var result = m_ommOpaquePool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmXml GetOmmXml()
        {
            var result = m_ommXmlPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmAnsiPage GetOmmAnsiPage()
        {
            var result = m_ommAnsiPagePool.Get();
            result.m_inPool = false;
            return result;
        }

        public FieldList GetOmmFieldList()
        {
            var result = m_ommFieldListPool.Get();
            result.m_inPool = false;
            return result;
        }

        public ElementList GetOmmElementList()
        {
            var result = m_ommElementListPool.Get();
            result.m_inPool = false;
            return result;
        }

        public FilterList GetOmmFilterList()
        {
            var result = m_ommFilterListPool.Get();
            result.m_inPool = false;
            return result;
        }

        public Vector GetOmmVector()
        {
            var result = m_ommVectorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public Series GetOmmSeries()
        {
            var result = m_ommSeriesPool.Get();
            result.m_inPool = false;
            return result;
        }

        public Map GetOmmMap()
        {
            var result = m_ommMapPool.Get();
            result.m_inPool = false;
            return result;
        }

        public AckMsg GetOmmAckMsg()
        {
            var result = m_ommAckMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public PostMsg GetOmmPostMsg()
        {
            var result = m_ommPostMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public RequestMsg GetOmmRequestMsg()
        {
            var result = m_ommRequestMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public RefreshMsg GetOmmRefreshMsg()
        {
            var result = m_ommRefreshMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public StatusMsg GetOmmStatusMsg()
        {
            var result = m_ommStatusMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public GenericMsg GetOmmGenericMsg()
        {
            var result = m_ommGenericMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public UpdateMsg GetOmmUpdateMsg()
        {
            var result = m_ommUpdateMsgPool.Get();
            result.m_inPool = false;
            return result;
        }

        public NoData GetOmmNoData()
        {
            var result = m_ommNoDataPool.Get();
            result.m_inPool = false;
            return result;
        }

        public OmmError GetOmmError()
        {
            var result = m_ommErrorPool.Get();
            result.m_inPool = false;
            return result;
        }

        #endregion

        #region Methods for getting Enumerators

        public ElementListEnumerator GetElementListEnumerator()
        {
            var result = (ElementListEnumerator)m_ommElementListEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public FieldListEnumerator GetFieldListEnumerator()
        {
            var result = (FieldListEnumerator)m_ommFieldListEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public FilterListEnumerator GetFilterListEnumerator()
        {
            var result = (FilterListEnumerator)m_ommFilterListEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public MapEnumerator GetMapEnumerator()
        {
            var result = (MapEnumerator)m_ommMapEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public VectorEnumerator GetVectorEnumerator()
        {
            var result = (VectorEnumerator)m_ommVectorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public SeriesEnumerator GetSeriesEnumerator()
        {
            var result = (SeriesEnumerator)m_ommSeriesEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public ElementListErrorEnumerator GetElementListErrorEnumerator()
        {
            var result = (ElementListErrorEnumerator)m_ommElementListErrorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public FieldListErrorEnumerator GetFieldListErrorEnumerator()
        {
            var result = (FieldListErrorEnumerator)m_ommFieldListErrorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public FilterListErrorEnumerator GetFilterListErrorEnumerator()
        {
            var result = (FilterListErrorEnumerator)m_ommFilterListErrorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public MapErrorEnumerator GetMapErrorEnumerator()
        {
            var result = (MapErrorEnumerator)m_ommMapErrorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public VectorErrorEnumerator GetVectorErrorEnumerator()
        {
            var result = (VectorErrorEnumerator)m_ommVectorErrorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
        }

        public SeriesErrorEnumerator GetSeriesErrorEnumerator()
        {
            var result = (SeriesErrorEnumerator)m_ommSeriesErrorEnumeratorPool.Get();
            result.m_inPool = false;
            return result;
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

        internal void Free()
        {
            if (FreeSingleItemPool != null) FreeSingleItemPool();
            if (primitivePool != null)
            {
                for (int i = 0; i < primitivePool.pool.Length; i++)
                {
                    primitivePool.pool[i].m_handle.Free();
                }
            }
            if (complexTypePool != null)
            {
                for (int i = 0; i < complexTypePool.pool.Length; i++)
                {
                    complexTypePool.pool[i].m_handle.Free();
                }
            }
            if (msgTypePool != null)
            {
                for (int i = 0; i < msgTypePool.pool.Length; i++)
                {
                    msgTypePool.pool[i].m_handle.Free();
                }
            }

            foreach (var pool in pools) { if (pool != null) pool.FreePool(); }
        }
    
    }
}
