/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;

class ReactorWarmStandbyEventPool
{
	final int NUM_POOLS = 32;
	@SuppressWarnings("unchecked")
	VaDoubleLinkList<ReactorWarmStandbyEvent>[] _pools = new VaDoubleLinkList[NUM_POOLS];
    int _maxPool;
    int _maxNumEvents;
    int _currentNumEvents;

    ReactorWarmStandbyEventPool(int numBuffers)
	{
        _maxNumEvents = numBuffers;
	}
	
    ReactorWarmStandbyEvent getEvent(ReactorErrorInfo errorInfo)
	{
    	ReactorWarmStandbyEvent event = null;
		
		if (_currentNumEvents < _maxNumEvents)
		{
			_currentNumEvents++;
	        int poolIndex = 0;
	        // Traverse through pools to find available buffer
	        for (int i = poolIndex; i <= _maxPool; i++)
	        {
	            if (_pools[i] != null)
	            {
	                event = _pools[i].pop(ReactorWarmStandbyEvent.BIG_BUFFER_LINK);
	                if (event != null)
	                {
	                	event.clear();
	                    return event;
	                }
	            }
	        }
	
	        // There was no available event, create new
	        // First check if the pool exist
	        if (_pools[poolIndex] == null)
	        {
	        	VaDoubleLinkList<ReactorWarmStandbyEvent> pool = new VaDoubleLinkList<ReactorWarmStandbyEvent>();
	            _pools[poolIndex] = pool;
	        }
	        return new ReactorWarmStandbyEvent(_pools[poolIndex]);
		}
		else // max number of events reached, return NULL event
		{
			if (errorInfo != null)
			{
		        errorInfo.clear();
		        errorInfo.code(ReactorReturnCodes.FAILURE).location("ReactorWarmStandbyEventPool.getEvent");
		        errorInfo.error().errorId(ReactorReturnCodes.FAILURE);
		        errorInfo.error().text("Max number of event limit reached");
			}
			return event;
		}
	}
	
	void releaseEvent(ReactorWarmStandbyEvent event)
	{
		_currentNumEvents--;
		event.pool().pushBack(event, ReactorWarmStandbyEvent.BIG_BUFFER_LINK);
	}

	int getBuffersUsed(){
		return _currentNumEvents;
	}
}

