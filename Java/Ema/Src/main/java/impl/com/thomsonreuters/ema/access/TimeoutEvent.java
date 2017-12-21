///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;
import com.thomsonreuters.upa.valueadd.common.VaNode;

interface TimeoutClient 
{
	void handleTimeoutEvent();
}

class TimeoutEvent extends VaNode
{
	private long _timeoutInNanoSec;
	private boolean _cancelled;
	private TimeoutClient _client;
	
	TimeoutEvent(long timeoutInNanoSec, TimeoutClient client)
	{
		_timeoutInNanoSec = timeoutInNanoSec + System.nanoTime();
		_client = client;
		_cancelled = false;
	}
	
	void timeoutInNanoSec(long timeoutInNanoSec, TimeoutClient client)
	{
		_timeoutInNanoSec = timeoutInNanoSec + System.nanoTime();
		_client = client;
		_cancelled = false;
	}
	
	long timeoutInNanoSerc()
	{
		return _timeoutInNanoSec;
	}
	
	boolean cancelled()
	{
		return _cancelled;
	}
	
	void cancel()
	{
		_cancelled = true;
	}
	
	TimeoutClient client()
	{
		return _client;
	}
	
	static <T> long userTimeOutExist(ConcurrentLinkedQueue<TimeoutEvent> timeoutEventQueue)
	{
		if (timeoutEventQueue.size() == 0)
			return -1;
		
		TimeoutEvent event;
		Iterator<TimeoutEvent> iter = timeoutEventQueue.iterator();
		 while (iter.hasNext())
		 {
        	event = iter.next();
        	if (event.cancelled())
        	{
        		iter.remove();
        		event.returnToPool();
        		continue;
        	}
           
        	long currentTime = System.nanoTime();
        	if ((currentTime - event.timeoutInNanoSerc()) >= 0)
				return 0;
            else  
            	return (event.timeoutInNanoSerc() - currentTime);
        }
        
        return -1;
	}
	
	static <T> void execute(ConcurrentLinkedQueue<TimeoutEvent> timeoutEventQueue)
	{
		if ( timeoutEventQueue.size() == 0)
			return; 
		
		TimeoutEvent event;
		Iterator<TimeoutEvent> iter = timeoutEventQueue.iterator();
		while (iter.hasNext())
        {
			event = iter.next();
	    	if (System.nanoTime() >= event.timeoutInNanoSerc())
	    	{
	    		 iter.remove();
	    		 
	        	if (!event.cancelled() && event.client() != null)
	        		event.client().handleTimeoutEvent();
	        		
	    		event.returnToPool();
	    	}
	    	else
	    		 return;
        }
	}
}