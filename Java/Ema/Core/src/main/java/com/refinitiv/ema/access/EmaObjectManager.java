///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.ArrayDeque;

import com.refinitiv.eta.valueadd.common.VaPool;

class EmaObjectManager
{
	final static int DATA_POOL_INITIAL_SIZE = 5;
	private final static int DEFAULT_BYTE_BUFFER_SIZE = 5;
	private final static int MAX_NUM_BYTE_BUFFER = 5;
	private final static int MAX_BYTE_BUFFER_CAPABILITY = 2000;
	private final static int DEFAULT_ETA_CONTAINER_SIZE = 10;

	private List<ByteBuffer>[] _byteBufferList;
	private boolean _intialized;
	
	static EmaObjectManager GlobalObjectManager;
	
	static 
	{
		GlobalObjectManager = new EmaObjectManager(true);
		GlobalObjectManager.initialize(DATA_POOL_INITIAL_SIZE * 2);
	}
	
	VaPool _ommIntPool;
	VaPool _ommUIntPool;
	VaPool _ommFloatPool;
	VaPool _ommDoublePool;
	VaPool _ommBufferPool;
	VaPool _ommAsciiPool;
	VaPool _ommUtf8Pool;
	VaPool _ommRmtesPool;
	VaPool _ommRealPool;
	VaPool _ommDatePool;
	VaPool _ommTimePool;
	VaPool _ommDateTimePool;
	VaPool _ommQosPool;
	VaPool _ommStatePool;
	VaPool _ommEnumPool;
	VaPool _ommArrayPool;
	VaPool _fieldListPool;
	VaPool _mapPool;
	VaPool _elementListPool;
	VaPool _filterListPool;
	VaPool _vectorPool;
	VaPool _seriesPool;
	VaPool _opaquePool;
	VaPool _ansiPagePool;
	VaPool _xmlPool;
	VaPool _jsonPool;
	VaPool _reqMsgPool;
	VaPool _refreshMsgPool;
	VaPool _statusMsgPool;
	VaPool _updateMsgPool;
	VaPool _ackMsgPool;
	VaPool _postMsgPool;
	VaPool _genericMsgPool;
	VaPool _noDataPool;
	VaPool _ommErrorPool;

	VaPool _singleItemPool;
	VaPool _batchItemPool;
	VaPool _subItemPool;
	VaPool _tunnelItemPool;
	VaPool _dictionaryItemPool;
	VaPool _niproviderDictionaryItemPool;
	VaPool _iproviderDictionaryItemPool;
	VaPool _directoryItemPool;
	VaPool _loginItemPool;
	VaPool _longObjectPool;
	VaPool _intObjectPool;

	VaPool _timeoutEventPool;

	VaPool _fieldEntryPool;
	VaPool _elementEntryPool;
	VaPool _arrayEntryPool;
	VaPool _filterEntryPool;
	VaPool _mapEntryPool;
	VaPool _seriesEntryPool;
	VaPool _vectorEntryPool;
	VaPool _ommServiceIdIntegerPool;
	VaPool _streamInfoPool;
	
	Deque<com.refinitiv.eta.codec.ElementList> _rsslElementListPool;
	Deque<com.refinitiv.eta.codec.Vector> _rsslVectorPool;
	Deque<com.refinitiv.eta.codec.FieldList> _rsslFieldListPool;
	Deque<com.refinitiv.eta.codec.FilterList> _rsslFilterListPool;
	Deque<com.refinitiv.eta.codec.Map> _rsslMapPool;
	Deque<com.refinitiv.eta.codec.Series> _rsslSeriesPool;
	Deque<com.refinitiv.eta.codec.Array> _rsslArrayPool;
	
	Deque<com.refinitiv.eta.codec.DecodeIterator> _etaDecodeIteratorPool;
	
	EmaObjectManager()
	{
		this(false);
	}
	
	EmaObjectManager(boolean globalLock)
	{
		_ommIntPool = new VaPool(globalLock);
		_ommUIntPool = new VaPool(globalLock);
		_ommFloatPool = new VaPool(globalLock);
		_ommDoublePool = new VaPool(globalLock);
		_ommBufferPool = new VaPool(globalLock);
		_ommAsciiPool = new VaPool(globalLock);
		_ommUtf8Pool = new VaPool(globalLock);
		_ommRmtesPool = new VaPool(globalLock);
		_ommRealPool = new VaPool(globalLock);
		_ommDatePool = new VaPool(globalLock);
		_ommTimePool = new VaPool(globalLock);
		_ommDateTimePool = new VaPool(globalLock);
		_ommQosPool = new VaPool(globalLock);
		_ommStatePool = new VaPool(globalLock);
		_ommEnumPool = new VaPool(globalLock);
		_ommArrayPool = new VaPool(globalLock);
		_fieldListPool = new VaPool(globalLock);
		_mapPool = new VaPool(globalLock);
		_elementListPool = new VaPool(globalLock);
		_filterListPool = new VaPool(globalLock);
		_vectorPool = new VaPool(globalLock);
		_seriesPool = new VaPool(globalLock);
		_opaquePool = new VaPool(globalLock);
		_ansiPagePool = new VaPool(globalLock);
		_xmlPool = new VaPool(globalLock);
		_jsonPool = new VaPool(globalLock);
		_reqMsgPool = new VaPool(globalLock);
		_refreshMsgPool = new VaPool(globalLock);
		_statusMsgPool = new VaPool(globalLock);
		_updateMsgPool = new VaPool(globalLock);
		_ackMsgPool = new VaPool(globalLock);
		_postMsgPool = new VaPool(globalLock);
		_genericMsgPool = new VaPool(globalLock);
		_noDataPool = new VaPool(globalLock);
		_ommErrorPool = new VaPool(globalLock);

		_singleItemPool = new VaPool(globalLock);
		_batchItemPool = new VaPool(globalLock);
		_subItemPool = new VaPool(globalLock);
		_tunnelItemPool = new VaPool(globalLock);
		_dictionaryItemPool = new VaPool(globalLock);
		_niproviderDictionaryItemPool = new VaPool(globalLock);
		_iproviderDictionaryItemPool = new VaPool(globalLock);
		_directoryItemPool = new VaPool(globalLock);
		_loginItemPool = new VaPool(globalLock);
		_longObjectPool = new VaPool(globalLock);
		_intObjectPool = new VaPool(globalLock);

		_timeoutEventPool = new VaPool(globalLock);

		_fieldEntryPool = new VaPool(globalLock);
		_elementEntryPool = new VaPool(globalLock);
		_arrayEntryPool = new VaPool(globalLock);
		_filterEntryPool = new VaPool(globalLock);
		_mapEntryPool = new VaPool(globalLock);
		_seriesEntryPool = new VaPool(globalLock);
		_vectorEntryPool = new VaPool(globalLock);
		_ommServiceIdIntegerPool = new VaPool(true);
		_streamInfoPool = new VaPool(true);
		
		if(globalLock)
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

	void releaseByteBuffer(ByteBuffer buffer)
	{
		if (buffer == null)
			return;

		int pos = buffer.capacity() / DEFAULT_BYTE_BUFFER_SIZE - 1;

		if (pos < MAX_NUM_BYTE_BUFFER)
			_byteBufferList[pos].add(buffer);
		else
			_byteBufferList[MAX_NUM_BYTE_BUFFER].add(buffer);
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
