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

import com.thomsonreuters.ema.access.NoDataImpl;
import com.thomsonreuters.ema.access.FieldListImpl;
import com.thomsonreuters.ema.access.SingleItem;
import com.thomsonreuters.upa.valueadd.common.VaPool;

class EmaObjectManager
{
	private final static int DATA_POOL_INITIAL_SIZE = 5;
	private final static int DEFAULT_BYTE_BUFFER_SIZE = 5;
	private final static int MAX_NUM_BYTE_BUFFER = 5;
	private final static int MAX_BYTE_BUFFER_CAPABILITY = 2000;

	private List<ByteBuffer>[] _byteBufferList;
	private boolean _intialized;
	
	VaPool _ommIntPool = new VaPool(false);
	VaPool _ommUIntPool = new VaPool(false);
	VaPool _ommFloatPool = new VaPool(false);
	VaPool _ommDoublePool = new VaPool(false);
	VaPool _ommBufferPool = new VaPool(false);
	VaPool _ommAsciiPool = new VaPool(false);
	VaPool _ommUtf8Pool = new VaPool(false);
	VaPool _ommRmtesPool = new VaPool(false);
	VaPool _ommRealPool = new VaPool(false);
	VaPool _ommDatePool = new VaPool(false);
	VaPool _ommTimePool = new VaPool(false);
	VaPool _ommDateTimePool = new VaPool(false);
	VaPool _ommQosPool = new VaPool(false);
	VaPool _ommStatePool = new VaPool(false);
	VaPool _ommEnumPool = new VaPool(false);
	VaPool _ommArrayPool = new VaPool(false);
	VaPool _fieldListPool = new VaPool(false);
	VaPool _mapPool = new VaPool(false);
	VaPool _elementListPool = new VaPool(false);
	VaPool _filterListPool = new VaPool(false);
	VaPool _vectorPool = new VaPool(false);
	VaPool _seriesPool = new VaPool(false);
	VaPool _opaquePool = new VaPool(false);
	VaPool _ansiPagePool = new VaPool(false);
	VaPool _xmlPool = new VaPool(false);
	VaPool _reqMsgPool = new VaPool(false);
	VaPool _refreshMsgPool = new VaPool(false);
	VaPool _statusMsgPool = new VaPool(false);
	VaPool _updateMsgPool = new VaPool(false);
	VaPool _ackMsgPool = new VaPool(false);
	VaPool _postMsgPool = new VaPool(false);
	VaPool _genericMsgPool = new VaPool(false);
	VaPool _noDataPool = new VaPool(false);
	VaPool _ommErrorPool = new VaPool(false);

	VaPool _singleItemPool = new VaPool(false);
	VaPool _batchItemPool = new VaPool(false);
	VaPool _subItemPool = new VaPool(false);
	VaPool _dictionaryItemPool = new VaPool(false);
	VaPool _directoryItemPool = new VaPool(false);
	VaPool _loginItemPool = new VaPool(false);
	VaPool _longObjectPool = new VaPool(false);

	VaPool _timeoutEventPool = new VaPool(false);

	VaPool _fieldEntryPool = new VaPool(false);
	VaPool _elementEntryPool = new VaPool(false);
	VaPool _arrayEntryPool = new VaPool(false);
	VaPool _filterEntryPool = new VaPool(false);
	VaPool _mapEntryPool = new VaPool(false);
	VaPool _seriesEntryPool = new VaPool(false);
	VaPool _vectorEntryPool = new VaPool(false);
	VaPool _ommServiceIdIntegerPool = new VaPool(true);
	VaPool _streamInfoPool = new VaPool(true);
	
	<T> void initialize()
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
			
			_timeoutEventPool.add(new TimeoutEvent(0, null));
			
			load = new NoDataImpl();
       	 	_noDataPool .updatePool(load);
			_fieldEntryPool.add(new FieldEntryImpl(com.thomsonreuters.upa.codec.CodecFactory.createFieldEntry(), load));
			
			load = new NoDataImpl();
       	 	_noDataPool .updatePool(load);
			_elementEntryPool.add(new ElementEntryImpl(com.thomsonreuters.upa.codec.CodecFactory.createElementEntry(), load));
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
	
	ByteBuffer acquireByteBuffer(int length)
	{
		int pos = length / DEFAULT_BYTE_BUFFER_SIZE;
		ByteBuffer retVal;

		if (pos < MAX_NUM_BYTE_BUFFER)
		{
			if (!_byteBufferList[pos].isEmpty())
			{
				retVal = (ByteBuffer) (_byteBufferList[pos].remove(_byteBufferList[pos].size() - 1).clear());
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
						retVal = (ByteBuffer) (_byteBufferList[MAX_NUM_BYTE_BUFFER].remove(index).clear());
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
