/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;

public class TunnelStreamBigBufferPool
{
	final int NUM_POOLS = 32;
	@SuppressWarnings("unchecked")
	VaDoubleLinkList<TunnelStreamBigBuffer>[] _pools = new VaDoubleLinkList[NUM_POOLS];
    int _maxSize;
    int _maxPool;
    int _fragmentSize;
    int _maxNumBuffers;
    int _currentNumBuffers;

    TunnelStreamBigBufferPool(int fragmentSize, int numBuffers)
	{
        // this pool should be created after the tunnel stream fragment size is known
        _fragmentSize = fragmentSize;
        _maxSize = fragmentSize * 2;
        _maxNumBuffers = numBuffers;
	}
	
	TunnelStreamBigBuffer getBuffer(int size, ReactorErrorInfo errorInfo)
	{
		TunnelStreamBigBuffer buffer = null;
		
		if (_currentNumBuffers < _maxNumBuffers)
		{
			_currentNumBuffers++;
	        // determine which pool to use
	        int poolSize = _fragmentSize * 2;
	        int poolIndex = 0;
	        while (size > poolSize)
	        {
	            poolSize = poolSize * 2;
	            poolIndex++;
	        }
	        if (poolSize > _maxSize)
	        {
	            _maxSize = poolSize;
	            _maxPool = poolIndex;
	            // create pool and a buffer of this size
	            VaDoubleLinkList<TunnelStreamBigBuffer> pool = new VaDoubleLinkList<TunnelStreamBigBuffer>();
	            _pools[poolIndex] = pool;
	            return new TunnelStreamBigBuffer(pool, poolSize, size);
	        }
	
	        // The size is smaller then max, so traverse through pools to find
	        // available buffer
	        for (int i = poolIndex; i <= _maxPool; i++)
	        {
	            if (_pools[i] != null)
	            {
	                buffer = _pools[i].pop(TunnelStreamBigBuffer.BIG_BUFFER_LINK);
	                if (buffer != null)
	                {
	                	buffer.clear(size);
	                    return buffer;
	                }
	            }
	        }
	
	        // There was no available buffer, create new
	        // First check if the pool exist
	        if (_pools[poolIndex] == null)
	        {
	        	VaDoubleLinkList<TunnelStreamBigBuffer> pool = new VaDoubleLinkList<TunnelStreamBigBuffer>();
	            _pools[poolIndex] = pool;
	        }
	        return new TunnelStreamBigBuffer(_pools[poolIndex], poolSize, size);
		}
		else // max number of big buffers reached, return NULL buffer
		{
	        errorInfo.clear();
	        errorInfo.code(ReactorReturnCodes.FAILURE).location("TunnelStreamBigBufferPool.getBuffer");
	        errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
	        errorInfo.error().text("Max number of big buffers limit reached");

			return buffer;
		}
	}
	
	void releaseBuffer(TunnelStreamBigBuffer buffer)
	{
		_currentNumBuffers--;
		buffer.pool().pushBack(buffer, TunnelStreamBigBuffer.BIG_BUFFER_LINK);
	}

	int getBuffersUsed(){
		return _currentNumBuffers;
	}
}
