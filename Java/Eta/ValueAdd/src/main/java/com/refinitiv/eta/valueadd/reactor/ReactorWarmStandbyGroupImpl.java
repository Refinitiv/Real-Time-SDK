/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.valueadd.common.VaNode;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceState;

/**
 * Configuration options for creating a Warm Standby server information. See {@link ReactorConnect}.
 */
class ReactorWarmStandbyGroupImpl implements ReactorWarmStandbyGroup
{
	/*
	 *  The active server to which client should to connect.
	 *  Reactor chooses a server from the standByServerList instead if this parameter is not configured.
	 */
	private ReactorWarmStandbyServerInfo startingActiveServer;
	ConnectOptionsInfo startingConnectOptionsInfo = new ConnectOptionsInfo();
    
	private List<ReactorWarmStandbyServerInfo> standbyServerList;		// A list of standby servers.
	List<ConnectOptionsInfo> standbyConnectOptionsInfoList;
	private int warmStandbyMode; 										// Specifies a warm standby mode. see ReactorWarmStandbyMode
	
	boolean downloadConnectionConfig;  										/* Specifies whether to download connection configurations from a provider by
	 																		 * setting the DownloadConnectionConfig element with the login request. 
	 																		 */
	
	HashMap<WlInteger, ReactorWSBService> _perServiceById = null; 						// This hash table provides mapping between service ID and ReactorChannel.
	HashMap<Buffer, ReactorWSBService> _startupServiceNameList = null; 						// This hash table provides mapping between service ID and ReactorChannel.

	List<ReactorWSBService> _updateServiceList; 											// Keeps a list of update service list which belong to this group.
	boolean sendQueueReqForAll = false; 											// Keeps track whether all reactor channels have been created for this warmstandby group.
	int sendReqQueueCount = 0;													// Keeps track the number of channels that the requests has been sent.
	int currentStartingServerIndex = 0; 									    /* Keeps track of the current starting server index. REACTOR_WSB_STARTING_SERVER_INDEX for ReactorWarmStandbyGroup.startingActiveServer 
												  							 * and non-negative values represent an index of RsslReactorWarmStandbyGroup.standbyServerList 
												  							 */
	int numOfClosingStandbyServers = 0; 										// Keeps track of number of standby servers being closed.
	int downloadConfigActiveServer = 0; 										// Select an active server from downloadConnectionConfig
	int directoryStreamId = 0; 															// Stream ID for the source directory.
	
	int closingStandbyCount;
	
	boolean startingServerIsDown = false;
		
	final static int REACTOR_WSB_STARTING_SERVER_INDEX = -1; 						// Indicates the ReactorWarmStandbyGroup.startingActiveServer
	
	public ReactorWarmStandbyGroupImpl()
	{
		startingActiveServer = new ReactorWarmStandbyServerInfo();
		
		clear();
	}
	
	public void clear()
	{
		startingActiveServer.clear();
		if (standbyServerList == null)
		{
			standbyServerList = new LinkedList<ReactorWarmStandbyServerInfo>();
		}
		standbyServerList.clear();

		if (_updateServiceList == null)
		{
			_updateServiceList = new ArrayList<ReactorWSBService>();
		}
		_updateServiceList.clear();

		if(_perServiceById == null)
		{
			_perServiceById = new HashMap<WlInteger, ReactorWSBService>();
		}
		
		Iterator<Map.Entry<WlInteger, ReactorWSBService>> iter = _perServiceById.entrySet().iterator();
		while(iter.hasNext())
		{
			iter.next().getValue().returnToPool();
		}
		
		_perServiceById.clear();
		
		if(_startupServiceNameList == null)
		{
			_startupServiceNameList = new HashMap<Buffer, ReactorWSBService>();
		}
		
		if(standbyConnectOptionsInfoList == null)
		{
			standbyConnectOptionsInfoList = new ArrayList<ConnectOptionsInfo>();
		}
		
		standbyConnectOptionsInfoList.clear();
		
		Iterator<Map.Entry<Buffer, ReactorWSBService>> startupIter = _startupServiceNameList.entrySet().iterator();
		while(startupIter.hasNext())
		{
			startupIter.next().getValue().returnToPool();
		}
		
		_startupServiceNameList.clear();

		warmStandbyMode = ReactorWarmStandbyMode.LOGIN_BASED;
		sendQueueReqForAll = false;
		sendReqQueueCount = 0;
		currentStartingServerIndex = REACTOR_WSB_STARTING_SERVER_INDEX;
		numOfClosingStandbyServers = 0;
		downloadConfigActiveServer = 0;
		directoryStreamId = 0;
	}
	
	@Override
	public ReactorWarmStandbyServerInfo startingActiveServer()
	{
		return startingActiveServer;
	}
	
	@Override
	public List<ReactorWarmStandbyServerInfo> standbyServerList()
	{
		return standbyServerList;
	}

	@Override
	public int warmStandbyMode()
	{
		return warmStandbyMode;
	}

	@Override
	public void warmStandbyMode(int warmStandbyMode)
	{
		this.warmStandbyMode = warmStandbyMode;
	}
	
	void incrementClosingStandbyCount()
	{
		closingStandbyCount++;
	}
	
	int closingStandbyCount()
	{
		return closingStandbyCount;
	}
	
	/* Deep copies the config portion of the warm standby group. */
	public void copy(ReactorWarmStandbyGroupImpl copyTo)
	{
		copyTo.clear();
		
		startingActiveServer.copy(copyTo.startingActiveServer);
		for (int i = 0; i < standbyServerList.size(); ++i)
		{
			ReactorWarmStandbyServerInfo standbyInfo = new ReactorWarmStandbyServerInfo();
			standbyServerList.get(i).copy(standbyInfo);
			copyTo.standbyServerList.add(standbyInfo);
		}
		copyTo.warmStandbyMode = warmStandbyMode;
		copyTo.downloadConnectionConfig = downloadConnectionConfig;
	}
}

class ReactorWSBService extends VaNode
{
	List<ReactorChannel> channels; // List of channels that have this service as active
	ReactorChannel activeChannel;	// Current active channel for this service
	WlInteger serviceId;			// Service Id, used as the key for the hash table lookups
	Service serviceInfo;			// Service information
	ServiceState serviceState;		// Aggregated service state information for this service.  
									// If there any channels in the channels list, serviceState.ServiceState() should be 1
	
	int updateServiceFilter;		// Filter for the service state to be fanned out to the user if this ReactorWSBService is in the group handler's _updateServiceList
	int serviceAction;				// Map entry action for this service state, again to be fanned out to the user.
	Buffer serviceName;				// Name of the service.
	int standbyListIndex;	
	boolean preferredHostSwitched;
	
	ReactorWSBService()
	{
		channels = new ArrayList<ReactorChannel>();
		serviceInfo = DirectoryMsgFactory.createService();
		serviceState = new ServiceState();
		serviceId = ReactorFactory.createWlInteger();
		serviceName = CodecFactory.createBuffer();

		clear();
	}
	
	void clear()
	{
		channels.clear();
		activeChannel = null;
		serviceId.clear();
		serviceInfo.clear();
		serviceState.clear();
		updateServiceFilter = 0;
		serviceAction = 0;
		standbyListIndex = -1;
	}
	
	public void returnToPool()
	{
		clear();
		super.returnToPool();
	}
}
