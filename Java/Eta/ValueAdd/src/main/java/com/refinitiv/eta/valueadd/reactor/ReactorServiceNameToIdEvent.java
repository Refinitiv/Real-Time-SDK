package com.refinitiv.eta.valueadd.reactor;

/**
 * This event occurs when the Reactor needs to convert from a service name to a service Id.
 * 
 * @see ReactorJsonConverterOptions
 */
public class ReactorServiceNameToIdEvent extends ReactorEvent
{
	Object userSpecObj = null;
	
	ReactorServiceNameToIdEvent()
	{
		super();
	}
	
	/**
	 * Returns the userSpecObj specified in {@link ReactorJsonConverterOptions}.
	 * 
	 * @return the user specified object.
	 */
	public Object userSpecObj()
	{
		return userSpecObj;
	}
}
