///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

abstract class ActiveServerConfig extends BaseConfig
{
	final static boolean DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS   = false;
	final static boolean DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS       = false;
	final static boolean DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN        = false;
	final static boolean DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM            = false;
	final static boolean DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE    = false;
	final static boolean DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE          = false;
	final static boolean DEFAULT_ENFORCE_ACK_ID_VALIDATION          = false;
	final static boolean DEFAULT_SERVER_SHARED_SOCKET          = false;
	final static int DEFAULT_CONNECTION_PINGTIMEOUT				  = 60000;
	final static int DEFAULT_CONNECTION_MINPINGTIMEOUT            = 20000;
	final static int DEFAULT_SERVER_SYS_SEND_BUFFER_SIZE		  = 65535;
	final static int DEFAULT_SERVER_SYS_RECEIVE_BUFFER_SIZE		  = 65535;
	
	ServerConfig                           serverConfig;
	static String                          defaultServiceName;
	int                                    operationModel;
	boolean                                acceptMessageWithoutAcceptingRequests;
	boolean                                acceptDirMessageWithoutMinFilters;
	boolean                                acceptMessageWithoutBeingLogin;
	boolean                                acceptMessageSameKeyButDiffStream;
	boolean                                acceptMessageThatChangesService;
	boolean                                acceptMessageWithoutQosInRange;
	boolean                                enforceAckIDValidation;
	
	private LongObject							         serviceId = new LongObject();
	private HashMap<LongObject, ServiceDictionaryConfig> serviceDictionaryConfigMap;

	ActiveServerConfig(String defaultServiceName)
	{
		super();
		ActiveServerConfig.defaultServiceName = defaultServiceName;
		serviceDictionaryConfigMap = new HashMap<>();
		
		acceptMessageWithoutAcceptingRequests = DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS;
		acceptDirMessageWithoutMinFilters = DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS;
		acceptMessageWithoutBeingLogin = DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN;
		acceptMessageSameKeyButDiffStream = DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM;
		acceptMessageThatChangesService = DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE;
		acceptMessageWithoutQosInRange = DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE;
		enforceAckIDValidation = DEFAULT_ENFORCE_ACK_ID_VALIDATION;
	}
	
	void clear()
	{
		super.clear();
		acceptMessageWithoutAcceptingRequests = DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS;
		acceptDirMessageWithoutMinFilters = DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS;
		acceptMessageWithoutBeingLogin = DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN;
		acceptMessageSameKeyButDiffStream = DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM;
		acceptMessageThatChangesService = DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE;
		acceptMessageWithoutQosInRange = DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE;
		enforceAckIDValidation = DEFAULT_ENFORCE_ACK_ID_VALIDATION;
		serviceDictionaryConfigMap.clear();
	}
	
	StringBuilder configTrace()
	{
		super.configTrace();
		traceStr.append("\n\t defaultServiceName: ").append(defaultServiceName)
				.append("\n\t acceptMessageWithoutAcceptingRequests: ").append(acceptMessageWithoutAcceptingRequests)
				.append("\n\t acceptDirMessageWithoutMinFilters: ").append(acceptDirMessageWithoutMinFilters)
				.append("\n\t acceptMessageWithoutBeingLogin: ").append(acceptMessageWithoutBeingLogin)
				.append("\n\t acceptMessageSameKeyButDiffStream: ").append(acceptMessageSameKeyButDiffStream)
				.append("\n\t acceptMessageThatChangesService: ").append(acceptMessageThatChangesService)
				.append("\n\t acceptMessageWithoutQosInRange: ").append(acceptMessageWithoutQosInRange)
				.append("\n\t enforceAckIDValidation: ").append(enforceAckIDValidation);

		return traceStr;
	}
	
	abstract int dictionaryAdminControl();
	abstract int directoryAdminControl();
	
	ServiceDictionaryConfig getServiceDictionaryConfig(int id)
	{
		return serviceDictionaryConfigMap.get(serviceId.value(id));
	}
	
	void addServiceDictionaryConfig(int id, ServiceDictionaryConfig serviceDictionaryConfig)
	{
		serviceDictionaryConfigMap.put( new LongObject().value(id) , serviceDictionaryConfig);
	}

	void removeServiceDictionaryConfig(int id)
	{
		serviceDictionaryConfigMap.remove(serviceId.value(id));
	}
	
	Collection<ServiceDictionaryConfig> getServiceDictionaryConfigCollection()
	{
		return serviceDictionaryConfigMap.values();
	}
	
	void setServiceDictionaryConfigCollection(Collection<ServiceDictionaryConfig> serviceDictionaryConfigCollection)
	{
		Iterator<ServiceDictionaryConfig> iterator = serviceDictionaryConfigCollection.iterator();
		
		while(iterator.hasNext())
		{
			ServiceDictionaryConfig serviceDictionaryConfig = iterator.next();

			if( ( serviceDictionaryConfig.dictionaryProvidedList.size() != 0 ) || ( serviceDictionaryConfig.dictionaryUsedList.size() != 0 ) )
				serviceDictionaryConfigMap.put( new LongObject().value(serviceDictionaryConfig.serviceId), serviceDictionaryConfig);
		}
	}
}
