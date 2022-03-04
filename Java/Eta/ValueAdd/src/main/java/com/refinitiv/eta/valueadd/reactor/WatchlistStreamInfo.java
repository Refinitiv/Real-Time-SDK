/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/** 
 * Information about the stream associated with a message.
 * Only used when a watchlist is enabled.
 */
public class WatchlistStreamInfo
{
    String _serviceName;
    Object _userSpecObject;
    
    /**
     * Name of service associated with the stream, if any.
     * 
     * @return the service name
     */
    public String serviceName()
    {
        return _serviceName;
    }
    
    /**
     * Name of service associated with the stream, if any.
     * 
     * @param service name
     */
    void serviceName(String serviceName)
    {
        _serviceName = serviceName;
    }

    /**
     * User-specified object given when the stream was opened.
     * 
     * @return the user-specified object
     */
    public Object userSpecObject()
    {
        return _userSpecObject;
    }
    
    /**
     * User-specified object given when the stream was opened.
     * 
     * @param user-specified object
     */
    void userSpecObject(Object userSpecObject)
    {
        _userSpecObject = userSpecObject;
    }
    
    /**
     * Clears this object for reuse.
     */
    void clear()
    {
        _serviceName = null;
        _userSpecObject = null;
    }
}

