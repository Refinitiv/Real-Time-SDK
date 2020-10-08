///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

public class ServiceEndpointDiscoveryEventImpl implements ServiceEndpointDiscoveryEvent
{
	Object _closure;
	ServiceEndpointDiscovery _serviceEndpointDiscovery;
	

	@Override
	public Object closure()
	{
		return _closure;
	}

	@Override
	public ServiceEndpointDiscovery serviceEndpointDiscovery()
	{
		return _serviceEndpointDiscovery;
	}

}
