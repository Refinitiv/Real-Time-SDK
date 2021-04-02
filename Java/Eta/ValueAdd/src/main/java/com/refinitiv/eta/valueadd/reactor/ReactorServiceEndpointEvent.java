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
