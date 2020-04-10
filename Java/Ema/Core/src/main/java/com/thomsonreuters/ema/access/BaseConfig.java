///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

abstract class BaseConfig
{
	final static int DEFAULT_ITEM_COUNT_HINT					= 100000;
	final static int DEFAULT_SERVICE_COUNT_HINT				    = 513;
	final static int DEFAULT_REQUEST_TIMEOUT					= 15000;
	final static int DEFAULT_MAX_DISPATCH_COUNT_API_THREAD		= 100;
	final static int DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD	    = 100;
	final static int DEFAULT_DISPATCH_TIMEOUT_API_THREAD		= 0;
	final static int DEFAULT_USER_DISPATCH						= OmmConsumerConfig.OperationModel.API_DISPATCH;
	
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
		traceStr = new StringBuilder(500);
		globalConfig = new GlobalConfig();
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
		globalConfig.clear();
		traceStr.setLength(0);
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
	StringBuilder			traceStr;
	GlobalConfig            globalConfig;
}
