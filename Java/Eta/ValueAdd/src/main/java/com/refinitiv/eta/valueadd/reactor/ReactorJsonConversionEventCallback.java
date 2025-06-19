/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The JSON conversion event callback function.
 *
 * @see ReactorJsonConverterOptions
 */
public interface ReactorJsonConversionEventCallback
{

	/**
	 * A callback function that the {@link Reactor} will use to report failures when converting from JSON to RWF.
	 * 
	 * @param jsonConversionEvent containing conversion information.
	 * @return ReactorCallbackReturnCodes A callback return code that can
	 *         trigger specific Reactor behavior based on the outcome of the
	 *         callback function
	 */
	public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent);
}
