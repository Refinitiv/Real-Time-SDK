package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

/**
 * Configuration options for selecting a list of services to support the per service based warm standby feature in {@link ReactorWarmStandbyServerInfo}.
 */
public class ReactorPerServiceBasedOptions {
	private List<Buffer> serviceNameList; 			// List of service names to provide active services for a warm standby server.

	/*
	 * Constructor
	 */
	ReactorPerServiceBasedOptions()
	{	
		clear();
	}
	
	void clear()
	{
		if (serviceNameList == null)
		{
			serviceNameList = new LinkedList<Buffer>();
		}
		serviceNameList.clear();
	}
	
	/**
	 * Deep copies the input service name list to this object's serviceNameList.
	 * @param serviceNameList input list of buffers to be copied
	 */
	public void serviceNameList(List<Buffer> serviceNameList)
	{
		for (int i = 0; i < serviceNameList.size(); ++i)
		{
			Buffer newBuffer = CodecFactory.createBuffer();
			newBuffer.data(ByteBuffer.allocate(serviceNameList.get(i).length()));
			serviceNameList.get(i).copy(newBuffer);
			serviceNameList.add(newBuffer);
		}
	}

	/**
	 * Retrieves the service name list for this connection.
	 * This list will be used on service-based connections to determine which service is preferred for this connection.
	 * If another connection in the warm standby group provides this service, that connection will be registered as a 
	 * standby for that service. 
	 * @param serviceNameList input list of buffers to be copied
	 */
	public List<Buffer> serviceNameList()
	{
		return serviceNameList;
	}

	/**
	 * Deep copies the full object.
	 * @param perServiceBasedOptions input ReactorPerServiceBasedOptions to be copied
	 */
	public void copy(ReactorPerServiceBasedOptions perServiceBasedOptions) {
		perServiceBasedOptions.clear();
		for (int i = 0; i < serviceNameList.size(); ++i)
		{
			Buffer newBuffer = CodecFactory.createBuffer();
			newBuffer.data(ByteBuffer.allocate(serviceNameList.get(i).length()));
			serviceNameList.get(i).copy(newBuffer);
			perServiceBasedOptions.serviceNameList.add(newBuffer);
		}
	}
}
