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
