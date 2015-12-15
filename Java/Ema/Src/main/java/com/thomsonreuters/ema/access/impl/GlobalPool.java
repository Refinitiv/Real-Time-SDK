///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.impl.RefreshMsgImpl;
import com.thomsonreuters.ema.access.impl.NoDataImpl;
import com.thomsonreuters.ema.access.impl.FieldListImpl;
import com.thomsonreuters.ema.access.impl.SingleItem;
import com.thomsonreuters.upa.valueadd.common.VaPool;

class GlobalPool
{
	private final static int DATA_POOL_INITIAL_SIZE = 2;
	private final static int DEFAULT_BYTE_BUFFER_SIZE = 25;
	private final static int MAX_NUM_BYTE_BUFFER = 10;
	private final static int MAX_BYTE_BUFFER_CAPABILITY = 5000;

	public static VaPool _ommIntPool = new VaPool(true);
	public static VaPool _ommUIntPool = new VaPool(true);
	public static VaPool _ommFloatPool = new VaPool(true);
	public static VaPool _ommDoublePool = new VaPool(true);
	public static VaPool _ommBufferPool = new VaPool(true);
	public static VaPool _ommAsciiPool = new VaPool(true);
	public static VaPool _ommUtf8Pool = new VaPool(true);
	public static VaPool _ommRmtesPool = new VaPool(true);
	public static VaPool _ommRealPool = new VaPool(true);
	public static VaPool _ommDatePool = new VaPool(true);
	public static VaPool _ommTimePool = new VaPool(true);
	public static VaPool _ommDateTimePool = new VaPool(true);
	public static VaPool _ommQosPool = new VaPool(true);
	public static VaPool _ommStatePool = new VaPool(true);
	public static VaPool _ommEnumPool = new VaPool(true);
	public static VaPool _ommArrayPool = new VaPool(true);
	public static VaPool _fieldListPool = new VaPool(true);
	public static VaPool _mapPool = new VaPool(true);
	public static VaPool _elementListPool = new VaPool(true);
	public static VaPool _filterListPool = new VaPool(true);
	public static VaPool _vectorPool = new VaPool(true);
	public static VaPool _seriesPool = new VaPool(true);
	public static VaPool _opaquePool = new VaPool(true);
	public static VaPool _ansiPagePool = new VaPool(true);
	public static VaPool _xmlPool = new VaPool(true);
	public static VaPool _reqMsgPool = new VaPool(true);
	public static VaPool _refreshMsgPool = new VaPool(true);
	public static VaPool _statusMsgPool = new VaPool(true);
	public static VaPool _updateMsgPool = new VaPool(true);
	public static VaPool _ackMsgPool = new VaPool(true);
	public static VaPool _postMsgPool = new VaPool(true);
	public static VaPool _genericMsgPool = new VaPool(true);
	public static VaPool _noDataPool = new VaPool(true);
	public static VaPool _ommErrorPool = new VaPool(true);
	
	public static VaPool _singleItemPool = new VaPool(true);
	public static VaPool _batchItemPool = new VaPool(true);
	public static VaPool _subItemPool = new VaPool(true);
	public static VaPool _dictionaryItemPool = new VaPool(true);
	public static VaPool _directoryItemPool = new VaPool(true);
	public static VaPool _loginItemPool = new VaPool(true);
	
	public static VaPool _ommInvalidUsageExceptionPool = new VaPool(true);
	public static VaPool _ommInvalidConfigurationExceptionPool = new VaPool(true);
	public static VaPool _ommSystemExceptionPool = new VaPool(true);
	public static VaPool _ommOutOfRangeExceptionPool = new VaPool(true);
	public static VaPool _ommInvalidHandleExceptionPool = new VaPool(true);
	public static VaPool _ommMemoryExhaustionExceptionPool = new VaPool(true);
	public static VaPool _ommInaccessibleLogFileExceptionPool = new VaPool(true);
	public static VaPool _ommUnsupportedDomainTypeExceptionPool = new VaPool(true);
	public static VaPool _timeoutEventPool = new VaPool(true);

	public static List<com.thomsonreuters.upa.codec.LocalFieldSetDefDb> _rsslFieldListSetDefList;
	public static List<com.thomsonreuters.upa.codec.LocalElementSetDefDb> _rsslElementListSetDefList;
	private static List<ByteBuffer>[] _byteBufferList;
	private static boolean _intialized;
	private static ReentrantLock _globalLock = new java.util.concurrent.locks.ReentrantLock();

	private GlobalPool()
	{
		throw new AssertionError();
	}

	public static void intialize() 
	{
		if (_intialized)
			return;
		
		_intialized = true;
		
		for (int index = 0; index < DATA_POOL_INITIAL_SIZE; ++index)
		{
			_ommIntPool.add(new OmmIntImpl());;
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
			_ommArrayPool.add(new OmmArrayImpl());
			_fieldListPool.add(new FieldListImpl());
			_mapPool.add(new MapImpl());
			_elementListPool.add(new ElementListImpl());
			_filterListPool.add(new FilterListImpl());
			_vectorPool.add(new VectorImpl());
			_seriesPool.add(new SeriesImpl());
			_opaquePool.add(new OmmOpaqueImpl());
			_ansiPagePool.add(new OmmAnsiPageImpl());
			_xmlPool.add(new OmmXmlImpl());
			_reqMsgPool.add(new ReqMsgImpl());
			_refreshMsgPool.add(new RefreshMsgImpl());
			_statusMsgPool.add(new StatusMsgImpl());
			_updateMsgPool.add(new UpdateMsgImpl());
			_ackMsgPool.add(new AckMsgImpl());
			_postMsgPool.add(new PostMsgImpl());
			_genericMsgPool.add(new GenericMsgImpl());
			_noDataPool.add(new NoDataImpl());
			_ommErrorPool.add(new OmmErrorImpl());
			
			_singleItemPool.add(new SingleItem());
			_batchItemPool.add(new BatchItem());

			//TODO add subItemPool and OmmException
		}

		initByteBufferList();	
			
		_rsslFieldListSetDefList = new ArrayList<com.thomsonreuters.upa.codec.LocalFieldSetDefDb>();
		_rsslElementListSetDefList = new ArrayList<com.thomsonreuters.upa.codec.LocalElementSetDefDb>();
	}

	@SuppressWarnings("unchecked")
	private static void initByteBufferList()
	{
		_byteBufferList = new ArrayList[11];
		for (int pos = 0; pos <= MAX_NUM_BYTE_BUFFER; ++pos)
		{
			_byteBufferList[pos] = new ArrayList<ByteBuffer>();
		}
		
		for (int pos = 0; pos < MAX_NUM_BYTE_BUFFER; ++pos)
		{
			int allocatedSize = (pos+1) * DEFAULT_BYTE_BUFFER_SIZE;
			_byteBufferList[pos].add(ByteBuffer.allocate(allocatedSize));
			_byteBufferList[pos].add(ByteBuffer.allocate(allocatedSize));
		}
		
		_byteBufferList[MAX_NUM_BYTE_BUFFER].add(ByteBuffer.allocate(MAX_BYTE_BUFFER_CAPABILITY));
	}
	
	public static ByteBuffer acquireByteBuffer(int length)
	{
    	int pos = length/DEFAULT_BYTE_BUFFER_SIZE;
    	
    	if (pos <  MAX_NUM_BYTE_BUFFER)
    	{
    		_globalLock.lock();
    		
    		if (!_byteBufferList[pos].isEmpty())
    			return (ByteBuffer)(_byteBufferList[pos].remove(_byteBufferList[pos].size()-1).clear());

    		_globalLock.unlock();
    		
    		return ByteBuffer.allocate((pos+1) * DEFAULT_BYTE_BUFFER_SIZE);
    	}
    	else 
    	{
    		_globalLock.lock();
    		
    		if (!_byteBufferList[MAX_NUM_BYTE_BUFFER].isEmpty())
    		{
	    		int size = _byteBufferList[MAX_NUM_BYTE_BUFFER].size();
	    		for (int index = 0; index < size; ++index)
	    		{
	    			if (length < _byteBufferList[MAX_NUM_BYTE_BUFFER].get(index).capacity())
	    				return (ByteBuffer)(_byteBufferList[MAX_NUM_BYTE_BUFFER].remove(index).clear());
	    		}
    		}
    		
    		_globalLock.unlock();
    		
    		return ByteBuffer.allocate(length);
    	}
	}
	
	public static void releaseByteBuffer(ByteBuffer buffer)
    {
		if (buffer == null)
			return;
		
		int pos = buffer.capacity()/DEFAULT_BYTE_BUFFER_SIZE;
		
		_globalLock.lock();
		
		if (pos <  MAX_NUM_BYTE_BUFFER)
			_byteBufferList[pos].add(buffer);
		else
			_byteBufferList[MAX_NUM_BYTE_BUFFER].add(buffer);
		
		_globalLock.unlock();
    }
}
