/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * An event that has occurred on a ReactorChannel.
 * 
 * @see ReactorEvent
 */
public class ReactorChannelEvent extends ReactorEvent
{
    int _eventType = 0;

    /**
     * The event type.
     * 
     * @see ReactorChannelEventTypes
     * 
     * @return the event type
     */
    public int eventType()
    {
        return _eventType;
    }

    void eventType(int eventType)
    {
        _eventType = eventType;
    }
    
    void clear()
    {
        super.clear();
        _eventType = 0;
    }
    
    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return super.toString() + ", "
                + (_reactorChannel == null ? "ReactorChannel null" : _reactorChannel.toString())
                + ", " + ReactorChannelEventTypes.toString(_eventType);
    }
}
