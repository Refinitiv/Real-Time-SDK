/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

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
