/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Event provided to TunnelStreamMsgCallback methods.
 * 
 * @see TunnelStream
 * @see ReactorMsgEvent
 */
public class TunnelStreamMsgEvent extends ReactorMsgEvent
{
    TunnelStream _tunnelStream;
    int _containerType;

    /**
     * Tunnel stream.
     *
     * @param tunnelStream the tunnel stream
     */
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
    
    /**
     * Container type.
     *
     * @param containerType the container type
     */
    void containerType(int containerType)
    {
        _containerType = containerType;
    }

    /**
     * Returns the container type associated with this message event's buffer.
     *
     * @return the int
     */
    public int containerType()
    {
        return _containerType;
    }
    
    /**
     * Clears {@link TunnelStreamMsgEvent}.
     */
    public void clear()
    {
        super.clear();
        _tunnelStream = null;
        _containerType = 0;
    }
    
    @Override
    public void returnToPool()
    {
    	_tunnelStream = null;
    	
    	super.returnToPool();
    }
}
