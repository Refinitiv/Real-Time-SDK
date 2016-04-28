///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.RefreshMsgImpl;
import com.thomsonreuters.ema.access.NoDataImpl;
import com.thomsonreuters.ema.access.FieldListImpl;
import com.thomsonreuters.ema.access.SingleItem;
import com.thomsonreuters.upa.valueadd.common.VaPool;

class GlobalPool {
	private final static int DATA_POOL_INITIAL_SIZE = 3;
	private final static int DEFAULT_BYTE_BUFFER_SIZE = 25;
	private final static int MAX_NUM_BYTE_BUFFER = 10;
	private final static int MAX_BYTE_BUFFER_CAPABILITY = 2000;

	private static List<ByteBuffer>[] _byteBufferList;
	private static boolean _intialized;
	private static ReentrantLock _globalLock = new java.util.concurrent.locks.ReentrantLock();
	
	static VaPool _ommIntPool = new VaPool(true);
	static VaPool _ommUIntPool = new VaPool(true);
	static VaPool _ommFloatPool = new VaPool(true);
	static VaPool _ommDoublePool = new VaPool(true);
	static VaPool _ommBufferPool = new VaPool(true);
	static VaPool _ommAsciiPool = new VaPool(true);
	static VaPool _ommUtf8Pool = new VaPool(true);
	static VaPool _ommRmtesPool = new VaPool(true);
	static VaPool _ommRealPool = new VaPool(true);
	static VaPool _ommDatePool = new VaPool(true);
	static VaPool _ommTimePool = new VaPool(true);
	static VaPool _ommDateTimePool = new VaPool(true);
	static VaPool _ommQosPool = new VaPool(true);
	static VaPool _ommStatePool = new VaPool(true);
	static VaPool _ommEnumPool = new VaPool(true);
	static VaPool _ommArrayPool = new VaPool(true);
	static VaPool _fieldListPool = new VaPool(true);
	static VaPool _mapPool = new VaPool(true);
	static VaPool _elementListPool = new VaPool(true);
	static VaPool _filterListPool = new VaPool(true);
	static VaPool _vectorPool = new VaPool(true);
	static VaPool _seriesPool = new VaPool(true);
	static VaPool _opaquePool = new VaPool(true);
	static VaPool _ansiPagePool = new VaPool(true);
	static VaPool _xmlPool = new VaPool(true);
	static VaPool _reqMsgPool = new VaPool(true);
	static VaPool _refreshMsgPool = new VaPool(true);
	static VaPool _statusMsgPool = new VaPool(true);
	static VaPool _updateMsgPool = new VaPool(true);
	static VaPool _ackMsgPool = new VaPool(true);
	static VaPool _postMsgPool = new VaPool(true);
	static VaPool _genericMsgPool = new VaPool(true);
	static VaPool _noDataPool = new VaPool(true);
	static VaPool _ommErrorPool = new VaPool(true);
	
	static VaPool _singleItemPool = new VaPool(true);
	static VaPool _batchItemPool = new VaPool(true);
	static VaPool _subItemPool = new VaPool(true);
	static VaPool _dictionaryItemPool = new VaPool(true);
	static VaPool _directoryItemPool = new VaPool(true);
	static VaPool _loginItemPool = new VaPool(true);
	
	static VaPool _timeoutEventPool = new VaPool(true);

	static List<com.thomsonreuters.upa.codec.LocalFieldSetDefDb> _rsslFieldListSetDefList;
	static List<com.thomsonreuters.upa.codec.LocalElementSetDefDb> _rsslElementListSetDefList;
	
	private GlobalPool()
	{
		throw new AssertionError();
	}

	static void initialize() 
	{
		_globalLock.lock();
		
		if (_intialized)
		{
			_globalLock.unlock();
			return;
		}
		
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
			_ommArrayPool.add(new OmmArrayImpl(true));
			_fieldListPool.add(new FieldListImpl(true));
			_mapPool.add(new MapImpl(true));
			_elementListPool.add(new ElementListImpl(true));
			_filterListPool.add(new FilterListImpl(true));
			_vectorPool.add(new VectorImpl(true));
			_seriesPool.add(new SeriesImpl(true));
			_opaquePool.add(new OmmOpaqueImpl());
			_ansiPagePool.add(new OmmAnsiPageImpl());
			_xmlPool.add(new OmmXmlImpl());
			_reqMsgPool.add(new ReqMsgImpl(true));
			_refreshMsgPool.add(new RefreshMsgImpl(true));
			_statusMsgPool.add(new StatusMsgImpl(true));
			_updateMsgPool.add(new UpdateMsgImpl(true));
			_ackMsgPool.add(new AckMsgImpl(true));
			_postMsgPool.add(new PostMsgImpl(true));
			_genericMsgPool.add(new GenericMsgImpl(true));
			_noDataPool.add(new NoDataImpl());
			_ommErrorPool.add(new OmmErrorImpl());
			
			_singleItemPool.add(new SingleItem());
			_batchItemPool.add(new BatchItem());
			
			_timeoutEventPool.add(new TimeoutEvent(0, null));
		}

		initByteBufferList();	
			
		_rsslFieldListSetDefList = new ArrayList<com.thomsonreuters.upa.codec.LocalFieldSetDefDb>();
		_rsslElementListSetDefList = new ArrayList<com.thomsonreuters.upa.codec.LocalElementSetDefDb>();
		
		_globalLock.unlock();
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
	
	static ByteBuffer acquireByteBuffer(int length)
	{
    	int pos = length/DEFAULT_BYTE_BUFFER_SIZE;
    	ByteBuffer retVal;
    	
    	if (pos <  MAX_NUM_BYTE_BUFFER)
    	{
    		_globalLock.lock();
    		
    		if (!_byteBufferList[pos].isEmpty())
    		{
    			retVal = (ByteBuffer)(_byteBufferList[pos].remove(_byteBufferList[pos].size()-1).clear());
    			_globalLock.unlock();
    			return retVal;
    		}
    		_globalLock.unlock();
    		
    		return ByteBuffer.allocate((pos+1) * DEFAULT_BYTE_BUFFER_SIZE);
    	}
    	else 
    	{
    		_globalLock.lock();
    		
    		if (!_byteBufferList[MAX_NUM_BYTE_BUFFER].isEmpty())
    		{
	    		int size = _byteBufferList[MAX_NUM_BYTE_BUFFER].size() -1;
	    		for (int index = size; index >= 0; --index)
	    		{
	    			if (length < _byteBufferList[MAX_NUM_BYTE_BUFFER].get(index).capacity())
	    			{
	    				retVal = (ByteBuffer)(_byteBufferList[MAX_NUM_BYTE_BUFFER].remove(index).clear());
	    				_globalLock.unlock();
	    				return retVal;
	    			}
	    		}
    		}
    		
    		_globalLock.unlock();
    		
    		return ByteBuffer.allocate(length);
    	}
	}
	
	static void releaseByteBuffer(ByteBuffer buffer)
    {
		if (buffer == null)
			return;
		
		int pos = buffer.capacity()/DEFAULT_BYTE_BUFFER_SIZE - 1;
		
		_globalLock.lock();
		
		if (pos <  MAX_NUM_BYTE_BUFFER)
			_byteBufferList[pos].add(buffer);
		else
			_byteBufferList[MAX_NUM_BYTE_BUFFER].add(buffer);
		
		_globalLock.unlock();
    }
	
}
