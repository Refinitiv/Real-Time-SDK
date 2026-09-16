///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024-2025 LSEG. All rights reserved.           --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.OmmBaseImpl.OmmImplState;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.OmmState.DataState;
import com.refinitiv.ema.access.OmmState.StatusCode;
import com.refinitiv.ema.access.OmmState.StreamState;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceInfo;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceState;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelType;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;

interface DirectoryServiceClient<T>
{
	void onServiceAdd(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory);
	
	void onServiceDelete(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory);
	
	void onServiceStateChange(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory,  ServiceState serviceState);
	
	void onServiceInfoChange(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory,  ServiceInfo serviceInfo);
}

class ConsumerSessionTimeOut<T> implements TimeoutClient
{
	private ConsumerSession<T>			_consumerSession;
	private ReentrantLock				_userLock;
	private OmmBaseImpl<T>				_ommBaseImpl;
	private boolean						_installedTimeout;
	
	ConsumerSessionTimeOut(ConsumerSession<T> consumerSession)
	{
		_consumerSession = consumerSession;
		_ommBaseImpl = consumerSession.ommBaseImpl();
		_userLock = _ommBaseImpl.userLock();
	}
	
	/* timeout is in microsecond */
	void installTimeout(int timeout)
	{
		if(_installedTimeout) return;
		
		_ommBaseImpl.addTimeoutEvent(timeout, this);
		
		_installedTimeout = true;
	}

	@Override
	public void handleTimeoutEvent() {
		
		_installedTimeout = false;
		_consumerSession.dispatch();
	}

	@Override
	public ReentrantLock userLock() {
		return _userLock;
	}
	
}

class ScheduleCloseConsumerSessionChannel<T> implements TimeoutClient
{
	private ConsumerSession<T>			_consumerSession;
	private SessionChannelInfo<T>	_sessionChannelInfo;
	private ReentrantLock _userLock;
	private ChannelInfo _channelInfo;
	
	ScheduleCloseConsumerSessionChannel(ConsumerSession<T> consumerSession, SessionChannelInfo<T>	sessionChannelInfo, Directory<T> directory)
	{
		_consumerSession = consumerSession;
		_sessionChannelInfo = sessionChannelInfo;
		_userLock = _consumerSession.ommBaseImpl().userLock();
		_channelInfo = directory.channelInfo();
	}

	@Override
	public void handleTimeoutEvent() 
	{	
		if(_sessionChannelInfo != null)
		{
			/* Keep the current size before SessionChannelInfo is removed from closeSessionChannel() */
			int size = _consumerSession.sessionChannelList().size();
			
			/* Closes ReactorChannel and removes SessionChannelInfo */
			_consumerSession.ommBaseImpl().closeSessionChannelOnly(_sessionChannelInfo);
			
			_consumerSession.handleLoginStreamForChannelDown(_consumerSession.ommBaseImpl().loginCallbackClient().loginItemList(), _sessionChannelInfo.reactorChannel(), size);
			
			_consumerSession.ommBaseImpl()._channelCallbackClient.removeSessionChannel(_sessionChannelInfo);
			
			_sessionChannelInfo.onChannelClose(_channelInfo);
			
			_sessionChannelInfo = null;
		}
		
		_consumerSession = null;
	}

	@Override
	public ReentrantLock userLock() {
		return _userLock;
	}
}

class ConsumerSession<T> extends BaseSession<T> implements DirectoryServiceClient<T>
{
	
	protected static final String CLIENT_NAME = "ConsumerSession";
	
	protected static final int MSG_BUFFER_SIZE 	= 8192;

	int								_generateServiceId; // This is used to generate an unique service Id for service name for source directory response
	
	protected DirectoryMsg 			_directoryMsg;
	
	protected Map<String, SessionDirectory<T>>		_sessionDirByName;
	
	protected Map<Integer, SessionDirectory<T>>		_sessionDirById;
	
	// This keeps the list of deleted SessionDirectory
	protected HashSet<SessionDirectory<T>> _removedSessionDirSet;
	
	protected boolean 				_sendDirectoryResponse;
	
	protected boolean 				_sendInitialLoginRefresh;
	
	protected SessionWatchlist<T> _watchlist;
	
	protected Map<String, ServiceListImpl> 	_serviceListMap;
	
	protected ConsumerSessionTimeOut<T>		_dispatchTimeout;
		
	/* This is used to download data dictionary from network */
	protected ChannelDictionary<T>		_channelDictionary;

	private int _currentRsslDataState;

	/* This is used to indicate whether the ConsumerSession instance is initialized and ready to use. */
	protected boolean				_isConsumerSessionInitialized = false;
	
	/* Keeps a list of mismatch service names between session channels that need to be handled */
	protected HashSet<String>		_mismatchServiceSet;

	ConsumerSession(OmmBaseImpl<T> baseImpl, Map<String, ServiceListImpl> serviceListMap)
	{
		super(baseImpl);
	
		int initialHashSize =  (int)(_ommBaseImpl.activeConfig().serviceCountHint/ 0.75 + 1);
		_sessionDirByName = new LinkedHashMap<String, SessionDirectory<T>>(initialHashSize);
		_sessionDirById = new LinkedHashMap<Integer, SessionDirectory<T>>(initialHashSize);
		
		_removedSessionDirSet = new HashSet<SessionDirectory<T>>();
		
		_generateServiceId = 32766;
		
		_watchlist = new SessionWatchlist<T>(this, baseImpl.activeConfig().itemCountHint);
		
		_serviceListMap = serviceListMap;
		
		_dispatchTimeout = new ConsumerSessionTimeOut<T>(this);
		
		/* Sets the current state of OmmBaseImpl */
		_state = baseImpl._state;
		
		_rsslState = CodecFactory.createState();
		_rsslStatusMsg = (com.refinitiv.eta.codec.StatusMsg)CodecFactory.createMsg();
		_statusMsgImpl = new StatusMsgImpl(_ommBaseImpl.objManager());
		
		_eventImpl = new OmmEventImpl<T>(baseImpl);

		_currentRsslDataState = DataStates.SUSPECT;

		/* Generate an unique service ID for ServiceList if any */
		generateServiceIdForServiceMap();
	}
	
	void generateServiceIdForServiceMap()
	{
		if(_serviceListMap != null)
		{
			for(ServiceListImpl entry : _serviceListMap.values())
			{
				entry.serviceId(++_generateServiceId);
			}
		}
	}
	
	
	/* timeout is in microsecond to dispatch events from ConsumerSession */
	void nextDispatchTime(int timeout)
	{
		_dispatchTimeout.installTimeout(timeout);
	}
	
	 SessionWatchlist<T> watchlist()
	 {
		 return _watchlist;
	 }
	 
	 SessionDirectory<T> sessionDirectoryByName(String name)
	 {
		 return _sessionDirByName.get(name);
	 }
	 
	 SessionDirectory<T> sessionDirectoryById(int id)
	 {
		 return _sessionDirById.get(id);
	 }

	public void downloadDataDictionary(Directory<T> directory)
	{
		if(!_ommBaseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
		{
			if(_channelDictionary == null)
			{
				_channelDictionary = _ommBaseImpl.dictionaryCallbackClient().pollChannelDict(_ommBaseImpl);
		
				_ommBaseImpl.dictionaryCallbackClient().downloadDictionary(directory, _channelDictionary);
			}
			else
			{
				/* Shares the same data dictionary from network with the channel dictionary */
				directory.channelInfo().rsslDictionary(_channelDictionary.rsslDictionary());
			}
		}
		else
		{
			/* This gets data dictionary from the local file only */
			_ommBaseImpl.dictionaryCallbackClient().downloadDictionary(directory, _channelDictionary);
		}
	}
	
	public void loadDirectory()
	{
		if (_activeConfig.directoryRequestTimeOut == 0)
		{
			while (ommImplState() < OmmImplState.DIRECTORY_STREAM_OPEN_OK)
			{
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
				
				 if(checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_OK))
		         {
					 _ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
		         }
			}
		}
		else
		{
            if(checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_OK))
            {
                  ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
                  return;
             }
			
            _ommBaseImpl.resetEventTimeout();
			TimeoutEvent timeoutEvent = _ommBaseImpl.addTimeoutEvent(_activeConfig.directoryRequestTimeOut * 1000, _ommBaseImpl);
	
			while (!_ommBaseImpl.eventTimeout() && (ommImplState() < OmmImplState.DIRECTORY_STREAM_OPEN_OK))
			{
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
				
				if(checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_OK))
		         {
					 _ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
		         }
			}
	
			if (_ommBaseImpl.eventTimeout())
			{
				if(checkAtLeastOneSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_OK))
				{
					_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
					return;
				}
				
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("directory retrieval failed (timed out after waiting ")
						.append(_activeConfig.directoryRequestTimeOut).append(" milliseconds) for ");
				
				int count = _activeConfig.configSessionChannelSet.size();
				for(ConsumerSessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}		
				
				String excepText = strBuilder.toString();
				
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.DIRECTORY_REQUEST_TIME_OUT);
			} else
				timeoutEvent.cancel();
		}
	}
	
	public void loadDictionary() 
	{
		if (_activeConfig.dictionaryRequestTimeOut == 0)
		{
			while (!_ommBaseImpl.dictionaryCallbackClient().isDictionaryReady())
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
		}
		else
		{
			_ommBaseImpl.resetEventTimeout();
			TimeoutEvent timeoutEvent = _ommBaseImpl.addTimeoutEvent(_activeConfig.dictionaryRequestTimeOut * 1000, _ommBaseImpl);
	
			while (!_ommBaseImpl.eventTimeout() && !_ommBaseImpl.dictionaryCallbackClient().isDictionaryReady())
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
	
			if (_ommBaseImpl.eventTimeout())
			{
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("dictionary retrieval failed (timed out after waiting ")
						.append(_activeConfig.dictionaryRequestTimeOut).append(" milliseconds) for ");
				
				int count = _activeConfig.configSessionChannelSet.size();
				for(ConsumerSessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}	
	
				String excepText = strBuilder.toString();
				
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.DICTIONARY_REQUEST_TIME_OUT);
			} else
				timeoutEvent.cancel();
		}
	}
	
	/* This is used to reorder all SessionChannelInfo of every SessionDirectory according to the session's connection list */
	public void reorderSessionChannelInfoForSessionDirectory()
	{
		if(_isConsumerSessionInitialized)
			return;
		
		if(_mismatchServiceSet != null)
		{
			/* Removed the session channel which has mismatch service names */
			for(String serviceName : _mismatchServiceSet)
			{
				SessionDirectory<T> sessionDirectory = _sessionDirByName.get(serviceName);
				if(sessionDirectory != null)
				{
					int generatedServiceId = sessionDirectory.service().serviceId();
					boolean isFirstSessionChannel = false;
					for(int index = 0; index < _sessionChannelList.size();)
					{
						SessionChannelInfo<T> sessionChanelInfo = (SessionChannelInfo<T>)_sessionChannelList.get(index);
						Directory<T> directory = sessionChanelInfo.getDirectoryByName(sessionDirectory.serviceName());
						if(directory != null)
						{
							if(!isFirstSessionChannel)
							{
								/* Update the service from the first SessionChannelInfo which has the service name */
								directory.service().copy(sessionDirectory.service());
								sessionDirectory.service().serviceId(generatedServiceId);
								directory.generatedServiceId(generatedServiceId);
								sessionDirectory.sessionChannelList().clear();
								sessionDirectory.sessionChannelList().add(sessionChanelInfo);
								isFirstSessionChannel = true;
							}
							else
							{
								boolean result = compareServiceForAggregation(sessionDirectory, directory);
								
								if(result == false)
								{
									if (_ommBaseImpl.loggerClient().isErrorEnabled())
									{
										StringBuilder temp = _ommBaseImpl.strBuilder();
										temp.append("Failed to compare service for aggregation, closing session channel: " + sessionChanelInfo).append(OmmLoggerClient.CR)
										.append("Instance Name ").append(_ommBaseImpl.instanceName());
										_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
									}
									
									/* Closes this SessionChannelInfo from the consumer session */
									new ScheduleCloseConsumerSessionChannel<T>(this, sessionChanelInfo, directory).handleTimeoutEvent();
									
									/* Don't increase the index as this session channel is removed from the list */
									continue;
								}
								else
								{
									directory.generatedServiceId(generatedServiceId);
									sessionDirectory.sessionChannelList().add(sessionChanelInfo);
								}
							}	
						}
						
						/* Increase the index to the next session channel info if any*/
						++index; 
					}
				}
			}
			
			_mismatchServiceSet = null;
		}
		
		Iterator<SessionDirectory<T>> sessionDirIt =  _sessionDirByName.values().iterator();
		
		List<SessionChannelInfo<T>> tempChannelInfoList;
		
		while(sessionDirIt.hasNext())
		{
			SessionDirectory<T> sessionDir = sessionDirIt.next();
			
			Iterator<SessionChannelInfo<T>> sessionChannelInfoIt;
			
			tempChannelInfoList = new ArrayList<SessionChannelInfo<T>>();
			
			for(BaseSessionChannelInfo<T> channelInfo : _sessionChannelList)
			{
				sessionChannelInfoIt =  sessionDir.sessionChannelList().iterator();
				while(sessionChannelInfoIt.hasNext())
				{
					SessionChannelInfo<T> sessionChannelInfo = sessionChannelInfoIt.next();
					
					if(channelInfo == sessionChannelInfo)
					{
						tempChannelInfoList.add(sessionChannelInfo);
						break;
					}
				}
				
			}
			
			/* Replace with the ordered list according to the connection list */
			sessionDir.sessionChannelList().clear();
			sessionDir.sessionChannelList(tempChannelInfoList);
		}
		
		_isConsumerSessionInitialized = true; // Indicates that the ConsumerSession is initialized.
	}
	
	
	void processDirectoryPayload(List<Service> serviceList, ChannelInfo channelInfo)
	{
		_sendDirectoryResponse = false; 
		
		@SuppressWarnings("unchecked")
		SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) channelInfo.sessionChannelInfo();
		
		resetUpdateFlag();
		
		sessionChannelInfo.processDirectoryPayload(serviceList, channelInfo);
	}

	// This is used to fan out source directory response for all directory items.
	void fanoutSourceDirectoryResponse(DirectoryMsgType msgType)
	{
		if(_sendDirectoryResponse == false || _ommBaseImpl.directoryCallbackClient().directoryItemList().isEmpty())
			return;
		
		for(DirectoryItem<T> item : _ommBaseImpl.directoryCallbackClient().directoryItemList())
		{			
			fanoutSourceDirectoryResponsePerItem(item, msgType, false);
		}
		
		// Cleaning up Session directory after fanning out.
		Iterator<SessionDirectory<T>> it =  _sessionDirByName.values().iterator();
		SessionDirectory<T> sessionDirectory;
		
		
		while(it.hasNext())
		{
			sessionDirectory = it.next();
			
			if(sessionDirectory.sessionChannelList().size() == 0)
			{
				_removedSessionDirSet.add(sessionDirectory);
				continue;
			}
		}		
	}
	
	void fanoutSourceDirectoryResponsePerItem(DirectoryItem<T> item, DirectoryMsgType msgType, boolean isInitialRequest)
	{
		if(item == null)
			return;
		
		/* Generate DirectoryMsg from ConsumerSession's source directory cache */
		DirectoryMsg directoryMsg  = generateDirectoryMsg(item, msgType, isInitialRequest);
		
		if(directoryMsg == null)
			return;
		
		int ret = _ommBaseImpl.consumerSession().convertRdmDirectoryToBuffer(directoryMsg);
		if(ret != ReactorReturnCodes.SUCCESS)
		{
			if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
        		StringBuilder temp = _ommBaseImpl.strBuilder();
				
	        	temp.append("Internal error: failed to convert DirectoryMsg to encoded buffer in ConsumerSession.fanoutSourceDirectoryResponsePerItem()")
	        		.append(OmmLoggerClient.CR)
	        		.append("Error num ").append(ret).append(OmmLoggerClient.CR)
	        		.append("Error Text ").append(CodecReturnCodes.toString(ret));
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(ConsumerSession.CLIENT_NAME, 
	        									temp.toString(), Severity.ERROR));
        	}
			
			return;
		}
		
		ReactorChannel rsslReactorChannel = sessionChannelList().get(0).reactorChannel();
		
		Buffer encoededBuffer = encodedBuffer();
		
		restoreServiceFlags();
		
		if(msgType == DirectoryMsgType.REFRESH)
		{
			RefreshMsgImpl refreshMsgImpl = _ommBaseImpl.directoryCallbackClient()._refreshMsg;
			
			refreshMsgImpl.decode(encoededBuffer, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null, null);
			
			if(!item.serviceName().isEmpty())
			{
				refreshMsgImpl.service(item.serviceName());
			}
			
			_ommBaseImpl.directoryCallbackClient()._eventImpl._item = (Item<T>)item;
			_ommBaseImpl.directoryCallbackClient()._eventImpl._channel = rsslReactorChannel;
			
			_ommBaseImpl.directoryCallbackClient().notifyOnAllMsg(refreshMsgImpl);
			_ommBaseImpl.directoryCallbackClient().notifyOnRefreshMsg();
	
			if (refreshMsgImpl.state().streamState() != OmmState.StreamState.OPEN)
			{
				_ommBaseImpl.directoryCallbackClient().directoryItemList().remove(item);
				item.remove();
			}
		}
		else if (msgType == DirectoryMsgType.UPDATE)
		{
			UpdateMsgImpl updateMsgImpl = _ommBaseImpl.directoryCallbackClient()._updateMsg;
			
			updateMsgImpl.decode(encoededBuffer, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null, null);
			
			if(!item.serviceName().isEmpty())
			{
				updateMsgImpl.service(item.serviceName());
			}
			
			_ommBaseImpl.directoryCallbackClient()._eventImpl._item = (Item<T>)item;
			_ommBaseImpl.directoryCallbackClient()._eventImpl._channel = rsslReactorChannel;
			
			_ommBaseImpl.directoryCallbackClient().notifyOnAllMsg(updateMsgImpl);
			_ommBaseImpl.directoryCallbackClient().notifyOnUpdateMsg();
		}
	}
	
	public boolean compareServiceForAggregation(SessionDirectory<T> sessionDirectory, Directory<T> directory)
	{
		// Compares ItemList, Capabilities, QoS, SupportsQoSRange
		ServiceInfo sessionServiceInfo = sessionDirectory.service().info();
		ServiceInfo serviceInfo = directory.service().info();
		
		if(sessionServiceInfo.checkHasQos())
		{
			if(!serviceInfo.checkHasQos())
				return false;
			
			if(sessionServiceInfo.qosList().size() != serviceInfo.qosList().size())
				return false;
			
			for(Qos qos : sessionServiceInfo.qosList())
			{
				boolean found = false;
				
				for(Qos other : serviceInfo.qosList())
				{
					if(qos.equals(other))
					{
						found = true;
						break;
					}
				}
				
				if(!found)
					return false;
			}
		}
		
		/* Comment this section to support different capabilities with the same service name.
		if(sessionServiceInfo.capabilitiesList().size() == serviceInfo.capabilitiesList().size())
		{
			for(Long capability : sessionServiceInfo.capabilitiesList())
			{
				if(!sessionServiceInfo.capabilitiesList().contains(capability))
					return false;
			}
		}
		else
		{
			return false;
		}*/
		
		if(!sessionServiceInfo.itemList().equals(serviceInfo.itemList()))
		{
			return false;
		}
		
		if(sessionServiceInfo.checkHasSupportsQosRange() != serviceInfo.checkHasSupportsQosRange())
		{
			return false;
		}
		
		if(sessionServiceInfo.checkHasSupportsQosRange())
		{
			if(sessionServiceInfo.supportsQosRange() != serviceInfo.supportsQosRange())
			{
				return false;
			}
		}
		
		return true;
	}
	
	int convertRdmDirectoryToBuffer(DirectoryMsg directoryMsg)
	{
	    if (_rsslEncBuffer == null)
        {
	    	_rsslEncBuffer = CodecFactory.createBuffer();
	    	_rsslEncBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
        }
        else
        {
        	ByteBuffer byteBuf = _rsslEncBuffer.data();
        	byteBuf.clear();
        	_rsslEncBuffer.data(byteBuf, 0, byteBuf.capacity()); 
        }
	     
	    EncodeIterator rsslEncIter = _ommBaseImpl.rsslEncIter();
        rsslEncIter.clear();
        if (rsslEncIter.setBufferAndRWFVersion(_rsslEncBuffer, Codec.majorVersion(), Codec.minorVersion()) != CodecReturnCodes.SUCCESS)
        {
        	if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
        		_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(ConsumerSession.CLIENT_NAME, 
	        			"Internal error. Failed to set encode iterator buffer in ConsumerSession.convertRdmDirectoryToBuffer()",
	        			Severity.ERROR));
        	}
        	
        	return ReactorReturnCodes.FAILURE;
        }
        
        int ret = 0;
        if ((ret = directoryMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
        {
        	if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
        		StringBuilder temp = _ommBaseImpl.strBuilder();
				
	        	temp.append("Internal error: failed to encode DirectoryMsg in ConsumerSession.convertRdmDirectoryToBuffer()")
	        		.append(OmmLoggerClient.CR)
	        		.append("Error num ").append(ret).append(OmmLoggerClient.CR)
	        		.append("Error Text ").append(CodecReturnCodes.toString(ret));
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(ConsumerSession.CLIENT_NAME, 
	        									temp.toString(), Severity.ERROR));
        	}
        
        	return ReactorReturnCodes.FAILURE;
        }
        
        return ReactorReturnCodes.SUCCESS;
	}
	
	Buffer encodedBuffer()
	{
		return _rsslEncBuffer;
	}
	
	public Map<String, SessionDirectory<T>> sessionDirectoryByName()
	{
		return _sessionDirByName;
	}
	
	public Map<Integer, SessionDirectory<T>> sessionDirectoryById()
	{
		return _sessionDirById;
	}
	
	DirectoryMsg generateDirectoryMsg(DirectoryItem<T> item, DirectoryMsgType msgType, boolean isInitalRequest)
	{
		if(_directoryMsg == null)
		{
			_directoryMsg = DirectoryMsgFactory.createMsg();
		}
		else
		{
			_directoryMsg.clear();
		}
		
		_directoryMsg.rdmMsgType(msgType);
		_directoryMsg.streamId(2);
		
		if(msgType == DirectoryMsgType.REFRESH)
		{
			DirectoryRefresh directoryRefresh = (DirectoryRefresh)_directoryMsg;
			
			directoryRefresh.filter(item.filterId());
			directoryRefresh.applySolicited();
			directoryRefresh.state().streamState(StreamState.OPEN);
			directoryRefresh.state().dataState(DataState.OK);
			directoryRefresh.state().code(StatusCode.NONE);
			
			if(item.serviceName().isEmpty() && !item.hasServiceId())
			{
				Iterator<Entry<String, SessionDirectory<T>>> it = _sessionDirByName.entrySet().iterator();
				Entry<String, SessionDirectory<T>> entry;
				SessionDirectory<T> sessionDir;
				Service service;
				
				while(it.hasNext())
				{
					entry = it.next();
					sessionDir = entry.getValue();
					
					if(isInitalRequest || sessionDir.isUpdated())
					{
						service = sessionDir.service();
				
						if(service != null)
						{
							// Adjusts the service flags according to the request filter.
							service.flags(service.flags() & (int)item.filterId());
							directoryRefresh.serviceList().add(service);
						}
					}
				}
			}
			else
			{
				// Send empty service list as the specified service name or Id doesn't exist in the source directory of all channels.
				
				if(!item.serviceName().isEmpty())
				{
					SessionDirectory<T> sessionDirectory = _sessionDirByName.get(item.serviceName());
					
					/* Checks to ensure that the SessionDir is not removed */
					if(sessionDirectory != null && !_removedSessionDirSet.contains(sessionDirectory))
					{
						Service service = sessionDirectory.service();
						service.flags(service.flags() & (int)item.filterId());
						directoryRefresh.serviceList().add(service);
					}
				}
				else if(item.hasServiceId()) {
					directoryRefresh.applyHasServiceId();
					directoryRefresh.serviceId(item.serviceId());
					
					SessionDirectory<T> sessionDirectory = _sessionDirById.get(item.serviceId());
					
					/* Checks to ensure that the SessionDir is not removed */
					if(sessionDirectory != null && !_removedSessionDirSet.contains(sessionDirectory))
					{
						Service service = sessionDirectory.service();
						service.flags(service.flags() & (int)item.filterId());
						directoryRefresh.serviceList().add(service);
					}
				}
			}
		}
		else if(msgType == DirectoryMsgType.UPDATE)
		{
			DirectoryUpdate directoryUpdate = (DirectoryUpdate)_directoryMsg;
			directoryUpdate.applyHasFilter();
			directoryUpdate.filter(item.filterId());
			
			if(item.serviceName().isEmpty() && !item.hasServiceId())
			{
				Iterator<Entry<String, SessionDirectory<T>>> it = _sessionDirByName.entrySet().iterator();
				Entry<String, SessionDirectory<T>> entry;
				SessionDirectory<T> sessionDirectory;
				Service service;
				
				if(_sessionDirByName.entrySet().size() > 0)
				{
					boolean isUpdated = false;
					while(it.hasNext())
					{
						entry = it.next();
						sessionDirectory = entry.getValue();
						
						/* Checks to ensure that the SessionDir is not removed */
						if(!_removedSessionDirSet.contains(sessionDirectory))
						{
							if(sessionDirectory.isUpdated())
							{
								isUpdated = true;
								service = sessionDirectory.service();
								
								// Adjusts the service flags according to the request filter.
								service.flags(service.flags() & (int)item.filterId());
								directoryUpdate.serviceList().add(service);
							}
						}
					}
					
					if(isUpdated == false)
					{
						return null; // There is no need to send source directory response.
					}
				}
				else
				{
					return null;
				}
			}
			else
			{
				// Send empty service list as the specified service name or Id doesn't exist in the source directory of all channels.
				if(!item.serviceName().isEmpty())
				{
					SessionDirectory<T> sessionDirectory = _sessionDirByName.get(item.serviceName());
					
					/* Checks to ensure that the SessionDir is not removed */
					if(sessionDirectory != null && !_removedSessionDirSet.contains(sessionDirectory) && sessionDirectory.isUpdated())
					{
						Service service = sessionDirectory.service();
						service.flags(service.flags() & (int)item.filterId());
						
						directoryUpdate.serviceList().add(service);
					}
					else
					{
						return null;
					}
				}
				else if(item.hasServiceId()) {
					directoryUpdate.applyHasServiceId();
					directoryUpdate.serviceId(item.serviceId());
					
					SessionDirectory<T> sessionDirectory = _sessionDirById.get(item.serviceId());
					
					/* Checks to ensure that the SessionDir is not removed */
					if(sessionDirectory != null && !_removedSessionDirSet.contains(sessionDirectory) && sessionDirectory.isUpdated())
					{
						Service service = sessionDirectory.service();
						service.flags(service.flags() & (int)item.filterId());
						
						directoryUpdate.serviceList().add(service);
					}
					else
					{
						return null;
					}
				}
			}
		}
		
		return _directoryMsg;
	}
	
	void restoreServiceFlags()
	{
		Iterator<SessionDirectory<T>> it = _sessionDirByName.values().iterator();
		SessionDirectory<T> sessionDirectory;
		
		while(it.hasNext())
		{
			sessionDirectory = it.next();
			sessionDirectory.restoreServiceFlags();
		}
	}
	
	void resetUpdateFlag()
	{
		Iterator<SessionDirectory<T>> it = _sessionDirByName.values().iterator();
		SessionDirectory<T> sessionDirectory;
		
		while(it.hasNext())
		{
			sessionDirectory = it.next();
			sessionDirectory.resetUpdateFlags();
		}
	}

	@SuppressWarnings("unchecked")
	@Override
	public void onServiceStateChange(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory,
			ServiceState serviceState) 
	{
		
		String serviceName = directory.serviceName();
		
		SessionDirectory<T> sessionDirectory = _sessionDirByName.get(serviceName);
		
		if(sessionDirectory != null)
		{
			// Checks service state change from all session
			ServiceState currentState = sessionDirectory.service().state();
			
			if(currentState.serviceState() == 0 && serviceState.serviceState() == 1)
			{
				currentState.serviceState(1);
				sessionDirectory.isUpdated(true);
				_sendDirectoryResponse = true;
			}
			else if(currentState.serviceState() == 1 && serviceState.serviceState() == 0)
			{
				long serviceStateValue = 0;
				
				for(SessionChannelInfo<T> channelInfo : sessionDirectory.sessionChannelList())
				{
					Directory<T> tempDirectory = channelInfo.getDirectoryByName(serviceName);
					
					if(tempDirectory != null)
					{
						serviceStateValue |= tempDirectory.service().state().serviceState();
					}
				}
				
				if(serviceStateValue != currentState.serviceState())
				{
					currentState.serviceState(serviceStateValue);
					sessionDirectory.isUpdated(true);
					_sendDirectoryResponse = true;
				}
			}
			
			// Checks Accepting request change from all session
			if(sessionDirectory.service().state().checkHasAcceptingRequests())
			{
				if(serviceState.checkHasAcceptingRequests())
				{
					if(currentState.acceptingRequests() == 0 && serviceState.acceptingRequests() == 1)
					{
						currentState.acceptingRequests(1);
						sessionDirectory.isUpdated(true);
						_sendDirectoryResponse = true;
					}
					else if(currentState.acceptingRequests() == 1 && serviceState.acceptingRequests() == 0)
					{
						long acceptingRequests = 0;
						
						for(SessionChannelInfo<T> channelInfo : sessionDirectory.sessionChannelList())
						{
							Directory<T> tempDirectory = channelInfo.getDirectoryByName(serviceName);
							
							if(tempDirectory != null)
							{
								acceptingRequests |= tempDirectory.service().state().acceptingRequests();
							}
						}
						
						if(acceptingRequests != currentState.acceptingRequests())
						{
							currentState.acceptingRequests(acceptingRequests);
							sessionDirectory.isUpdated(true);
							_sendDirectoryResponse = true;
						}
					}
				}
			}
			else if(serviceState.checkHasAcceptingRequests())
			{
				sessionDirectory.service().state().applyHasAcceptingRequests();
				sessionDirectory.service().state().acceptingRequests(serviceState.acceptingRequests());
				sessionDirectory.isUpdated(true);
				
				_sendDirectoryResponse = true;
			}
			
			if(sessionChannelInfo.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
			{
				boolean serviceDown = sessionDirectory.checkServiceAreDownForWSBChannel(sessionChannelInfo);
				
				if(serviceDown)
				{
					sessionChannelInfo.onServiceDown(serviceName);
				}
			}
			
			sessionDirectory.handlePendingRequests(sessionChannelInfo, sessionDirectory.service());
			
			handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) sessionDirectory);
			
			if (_ommBaseImpl.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _ommBaseImpl.strBuilder();
				temp.append("Session name: " + sessionChannelInfo.sessionChannelConfig().name).append(OmmLoggerClient.CR)
	        	.append("    onServiceStateChange for " + directory).append(OmmLoggerClient.CR)
				.append("    Instance Name ").append(_ommBaseImpl.instanceName());
				_ommBaseImpl.loggerClient().trace(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
			}
			
			nextDispatchTime(1000); // Wait for 1 millisecond to recover
		}
	}

	public void enableSingleOpen(boolean enableSingleOpen)
	{
		_enableSingleOpen = enableSingleOpen;
		
	}

	public ServiceListImpl serviceList(String name)
	{
		return _ommBaseImpl.serviceList(name);
	}
	
	/* Checks every ServiceList to recover pending requests */
	public void handlePendingRequestsForServiceList(SessionDirectory<OmmConsumerClient> sessionDirectory)
	{
		if(_serviceListMap != null )
		{
			Iterator<ServiceListImpl> serviceListIt = _serviceListMap.values().iterator();
			
			ServiceListImpl serviceListImpl;
			while(serviceListIt.hasNext())
			{
				serviceListImpl = serviceListIt.next();
				
				if(serviceListImpl.pendingQueueSize() > 0)
				{
					serviceListImpl.handlePendingRequests(sessionDirectory);
				}
			}
		}
	}
	

	/* This function is used to add to SessionDirectory's pending queue as the requested service is not available yet. 
	 * The request will be recovered once the service is available. */
	public void addPendingRequestByServiceName(String serviceName, SingleItem<T> singleItem, ReqMsg reqMsg)
	{
		SessionDirectory<T> sessionDirectory = _sessionDirByName.get(serviceName);
		
		singleItem.state(SingleItem.ItemStates.RECOVERING);
		
		if(sessionDirectory == null)
		{
			Service newService = DirectoryMsgFactory.createService();
			
			// Generates an unique service ID for source directory response
			_generateServiceId++;
			
			newService.serviceId(_generateServiceId);
			
			sessionDirectory = new SessionDirectory<T>(this, serviceName);
			sessionDirectory.service(newService);
			
			_sessionDirByName.put(serviceName, sessionDirectory);
			_sessionDirById.put(_generateServiceId, sessionDirectory);
		}
		
		sessionDirectory.addPendingRequest(singleItem, reqMsg);
	}
	
	/* This function is used to add to SessionDirectory's pending queue with an existing Session directory
	 * The request will be recovered once the service is ready to accept requests. */
	public void addPendingRequestByServiceId(SessionDirectory<T> sessionDirectory, SingleItem<T> singleItem, ReqMsg reqMsg)
	{	
		singleItem.state(SingleItem.ItemStates.RECOVERING);
		sessionDirectory.addPendingRequest(singleItem, reqMsg);
	}
	
	public void dispatch()
	{
		_watchlist.submitItemRecovery();
	}

	@SuppressWarnings("unchecked")
	@Override
	public void onServiceAdd(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory) {
		
		SessionDirectory<T> sessionDirectory = _sessionDirByName.get(directory.serviceName());
		
		if(sessionDirectory == null)
		{
			Service newService = DirectoryMsgFactory.createService();
			directory.service().copy(newService);
			
			// Generates an unique service ID for source directory response
			_generateServiceId++;
			
			newService.serviceId(_generateServiceId);
			
			directory.generatedServiceId(newService.serviceId());
			
			SessionDirectory<T> newSessionDirectory = new SessionDirectory<T>(this, directory.serviceName());
			newSessionDirectory.service(newService);
			
			/* Adds this SessionChannelInfo to the SessionDirectory for this service */
			newSessionDirectory.sessionChannelList().add(sessionChannelInfo);
			
			_sessionDirByName.put(directory.serviceName(), newSessionDirectory);
			_sessionDirById.put(_generateServiceId, newSessionDirectory);
			
			_sendDirectoryResponse = true;
			newSessionDirectory.isUpdated(true);
			
			if(_isConsumerSessionInitialized)
			{
				handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) newSessionDirectory);
				
				/* Recover items in the item recovery queue if any. */
				nextDispatchTime(1000);
			}
		}
		else if (sessionDirectory.service().checkHasInfo() == false) 
		{
			/* This is blank SessionDirectory waiting to add this service. This also covers recovery case after the the SessionChannelInfo is deleted when the service is deleted*/
			
			int generatedServiceId = sessionDirectory.service().serviceId();
			directory.service().copy(sessionDirectory.service());
			
			sessionDirectory.service().serviceId(generatedServiceId);
			directory.generatedServiceId(generatedServiceId);
			
			sessionDirectory.sessionChannelList().add(sessionChannelInfo);
			
			_sendDirectoryResponse = true;
			sessionDirectory.isUpdated(true);
			
			sessionDirectory.handlePendingRequests(sessionChannelInfo, sessionDirectory.service());
			
			handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) sessionDirectory);
			
			/* Recover items in the item recovery queue if any. */
			nextDispatchTime(1000);
			
			/* Removed the SessionDirectory from the list as it is added again */
			_removedSessionDirSet.remove(sessionDirectory);
		}
		else
		{
			boolean result = compareServiceForAggregation(sessionDirectory, directory);
			
			if(result == false)
			{
				if(_isConsumerSessionInitialized)
				{
					if (_ommBaseImpl.loggerClient().isErrorEnabled())
					{
						StringBuilder temp = _ommBaseImpl.strBuilder();
			        	temp.append("Failed to compare service for aggregation, ignoring the " + sessionDirectory.serviceName() + " from session channel name: " + 
						sessionChannelInfo.sessionChannelConfig().name + ", channel name: " + directory.channelInfo()._channelConfig.name)
			        	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommBaseImpl.instanceName());
						_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
					}
				}
				else
				{
					if(_mismatchServiceSet == null)
					{
						_mismatchServiceSet = new HashSet<String>();
					}
					
					_mismatchServiceSet.add(sessionDirectory.serviceName());
				}
			}
			else
			{
				directory.generatedServiceId(sessionDirectory.service().serviceId());
				
				/* Adds this SessionChannelInfo to the SessionDirectory for this service if it is not added yet.*/
				Iterator<SessionChannelInfo<T>> it = sessionDirectory.sessionChannelList().iterator();
				boolean isAdded = false;
				
				while(it.hasNext())
				{
					if(sessionChannelInfo == it.next())
					{
						isAdded = true;
						break;
					}
				}
				
				if(!isAdded)
					sessionDirectory.sessionChannelList().add(sessionChannelInfo);
				
				/* Recovers the pending requests after the consumer session initialization */
				if(_isConsumerSessionInitialized)
				{
					sessionDirectory.handlePendingRequests(sessionChannelInfo, sessionDirectory.service());
				
					handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) sessionDirectory);
				
					/* Recover items in the item recovery queue if any. */
					nextDispatchTime(1000);
				}
			}
		}
		
		if (_ommBaseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _ommBaseImpl.strBuilder();
			temp.append("Session name: " + sessionChannelInfo.sessionChannelConfig().name).append(OmmLoggerClient.CR)
        	.append("    onServiceAdd for " + directory).append(OmmLoggerClient.CR)
			.append("    Instance Name ").append(_ommBaseImpl.instanceName());
			_ommBaseImpl.loggerClient().trace(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
		}
	}

	@Override
	public void onServiceDelete(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory) {

		if (_ommBaseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _ommBaseImpl.strBuilder();
			temp.append("Session name: " + sessionChannelInfo.sessionChannelConfig().name).append(OmmLoggerClient.CR)
        	.append("    onServiceDelete for " + directory).append(OmmLoggerClient.CR)
			.append("    Instance Name ").append(_ommBaseImpl.instanceName());
			_ommBaseImpl.loggerClient().trace(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
		}
		
		SessionDirectory<T> sessionDirectory = _sessionDirByName.get(directory.serviceName());
		
		if(sessionDirectory != null)
		{
			sessionDirectory.sessionChannelList().remove(sessionChannelInfo);
			
			if(sessionDirectory.sessionChannelList().size() == 0)
			{
				sessionDirectory.service().action(MapEntryActions.DELETE);
				sessionDirectory.service().flags(0);
				sessionDirectory.originalFlags(0);
				_sendDirectoryResponse = true;
				sessionDirectory.isUpdated(true);
				
				if(sessionChannelInfo.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					sessionChannelInfo.onServiceDown(directory.serviceName());
				}
			}
		}
	}
	
	@SuppressWarnings("unchecked")
	@Override
	public void onServiceInfoChange(SessionChannelInfo<T> sessionChannelInfo, Directory<T> directory,
			ServiceInfo serviceInfo) 
	{
		String serviceName = directory.serviceName();
		
		SessionDirectory<T> sessionDirectory = _sessionDirByName.get(serviceName);
		
		if(sessionDirectory != null)
		{
			sessionDirectory.handlePendingRequests(sessionChannelInfo, sessionDirectory.service());
			
			handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) sessionDirectory);
			
			/* Recover items in the item recovery queue if any. */
			nextDispatchTime(1000);
			
			if (_ommBaseImpl.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _ommBaseImpl.strBuilder();
				temp.append("Session name: " + sessionChannelInfo.sessionChannelConfig().name).append(OmmLoggerClient.CR)
	        	.append("    onServiceInfoChange for " + directory).append(OmmLoggerClient.CR)
				.append("    Instance Name ").append(_ommBaseImpl.instanceName());
				_ommBaseImpl.loggerClient().trace(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
			}
		}
	}
	
	private boolean translateUserServiceId(SessionChannelInfo<T> sessionChannelInfo, com.refinitiv.eta.codec.MsgKey msgKey)
	{
		SessionDirectory<T> sessionDirectory = _sessionDirById.get(msgKey.serviceId());
		
		if(sessionDirectory != null)
		{
			/* Search from the generated service Id */
			Directory<T> directory = sessionChannelInfo.getDirectoryByName(sessionDirectory.serviceName());
			
			if(directory != null && directory.service().action() != MapEntryActions.DELETE)
			{
				/* Translate to the actual service ID from the provider */
				msgKey.serviceId(directory.service().serviceId());
				return true;
			}
		}
		
		return false;
	}

	public boolean validateServiceName(SessionChannelInfo<T> sessionChannelInfo, com.refinitiv.eta.codec.PostMsg rsslPostMsg, String serviceName)
	{
		if(serviceName != null)
		{
			Directory<T> directory = sessionChannelInfo.getDirectoryByName(serviceName);
			
			if(directory != null && directory.service().action() != MapEntryActions.DELETE)
			{	
				return true;
			}
			
			if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _ommBaseImpl.strBuilder();
				
	        	temp.append("The specified service name " + serviceName + " does not exist for " + sessionChannelInfo.sessionChannelConfig().name + ". Droping this PosgMsg.")
	        		.append(OmmLoggerClient.CR);
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
		}
		else if (rsslPostMsg.checkHasMsgKey() && rsslPostMsg.msgKey().checkHasServiceId())
		{
			if(translateUserServiceId(sessionChannelInfo, rsslPostMsg.msgKey()))
			{
				return true;
			}
			
			if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _ommBaseImpl.strBuilder();
				
	        	temp.append("The specified service Id " + rsslPostMsg.msgKey().serviceId() + " does not exist for " + sessionChannelInfo.sessionChannelConfig().name + ". Droping this PosgMsg.")
	        		.append(OmmLoggerClient.CR);
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
		}
		else
		{
			if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _ommBaseImpl.strBuilder();
				
	        	temp.append("Niether service Id or name is specified for the PostMsg. Droping this PosgMsg from "  + sessionChannelInfo.sessionChannelConfig().name + ".")
	        		.append(OmmLoggerClient.CR);
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
		}
		
		return false;
	}
	
	public boolean checkServiceId(SessionChannelInfo<T> sessionChannelInfo, com.refinitiv.eta.codec.GenericMsg rsslGenericMsg)
	{
		if (rsslGenericMsg.checkHasMsgKey() && rsslGenericMsg.msgKey().checkHasServiceId())
		{			
			return translateUserServiceId(sessionChannelInfo, rsslGenericMsg.msgKey());
		}
		
		return false;
	}
	
	void populateStatusMsg()
	{		
		_rsslStatusMsg.clear();
		_rsslStatusMsg.msgClass(MsgClasses.STATUS);
		_rsslStatusMsg.streamId(1);
		_rsslStatusMsg.domainType(DomainTypes.LOGIN);
		_rsslStatusMsg.applyHasMsgKey();
		MsgKey msgKey = _rsslStatusMsg.msgKey();
		msgKey.clear();
		
		if(_loginRefresh.checkHasUserNameType())
		{
			msgKey.applyHasNameType();
			msgKey.nameType(_loginRefresh.userNameType());
		}
		
		if(_loginRefresh.checkHasUserName())
		{
			msgKey.applyHasName();
			msgKey.name(_loginRefresh.userName());
		}
	}
	
	/* Fan-out all pending and recovering items */
	@SuppressWarnings("unchecked")
	public void close()
	{
		_watchlist.close();
		
		for(SessionDirectory<T> sessionDirectory : _sessionDirByName.values())
		{
			sessionDirectory.close();
		}
		
		if(_serviceListMap != null )
		{
			Iterator<ServiceListImpl> serviceListIt = _serviceListMap.values().iterator();
			
			ServiceListImpl serviceListImpl;
			while(serviceListIt.hasNext())
			{
				serviceListImpl = serviceListIt.next();
				
				if(serviceListImpl.pendingQueueSize() > 0)
				{
					serviceListImpl.close((ConsumerSession<OmmConsumerClient>) this);
				}
			}
			
			_serviceListMap.clear();
		}
		
		/* Clears all SessionDirectory mapping */
		_sessionDirByName.clear();
		_sessionDirById.clear();
		_removedSessionDirSet.clear();
	}
	
	public void currentLoginDataState(int dataState)
	{
		_currentRsslDataState = dataState;	
	}
}
