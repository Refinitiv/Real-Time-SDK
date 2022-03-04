/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;

/**
 * This event occurs when the Reactor fails to convert from JSON to RWF protocol.
 *
 */
public class ReactorJsonConversionEvent extends ReactorEvent 
{
	Object userSpec;
	Error error;
	
	ReactorJsonConversionEvent()
	{
		error = TransportFactory.createError();
	}
	
	void clear()
	{
		userSpec = null;
		error.clear();
	}
	
	
	/**
	 * Gets the information about the error that occurred with the JSON conversion.
	 * 
	 * @return The Error
	 */
	public Error error()
	{
		return error;
	}
	
	/**
	 * Returns the userSpecObj specified in {@link ReactorJsonConverterOptions}.
	 * 
	 * @return the user specified object.
	 */
	public Object userSpecObj()
	{
		return userSpec;
	}
}
