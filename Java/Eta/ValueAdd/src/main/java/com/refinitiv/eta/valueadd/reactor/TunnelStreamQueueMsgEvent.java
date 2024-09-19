/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsg;

/**
 * Event provided to QueueMsgCallback methods.
 * 
 * @see TunnelStream
 * @see ReactorMsgEvent
 */
public class TunnelStreamQueueMsgEvent extends ReactorMsgEvent
{
    TunnelStream _tunnelStream;
    QueueMsg _queueMsg;

    void tunnelStream(TunnelStream tunnelStream)
    {
        _tunnelStream = tunnelStream;
    }

    /**
     * The tunnel stream associated with this message event.
     * 
     * @return TunnelStream
     */
    public TunnelStream tunnelStream()
    {
        return _tunnelStream;
    }
    
    void queueMsg(QueueMsg queueMsg)
    {
        _queueMsg = queueMsg;
    }
    
    /**
     * The queue message associated with this message event. This is the primary
     * message for the message event. {@link TunnelStreamQueueMsgEvent#msg()} and
     * {@link TunnelStreamQueueMsgEvent#transportBuffer()} are secondary.
     * 
     * @return The queue message associated with this message event
     */
    public QueueMsg queueMsg()
    {
        return _queueMsg;
    }

    /**
     * Clears {@link TunnelStreamQueueMsgEvent}.
     */
    public void clear()
    {
        super.clear();
        _tunnelStream = null;
        _queueMsg = null;
    }
    
    @Override
    public void returnToPool()
    {
    	_tunnelStream = null;
        _queueMsg = null;
    	
    	super.returnToPool();
    }
}
