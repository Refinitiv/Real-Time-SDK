/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2023-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import java.util.*;

import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.valueadd.common.LimitedVaPool;

class EmaObjectManager
{
	enum InitResult
	{
		SUCCESS,
		NO_CHANGE,
		FAILURE
	}

	final static int DATA_POOL_INITIAL_SIZE = 5;
	private final static int DEFAULT_BYTE_BUFFER_SIZE = 5;
	private final static int MAX_NUM_BYTE_BUFFER = 5;
	private final static int MAX_BYTE_BUFFER_CAPABILITY = 2000;
	private final static int DEFAULT_ETA_CONTAINER_SIZE = 10;

	private final static int ENTRY_MULTIPLIER = 5;

	private Lock _byteBufferLock = null;
	private static Lock _globalInitLock = new ReentrantLock();
	private List<ByteBuffer>[] _byteBufferList;
	private boolean _intialized;

	private boolean _limitsSetFromConfig = false;

	int _initDataTypePoolLimit;
	int _initComplexTypePoolLimit;
	int _initMsgTypePoolLimit;
	int _initSessionObjectsPoolLimit;
	int _initEtaObjectsPoolsLimit;

	int _dataTypePoolLimit;
	int _complexTypePoolLimit;
	int _msgTypePoolLimit;
	int _sessionObjectsPoolLimit;
	int _etaObjectsPoolsLimit;

	static EmaObjectManager GlobalObjectManager;

	LimitedVaPool[] _dataTypePools = new LimitedVaPool[20];
	LimitedVaPool[] _complexTypePools = new LimitedVaPool[16];
	LimitedVaPool[] _msgTypePools = new LimitedVaPool[8];

	static int dataTypeOffset = DataType.DataTypes.INT;
	static int complexTypeOffset = DataType.DataTypes.NO_DATA;
	static int msgTypeOffset = DataType.DataTypes.REQ_MSG;

	LimitedVaPool[] _entryPoolsArray = new LimitedVaPool[20];
	
	static 
	{
		GlobalObjectManager = new EmaObjectManager(true);
		GlobalObjectManager.initialize(GlobalConfig.DEFAULT_EMA_POOLS_LIMIT,
				GlobalConfig.DEFAULT_EMA_POOLS_LIMIT,
				GlobalConfig.DEFAULT_EMA_POOLS_LIMIT,
				GlobalConfig.DEFAULT_EMA_POOLS_LIMIT,
				GlobalConfig.DEFAULT_EMA_POOLS_LIMIT,
				true);
	}

	LimitedVaPool _ommIntPool;
	LimitedVaPool _ommUIntPool;
	LimitedVaPool _ommFloatPool;
	LimitedVaPool _ommDoublePool;
	LimitedVaPool _ommBufferPool;
	LimitedVaPool _ommAsciiPool;
	LimitedVaPool _ommUtf8Pool;
	LimitedVaPool _ommRmtesPool;
	LimitedVaPool _ommRealPool;
	LimitedVaPool _ommDatePool;
	LimitedVaPool _ommTimePool;
	LimitedVaPool _ommDateTimePool;
	LimitedVaPool _ommQosPool;
	LimitedVaPool _ommStatePool;
	LimitedVaPool _ommEnumPool;
	LimitedVaPool _ommArrayPool;
	LimitedVaPool _fieldListPool;
	LimitedVaPool _mapPool;
	LimitedVaPool _elementListPool;
	LimitedVaPool _filterListPool;
	LimitedVaPool _vectorPool;
	LimitedVaPool _seriesPool;
	LimitedVaPool _opaquePool;
	LimitedVaPool _ansiPagePool;
	LimitedVaPool _xmlPool;
	LimitedVaPool _jsonPool;
	LimitedVaPool _reqMsgPool;
	LimitedVaPool _refreshMsgPool;
	LimitedVaPool _statusMsgPool;
	LimitedVaPool _updateMsgPool;
	LimitedVaPool _ackMsgPool;
	LimitedVaPool _postMsgPool;
	LimitedVaPool _genericMsgPool;
	LimitedVaPool _noDataPool;
	LimitedVaPool _ommErrorPool;

	LimitedVaPool _fieldEntryPool;
	LimitedVaPool _elementEntryPool;
	LimitedVaPool _arrayEntryPool;
	LimitedVaPool _filterEntryPool;
	LimitedVaPool _mapEntryPool;
	LimitedVaPool _seriesEntryPool;
	LimitedVaPool _vectorEntryPool;

	LimitedVaPool _singleItemPool;
	LimitedVaPool _batchItemPool;
	LimitedVaPool _subItemPool;
	LimitedVaPool _tunnelItemPool;
	LimitedVaPool _dictionaryItemPool;
	LimitedVaPool _niproviderDictionaryItemPool;
	LimitedVaPool _iproviderDictionaryItemPool;
	LimitedVaPool _directoryItemPool;
	LimitedVaPool _loginItemPool;
	LimitedVaPool _longObjectPool;
	LimitedVaPool _intObjectPool;
	LimitedVaPool _ommServiceIdIntegerPool;
	LimitedVaPool _streamInfoPool;
	LimitedVaPool _timeoutEventPool;

	Deque<com.refinitiv.eta.codec.ElementList> _rsslElementListPool;
	Deque<com.refinitiv.eta.codec.Vector> _rsslVectorPool;
	Deque<com.refinitiv.eta.codec.FieldList> _rsslFieldListPool;
	Deque<com.refinitiv.eta.codec.FilterList> _rsslFilterListPool;
	Deque<com.refinitiv.eta.codec.Map> _rsslMapPool;
	Deque<com.refinitiv.eta.codec.Series> _rsslSeriesPool;
	Deque<com.refinitiv.eta.codec.Array> _rsslArrayPool;

	Deque<com.refinitiv.eta.codec.DecodeIterator> _etaDecodeIteratorPool;

	List<Deque> _etaPools = new ArrayList<>();
	
	EmaObjectManager()
	{
		this(false);
	}
	
	EmaObjectManager(boolean globalLock)
	{
		if (globalLock)
		{
			_byteBufferLock = new ReentrantLock();
		}

		_ommIntPool = new LimitedVaPool(globalLock);
		_ommUIntPool = new LimitedVaPool(globalLock);
		_ommFloatPool = new LimitedVaPool(globalLock);
		_ommDoublePool = new LimitedVaPool(globalLock);
		_ommBufferPool = new LimitedVaPool(globalLock);
		_ommAsciiPool = new LimitedVaPool(globalLock);
		_ommUtf8Pool = new LimitedVaPool(globalLock);
		_ommRmtesPool = new LimitedVaPool(globalLock);
		_ommRealPool = new LimitedVaPool(globalLock);
		_ommDatePool = new LimitedVaPool(globalLock);
		_ommTimePool = new LimitedVaPool(globalLock);
		_ommDateTimePool = new LimitedVaPool(globalLock);
		_ommQosPool = new LimitedVaPool(globalLock);
		_ommStatePool = new LimitedVaPool(globalLock);
		_ommEnumPool = new LimitedVaPool(globalLock);
		_ommArrayPool = new LimitedVaPool(globalLock);

		_dataTypePools[DataType.DataTypes.INT - dataTypeOffset] = _ommIntPool;
		_dataTypePools[DataType.DataTypes.UINT - dataTypeOffset] = _ommUIntPool;
		_dataTypePools[DataType.DataTypes.FLOAT - dataTypeOffset] = _ommFloatPool;
		_dataTypePools[DataType.DataTypes.DOUBLE - dataTypeOffset] = _ommDoublePool;
		_dataTypePools[DataType.DataTypes.BUFFER - dataTypeOffset] = _ommBufferPool;
		_dataTypePools[DataType.DataTypes.ASCII - dataTypeOffset] = _ommAsciiPool;
		_dataTypePools[DataType.DataTypes.UTF8 - dataTypeOffset] = _ommUtf8Pool;
		_dataTypePools[DataType.DataTypes.RMTES - dataTypeOffset] = _ommRmtesPool;
		_dataTypePools[DataType.DataTypes.REAL - dataTypeOffset] = _ommRealPool;
		_dataTypePools[DataType.DataTypes.DATE - dataTypeOffset] = _ommDatePool;
		_dataTypePools[DataType.DataTypes.TIME - dataTypeOffset] = _ommTimePool;
		_dataTypePools[DataType.DataTypes.DATETIME - dataTypeOffset] = _ommDateTimePool;
		_dataTypePools[DataType.DataTypes.QOS - dataTypeOffset] = _ommQosPool;
		_dataTypePools[DataType.DataTypes.STATE - dataTypeOffset] = _ommStatePool;
		_dataTypePools[DataType.DataTypes.ENUM - dataTypeOffset] = _ommEnumPool;
		_dataTypePools[DataType.DataTypes.ARRAY - dataTypeOffset] = _ommArrayPool;

		_fieldListPool = new LimitedVaPool(globalLock);
		_mapPool = new LimitedVaPool(globalLock);
		_elementListPool = new LimitedVaPool(globalLock);
		_filterListPool = new LimitedVaPool(globalLock);
		_vectorPool = new LimitedVaPool(globalLock);
		_seriesPool = new LimitedVaPool(globalLock);
		_opaquePool = new LimitedVaPool(globalLock);
		_ansiPagePool = new LimitedVaPool(globalLock);
		_xmlPool = new LimitedVaPool(globalLock);
		_jsonPool = new LimitedVaPool(globalLock);
		_noDataPool = new LimitedVaPool(globalLock);

		_complexTypePools[DataType.DataTypes.FIELD_LIST - complexTypeOffset] = _fieldListPool;
		_complexTypePools[DataType.DataTypes.MAP - complexTypeOffset] = _mapPool;
		_complexTypePools[DataType.DataTypes.ELEMENT_LIST - complexTypeOffset] = _elementListPool;
		_complexTypePools[DataType.DataTypes.FILTER_LIST - complexTypeOffset] = _filterListPool;
		_complexTypePools[DataType.DataTypes.VECTOR - complexTypeOffset] = _vectorPool;
		_complexTypePools[DataType.DataTypes.SERIES - complexTypeOffset] = _seriesPool;
		_complexTypePools[DataType.DataTypes.OPAQUE - complexTypeOffset] = _opaquePool;
		_complexTypePools[DataType.DataTypes.ANSI_PAGE - complexTypeOffset] = _ansiPagePool;
		_complexTypePools[DataType.DataTypes.XML - complexTypeOffset] = _xmlPool;
		_complexTypePools[DataType.DataTypes.JSON - complexTypeOffset] = _jsonPool;
		_complexTypePools[DataType.DataTypes.NO_DATA - complexTypeOffset] = _noDataPool;

		_reqMsgPool = new LimitedVaPool(globalLock);
		_refreshMsgPool = new LimitedVaPool(globalLock);
		_statusMsgPool = new LimitedVaPool(globalLock);
		_updateMsgPool = new LimitedVaPool(globalLock);
		_ackMsgPool = new LimitedVaPool(globalLock);
		_postMsgPool = new LimitedVaPool(globalLock);
		_genericMsgPool = new LimitedVaPool(globalLock);

		_msgTypePools[DataType.DataTypes.REQ_MSG - msgTypeOffset] = _reqMsgPool;
		_msgTypePools[DataType.DataTypes.REFRESH_MSG - msgTypeOffset] = _refreshMsgPool;
		_msgTypePools[DataType.DataTypes.STATUS_MSG - msgTypeOffset] = _statusMsgPool;
		_msgTypePools[DataType.DataTypes.UPDATE_MSG - msgTypeOffset] = _updateMsgPool;
		_msgTypePools[DataType.DataTypes.ACK_MSG - msgTypeOffset] = _ackMsgPool;
		_msgTypePools[DataType.DataTypes.POST_MSG - msgTypeOffset] = _postMsgPool;
		_msgTypePools[DataType.DataTypes.GENERIC_MSG - msgTypeOffset] = _genericMsgPool;

		_fieldEntryPool = new LimitedVaPool(globalLock);
		_elementEntryPool = new LimitedVaPool(globalLock);
		_arrayEntryPool = new LimitedVaPool(globalLock);
		_filterEntryPool = new LimitedVaPool(globalLock);
		_mapEntryPool = new LimitedVaPool(globalLock);
		_seriesEntryPool = new LimitedVaPool(globalLock);
		_vectorEntryPool = new LimitedVaPool(globalLock);

		_entryPoolsArray[DataType.DataTypes.FIELD_LIST - complexTypeOffset] = _fieldEntryPool;
		_entryPoolsArray[DataType.DataTypes.ELEMENT_LIST - complexTypeOffset] = _elementEntryPool;
		_entryPoolsArray[DataType.DataTypes.FILTER_LIST - complexTypeOffset] = _filterEntryPool;
		_entryPoolsArray[DataType.DataTypes.MAP - complexTypeOffset] = _mapEntryPool;
		_entryPoolsArray[DataType.DataTypes.SERIES - complexTypeOffset] = _seriesEntryPool;
		_entryPoolsArray[DataType.DataTypes.VECTOR - complexTypeOffset] = _vectorEntryPool;

		_ommErrorPool = new LimitedVaPool(globalLock);

		_singleItemPool = new LimitedVaPool(globalLock);
		_batchItemPool = new LimitedVaPool(globalLock);
		_subItemPool = new LimitedVaPool(globalLock);
		_tunnelItemPool = new LimitedVaPool(globalLock);
		_dictionaryItemPool = new LimitedVaPool(globalLock);
		_niproviderDictionaryItemPool = new LimitedVaPool(globalLock);
		_iproviderDictionaryItemPool = new LimitedVaPool(globalLock);
		_directoryItemPool = new LimitedVaPool(globalLock);
		_loginItemPool = new LimitedVaPool(globalLock);
		_longObjectPool = new LimitedVaPool(globalLock);
		_intObjectPool = new LimitedVaPool(globalLock);
		_timeoutEventPool = new LimitedVaPool(globalLock);

		_ommServiceIdIntegerPool = new LimitedVaPool(true);
		_streamInfoPool = new LimitedVaPool(true);

		if (globalLock)
		{
			_rsslElementListPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.ElementList>();
			_rsslVectorPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.Vector>();
			_rsslFieldListPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.FieldList>();
			_rsslFilterListPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.FilterList>();
			_rsslMapPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.Map>();
			_rsslSeriesPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.Series>();
			_rsslArrayPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.Array>();
			_etaDecodeIteratorPool = new ConcurrentLinkedDeque<com.refinitiv.eta.codec.DecodeIterator>();
		}
		else
		{
			_rsslElementListPool = new ArrayDeque<com.refinitiv.eta.codec.ElementList>(DEFAULT_ETA_CONTAINER_SIZE);
			_rsslVectorPool = new ArrayDeque<com.refinitiv.eta.codec.Vector>(DEFAULT_ETA_CONTAINER_SIZE);
			_rsslFieldListPool = new ArrayDeque<com.refinitiv.eta.codec.FieldList>(DEFAULT_ETA_CONTAINER_SIZE);
			_rsslFilterListPool = new ArrayDeque<com.refinitiv.eta.codec.FilterList>(DEFAULT_ETA_CONTAINER_SIZE);
			_rsslMapPool = new ArrayDeque<com.refinitiv.eta.codec.Map>(DEFAULT_ETA_CONTAINER_SIZE);
			_rsslSeriesPool = new ArrayDeque<com.refinitiv.eta.codec.Series>(DEFAULT_ETA_CONTAINER_SIZE);
			_rsslArrayPool = new ArrayDeque<com.refinitiv.eta.codec.Array>(DEFAULT_ETA_CONTAINER_SIZE);
			_etaDecodeIteratorPool = new ArrayDeque<com.refinitiv.eta.codec.DecodeIterator>(DEFAULT_ETA_CONTAINER_SIZE);
		}

		_etaPools.add(_rsslElementListPool);
		_etaPools.add(_rsslVectorPool);
		_etaPools.add(_rsslFieldListPool);
		_etaPools.add(_rsslFilterListPool);
		_etaPools.add(_rsslMapPool);
		_etaPools.add(_rsslSeriesPool);
		_etaPools.add(_rsslArrayPool);
		_etaPools.add(_etaDecodeIteratorPool);
	}

	<T> void initialize(int dataTypePoolLimit,
						int complexTypePoolLimit,
						int msgTypePoolLimit,
						int sessionObjectsPoolLimit,
						int etaObjectsPoolsLimit)
	{
		if (_intialized)
			return;

		_intialized = true;

		NoDataImpl load;

		_dataTypePoolLimit = dataTypePoolLimit >= 0 ? dataTypePoolLimit : -1;
		_complexTypePoolLimit = complexTypePoolLimit >= 0 ? complexTypePoolLimit : -1;
		_msgTypePoolLimit = msgTypePoolLimit >= 0 ? msgTypePoolLimit : -1;
		_sessionObjectsPoolLimit = sessionObjectsPoolLimit >= 0 ? sessionObjectsPoolLimit : -1;
		_etaObjectsPoolsLimit = etaObjectsPoolsLimit >= 0 ? etaObjectsPoolsLimit : -1;

		_initDataTypePoolLimit = _dataTypePoolLimit;
		_initComplexTypePoolLimit = _complexTypePoolLimit;
		_initMsgTypePoolLimit = _msgTypePoolLimit;
		_initSessionObjectsPoolLimit = _sessionObjectsPoolLimit;
		_initEtaObjectsPoolsLimit = _etaObjectsPoolsLimit;

		for (int index = 0; index < _dataTypePoolLimit; ++index)
		{
			_ommIntPool.add(new OmmIntImpl());
			_ommUIntPool.add(new OmmUIntImpl());
			_ommFloatPool.add(new OmmFloatImpl());
			_ommDoublePool.add(new OmmDoubleImpl());
			_ommBufferPool.add(new OmmBufferImpl());
			_ommAsciiPool.add(new OmmAsciiImpl());
			_ommUtf8Pool.add(new OmmUtf8Impl());
			_ommRmtesPool.add(new OmmRmtesImpl());
			_ommRealPool.add(new OmmRealImpl());
			_ommDatePool.add(new OmmDateImpl());
			_ommTimePool.add(new OmmTimeImpl());
			_ommDateTimePool.add(new OmmDateTimeImpl());
			_ommQosPool.add(new OmmQosImpl());
			_ommStatePool.add(new OmmStateImpl());
			_ommEnumPool.add(new OmmEnumImpl());
			_ommArrayPool.add(new OmmArrayImpl(this));
		}

		_ommIntPool.setLimit(_dataTypePoolLimit);
		_ommUIntPool.setLimit(_dataTypePoolLimit);
		_ommFloatPool.setLimit(_dataTypePoolLimit);
		_ommDoublePool.setLimit(_dataTypePoolLimit);
		_ommBufferPool.setLimit(_dataTypePoolLimit);
		_ommAsciiPool.setLimit(_dataTypePoolLimit);
		_ommUtf8Pool.setLimit(_dataTypePoolLimit);
		_ommRmtesPool.setLimit(_dataTypePoolLimit);
		_ommRealPool.setLimit(_dataTypePoolLimit);
		_ommDatePool.setLimit(_dataTypePoolLimit);
		_ommTimePool.setLimit(_dataTypePoolLimit);
		_ommDateTimePool.setLimit(_dataTypePoolLimit);
		_ommQosPool.setLimit(_dataTypePoolLimit);
		_ommStatePool.setLimit(_dataTypePoolLimit);
		_ommEnumPool.setLimit(_dataTypePoolLimit);
		_ommArrayPool.setLimit(_dataTypePoolLimit);

		for (int index = 0; index < _complexTypePoolLimit; ++index)
		{
			_fieldListPool.add(new FieldListImpl(this));
			_elementListPool.add(new ElementListImpl(this));
			_ommErrorPool.add(new OmmErrorImpl());
			_mapPool.add(new MapImpl(this));
			_seriesPool.add(new SeriesImpl(this));
			_filterListPool.add(new FilterListImpl(this));
			_vectorPool.add(new VectorImpl(this));
			_xmlPool.add(new OmmXmlImpl());
			_jsonPool.add(new OmmJsonImpl());
			_ansiPagePool.add(new OmmAnsiPageImpl());
			_opaquePool.add(new OmmOpaqueImpl());
		}

		_fieldListPool.setLimit(_complexTypePoolLimit);
		_elementListPool.setLimit(_complexTypePoolLimit);
		_noDataPool.setLimit(_complexTypePoolLimit);
		_ommErrorPool.setLimit(_complexTypePoolLimit);
		_mapPool.setLimit(_complexTypePoolLimit);
		_seriesPool.setLimit(_complexTypePoolLimit);
		_filterListPool.setLimit(_complexTypePoolLimit);
		_vectorPool.setLimit(_complexTypePoolLimit);
		_xmlPool.setLimit(_complexTypePoolLimit);
		_jsonPool.setLimit(_complexTypePoolLimit);
		_ansiPagePool.setLimit(_complexTypePoolLimit);
		_opaquePool.setLimit(_complexTypePoolLimit);

		for (int index = 0; index < _complexTypePoolLimit * ENTRY_MULTIPLIER; ++index)
		{
			load = new NoDataImpl();
			_noDataPool.updatePool(load);
			_fieldEntryPool.add(new FieldEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFieldEntry(), load));

			load = new NoDataImpl();
			_noDataPool.updatePool(load);
			_elementEntryPool.add(new ElementEntryImpl(com.refinitiv.eta.codec.CodecFactory.createElementEntry(), load));

			load = new NoDataImpl();
			_noDataPool.updatePool(load);
			_seriesEntryPool.add(new SeriesEntryImpl(com.refinitiv.eta.codec.CodecFactory.createSeriesEntry(), load));

			load = new NoDataImpl();
			_noDataPool .updatePool(load);
			_vectorEntryPool.add(new VectorEntryImpl(com.refinitiv.eta.codec.CodecFactory.createVectorEntry(), load, this));

			load = new NoDataImpl();
			_noDataPool.updatePool(load);
			_filterEntryPool.add(new FilterEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFilterEntry(), load, this));

			load = new NoDataImpl();
			_noDataPool.updatePool(load);
			_mapEntryPool.add(new MapEntryImpl(com.refinitiv.eta.codec.CodecFactory.createMapEntry(), new DataImpl(), load, this));
		}

		_fieldEntryPool.setLimit(_complexTypePoolLimit * ENTRY_MULTIPLIER);
		_elementEntryPool.setLimit(_complexTypePoolLimit * ENTRY_MULTIPLIER);
		_filterEntryPool.setLimit(_complexTypePoolLimit * ENTRY_MULTIPLIER);
		_mapEntryPool.setLimit(_complexTypePoolLimit * ENTRY_MULTIPLIER);
		_seriesEntryPool.setLimit(_complexTypePoolLimit * ENTRY_MULTIPLIER);
		_vectorEntryPool.setLimit(_complexTypePoolLimit * ENTRY_MULTIPLIER);

		_arrayEntryPool.setLimit(_dataTypePoolLimit * ENTRY_MULTIPLIER);

		_reqMsgPool.setLimit(_msgTypePoolLimit);
		_refreshMsgPool.setLimit(_msgTypePoolLimit);
		_statusMsgPool.setLimit(_msgTypePoolLimit);
		_updateMsgPool.setLimit(_msgTypePoolLimit);
		_ackMsgPool.setLimit(_msgTypePoolLimit);
		_postMsgPool.setLimit(_msgTypePoolLimit);
		_genericMsgPool.setLimit(_msgTypePoolLimit);

		for (int i = 0; i < _msgTypePoolLimit; i++)
		{
			_reqMsgPool.add(new ReqMsgImpl(this));
			_refreshMsgPool.add(new RefreshMsgImpl(this));
			_statusMsgPool.add(new StatusMsgImpl(this));
			_updateMsgPool.add(new UpdateMsgImpl(this));
			_ackMsgPool.add(new AckMsgImpl(this));
			_postMsgPool.add(new PostMsgImpl(this));
			_genericMsgPool.add(new GenericMsgImpl(this));
		}

		_singleItemPool.setLimit(_sessionObjectsPoolLimit);
		_batchItemPool.setLimit(_sessionObjectsPoolLimit);
		_longObjectPool.setLimit(_sessionObjectsPoolLimit);
		_intObjectPool.setLimit(_sessionObjectsPoolLimit);
		_subItemPool.setLimit(_sessionObjectsPoolLimit);
		_tunnelItemPool.setLimit(_sessionObjectsPoolLimit);
		_timeoutEventPool.setLimit(_sessionObjectsPoolLimit);

		for (int index = 0; index < _sessionObjectsPoolLimit; ++index)
		{
			_singleItemPool.add(new SingleItem<T>());
			_batchItemPool.add(new BatchItem<T>());
			_longObjectPool.add(new LongObject());
			_intObjectPool.add(new IntObject());
			_subItemPool.add(new SubItem<T>());
			_tunnelItemPool.add(new TunnelItem<T>());

			_timeoutEventPool.add(new TimeoutEvent(0, null));
		}

		for (int index = 0; index < _complexTypePoolLimit; ++index)
		{
			_noDataPool.add(new NoDataImpl());
		}

		initByteBufferList();
	}

	<T> void initialize(int dataTypePoolLimit,
						int complexTypePoolLimit,
						int msgTypePoolLimit,
						int sessionObjectsPoolLimit,
						int etaObjectsPoolsLimit,
						boolean lock)
	{
		if (lock) _globalInitLock.lock();
		try {
			initialize(dataTypePoolLimit, complexTypePoolLimit, msgTypePoolLimit, sessionObjectsPoolLimit, etaObjectsPoolsLimit);
		}
		finally
		{
			if (lock) _globalInitLock.unlock();
		}
	}
	
	<T> void initialize(long poolSize)
	{
		if (_intialized)
			return;
		
		_intialized = true;
		NoDataImpl load;
		
		for (int index = 0; index < DATA_POOL_INITIAL_SIZE; ++index)
		{
			_ommIntPool.add(new OmmIntImpl());
			_ommUIntPool.add(new OmmUIntImpl());
			_ommFloatPool.add(new OmmFloatImpl());
			_ommDoublePool.add(new OmmDoubleImpl());
			_ommBufferPool.add(new OmmBufferImpl());
			_ommAsciiPool.add(new OmmAsciiImpl());
			_ommUtf8Pool.add(new OmmUtf8Impl());
			_ommRmtesPool.add(new OmmRmtesImpl());
			_ommRealPool.add(new OmmRealImpl());
			_ommDatePool.add(new OmmDateImpl());
			_ommTimePool.add(new OmmTimeImpl());
			_ommDateTimePool.add(new OmmDateTimeImpl());
			_ommQosPool.add(new OmmQosImpl());
			_ommStatePool.add(new OmmStateImpl());
			_ommEnumPool.add(new OmmEnumImpl());

			_ommArrayPool.add(new OmmArrayImpl(this));
			_fieldListPool.add(new FieldListImpl(this));
			_elementListPool.add(new ElementListImpl(this));
			_noDataPool.add(new NoDataImpl());
			_ommErrorPool.add(new OmmErrorImpl());

			_singleItemPool.add(new SingleItem<T>());
			_batchItemPool.add(new BatchItem<T>());
			_longObjectPool.add(new LongObject());
			_intObjectPool.add(new IntObject());
			_subItemPool.add(new SubItem<T>());
			_tunnelItemPool.add(new TunnelItem<T>());

			_timeoutEventPool.add(new TimeoutEvent(0, null));
			
			load = new NoDataImpl();
       	 	_noDataPool .updatePool(load);
			_fieldEntryPool.add(new FieldEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFieldEntry(), load));
			
			load = new NoDataImpl();
       	 	_noDataPool .updatePool(load);
			_elementEntryPool.add(new ElementEntryImpl(com.refinitiv.eta.codec.CodecFactory.createElementEntry(), load));
		}

		initByteBufferList();	
	}

	InitResult setObjectPoolsLimits(int dataTypeLimit,
									int complexTypeLimit,
									int msgTypeLimit,
									int sessionObjectsLimit,
									int etaObjectsLimit)
	{
		_globalInitLock.lock();
		try
		{
			if (_limitsSetFromConfig)
			{
				if (dataTypeLimit != _initDataTypePoolLimit
						|| complexTypeLimit != _initComplexTypePoolLimit
						|| msgTypeLimit != _initMsgTypePoolLimit
						|| sessionObjectsLimit != _initSessionObjectsPoolLimit
						|| etaObjectsLimit != _initEtaObjectsPoolsLimit)
				{
					return InitResult.FAILURE;
				}
				else
				{
					return InitResult.NO_CHANGE;
				}
			}

			_limitsSetFromConfig = true;

			_initDataTypePoolLimit = dataTypeLimit;
			_initComplexTypePoolLimit = complexTypeLimit;
			_initMsgTypePoolLimit = msgTypeLimit;
			_initSessionObjectsPoolLimit = sessionObjectsLimit;
			_initEtaObjectsPoolsLimit = etaObjectsLimit;

			setDataTypePoolsLimit(dataTypeLimit);
			setComplexTypePoolsLimit(complexTypeLimit);
			setMsgTypePoolsLimit(msgTypeLimit);
			setSessionObjectPoolLimit(sessionObjectsLimit);
			setEtaObjectsPoolsLimit(etaObjectsLimit);

			return InitResult.SUCCESS;
		}
		finally
		{
			_globalInitLock.unlock();
		}
	}

	void setEmaObjectPoolLimit(int limit, int dataType)
	{
		int newLimit = limit >= 0 ? limit : -1;
		if (dataType >= DataType.DataTypes.INT && dataType <= DataType.DataTypes.RMTES)
		{
			_dataTypePools[dataType - dataTypeOffset].setLimit(newLimit);
			if (dataType == DataType.DataTypes.ARRAY)
			{
				_arrayEntryPool.setLimit(newLimit * ENTRY_MULTIPLIER);
			}
		}
		else if (dataType >= DataType.DataTypes.NO_DATA && dataType <= DataType.DataTypes.JSON && dataType != DataType.DataTypes.MSG)
		{
			_complexTypePools[dataType - complexTypeOffset].setLimit(newLimit);
			if (_entryPoolsArray[dataType - complexTypeOffset] != null)
				_entryPoolsArray[dataType - complexTypeOffset].setLimit(newLimit * ENTRY_MULTIPLIER);
		}
		else if (dataType >= DataType.DataTypes.REQ_MSG && dataType <= DataType.DataTypes.GENERIC_MSG)
		{
			_msgTypePools[dataType - msgTypeOffset].setLimit(newLimit);
		}
		else if (dataType == DataType.DataTypes.ERROR)
		{
			_ommErrorPool.setLimit(newLimit);
		}
	}

	void setDataTypePoolsLimit(int limit)
	{
		_dataTypePoolLimit = limit >= 0 ? limit : -1;
		for (int i = DataType.DataTypes.INT; i <= DataType.DataTypes.RMTES; i++)
		{
			if (_dataTypePools[i - dataTypeOffset] != null) _dataTypePools[i - dataTypeOffset].setLimit(_dataTypePoolLimit);
		}
	}

	void setComplexTypePoolsLimit(int limit)
	{
		_complexTypePoolLimit = limit >= 0 ? limit : -1;
		for (int i = DataType.DataTypes.NO_DATA; i <=  DataType.DataTypes.JSON; i++)
		{
			if (i != DataType.DataTypes.MSG && _complexTypePools[i - complexTypeOffset] != null)
				_complexTypePools[i - complexTypeOffset].setLimit(_complexTypePoolLimit);
		}
		_ommErrorPool.setLimit(_complexTypePoolLimit);
	}

	void setMsgTypePoolsLimit(int limit)
	{
		_msgTypePoolLimit = limit >= 0 ? limit : -1;
		for (int i = DataType.DataTypes.REQ_MSG; i <=  DataType.DataTypes.GENERIC_MSG; i++)
		{
			if (_msgTypePools[i - msgTypeOffset] != null) _msgTypePools[i - msgTypeOffset].setLimit(_msgTypePoolLimit);
		}
	}

	int getEmaObjectPoolLimit(int dataType)
	{
		if (dataType >= DataType.DataTypes.INT && dataType <= DataType.DataTypes.RMTES)
		{
			return _dataTypePools[dataType - dataTypeOffset].getLimit();
		}
		else if (dataType >= DataType.DataTypes.NO_DATA && dataType <= DataType.DataTypes.JSON && dataType != DataType.DataTypes.MSG)
		{
			return _complexTypePools[dataType - complexTypeOffset].getLimit();
		}
		else if (dataType >= DataType.DataTypes.REQ_MSG && dataType <= DataType.DataTypes.GENERIC_MSG)
		{
			return _msgTypePools[dataType - msgTypeOffset].getLimit();
		}
		else if (dataType == DataType.DataTypes.ERROR)
		{
			return _ommErrorPool.getLimit();
		}
		else
		{
			return -1;
		}
	}

	int getEmaObjectDataTypePoolLimit()
	{
		return _dataTypePoolLimit;
	}

	int getEmaObjectComplexTypePoolLimit()
	{
		return _complexTypePoolLimit;
	}

	int getEmaObjectMsgTypePoolLimit()
	{
		return _msgTypePoolLimit;
	}

	void setEtaObjectsPoolsLimit(int limit)
	{
		_etaObjectsPoolsLimit = limit >= 0 ? limit : -1;
		_etaPools.forEach(pool -> {
			if (limit >= 0)
			{
				if (pool.size() > _etaObjectsPoolsLimit)
				{
					int diff = pool.size() - _etaObjectsPoolsLimit;
					for (int i = 0; i < diff; i++)
					{
						pool.remove();
					}
				}
			}
		});
	}

	int getEtaObjectsPoolsLimit()
	{
		return _etaObjectsPoolsLimit;
	}

	void setSessionObjectPoolLimit(int limit)
	{
		_sessionObjectsPoolLimit = limit >= 0 ? limit : -1;

		_singleItemPool.setLimit(_sessionObjectsPoolLimit);
		_batchItemPool.setLimit(_sessionObjectsPoolLimit);
		_longObjectPool.setLimit(_sessionObjectsPoolLimit);
		_intObjectPool.setLimit(_sessionObjectsPoolLimit);
		_subItemPool.setLimit(_sessionObjectsPoolLimit);
		_tunnelItemPool.setLimit(_sessionObjectsPoolLimit);
		_timeoutEventPool.setLimit(_sessionObjectsPoolLimit);
	}

	int getSessionObjectPoolLimit()
	{
		return _sessionObjectsPoolLimit;
	}

	int getEmaObjectPoolCount(int dataType)
	{
		LimitedVaPool pool = null;
		if (dataType >= DataType.DataTypes.INT && dataType <= DataType.DataTypes.RMTES)
		{
			pool = _dataTypePools[dataType - dataTypeOffset];
		}
		else if (dataType >= DataType.DataTypes.NO_DATA && dataType <= DataType.DataTypes.JSON && dataType != DataType.DataTypes.MSG)
		{
			pool = _complexTypePools[dataType - complexTypeOffset];
		}
		else if (dataType >= DataType.DataTypes.REQ_MSG && dataType <= DataType.DataTypes.GENERIC_MSG)
		{
			pool = _msgTypePools[dataType - msgTypeOffset];
		}
		else if (dataType == DataType.DataTypes.ERROR)
		{
			pool = _ommErrorPool;
		}

		if (pool != null) return pool.size();
		else return -1;
	}

	<T> void initialize(int dataType)
	{
		if (_intialized)
			return;

		_intialized = true;
		NoDataImpl load;

		for (int index = 0; index < DATA_POOL_INITIAL_SIZE; ++index)
		{
			switch (dataType)
			{
				case DataType.DataTypes.INT:
					_ommIntPool.add(new OmmIntImpl());
					break;
				case DataType.DataTypes.UINT:
					_ommUIntPool.add(new OmmUIntImpl());
					break;
				case DataType.DataTypes.FLOAT:
					_ommFloatPool.add(new OmmFloatImpl());
					break;
				case DataType.DataTypes.DOUBLE:
					_ommDoublePool.add(new OmmDoubleImpl());
					break;
				case DataType.DataTypes.BUFFER:
					_ommBufferPool.add(new OmmBufferImpl());
					break;
				case DataType.DataTypes.ASCII:
					_ommAsciiPool.add(new OmmAsciiImpl());
					break;
				case DataType.DataTypes.UTF8:
					_ommUtf8Pool.add(new OmmUtf8Impl());
					break;
				case DataType.DataTypes.RMTES:
					_ommRmtesPool.add(new OmmRmtesImpl());
					break;
				case DataType.DataTypes.REAL:
					_ommRealPool.add(new OmmRealImpl());
					break;
				case DataType.DataTypes.DATE:
					_ommDatePool.add(new OmmDateImpl());
					break;
				case DataType.DataTypes.TIME:
					_ommTimePool.add(new OmmTimeImpl());
					break;
				case DataType.DataTypes.DATETIME:
					_ommDateTimePool.add(new OmmDateTimeImpl());
					break;
				case DataType.DataTypes.QOS:
					_ommQosPool.add(new OmmQosImpl());
					break;
				case DataType.DataTypes.STATE:
					_ommStatePool.add(new OmmStateImpl());
					break;
				case DataType.DataTypes.ENUM:
					_ommEnumPool.add(new OmmEnumImpl());
					break;
				case DataType.DataTypes.ARRAY:
					_ommArrayPool.add(new OmmArrayImpl(this));
					break;
				case DataType.DataTypes.FIELD_LIST:
					_fieldListPool.add(new FieldListImpl(this));
					load = new NoDataImpl();
					_noDataPool .updatePool(load);
					_fieldEntryPool.add(new FieldEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFieldEntry(), load));
					break;
				case DataType.DataTypes.ELEMENT_LIST:
					_elementListPool.add(new ElementListImpl(this));
					load = new NoDataImpl();
					_noDataPool .updatePool(load);
					_elementEntryPool.add(new ElementEntryImpl(com.refinitiv.eta.codec.CodecFactory.createElementEntry(), load));
					break;
				case DataType.DataTypes.NO_DATA:
					_noDataPool.add(new NoDataImpl());
					break;
				case DataType.DataTypes.ERROR:
					_ommErrorPool.add(new OmmErrorImpl());
					break;
			}

			_singleItemPool.add(new SingleItem<T>());
			_batchItemPool.add(new BatchItem<T>());
			_longObjectPool.add(new LongObject());
			_intObjectPool.add(new IntObject());
			_subItemPool.add(new SubItem<T>());
			_tunnelItemPool.add(new TunnelItem<T>());

			_timeoutEventPool.add(new TimeoutEvent(0, null));

		}

		initByteBufferList();
	}

	LongObject createLongObject()
    {
		LongObject longObj = (LongObject)_longObjectPool.poll();
        if (longObj == null)
        {
        	longObj = new LongObject();
            _longObjectPool.updatePool(longObj);
            return longObj;
        }
        else
        	return longObj.clear();
    }
	
	IntObject createIntObject()
    {
		IntObject intObj = (IntObject)_intObjectPool.poll();
        if (intObj == null)
        {
        	intObj = new IntObject();
        	_intObjectPool.updatePool(intObj);
            return intObj;
        }
        else
        	return intObj.clear();
    }
	
	ByteBuffer acquireByteBuffer(int length)
	{
		int pos = length / DEFAULT_BYTE_BUFFER_SIZE;
		ByteBuffer retVal;

		if (_byteBufferLock != null)
			_byteBufferLock.lock();

		try
		{
			if (pos < MAX_NUM_BYTE_BUFFER)
			{
				if (!_byteBufferList[pos].isEmpty())
				{
					retVal = _byteBufferList[pos].remove(_byteBufferList[pos].size() - 1);
					retVal.clear();
					return retVal;
				}

				return ByteBuffer.allocate((pos + 1) * DEFAULT_BYTE_BUFFER_SIZE);
			} else
			{
				if (!_byteBufferList[MAX_NUM_BYTE_BUFFER].isEmpty())
				{
					int size = _byteBufferList[MAX_NUM_BYTE_BUFFER].size() - 1;
					for (int index = size; index >= 0; --index)
					{
						if (length < _byteBufferList[MAX_NUM_BYTE_BUFFER].get(index).capacity())
						{
							retVal = _byteBufferList[MAX_NUM_BYTE_BUFFER].remove(index);
							retVal.clear();
							return retVal;
						}
					}
				}

				return ByteBuffer.allocate(length);
			}
		}
		finally
		{
			if (_byteBufferLock != null)
				_byteBufferLock.unlock();
		}
	}

	void releaseByteBuffer(ByteBuffer buffer)
	{
		if (buffer == null)
			return;

		if (_byteBufferLock != null)
			_byteBufferLock.lock();

		try
		{
			int pos = buffer.capacity() / DEFAULT_BYTE_BUFFER_SIZE - 1;

			if (pos < MAX_NUM_BYTE_BUFFER)
				_byteBufferList[pos].add(buffer);
			else
				_byteBufferList[MAX_NUM_BYTE_BUFFER].add(buffer);
		}
		finally
		{
			if (_byteBufferLock != null)
				_byteBufferLock.unlock();
		}
	}
	
	@SuppressWarnings("unchecked")
	private void initByteBufferList()
	{
		_byteBufferList = new ArrayList[MAX_NUM_BYTE_BUFFER+1];
		for (int pos = 0; pos <= MAX_NUM_BYTE_BUFFER; ++pos)
		{
			_byteBufferList[pos] = new ArrayList<ByteBuffer>();
		}

		for (int pos = 0; pos < MAX_NUM_BYTE_BUFFER; ++pos)
		{
			int allocatedSize = (pos + 1) * DEFAULT_BYTE_BUFFER_SIZE;
			_byteBufferList[pos].add(ByteBuffer.allocate(allocatedSize));
			_byteBufferList[pos].add(ByteBuffer.allocate(allocatedSize));
		}

		_byteBufferList[MAX_NUM_BYTE_BUFFER].add(ByteBuffer.allocate(MAX_BYTE_BUFFER_CAPABILITY));
	}
}

