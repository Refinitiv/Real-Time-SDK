/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.List;

/**
 * Event provided to ReactorServiceEndpointCallback methods.
 * 
 * @see ReactorServiceEndpointEvent
 */
public class ReactorServiceEndpointEvent extends ReactorEvent 
{
	List <ReactorServiceEndpointInfo> _reactorServiceEndpointInfoList;
	Object _userSpecObject = null;
	
	ReactorServiceEndpointEvent()
    {
        super();
    }

	void serviceEndpointInfo(List <ReactorServiceEndpointInfo> endpointInfo)
	{
		_reactorServiceEndpointInfoList = endpointInfo;
	}
	
	/**
     * The list of service endpoint information associated with this event.
     * 
     * @return The list of ReactorServiceEndpointInfo
     */
	public List <ReactorServiceEndpointInfo> serviceEndpointInfo()
	{
		return _reactorServiceEndpointInfoList;
	}
	
	public Object userSpecObject()
	{
		return _userSpecObject;
	}
	
	@Override
	public void returnToPool()
	{
		_userSpecObject = null;
		_reactorServiceEndpointInfoList = null;
    	
		super.returnToPool();
	}
}
