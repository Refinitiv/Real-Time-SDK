package com.thomsonreuters.upa.valueadd.reactor;

import java.util.List;

/**
 * Event provided to ReactorServiceEndpointCallback methods.
 * 
 * @see ReactorServiceEndpointEvent
 */
public class ReactorServiceEndpointEvent extends WorkerEvent 
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
	
	public List <ReactorServiceEndpointInfo> serviceEndpointInfo()
	{
		return _reactorServiceEndpointInfoList;
	}
	
	public Object userSpecObject()
	{
		return _userSpecObject;
	}
	
}
