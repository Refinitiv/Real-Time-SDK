/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * ReactorMsgEvent base class. Used by all other message event classes.
 * 
 * @see ReactorEvent
 */
public class ReactorMsgEvent extends ReactorEvent
{
    TransportBuffer _transportBuffer = null;
    Msg _msg = null;
    WatchlistStreamInfo _streamInfo = new WatchlistStreamInfo();

    void transportBuffer(TransportBuffer transportBuffer)
    {
        _transportBuffer = transportBuffer;
    }

    /**
     * The TransportBuffer associated with this message event.
     * 
     * @return TransportBuffer
     */
    public TransportBuffer transportBuffer()
    {
        return _transportBuffer;
    }

    void msg(Msg msg)
    {
        _msg = msg;
    }

    /**
     * The Msg associated with this message event.
     * 
     * @return Msg
     */
    public Msg msg()
    {
        return _msg;
    }

    /**
     * The WatchlistStreamInfo associated with this message event.
     * Only used when a watchlist is enabled.
     * 
     * @return WatchlistStreamInfo
     */
    public WatchlistStreamInfo streamInfo()
    {
        return _streamInfo;
    }

    /**
     * Clears {@link ReactorMsgEvent}.
     */
    public void clear()
    {
        super.clear();
        _transportBuffer = null;
        _msg = null;
        _streamInfo.clear();
    }
    
    @Override
    public void returnToPool()
    {
    	/* Clears user-specified object given when the stream was opened by users.*/
    	_streamInfo.clear();
    	
    	_transportBuffer = null;
        _msg = null;
    	
    	super.returnToPool();
    }

    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return super.toString() + ", " + (_transportBuffer != null ? "TransportBuffer present" : "TransportBuffer null") 
                + ", " + (_msg != null ? "Msg present" : "Msg null");
    }
}
