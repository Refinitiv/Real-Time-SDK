package com.refinitiv.eta.valueadd.reactor;

import java.util.Objects;

import com.refinitiv.eta.json.converter.JsonConverterError;
import com.refinitiv.eta.json.converter.ServiceNameIdConverter;

class ServiceNameIdConverterClient implements ServiceNameIdConverter
{	
	private Reactor reactor;
	private ReactorServiceNameToId serviceNameToId;
	private ReactorServiceNameToIdEvent serviceNameToIdEvent;
	private ReactorChannel reactorChannel;
	
	ServiceNameIdConverterClient(Reactor reactor)
	{
		this.reactor = reactor;
		serviceNameToId = new ReactorServiceNameToId();
		serviceNameToIdEvent = new ReactorServiceNameToIdEvent();
		serviceNameToIdEvent.userSpecObj = reactor.jsonConverterUserSpec;
	}
	
	void setReactorChannel(ReactorChannel reactorChannel)
	{
		this.reactorChannel = reactorChannel;
	}

	@Override
	public int serviceNameToId(String serviceName, JsonConverterError error)
	{
		if (Objects.nonNull(reactor.serviceNameToIdCallback))
		{
			serviceNameToId.clear();
			serviceNameToId.serviceName(serviceName);
			serviceNameToIdEvent.reactorChannel(reactorChannel);
			if ( reactor.serviceNameToIdCallback.reactorServiceNameToIdCallback(serviceNameToId, serviceNameToIdEvent) == ReactorReturnCodes.SUCCESS)
			{
				return serviceNameToId.serviceId();
			}
		}
		
		return -1; /* indicate not found */
	}

	@Override
	public String serviceIdToName(int id, JsonConverterError error) 
	{
		return null;
	}
}