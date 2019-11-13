///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

class OmmNiProviderActiveConfig extends ActiveConfig
{
	static final int DEFAULT_SERVICE_STATE								=	1;
	static final int DEFAULT_ACCEPTING_REQUESTS							=	1;
	static final boolean DEFAULT_IS_STATUS_CONFIGURED					=	false;
	static final int DEFAULT_SERVICE_ID									=	0;
	static final int DEFAULT_SERVICE_IS_SOURCE							=	0;
	static final int DEFAULT_SERVICE_SUPPORTS_QOS_RANGE					=	0;
	static final int DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS		=  	1;
    static final int DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE			=	1;
	static final boolean DEFAULT_REFRESH_FIRST_REQUIRED					=	true;
	static final boolean DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS			=	true;
	static final int DEFAULT_DIRECTORY_ADMIN_CONTROL					=	OmmNiProviderConfig.AdminControl.API_CONTROL;
	static final boolean DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY	=	true;
	static final boolean DEFAULT_REMOVE_ITEMS_ON_DISCONNECT				=	false;
	static final String DEFAULT_NIPROVIDER_SERVICE_NAME 				= 	"14003";
	static final String DEFAULT_SERVICE_NAME							=   "NI_PUB";
	
	int 						directoryAdminControl;
	boolean						refreshFirstRequired;
	boolean						mergeSourceDirectoryStreams;
	boolean						recoverUserSubmitSourceDirectory;
	boolean						removeItemsOnDisconnect;

	OmmNiProviderActiveConfig()
	{
		super(DEFAULT_NIPROVIDER_SERVICE_NAME);
		directoryAdminControl = DEFAULT_DIRECTORY_ADMIN_CONTROL;
		refreshFirstRequired = DEFAULT_REFRESH_FIRST_REQUIRED;
		mergeSourceDirectoryStreams = DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS;
		recoverUserSubmitSourceDirectory = DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY;
		removeItemsOnDisconnect = DEFAULT_REMOVE_ITEMS_ON_DISCONNECT;
	}
	
	StringBuilder configTrace()
	{
		super.configTrace();
		traceStr.append("\n\t directoryAdminControl: ").append(directoryAdminControl) 
		.append("\n\t refreshFirstRequired: ").append(refreshFirstRequired) 
		.append("\n\t mergeSourceDirectoryStreams: ").append(mergeSourceDirectoryStreams) 
		.append("\n\t recoverUserSubmitSourceDirectory: ").append(recoverUserSubmitSourceDirectory) 
		.append("\n\t removeItemsOnDisconnect: ").append(removeItemsOnDisconnect);
		
		return traceStr;
	}
}

