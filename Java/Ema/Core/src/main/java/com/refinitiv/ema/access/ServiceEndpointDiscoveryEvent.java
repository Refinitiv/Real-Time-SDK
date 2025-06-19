/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

/**
 * ServiceEndpointDiscoveryEvent encapsulates query identifiers.
 * 
 * <p>ServiceEndpointDiscoveryEvent is used to convey query identifiers to application.<br>
 * ServiceEndpointDiscoveryEvent is returned through ServiceEndpointDiscoveryClient callback methods.</p>
 * 
 * <p>ServiceEndpointDiscoveryEvent is read only. It is used for query identification only.</p>
 * 
 * @see ServiceEndpointDiscovery
 * @see ServiceEndpointDiscoveryClient
 */
public interface ServiceEndpointDiscoveryEvent
{
	/**
	 * Returns an identifier (a.k.a., closure) associated with a query by consumer application.<br>
	 * Application associates the closure with a query on 
	 * {@link com.refinitiv.ema.access.ServiceEndpointDiscovery#registerClient(ServiceEndpointDiscoveryOption, ServiceEndpointDiscoveryClient, Object)} method
	 * 
	 * @return closure value
	 */
	public Object closure();
	
	/**
	 * Returns ServiceEndpointDiscovery instance for this event.
	 * 
	 * @return reference to ServiceEndpointDiscovery
	 */
	public ServiceEndpointDiscovery serviceEndpointDiscovery();
}
