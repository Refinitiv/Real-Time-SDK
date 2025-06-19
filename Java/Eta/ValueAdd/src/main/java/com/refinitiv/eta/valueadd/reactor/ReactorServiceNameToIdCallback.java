/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The service name to Id callback is used to convert from a service name to service ID for the converter library.
 *
 * @see ReactorJsonConverterOptions
 * @see ReactorServiceNameToIdEvent
 * @see ReactorServiceNameToId
 */
public interface ReactorServiceNameToIdCallback
{
	/**
	 * A callback function that the {@link Reactor} will use to convert a service name to a service Id.
	 * 
	 * @param serviceNameToId this is used to convert the service name.
	 * @param serviceNameToIdEvent containing event information. ReactorServiceNameToIdEvent is valid only during callback.
	 * 
	 * @return {@link ReactorReturnCodes#SUCCESS} if a matching ID was found, {@link ReactorReturnCodes#FAILURE} otherwise.
	 */
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId, ReactorServiceNameToIdEvent serviceNameToIdEvent);
}
