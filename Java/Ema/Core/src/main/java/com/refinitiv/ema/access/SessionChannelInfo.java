///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceInfo;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelType;

class SessionChannelInfo<T>
{	
	private static final String CLIENT_NAME = "SessionChannelInfo";

	private SessionChannelConfig _sessionChannelConfig;
	
	private List<ChannelInfo> _channelInfoList;
	
	private ConsumerSession<T> _consumerSession;
	
	private boolean _receivedLoginRefresh;
	
	private ReactorChannel _reactorChannel;
	
	private LoginRefresh _loginRefresh; // Stores login refresh for this session channel
	
	private OmmBaseImpl<T> _baseImpl;
	
	// Stores source directory for this session channel.
	private Map<Integer, Directory<T>>			_serviceById;
	private Map<String, Directory<T>>			_serviceByName;
	
	/* This is used to keep the state of this connection for administrative domains from OmmBaseImpl.OmmImplState
	   when initializing OmmConsumer instance */
	private int								_state;

	public boolean sendChannelUp;
	
	private HashSet<SingleItem<T>>			_wsbStaleItems; /* This is used to keep a list of stall items for WSB channel to be recovered */
	
	/* Key is the requested service name */
	private Map<String, HashSet<SingleItem<T>>> _wsbStaleServiceBasedItems;  /* This is used to keep a list of stall items for WSB service based channel to be recovered */

	private boolean _phOperationInProgress;

	SessionChannelInfo(SessionChannelConfig sessionChannelConfig, ConsumerSession<T> consumerSession)
	{
		_sessionChannelConfig = sessionChannelConfig;
		
		_channelInfoList = new ArrayList<ChannelInfo>(sessionChannelConfig.configChannelSet.size());
		
		_consumerSession = consumerSession;
		
		_loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
		_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		
		_baseImpl = _consumerSession.ommBaseImpl();
		
		int initialHashSize =  (int)(_baseImpl.activeConfig().serviceCountHint/ 0.75 + 1);
		_serviceById = new HashMap<Integer, Directory<T>>(initialHashSize);
		_serviceByName = new HashMap<String, Directory<T>>(initialHashSize);
		
		_state = OmmBaseImpl.OmmImplState.REACTOR_INITIALIZED;
		
		_wsbStaleItems = new HashSet<SingleItem<T>>();
		
		_wsbStaleServiceBasedItems = new HashMap<String, HashSet<SingleItem<T>>>();
	}
	
	int state()
	{
		return _state;
	}
	
	void state(int state)
	{
		_state = state;
	}
	
	List<ChannelInfo> channelInfoList()
	{
		return _channelInfoList;
	}
	
	ConsumerSession<T> consumerSession()
	{
		return _consumerSession;
	}
	
	/* This is used to indicate whether the current session receives login refresh */
	void receivedLoginRefresh(boolean receivedLoginRefresh)
	{
		_receivedLoginRefresh = receivedLoginRefresh;
	}
	
	boolean receivedLoginRefresh()
	{
		return _receivedLoginRefresh;
	}

	void reactorChannel(ReactorChannel reactorChannel) 
	{
		_reactorChannel = reactorChannel;
	}
	
	ReactorChannel reactorChannel()
	{
		return _reactorChannel;
	}
	
	LoginRefresh loginRefresh()
	{
		return _loginRefresh;
	}
	
	SessionChannelConfig sessionChannelConfig()
	{
		return _sessionChannelConfig;
	}
	
	void updateSessionDirectory(Directory<T> directory, SessionDirectory<T> sessionDirectory)
	{
		if(directory != null)
		{
			directory.generatedServiceId(sessionDirectory.service().serviceId());
			directory.sessionDirectory(sessionDirectory);
		}
	}
	
	void processDirectoryPayload(List<Service> serviceList, ChannelInfo chnlInfo)
	{
		for (Service oneService : serviceList)
        {
			switch (oneService.action())
			{
				case MapEntryActions.ADD :
				{
					if (!(oneService.checkHasInfo()))
					{
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
							_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, "Received RDMService with Add action but no Service Info", Severity.ERROR));
			        	}
						break;
					}

					String serviceName = oneService.info().serviceName().toString();
		            if(serviceName == null)
		            {
		            	if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
							_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, "Received RDMService with Add action but no Service Info", Severity.ERROR));
			        	}
						break;
		            }

		            Service existingService = null;
		            Directory<T> existingDirectory = null;
		            if (_serviceByName.size() > 0)
		            {
		            	existingDirectory = _serviceByName.get(serviceName);
		            	existingService = existingDirectory != null ? existingDirectory.service() : null;
		            }
		            
		            if (existingService != null)
		            {
		            	if (existingService.serviceId() != oneService.serviceId())
						{
		            		_serviceById.remove(existingService.serviceId());
		            		existingService.serviceId(oneService.serviceId());
		            		_serviceById.put(existingService.serviceId(), existingDirectory);
		            		
						}
		            	if( existingDirectory.channelInfo() != chnlInfo )
		            	{
		            		chnlInfo.rsslDictionary(existingDirectory.channelInfo().rsslDictionary());
		            		existingDirectory.channelInfo(chnlInfo);
		            	}
		            	
		            	oneService.copy(existingService);
		            	
		            	// Notify service is added to ConsumerSession
						_consumerSession.onServiceAdd(this, existingDirectory);
		            }
		            else
		            {    
		            	Service newService = DirectoryMsgFactory.createService();
		            	oneService.copy(newService);
		            	
		            	Directory<T> directory = new Directory<T>(newService); 
		            	directory.channelInfo(chnlInfo);
		            	directory.serviceName(serviceName);
		            	
		            	_serviceById.put(oneService.serviceId(), directory);
		            	_serviceByName.put(serviceName, directory);
		            	
		            	// Notify service is added to ConsumerSession
						_consumerSession.onServiceAdd(this, directory);

						if (_baseImpl.activeConfig().dictionaryConfig.isLocalDictionary ||
						(newService.state().acceptingRequests() == 1 && newService.state().serviceState() == 1))
						{
							_consumerSession.downloadDataDictionary(directory);
						}
		            }
	
					break;
				}
				case MapEntryActions.UPDATE :
				{
					Service existingService = null;
					Directory<T> existingDirectory = null; 
			        if (_serviceById.size() > 0 && _serviceById.containsKey(oneService.serviceId()))
			        {
			        	existingDirectory = _serviceById.get(oneService.serviceId());
			        	existingService = existingDirectory.service();
			        }
			        
			        if (existingService == null)
					{
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("Received Update action for unknown Service with service id ")
				        		.append(oneService.serviceId());
				        	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						break;
					}
					else if (_baseImpl.loggerClient().isTraceEnabled())
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
						temp.append("Received Update action for RDMService").append(OmmLoggerClient.CR)
							.append("Service name ").append(existingService.info().serviceName().toString()).append(OmmLoggerClient.CR)
							.append("Service id ").append(existingService.serviceId());
						
						_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, temp.toString(), Severity.TRACE));
					}
	            	if((existingDirectory != null) && existingDirectory.channelInfo() != chnlInfo )
	            	{
	            		chnlInfo.rsslDictionary(existingDirectory.channelInfo().rsslDictionary());
	            		existingDirectory.channelInfo(chnlInfo);
	            	}

					if (oneService.checkHasInfo())
					{
						ServiceInfo  existInfo = existingService.info();
						if (!(existInfo.serviceName().equals(oneService.info().serviceName())))
						{
							if (_baseImpl.loggerClient().isErrorEnabled())
				        	{
					        	StringBuilder temp = _baseImpl.strBuilder();
								
					        	temp.append("Received Update action for RDMService").append(OmmLoggerClient.CR)
					        		.append("Service name ").append(existInfo.serviceName().toString()).append(OmmLoggerClient.CR)
									.append("Service id ").append(existingService.serviceId()).append(OmmLoggerClient.CR)
									.append("attempting to change service name to ").append(oneService.info().serviceName().toString());
					        	
					        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, temp.toString(), Severity.ERROR));
				        	}
							break;
						}
						
						oneService.info().copy(existInfo);
						
						// Notify service info change to ConsumerSession
						_consumerSession.onServiceInfoChange(this, existingDirectory, existInfo);
					}
					
					if (oneService.checkHasState())
					{
						oneService.state().copy(existingService.state());
						
						// Notify service state change to ConsumerSession
						_consumerSession.onServiceStateChange(this, existingDirectory, existingService.state());
						
						if (oneService.state().acceptingRequests() == 1 && oneService.state().serviceState() == 1)
						{
							_consumerSession.downloadDataDictionary(existingDirectory);
						}
					}
					
					existingService.action(MapEntryActions.UPDATE);
	
					break;
				}
				case MapEntryActions.DELETE :
				{
					Service existingService = null;
					Directory<T> existingDirectory = null;
			        if (_serviceById.size() > 0 && _serviceById.containsKey(oneService.serviceId()))
					{
			        	existingDirectory = _serviceById.get(oneService.serviceId());
						existingService = existingDirectory.service();
					}
					if (existingService == null)
					{
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("Received Delete action for unknown RDMService with service id ")
				        		.append(oneService.serviceId());
				        	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						break;
					}
					else if (_baseImpl.loggerClient().isTraceEnabled())
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
						temp.append("Received Delete action for RDMService").append(OmmLoggerClient.CR)
							.append("Service name ").append(existingService.info().serviceName().toString()).append(OmmLoggerClient.CR)
							.append("Service id ").append(existingService.serviceId());
						
						_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, temp.toString(), Severity.TRACE));
					}
	
					existingService.clear();
					existingService.action(MapEntryActions.DELETE);
					
					// Notify service is deleted to ConsumerSession
					_consumerSession.onServiceDelete(this, existingDirectory);
					break;
				}
				default :
				{
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("Received unknown action for RDMService. Action value ")
			        		.append(oneService.action());
			        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SessionChannelInfo.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
					break;
				}
			}
		}
	}
	
	Directory<T> getDirectoryByName(String name)
	{
		return _serviceByName.get(name);
	}
	
	Directory<T> getDirectoryById(Integer id)
	{
		return _serviceById.get(id);
	}
	
	void addWSBStaleItem(SingleItem<T> singleItem)
	{
		_wsbStaleItems.add(singleItem);
	}
	
	void removeWSBStaleItem(SingleItem<T> singleItem)
	{
		_wsbStaleItems.remove(singleItem);
	}
	
	void addWSBServiceBasedStaleItem(String serviceName, SingleItem<T> singleItem)
	{
		HashSet<SingleItem<T>> hashSet = _wsbStaleServiceBasedItems.get(serviceName);
		
		if(hashSet == null)
		{
			hashSet = new HashSet<SingleItem<T>>();
			
			_wsbStaleServiceBasedItems.put(serviceName, hashSet);
		}
		
		hashSet.add(singleItem);
	}
	
	void removeWSBServiceBasedStaleItem(String serviceName, SingleItem<T> singleItem)
	{
		HashSet<SingleItem<T>> hashSet = _wsbStaleServiceBasedItems.get(serviceName);
		if(hashSet != null)
		{
			hashSet.remove(singleItem);
		}
	}
	
	public void onServiceDown(String serviceName)
	{
		if (_reactorChannel.reactorChannelType() == ReactorChannelType.WARM_STANDBY)
		{
			HashSet<SingleItem<T>> hashSet = _wsbStaleServiceBasedItems.get(serviceName);
			if(hashSet != null)
			{
				_consumerSession.watchlist().recoverStaleWSBItems(this, hashSet, true);
			}
		}
	}
	
	public void onChannelClose(ChannelInfo channelInfo)
	{		
		if(channelInfo.getReactorChannelType() == ReactorChannelType.NORMAL)
		{
		 	Iterator<Directory<T>> directoryIt = _serviceByName.values().iterator();
		 	
		 	while(directoryIt.hasNext())
		 	{
		 		SessionDirectory<T> sessionDirectory = _consumerSession.sessionDirectoryByName(directoryIt.next().serviceName());
		 		
		 		sessionDirectory.HandleSessionChannelClose(this);
		 	}
		 	
		 	_serviceById.clear();
		 	_serviceByName.clear();
		}
		else if (channelInfo.getReactorChannelType() == ReactorChannelType.WARM_STANDBY)
		{
			_consumerSession.watchlist().recoverStaleWSBItems(this, _wsbStaleItems, false);
		}
	}
	
	public void close()
	{
		/* This function handles only for the warm standby channel only as the normal channel will be handled by SessionDirectory */
		if (_reactorChannel != null && _reactorChannel.reactorChannelType() == ReactorChannelType.WARM_STANDBY)
		{
			_consumerSession.watchlist().recoverStaleWSBItems(this, _wsbStaleItems, true);
		}
	}
	
	int getWarmStandbyMode(ReactorChannel reactorChannel)
	{
		ChannelInfo channelInfo = (ChannelInfo)reactorChannel.userSpecObj();
		
		String configName = channelInfo.getParentChannel().name();
		
		return _sessionChannelConfig.getWSBModeByChannelName(configName);
	}

	boolean phOperationInProgress() { return _phOperationInProgress; }

	void phOperationInProgress(boolean value) { _phOperationInProgress = value; }
}
