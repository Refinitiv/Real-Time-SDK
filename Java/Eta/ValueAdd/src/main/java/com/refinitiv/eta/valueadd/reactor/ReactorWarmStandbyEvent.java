/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;
import com.refinitiv.eta.valueadd.common.VaDoubleLinkList.Link;

class ReactorWarmStandbyEvent extends ReactorEvent
{
	ReactorEvent eventBase;
	int eventType;
	ReactorChannel reactorChannel;
	ReactorChannel nextReactorChannel;
	int serviceID; /* This is used for per service based warm standby. */
	int streamID; /* This is used for per service based warm standby. */
	ReactorErrorInfo reactorErrorInfo; /* This is used to covey error message if any*/
	ReactorWarmStandbyHandler pReactorWarmStandByHandlerImpl; /* Keeps the ReactorWarmStandByHandlerImpl for returning it back to the pool */
	ReactorWSBService wsbService;
	VaDoubleLinkList<ReactorWarmStandbyEvent> pool;
	
	private ReactorWarmStandbyEvent _reactorChannelNext, _reactorChannelPrev;
    static class ReactorWarmStandbyEventLink implements Link<ReactorWarmStandbyEvent>
    {
        public ReactorWarmStandbyEvent getPrev(ReactorWarmStandbyEvent thisPrev) { return thisPrev._reactorChannelPrev; }
        public void setPrev(ReactorWarmStandbyEvent thisPrev, ReactorWarmStandbyEvent thatPrev) { thisPrev._reactorChannelPrev = thatPrev; }
        public ReactorWarmStandbyEvent getNext(ReactorWarmStandbyEvent thisNext) { return thisNext._reactorChannelNext; }
        public void setNext(ReactorWarmStandbyEvent thisNext, ReactorWarmStandbyEvent thatNext) { thisNext._reactorChannelNext = thatNext; }
    }
	
	static final ReactorWarmStandbyEventLink BIG_BUFFER_LINK = new ReactorWarmStandbyEventLink();

	public ReactorWarmStandbyEvent(VaDoubleLinkList<ReactorWarmStandbyEvent> pool)
	{
		this.pool = pool;
		clear();
	}

	public void clear() {
		if (eventBase != null)
			eventBase.clear();
		eventType = ReactorWarmStandbyEventTypes.INIT;
		reactorChannel = null;
		nextReactorChannel = null;
		serviceID = 0;
		streamID = 0;
		if (reactorErrorInfo != null)
			reactorErrorInfo.clear();
		pReactorWarmStandByHandlerImpl = null;
		wsbService = null;
		
	}

	public VaDoubleLinkList<ReactorWarmStandbyEvent> pool() 
	{
		return pool;
	}
	
	public int warmStandbyEventType()
	{
		return eventType;
	}
}
