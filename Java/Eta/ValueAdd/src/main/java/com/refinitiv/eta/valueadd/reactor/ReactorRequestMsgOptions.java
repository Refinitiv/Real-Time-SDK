/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Options to use when submitting a RequestMsg.
 * Only used when a watchlist is enabled.
 *
 * @see ReactorSubmitOptions
 */
public class ReactorRequestMsgOptions
{
    Object _userSpecObj;

    /**
     * User-specified object to return as the application receives events related to this request.
     * 
     * @return the user-specified object
     */
    public Object userSpecObj()
    {
        return _userSpecObj;
    }
    
    /**
     * User-specified object to return as the application receives events related to this request.
     *
     * @param userSpecObj the user spec obj
     */
    public void userSpecObj(Object userSpecObj)
    {
        _userSpecObj = userSpecObj;
    }
    
    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _userSpecObj = null;
    }
}



