/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;


/**
 * Event provided to ReactorAuthTokenEventCallback methods.
 * 
 * @see ReactorAuthTokenInfo
 */
public class ReactorAuthTokenEvent extends ReactorEvent
{
	ReactorAuthTokenInfo _reactorAuthTokenInfo;
	
	ReactorAuthTokenEvent()
    {
        super();
    }

    void reactorAuthTokenInfo(ReactorAuthTokenInfo reactorAuthTokenInfo)
    {
    	_reactorAuthTokenInfo = reactorAuthTokenInfo;
    }   
    
    /**
     * The Authorization Token Info associated with this message event.
     * 
     * @return ReactorAuthTokenInfo
     */
    public ReactorAuthTokenInfo reactorAuthTokenInfo()
    {
        return _reactorAuthTokenInfo;
    }
    
    @Override
    public void returnToPool()
    {
    	_reactorAuthTokenInfo = null;
    	
    	super.returnToPool();
    }
}