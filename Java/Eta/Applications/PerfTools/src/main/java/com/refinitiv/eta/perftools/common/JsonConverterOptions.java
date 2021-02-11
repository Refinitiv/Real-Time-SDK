package com.refinitiv.eta.perftools.common;

import java.util.Objects;

import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.json.converter.JsonConverterError;
import com.refinitiv.eta.json.converter.ServiceNameIdConverter;

public class JsonConverterOptions implements ServiceNameIdConverter
{
	private JsonServiceNameToIdCallback serviceNameToIdCallback;
	private DataDictionary datadictionary;
	private int defaultServiceId;
	private boolean jsonExpandedEnumFields;
	private boolean catchUnknownJsonKeys;
	private boolean catchUnknownJsonFids;
	private Object userClosure;
	
	public JsonConverterOptions()
	{
		clear();
	}
	
	public void clear()
	{
		serviceNameToIdCallback(null);
		defaultServiceId(1);
		jsonExpandedEnumFields(false);
		catchUnknownJsonKeys(true);
		catchUnknownJsonFids(true);
		userClosure(null);
	}

	public JsonServiceNameToIdCallback serviceNameToIdCallback() 
	{
		return serviceNameToIdCallback;
	}

	public void serviceNameToIdCallback(JsonServiceNameToIdCallback serviceNameToIdCallback)
	{
		this.serviceNameToIdCallback = serviceNameToIdCallback;
	}

	public DataDictionary datadictionary() 
	{
		return datadictionary;
	}

	public void datadictionary(DataDictionary datadictionary)
	{
		this.datadictionary = datadictionary;
	}

	public int defaultServiceId()
	{
		return defaultServiceId;
	}

	public void defaultServiceId(int defaultServiceId)
	{
		this.defaultServiceId = defaultServiceId;
	}

	public boolean jsonExpandedEnumFields()
	{
		return jsonExpandedEnumFields;
	}

	public void jsonExpandedEnumFields(boolean jsonExpandedEnumFields)
	{
		this.jsonExpandedEnumFields = jsonExpandedEnumFields;
	}

	public boolean catchUnknownJsonKeys()
	{
		return catchUnknownJsonKeys;
	}

	public void catchUnknownJsonKeys(boolean catchUnknownJsonKeys)
	{
		this.catchUnknownJsonKeys = catchUnknownJsonKeys;
	}

	public boolean catchUnknownJsonFids()
	{
		return catchUnknownJsonFids;
	}

	public void catchUnknownJsonFids(boolean catchUnknownJsonFids)
	{
		this.catchUnknownJsonFids = catchUnknownJsonFids;
	}

	public Object userClosure()
	{
		return userClosure;
	}

	public void userClosure(Object userClosure)
	{
		this.userClosure = userClosure;
	}

	@Override
	public int serviceNameToId(String serviceName, JsonConverterError error) {
		
		if(Objects.nonNull(serviceNameToIdCallback))
		{
			return serviceNameToIdCallback.serviceNameToIdCallback(serviceName, userClosure);
		}
		else
		{
			return -1;
		}
	}

	@Override
	public String serviceIdToName(int id, JsonConverterError error) {
		return null;
	}
}
