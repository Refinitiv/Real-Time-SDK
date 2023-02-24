/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.json.util.JsonFactory;

/**
 * ReactorJsonConverterOptions is used to initialize the JSON converter library 
 * in the call.
 */
public class ReactorJsonConverterOptions
{
	private DataDictionary dataDictionary;
	private Object userSpec;
	private int defaultServiceId;
	private ReactorServiceNameToIdCallback serviceNameToIdCallback;
	private ReactorJsonConversionEventCallback JsonConversionEventCallback;
	private boolean jsonExpandedEnumFields;
	private boolean catchUnknownJsonKeys;
	private boolean catchUnknownJsonFids;
	private boolean closeChannelFromFailure;
	private int jsonConverterPoolsSize = JsonFactory.JSON_CONVERTER_DEFAULT_POOLS_SIZE;
	private boolean sendJsonConvError;
	
	ReactorJsonConverterOptions()
	{
		clear();
	}
	
	/**
	 * Clears this object to default values for reuse.
	 */
	public void clear()
	{
		userSpec(null);
		defaultServiceId(-1);
		serviceNameToIdCallback(null);
		jsonConversionEventCallback(null);
		jsonExpandedEnumFields(false);
		catchUnknownJsonKeys(false);
		catchUnknownJsonFids(true);
		closeChannelFromFailure(true);
		jsonConverterPoolsSize = JsonFactory.JSON_CONVERTER_DEFAULT_POOLS_SIZE;
		sendJsonConvError(false);
	}
	
	/**
	 * Specifies the DataDicitonary to initialize the RWF/JSON converter library.
	 * 
	 * @param dictionary The DataDictionary
	 */
	public void dataDictionary(DataDictionary dictionary)
	{
		dataDictionary = dictionary;
	}
	
	/**
	 * Gets the DataDicitonary for initializing the RWF/JSON converter library.
	 * 
	 * @return The DataDictionary
	 */
	public DataDictionary dataDictionary()
	{
		return dataDictionary;
	}

	/**
	 * Gets the user-specified object which will be retrieved in the callback function.
	 * 
	 * @return The user-specified object
	 */
	public Object userSpec()
	{
		return userSpec;
	}

	/**
	 * Sets the user-specified object which will be retrieved in the callback function.
	 * 
	 * @param userSpec The user-specified object
	 */
	public void userSpec(Object userSpec)
	{
		this.userSpec = userSpec;
	}

	/**
	 * Gets a default service ID for a request if both service name and ID are not set.
	 * 
	 * @return a user specified default service ID
	 */
	public int defaultServiceId()
	{
		return defaultServiceId;
	}

	/**
	 * Specifies a default service ID for a request if both service name and ID are not set.<br>
	 * A service ID must be in between 0 and 65535.
	 * 
	 * @param defaultServiceId the default service ID
	 */
	public void defaultServiceId(int defaultServiceId)
	{
		this.defaultServiceId = defaultServiceId;
	}

	/**
	 * Returns the callback function that handles conversion from service name to ID.
	 * 
	 * @return The {@link ReactorServiceNameToIdCallback}
	 */
	public ReactorServiceNameToIdCallback serviceNameToIdCallback()
	{
		return serviceNameToIdCallback;
	}

	/**
	 * Specifies the callback function that handles conversion from service name to ID.
	 * 
	 * @param serviceNameToIdCallback The {@link ReactorServiceNameToIdCallback}
	 */
	public void serviceNameToIdCallback(ReactorServiceNameToIdCallback serviceNameToIdCallback)
	{
		this.serviceNameToIdCallback = serviceNameToIdCallback;
	}

	/**
	 * Returns the callback function that receives RsslReactorJsonConversionEvent when the JSON converter failed to convert message.
	 * 
	 * @return The {@link ReactorJsonConversionEventCallback}
	 */
	public ReactorJsonConversionEventCallback jsonConversionEventCallback()
	{
		return JsonConversionEventCallback;
	}

	/**
	 * Specifies the callback function that receives RsslReactorJsonConversionEvent when the JSON converter failed to convert message.
	 * 
	 * @param jsonConversionEventCallback The {@link ReactorJsonConversionEventCallback}
	 */
	public void jsonConversionEventCallback(ReactorJsonConversionEventCallback jsonConversionEventCallback)
	{
		JsonConversionEventCallback = jsonConversionEventCallback;
	}

	/**
	 * Checks whether to expand enumerated values in field entries to their display values for JSON protocol.
	 *  
	 * @return true to expand enumerated values; false otherwise.
	 */
	public boolean jsonExpandedEnumFields()
	{
		return jsonExpandedEnumFields;
	}

	/**
	 * Specifies true to expand enumerated values in field entries to their display values for JSON protocol.
	 * <p>Defaults to false.</p>
	 * 
	 * @param jsonExpandedEnumFields specifies true to expand enumerated values; false otherwise.
	 */
	public void jsonExpandedEnumFields(boolean jsonExpandedEnumFields)
	{
		this.jsonExpandedEnumFields = jsonExpandedEnumFields;
	}

	/**
	 * Checks whether to catch unknown JSON keys when converting from JSON to RWF.
	 * 
	 * @return true to catch unknown JSON keys; false otherwise.
	 */
	public boolean catchUnknownJsonKeys() 
	{
		return catchUnknownJsonKeys;
	}

	/**
	 * Specifies true to catch unknown JSON keys when converting from JSON to RWF.
	 * <p>Defaults to false.</p>
	 * 
	 * @param catchUnknownJsonKeys specifies true to catch unknown JSON keys; false otherwise.
	 */
	public void catchUnknownJsonKeys(boolean catchUnknownJsonKeys)
	{
		this.catchUnknownJsonKeys = catchUnknownJsonKeys;
	}

	/**
	 * Checks whether to catch unknown field IDs when converting from JSON to RWF.
	 * 
	 * @return true to catch unknown field IDs; false otherwise.
	 */
	public boolean catchUnknownJsonFids()
	{
		return catchUnknownJsonFids;
	}

	/**
	 * Specifies true to catch unknown field IDs when converting from JSON to RWF.
	 * <p>Defaults to true.</p>
	 * 
	 * @param catchUnknownJsonFids specifies true to catch unknown field IDs; false otherwise.
	 */
	public void catchUnknownJsonFids(boolean catchUnknownJsonFids)
	{
		this.catchUnknownJsonFids = catchUnknownJsonFids;
	}

	/**
	 * Checks whether to close the channel when the Reactor failed to parse JSON message or received JSON error message.
	 * 
	 * @return true to close the channel; false otherwise.
	 */
	public boolean closeChannelFromFailure()
	{
		return closeChannelFromFailure;
	}

	/**
	 * Checks whether to send parse JSON convertor error.
	 *
	 * @return true to send parse JSON error; false otherwise.
	 */
	public boolean sendJsonConvError()
	{
		return sendJsonConvError;
	}

	/**
	 * Specifies true to close the channel when the Reactor failed to parse JSON message or received JSON error message.
	 * <p>Defaults to true.</p>
	 * 
	 * @param closeChannelFromFailure specifies true to close the channel; false otherwise.
	 */
	public void closeChannelFromFailure(boolean closeChannelFromFailure)
	{
		this.closeChannelFromFailure = closeChannelFromFailure;
	}

	/**
	 * Returns the value of jsonConverterPoolsSize. If it's not set, the default (10) value will be used.
	 *
	 * @return the value of jsonConverterPoolsSize.
	 */
	public int jsonConverterPoolsSize()
	{
		return jsonConverterPoolsSize;
	}

	/**
	 * Specifies the value that will be used to set the jsonConverterPoolsSize.
	 *
	 * @param jsonConverterPoolsSize specifies the new value of jsonConverterPoolsSize.
	 */
	public void jsonConverterPoolsSize(int jsonConverterPoolsSize)
	{
		this.jsonConverterPoolsSize = jsonConverterPoolsSize;
	}

	/**
	 * Specifies the value that enable/disable sending parse JSON converter error.
	 *
	 * @param sendJsonConvError specifies the new value of sendJsonConvError.
	 */
	public void sendJsonConvError(boolean sendJsonConvError)
	{
		this.sendJsonConvError = sendJsonConvError;
	}
}
