package com.refinitiv.eta.valueadd.reactor;

/**
 * This class is used to get a service ID for a service name used by the converter library.
 *
 * @see ReactorServiceNameToIdCallback
 */
public class ReactorServiceNameToId
{
	private String serviceName;
	private int serviceId;
	
	ReactorServiceNameToId()
	{
		
	}
	
	void clear()
	{
		serviceName = null;
		serviceId = -1;
	}
	
	void serviceName(String serviceName)
	{
		this.serviceName = serviceName;
	}
	
	int serviceId()
	{
		return serviceId;
	}
	
	/**
	 * Gets a service name to convert to a service ID
	 * 
	 * @return the service name
	 */
	public String serviceName()
	{
		return serviceName;
	}
	
	/**
	 * Specifies a service ID for the service name
	 * 
	 * @param serviceId a service ID
	 */
	public void serviceId(int serviceId)
	{
		this.serviceId = serviceId;
	}
}
