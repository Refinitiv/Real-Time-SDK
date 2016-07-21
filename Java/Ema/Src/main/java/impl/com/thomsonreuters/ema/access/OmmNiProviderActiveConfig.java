///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.List;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

class OmmNiProviderActiveConfig extends ActiveConfig
{

	static final int DEFAULT_USER_DISPATCH 								=   OmmNiProviderConfig.OperationModel.API_DISPATCH;
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
	DirectoryConfig				directoryConfig;
	DirectoryConfig				userSubmittedDirectoryConfig;

	OmmNiProviderActiveConfig()
	{
		super(DEFAULT_NIPROVIDER_SERVICE_NAME);
		operationModel = DEFAULT_USER_DISPATCH;
		directoryAdminControl = DEFAULT_DIRECTORY_ADMIN_CONTROL;
		refreshFirstRequired = DEFAULT_REFRESH_FIRST_REQUIRED;
		mergeSourceDirectoryStreams = DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS;
		recoverUserSubmitSourceDirectory = DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY;
		removeItemsOnDisconnect = DEFAULT_REMOVE_ITEMS_ON_DISCONNECT;
		directoryConfig = new DirectoryConfig();
		userSubmittedDirectoryConfig = new DirectoryConfig();
	}
}

class DirectoryConfig
{
	String						directoryName;
	private DirectoryRefresh 	directoryRefresh;
	private int					streamId;
	
	DirectoryConfig()
	{
		directoryName = "";
		directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
		clear();
	}
	
	void clear()
	{
		directoryName = "";
		directoryRefresh.clear();
	    streamId = 0;
	
	    directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
	       
	    directoryRefresh.streamId(streamId);
	    
	    Buffer stateText = CodecFactory.createBuffer();
	    stateText.data("Source Directory Refresh Completed");
	        
	    directoryRefresh.state().streamState(StreamStates.OPEN);
	    directoryRefresh.state().dataState(DataStates.OK);
	    directoryRefresh.state().code(StateCodes.NONE);
	    directoryRefresh.state().text(stateText);
	        
	    directoryRefresh.applyClearCache();

	    directoryRefresh.filter( com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO |
				 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE);
	}
	
	void addService(Service service)
	{
		directoryRefresh.serviceList().add(service);
	}
	
	Service getService(int serviceId)
	{
		for(int index = 0; index < directoryRefresh.serviceList().size(); index++ )
		{
			if ( directoryRefresh.serviceList().get(index).serviceId() == serviceId )
			{
				return directoryRefresh.serviceList().get(index);
			}
		}
		
		return null;
	}
	
	void removeService(int serviceId)
	{
		for(int index = 0; index < directoryRefresh.serviceList().size(); index++ )
		{
			if ( directoryRefresh.serviceList().get(index).serviceId() == serviceId )
			{
				directoryRefresh.serviceList().remove(index);
				break;
			}
		}
	}
	
	List<Service> serviceList()
	{
		return directoryRefresh.serviceList();
	}
	
	DirectoryRefresh getDirectoryRefresh()
	{		
		return directoryRefresh;
	}
}

