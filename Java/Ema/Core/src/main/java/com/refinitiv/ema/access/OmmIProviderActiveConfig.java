///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

class OmmIProviderActiveConfig extends ActiveServerConfig
{

	static final int DEFAULT_SERVICE_STATE								=	1;
	static final int DEFAULT_ACCEPTING_REQUESTS							=	1;
	static final boolean DEFAULT_IS_STATUS_CONFIGURED					=	false;
	static final int DEFAULT_SERVICE_ID									=	1;
	static final int DEFAULT_SERVICE_IS_SOURCE							=	1;
	static final int DEFAULT_SERVICE_SUPPORTS_QOS_RANGE					=	1;
	static final int DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS		=  	1;
    static final int DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE			=	1;
	static final boolean DEFAULT_REFRESH_FIRST_REQUIRED					=	true;
	static final int DEFAULT_DIRECTORY_ADMIN_CONTROL					=	OmmIProviderConfig.AdminControl.API_CONTROL;
	static final int DEFAULT_DICTIONARY_ADMIN_CONTROL					=	OmmIProviderConfig.AdminControl.API_CONTROL;
	static final boolean DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY	=	true;
	static final String DEFAULT_IPROVIDER_SERVICE_NAME 					= 	"14002";
	static final String DEFAULT_SERVICE_NAME							=   "DIRECT_FEED";
	static final int DEFAULT_FIELD_DICT_FRAGMENT_SIZE         = 8192;
	static final int DEFAULT_ENUM_TYPE_FRAGMENT_SIZE        = 128000;
	static final int DEFAULT_REQUEST_TIMEOUT				= 15000;
	    
	
	int 						directoryAdminControl;
	int                         dictionaryAdminControl;
	boolean						refreshFirstRequired;
	int							maxFieldDictFragmentSize;
	int							maxEnumTypeFragmentSize;

	OmmIProviderActiveConfig()
	{
		super(DEFAULT_IPROVIDER_SERVICE_NAME);
		operationModel = DEFAULT_USER_DISPATCH;
		directoryAdminControl = DEFAULT_DIRECTORY_ADMIN_CONTROL;
		dictionaryAdminControl = DEFAULT_DICTIONARY_ADMIN_CONTROL;
		refreshFirstRequired = DEFAULT_REFRESH_FIRST_REQUIRED;
		maxFieldDictFragmentSize = DEFAULT_FIELD_DICT_FRAGMENT_SIZE;
		maxEnumTypeFragmentSize = DEFAULT_ENUM_TYPE_FRAGMENT_SIZE;
	}
	
	StringBuilder configTrace()
	{
		super.configTrace();
		traceStr.append("\n\t operationModel: ").append(operationModel) 
		.append("\n\t directoryAdminControl: ").append(directoryAdminControl) 
		.append("\n\t dictionaryAdminControl: ").append(dictionaryAdminControl) 
		.append("\n\t refreshFirstRequired: ").append(refreshFirstRequired) 
		.append("\n\t maxFieldDictFragmentSize: ").append(maxFieldDictFragmentSize)
		.append("\n\t maxEnumTypeFragmentSize: ").append(maxEnumTypeFragmentSize);
		
		return traceStr;
	}

	@Override
	int dictionaryAdminControl()
	{
		return dictionaryAdminControl;
	}

	@Override
	int directoryAdminControl()
	{
		return directoryAdminControl;
	}
}

