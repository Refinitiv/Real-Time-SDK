///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

abstract class BaseConfig
{
	final static int DEFAULT_ITEM_COUNT_HINT					= 100000;
	final static int DEFAULT_SERVICE_COUNT_HINT				    = 513;
	final static int DEFAULT_REQUEST_TIMEOUT					= 15000;
	final static int DEFAULT_MAX_DISPATCH_COUNT_API_THREAD		= 100;
	final static int DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD	    = 100;
	final static int DEFAULT_DISPATCH_TIMEOUT_API_THREAD		= 0;
	final static int DEFAULT_USER_DISPATCH						= OmmConsumerConfig.OperationModel.API_DISPATCH;
	final static int DEFAULT_CONVERTER_SERVICE_ID 				= 1;
	final static boolean DEFAULT_JSON_ENUM_EXPAND_FIELDS		= false;
	final static boolean DEFAULT_CATCH_UNKNOWN_JSON_KEYS		= false;
	final static boolean DEFAULT_CATCH_UNKNOWN_JSON_FIDS		= true;
	final static boolean DEFAULT_CLOSE_CHANNEL_FROM_FAILURE		= true;
	final static boolean DEFAULT_SEND_JSON_CONV_ERROR			= false;

	BaseConfig()
	{
		itemCountHint = DEFAULT_ITEM_COUNT_HINT;
		serviceCountHint = DEFAULT_SERVICE_COUNT_HINT;
		requestTimeout = DEFAULT_REQUEST_TIMEOUT;
		dispatchTimeoutApiThread = DEFAULT_DISPATCH_TIMEOUT_API_THREAD;
		maxDispatchCountApiThread = DEFAULT_MAX_DISPATCH_COUNT_API_THREAD;
		maxDispatchCountUserThread = DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD;
		userDispatch = DEFAULT_USER_DISPATCH;
		xmlTraceEnable = ActiveConfig.DEFAULT_XML_TRACE_ENABLE;
		xmlTraceToFileEnable = ActiveConfig.DEFAULT_XML_TRACE_TO_FILE_ENABLE;
		xmlTraceMaxFileSize = ActiveConfig.DEFAULT_XML_TRACE_MAX_FILE_SIZE;
		xmlTraceFileName = ActiveConfig.DEFAULT_XML_TRACE_FILE_NAME;
		xmlTraceToMultipleFilesEnable = ActiveConfig.DEFAULT_XML_TRACE_TO_MULTIPLE_FILES;
		xmlTraceWriteEnable = ActiveConfig.DEFAULT_XML_TRACE_WRITE;
		xmlTraceReadEnable = ActiveConfig.DEFAULT_XML_TRACE_READ;
		xmlTracePingEnable = ActiveConfig.DEFAULT_XML_TRACE_PING;
		traceStr = new StringBuilder(500);
		globalConfig = new GlobalConfig();
		defaultConverterServiceId = DEFAULT_CONVERTER_SERVICE_ID;
		jsonExpandedEnumFields = DEFAULT_JSON_ENUM_EXPAND_FIELDS;
		catchUnknownJsonKeys = DEFAULT_CATCH_UNKNOWN_JSON_KEYS;
		catchUnknownJsonFids = DEFAULT_CATCH_UNKNOWN_JSON_FIDS;
		closeChannelFromFailure = DEFAULT_CLOSE_CHANNEL_FROM_FAILURE;
		sendJsonConvError = DEFAULT_SEND_JSON_CONV_ERROR;
	}
	
	void clear()
	{
		itemCountHint = DEFAULT_ITEM_COUNT_HINT;
		serviceCountHint = DEFAULT_SERVICE_COUNT_HINT;
		requestTimeout = DEFAULT_REQUEST_TIMEOUT;
		dispatchTimeoutApiThread = DEFAULT_DISPATCH_TIMEOUT_API_THREAD;
		maxDispatchCountApiThread = DEFAULT_MAX_DISPATCH_COUNT_API_THREAD;
		maxDispatchCountUserThread = DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD;
		userDispatch = DEFAULT_USER_DISPATCH;
		configuredName = null;
		instanceName = null;
		xmlTraceEnable = ActiveConfig.DEFAULT_XML_TRACE_ENABLE;
		xmlTraceToFileEnable = ActiveConfig.DEFAULT_XML_TRACE_TO_FILE_ENABLE;
		xmlTraceMaxFileSize = ActiveConfig.DEFAULT_XML_TRACE_MAX_FILE_SIZE;
		xmlTraceFileName = ActiveConfig.DEFAULT_XML_TRACE_FILE_NAME;
		xmlTraceToMultipleFilesEnable = ActiveConfig.DEFAULT_XML_TRACE_TO_MULTIPLE_FILES;
		xmlTraceWriteEnable = ActiveConfig.DEFAULT_XML_TRACE_WRITE;
		xmlTraceReadEnable = ActiveConfig.DEFAULT_XML_TRACE_READ;
		xmlTracePingEnable = ActiveConfig.DEFAULT_XML_TRACE_PING;
		globalConfig.clear();
		traceStr.setLength(0);
		defaultConverterServiceId = DEFAULT_CONVERTER_SERVICE_ID;
		jsonExpandedEnumFields = DEFAULT_JSON_ENUM_EXPAND_FIELDS;
		catchUnknownJsonKeys = DEFAULT_CATCH_UNKNOWN_JSON_KEYS;
		catchUnknownJsonFids = DEFAULT_CATCH_UNKNOWN_JSON_FIDS;
		closeChannelFromFailure = DEFAULT_CLOSE_CHANNEL_FROM_FAILURE;
		sendJsonConvError = DEFAULT_SEND_JSON_CONV_ERROR;
	}
	
	StringBuilder configTrace()
	{
		traceStr.append("\n\t itemCountHint: ").append(itemCountHint) 
		.append("\n\t serviceCountHint: ").append(serviceCountHint) 
		.append("\n\t requestTimeout: ").append(requestTimeout) 
		.append("\n\t dispatchTimeoutApiThread: ").append(dispatchTimeoutApiThread) 
		.append("\n\t maxDispatchCountApiThread: ").append(maxDispatchCountApiThread) 
		.append("\n\t maxDispatchCountUserThread: ").append(maxDispatchCountUserThread) 
		.append("\n\t userDispatch: ").append(userDispatch) 
		.append("\n\t configuredName: ").append(configuredName)
		.append("\n\t instanceName: ").append(instanceName) 
		.append("\n\t xmlTraceEnable: ").append(xmlTraceEnable)
		.append("\n\t xmlTraceToFileEnable: ").append(xmlTraceToFileEnable)
		.append("\n\t xmlTraceMaxFileSize: ").append(xmlTraceMaxFileSize)
		.append("\n\t xmlTraceFileName: ").append(xmlTraceFileName)
		.append("\n\t xmlTraceToMultipleFiles: ").append(xmlTraceToMultipleFilesEnable)
		.append("\n\t xmlTraceWriteEnable: ").append(xmlTraceWriteEnable)
		.append("\n\t xmlTraceReadEnable: ").append(xmlTraceReadEnable)
		.append("\n\t xmlTracePingEnable: ").append(xmlTracePingEnable)
		.append("\n\t defaultConverterServiceId: ").append(defaultConverterServiceId)
		.append("\n\t jsonExpandedEnumFields: ").append(jsonExpandedEnumFields)
		.append("\n\t catchUnknownJsonKeys: ").append(catchUnknownJsonKeys)
		.append("\n\t catchUnknownJsonFids: ").append(catchUnknownJsonFids)
		.append("\n\t closeChannelFromFailure: ").append(closeChannelFromFailure)
		.append("\n\t sendJsonConvError: ").append(sendJsonConvError)
		.append("\n\t globalConfig.reactorChannelEventPoolLimit: ").append(globalConfig.reactorChannelEventPoolLimit)
		.append("\n\t globalConfig.reactorMsgEventPoolLimit: ").append(globalConfig.reactorMsgEventPoolLimit)
		.append("\n\t globalConfig.workerEventPoolLimit: ").append(globalConfig.workerEventPoolLimit)
		.append("\n\t globalConfig.tunnelStreamMsgEventPoolLimit: ").append(globalConfig.tunnelStreamMsgEventPoolLimit)
		.append("\n\t globalConfig.tunnelStreamStatusEventPoolLimit: ").append(globalConfig.tunnelStreamStatusEventPoolLimit);
		
		return traceStr;
	}
	
	String					configuredName;
	String      			instanceName;
	int						itemCountHint;
	int		    			serviceCountHint;
	int						requestTimeout;
	int						dispatchTimeoutApiThread;
	int						maxDispatchCountApiThread;
	int						maxDispatchCountUserThread;
	int		    			userDispatch;
	boolean 				xmlTraceEnable;
	boolean 				xmlTraceToFileEnable;
	long 					xmlTraceMaxFileSize;
	String					xmlTraceFileName;
	boolean 				xmlTraceToMultipleFilesEnable;
	boolean 				xmlTraceReadEnable;
	boolean 				xmlTraceWriteEnable;
	boolean 				xmlTracePingEnable;

	StringBuilder			traceStr;
	GlobalConfig            globalConfig;
	int 					defaultConverterServiceId;
	boolean					jsonExpandedEnumFields;
	boolean					catchUnknownJsonKeys;
	boolean					catchUnknownJsonFids;
	boolean					closeChannelFromFailure;
	boolean 				sendJsonConvError;
}
