/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

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