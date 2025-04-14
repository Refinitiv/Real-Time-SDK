///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024-2025 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
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
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginAttribFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginSupportFeaturesFlags;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;
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

class ScheduleCloseSeesinChannel<T> implements TimeoutClient
{
	private ConsumerSession<T>			_consumerSession;
	private SessionChannelInfo<T>	_sessionChnanelInfo;
	private ReentrantLock _userLock;
	private ChannelInfo _channelInfo;
	
	ScheduleCloseSeesinChannel(ConsumerSession<T> consumerSession, SessionChannelInfo<T>	sessionChannelInfo, Directory<T> directory)
	{
		_consumerSession = consumerSession;
		_sessionChnanelInfo = sessionChannelInfo;
		_userLock = _consumerSession.ommBaseImpl().userLock();
		_channelInfo = directory.channelInfo();
	}

	@Override
	public void handleTimeoutEvent() 
	{	
		if(_sessionChnanelInfo != null)
		{
			/* Keep the current size before SessionChannelInfo is removed from closeSessionChannel() */
			int size = _consumerSession.sessionChannelList().size();
			
			/* Closes ReactorChannel and removes SessionChannelInfo */
			_consumerSession.ommBaseImpl().closeSessionChannelOnly(_sessionChnanelInfo);
			
			_consumerSession.handleLoginStreamForChannelDown(_consumerSession.ommBaseImpl().loginCallbackClient().loginItemList(), _sessionChnanelInfo.reactorChannel(), size);
			
			_consumerSession.ommBaseImpl()._channelCallbackClient.removeSessionChannel(_sessionChnanelInfo);
			
			_sessionChnanelInfo.onChannelClose(_channelInfo);
			
			_sessionChnanelInfo = null;
		}
		
		_consumerSession = null;
	}

	@Override
	public ReentrantLock userLock() {
		return _userLock;
	}
}

class ConsumerSession<T> implements DirectoryServiceClient<T>
{
	
	private static final String CLIENT_NAME = "ConsumerSession";
	
	private static final int MSG_BUFFER_SIZE 	= 8192;
	
	// Login section
	private OmmBaseImpl<T>			_ommBaseImpl;
	private ActiveConfig			_activeConfig;
	
	private List<SessionChannelInfo<T>> 		_sessionChannelList;
	private int                     _numOfLoginOk = 0;
	private int						_numOfLoginClose = 0;
	private LoginRefresh 			_loginRefresh; // login refresh aggregation for ConsumerSession
	
	LinkedHashSet<String>           _serviceList; // Keeps a list of concrete service name from all connections.
	
	int								_generateServiceId; // This is used to generate an unique service Id for service name for source directory response
	
	private DirectoryMsg 			_directoryMsg;
	
	private Map<String, SessionDirectory<T>>		_sessionDirByName;
	
	private Map<Integer, SessionDirectory<T>>		_sessionDirById;
	
	private Buffer 					_rsslEncBuffer;
	
	private boolean 				_sendDirectoryResponse;
	
	private List<SessionDirectory<T>> _removeSessionDirList; // This is temporary list to remove SessionDirectory
	
	private boolean 				_sendInitialLoginRefresh;
	
	private SessionWatchlist<T> _watchlist;
	
	private boolean					_enableSingleOpen = false;
	
	private Map<String, ServiceListImpl> 	_serviceListMap;
	
	private ConsumerSessionTimeOut<T>		_dispatchTimeout;
	
	private int 							_state = OmmBaseImpl.OmmImplState.NOT_INITIALIZED; /* This is used to handle the current state of OmmConsumer */
	
	private State						_rsslState; /* This is used to set the state of login status message for the login stream */

	com.refinitiv.eta.codec.StatusMsg   _rsslStatusMsg; /* This is used to set the state of login status message for the login stream */
	
	private StatusMsgImpl				_statusMsgImpl; /* This is used to set login status message for the login stream */
	
	private OmmEventImpl<T>				_eventImpl;
	
	/* This is used to download data dictionary from network */
	private ChannelDictionary<T>		_channelDictionary;

	private int _currentRsslDataState;

	ConsumerSession(OmmBaseImpl<T> baseImpl, Map<String, ServiceListImpl> serviceListMap)
	{
		_ommBaseImpl = baseImpl;
		_ommBaseImpl.consumerSession(this);
		_activeConfig = _ommBaseImpl.activeConfig();
		_sessionChannelList = new ArrayList<SessionChannelInfo<T>>();
		
		_loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
		_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		
		int initialHashSize =  (int)(_ommBaseImpl.activeConfig().serviceCountHint/ 0.75 + 1);
		_serviceList = new LinkedHashSet<String>(initialHashSize);
		_sessionDirByName = new LinkedHashMap<String, SessionDirectory<T>>(initialHashSize);
		_sessionDirById = new LinkedHashMap<Integer, SessionDirectory<T>>(initialHashSize);
		
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
	
	void ommImplState(int state)
	{
		_state = state;
	}
	
	int ommImplState()
	{
		return _state;
	}
	
	/* timeout is in microsecond to dispatch events from ConsumerSession */
	void nextDispatchTime(int timeout)
	{
		_dispatchTimeout.installTimeout(timeout);
	}
	
	 OmmBaseImpl<T> ommBaseImpl()
	 {
		 return _ommBaseImpl;
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
	
	void addSessionChannelInfo(SessionChannelInfo<T> sessionChannelInfo)
	{
		if(!_sessionChannelList.contains(sessionChannelInfo))
		{
			_sessionChannelList.add(sessionChannelInfo);
		}
	}
	
	void removeSessionChannelInfo(SessionChannelInfo<T> sessionChannelInfo)
	{
		if(_sessionChannelList.contains(sessionChannelInfo))
		{
			_sessionChannelList.remove(sessionChannelInfo);
		}
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
	
	public void handleLoginReqTimeout()
	{
		if (_activeConfig.loginRequestTimeOut == 0)
		{
			while (ommImplState() < OmmImplState.LOGIN_STREAM_OPEN_OK && ommImplState() != OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN
					&& ommImplState() != OmmImplState.RSSLCHANNEL_CLOSED)
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
			
			/* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
			if(ommImplState() == OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN)
			{
				throw _ommBaseImpl.ommIUExcept().message(_ommBaseImpl._loginCallbackClient.loginFailureMessage(), OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_REJECTED);
			}
			
			if(numOfLoginOk() > 0)
			{
				SessionChannelInfo<T> sessionChannelInfo = aggregateLoginResponse();
				
				_ommBaseImpl._loginCallbackClient.processRefreshMsg(null, sessionChannelInfo.reactorChannel(), loginRefresh());
				
				sendInitialLoginRefresh(true);
				
				checkLoginResponseAndCloseReactorChannel();
				
				return;
			}
			else
			{
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("login failed (timed out after waiting ").append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				int count = _activeConfig.configSessionChannelSet.size();
				for(SessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}	
				

				String excepText = strBuilder.toString();
	
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			}
		}
		else
		{
			_ommBaseImpl.resetEventTimeout();
			TimeoutEvent timeoutEvent = _ommBaseImpl.addTimeoutEvent(_activeConfig.loginRequestTimeOut * 1000, _ommBaseImpl);
	
			while (!_ommBaseImpl.eventTimeout() && (ommImplState() < OmmImplState.LOGIN_STREAM_OPEN_OK) && (ommImplState() != OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN)
					&& ommImplState() != OmmImplState.RSSLCHANNEL_CLOSED)
			{
				_ommBaseImpl.rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
			}
	
			if (_ommBaseImpl.eventTimeout())
			{
				if(numOfLoginOk() > 0 && isInitialLoginRefreshSent() == false)
				{
					SessionChannelInfo<T> sessionChannelInfo = aggregateLoginResponse();
					
					_ommBaseImpl._loginCallbackClient.processRefreshMsg(null, sessionChannelInfo.reactorChannel(), loginRefresh());
				
					sendInitialLoginRefresh(true);
					
					checkLoginResponseAndCloseReactorChannel();
				
					return;
				}
				
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("login failed (timed out after waiting ").append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				int count = _activeConfig.configSessionChannelSet.size();
				for(SessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}	
				

				String excepText = strBuilder.toString();
	
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			}
			else if (ommImplState() == OmmImplState.RSSLCHANNEL_CLOSED) /* Throws OmmInvalidUsageException when all session channels are down. */
			{
				timeoutEvent.cancel();
				
				StringBuilder strBuilder = _ommBaseImpl.strBuilder().append("login failed (timed out after waiting ").append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				int count = _activeConfig.configSessionChannelSet.size();
				for(SessionChannelConfig  config : _activeConfig.configSessionChannelSet)
				{
					if(--count > 0)
						strBuilder.append(config.name + ", ");
					else
						strBuilder.append(config.name);
				}
				
				String excepText = strBuilder.toString();
				
				if (_ommBaseImpl.loggerClient().isErrorEnabled())
					_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
				
				throw _ommBaseImpl.ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			}
			else if (ommImplState() == OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN) /* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
			{
				timeoutEvent.cancel();
				throw _ommBaseImpl.ommIUExcept().message(_ommBaseImpl._loginCallbackClient.loginFailureMessage(), OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_REJECTED);
			}
			else
			{
				timeoutEvent.cancel();
				
				/* This is used to notify login refresh after receiving source directory response */
				if(numOfLoginOk() > 0 && isInitialLoginRefreshSent() == false)
				{
					SessionChannelInfo<T> sessionChannelInfo = aggregateLoginResponse();
					
					sendInitialLoginRefresh(true);
					
					_ommBaseImpl._loginCallbackClient.processRefreshMsg(null, sessionChannelInfo.reactorChannel(), loginRefresh());
					
					checkLoginResponseAndCloseReactorChannel();
				
					return;
				}
			}
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
				for(SessionChannelConfig  config : _activeConfig.configSessionChannelSet)
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
				for(SessionChannelConfig  config : _activeConfig.configSessionChannelSet)
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
		Iterator<SessionDirectory<T>> sessionDirIt =  _sessionDirByName.values().iterator();
		
		List<SessionChannelInfo<T>> tempChannelInfoList;
		
		while(sessionDirIt.hasNext())
		{
			SessionDirectory<T> sessionDir = sessionDirIt.next();
			
			Iterator<SessionChannelInfo<T>> sessionChannelInfoIt;
			
			tempChannelInfoList = new ArrayList<SessionChannelInfo<T>>();
			
			for(SessionChannelInfo<T> channelInfo : _sessionChannelList)
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
	}
	
	public List<SessionChannelInfo<T>> sessionChannelList()
	{
		return _sessionChannelList;
	}
	
	void increaseNumOfLoginOk()
	{
		_numOfLoginOk++;
	}
	
	int numOfLoginOk()
	{
		return _numOfLoginOk;
	}
	
	void increaseNumOfLoginClose()
	{
		_numOfLoginClose++;
	}
	
	void sendInitialLoginRefresh(boolean vaue)
	{
		_sendInitialLoginRefresh = vaue;
	}
	
	boolean isInitialLoginRefreshSent()
	{
		return _sendInitialLoginRefresh;
	}

	public SessionChannelInfo<T> aggregateLoginResponse()
	{
		_loginRefresh.clear();
		SessionChannelInfo<T> firstLoginResponse = null;
		
		int originalAttribFlags = 0;
		int originalFeaturesFlags = 0;		
		
		int attribFlags = 0;
		int featuresFlags = 0;
		
		for(int index = 0; index < _sessionChannelList.size(); index++)
		{
			SessionChannelInfo<T> sessionChannel = _sessionChannelList.get(index);
			
			if(sessionChannel.loginRefresh().flags() > 0)
			{
				if(firstLoginResponse == null)
				{
					originalAttribFlags = sessionChannel.loginRefresh().attrib().flags();
					originalFeaturesFlags = sessionChannel.loginRefresh().features().flags();
					
					attribFlags = sessionChannel.loginRefresh().attrib().flags();
					featuresFlags = sessionChannel.loginRefresh().features().flags();
					
					firstLoginResponse = sessionChannel;
				}
				else
				{
					attribFlags &= sessionChannel.loginRefresh().attrib().flags();
					featuresFlags &= sessionChannel.loginRefresh().features().flags();
				}
			}
		}
		
		firstLoginResponse.loginRefresh().attrib().flags(attribFlags);
		firstLoginResponse.loginRefresh().features().flags(featuresFlags);
		
		/* Aggregate login response attributes and features from all session channels */
		for(int index = 1; index < _sessionChannelList.size(); index++)
		{
			SessionChannelInfo<T> sessionChannel = _sessionChannelList.get(index);
			
			if( (attribFlags & LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA) != 0)
			{
				long allowSuspectData = firstLoginResponse.loginRefresh().attrib().allowSuspectData();
				allowSuspectData &= sessionChannel.loginRefresh().attrib().allowSuspectData();
				
				firstLoginResponse.loginRefresh().attrib().allowSuspectData(allowSuspectData);
			}
			
			if( (attribFlags & LoginAttribFlags.HAS_PROVIDE_PERM_EXPR) != 0)
			{
				long providePermExpr = firstLoginResponse.loginRefresh().attrib().providePermissionExpressions();
				providePermExpr &= sessionChannel.loginRefresh().attrib().providePermissionExpressions();
				
				firstLoginResponse.loginRefresh().attrib().providePermissionExpressions(providePermExpr);
			}
			
			if( (attribFlags & LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE) != 0)
			{
				long providePermProfile = firstLoginResponse.loginRefresh().attrib().providePermissionProfile();
				providePermProfile &= sessionChannel.loginRefresh().attrib().providePermissionProfile();
				
				firstLoginResponse.loginRefresh().attrib().providePermissionProfile(providePermProfile);
			}
			

			if( (attribFlags & LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT) != 0)
			{
				long supportRTT = firstLoginResponse.loginRefresh().attrib().supportRTTMonitoring();
				supportRTT &= sessionChannel.loginRefresh().attrib().supportRTTMonitoring();
			
				firstLoginResponse.loginRefresh().attrib().supportRTTMonitoring(supportRTT);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS) != 0)
			{
				long batchRequests = firstLoginResponse.loginRefresh().features().supportBatchRequests();
				batchRequests &= sessionChannel.loginRefresh().features().supportBatchRequests();
				
				firstLoginResponse.loginRefresh().features().supportBatchRequests(batchRequests);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_POST) != 0)
			{
				long supportPost = firstLoginResponse.loginRefresh().features().supportOMMPost();
				supportPost &= sessionChannel.loginRefresh().features().supportOMMPost();
				
				firstLoginResponse.loginRefresh().features().supportOMMPost(supportPost);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE) != 0)
			{
				long supportOPTPause = firstLoginResponse.loginRefresh().features().supportOptimizedPauseResume();
				supportOPTPause &= sessionChannel.loginRefresh().features().supportOptimizedPauseResume();
				
				firstLoginResponse.loginRefresh().features().supportOptimizedPauseResume(supportOPTPause);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW) != 0)
			{
				long supportView = firstLoginResponse.loginRefresh().features().supportViewRequests();
				supportView &= sessionChannel.loginRefresh().features().supportViewRequests();
				
				firstLoginResponse.loginRefresh().features().supportViewRequests(supportView);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES) != 0)
			{
				long batchReissue = firstLoginResponse.loginRefresh().features().supportBatchReissues();
				batchReissue &= sessionChannel.loginRefresh().features().supportBatchReissues();
				
				firstLoginResponse.loginRefresh().features().supportBatchReissues(batchReissue);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES) != 0)
			{
				long batchClose = firstLoginResponse.loginRefresh().features().supportBatchCloses();
				batchClose &= sessionChannel.loginRefresh().features().supportBatchCloses();
				
				firstLoginResponse.loginRefresh().features().supportBatchCloses(batchClose);
			}
			
			if((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL) != 0)
			{
				long supportEHN_SL = firstLoginResponse.loginRefresh().features().supportEnhancedSymbolList();
				supportEHN_SL &= sessionChannel.loginRefresh().features().supportEnhancedSymbolList();
			
				firstLoginResponse.loginRefresh().features().supportEnhancedSymbolList(supportEHN_SL);
			}
		}
		
		/* Makes sure that only supported attributes and features are provided to users */
		attribFlags &= (LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA | LoginAttribFlags.HAS_PROVIDE_PERM_EXPR | LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE | LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT);
		featuresFlags &= (LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS | LoginSupportFeaturesFlags.HAS_SUPPORT_POST | LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE |
				LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW| LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES | LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES | LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL);
		
		/* Always enables the single open and allow suspect data flags */
		attribFlags |= (LoginAttribFlags.HAS_SINGLE_OPEN | LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA);
		firstLoginResponse.loginRefresh().attrib().flags(attribFlags);
		firstLoginResponse.loginRefresh().features().flags(featuresFlags);
		
		/* Copy to the aggregated login refresh message */
		firstLoginResponse.loginRefresh().copy(_loginRefresh);
		
		/* Always enable the single open feature */
		if(_enableSingleOpen)	
		{
			/* Overrides the single open as it is handled by EMA */
			_loginRefresh.attrib().allowSuspectData(1);
			_loginRefresh.attrib().singleOpen(1);
		}
		
		firstLoginResponse.loginRefresh().attrib().flags(originalAttribFlags);
		firstLoginResponse.loginRefresh().features().flags(originalFeaturesFlags);
		
		return firstLoginResponse;
	}
	
	public LoginRefresh loginRefresh()
	{
		return _loginRefresh;
	}
	
	void processDirectoryPayload(List<Service> serviceList, ReactorChannel reactorChannel)
	{
		_sendDirectoryResponse = false; 
		
		ChannelInfo channelInfo = (ChannelInfo)reactorChannel.userSpecObj();
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
		
		if(_removeSessionDirList == null)
		{
			_removeSessionDirList = new ArrayList<SessionDirectory<T>>();
		}
		else
		{
			_removeSessionDirList.clear();
		}
		
		while(it.hasNext())
		{
			sessionDirectory = it.next();
			
			if(sessionDirectory.sessionChannelList().size() == 0)
			{
				_removeSessionDirList.add(sessionDirectory);
				continue;
			}
		}
		
		for(SessionDirectory<T> entry : _removeSessionDirList)
		{
			_sessionDirByName.remove(entry.serviceName());
			_sessionDirById.remove(entry.service().serviceId());
			_serviceList.remove(entry.serviceName());
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
				Service service;
				
				if(_sessionDirByName.entrySet().size() > 0)
				{
					boolean isUpdated = false;
					while(it.hasNext())
					{
						entry = it.next();

						if(isUpdated || isInitalRequest)
						{
							isUpdated = true;
							service = entry.getValue().service();
					
							// Adjusts the service flags according to the request filter.
							service.flags(service.flags() & (int)item.filterId());
							directoryRefresh.serviceList().add(service);
						}
					}
					
					if(isUpdated == false)
					{
						return null; // There is no need to send source directory response.
					}
				}
			}
			else
			{
				// Send empty service list as the specified service name or Id doesn't exist in the source directory of all channels.
				
				if(!item.serviceName().isEmpty())
				{
					SessionDirectory<T> sessionDirectory = _sessionDirByName.get(item.serviceName());
					
					if(sessionDirectory != null)
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
					
					if(sessionDirectory != null)
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
				Service service;
				
				if(_sessionDirByName.entrySet().size() > 0)
				{
					boolean isUpdated = false;
					while(it.hasNext())
					{
						entry = it.next();
						
						if(entry.getValue().isUpdated())
						{
							isUpdated = true;
							service = entry.getValue().service();
							
							// Adjusts the service flags according to the request filter.
							service.flags(service.flags() & (int)item.filterId());
							directoryUpdate.serviceList().add(service);
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
					
					if(sessionDirectory != null && sessionDirectory.isUpdated())
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
					
					if(sessionDirectory != null && sessionDirectory.isUpdated())
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
				sessionDirectory.service().state().acceptingRequests(serviceState.serviceState());
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
			// Adds a concrete service name to the list
			_serviceList.add(directory.serviceName());
			
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
			
			handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) newSessionDirectory);
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
		}
		else
		{
			boolean result = compareServiceForAggregation(sessionDirectory, directory);
			
			if(result == false)
			{
				if (_ommBaseImpl.loggerClient().isTraceEnabled())
				{
					StringBuilder temp = _ommBaseImpl.strBuilder();
		        	temp.append("Failed to compare service for aggregation, closing session channel: " + sessionChannelInfo).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_ommBaseImpl.instanceName());
					_ommBaseImpl.loggerClient().trace(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
				}
				
				
				/* Schedule an timeout event to close this SessionChannelInfo outside of source directory callback */
				_ommBaseImpl.addTimeoutEvent(1, new ScheduleCloseSeesinChannel<T>(this, sessionChannelInfo, directory));
				
				/* Recover items in the item recovery queue if any. */
				nextDispatchTime(1000);
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
				
				sessionDirectory.handlePendingRequests(sessionChannelInfo, sessionDirectory.service());
				
				handlePendingRequestsForServiceList((SessionDirectory<OmmConsumerClient>) sessionDirectory);
				
				/* Recover items in the item recovery queue if any. */
				nextDispatchTime(1000); 
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
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.WARNING));
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
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.WARNING));
        	}
		}
		else
		{
			if (_ommBaseImpl.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _ommBaseImpl.strBuilder();
				
	        	temp.append("Niether service Id or name is specified for the PostMsg. Droping this PosgMsg from "  + sessionChannelInfo.sessionChannelConfig().name + ".")
	        		.append(OmmLoggerClient.CR);
	        	
	        	_ommBaseImpl.loggerClient().error(_ommBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.WARNING));
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
	
	/* This is used to check and close a ReactorChannel which doesn't provide a login response in time */
	public void checkLoginResponseAndCloseReactorChannel()
	{
		/* Closes channels if it doesn't receive a login response */
		if( (_numOfLoginOk + _numOfLoginClose) <  _sessionChannelList.size() )
		{
			List<SessionChannelInfo<T>> removeChannelList = new ArrayList<SessionChannelInfo<T>>();
			
			for(int i = 0; i < _sessionChannelList.size(); i++)
			{
				SessionChannelInfo<T> sessionChannelInfo = _sessionChannelList.get(i);
				
				if(!sessionChannelInfo.receivedLoginRefresh())
				{
					removeChannelList.add(sessionChannelInfo);
				}
			}
			
			for (int index = removeChannelList.size() -1; index >= 0; index--)
			{
				_ommBaseImpl.closeSessionChannel(removeChannelList.get(index));
			}
		}
	}
	
	public boolean checkAllSessionChannelHasState(int state)
	{
		int result = state;
		for(int i = 0; i < _sessionChannelList.size(); i++)
		{
			SessionChannelInfo<T> sessionChannelInfo = _sessionChannelList.get(i);
			
			result &= sessionChannelInfo.state();
		}
		
		return result == state ? true: false;
	}
	
	public boolean checkAtLeastOneSessionChannelHasState(int state)
	{
		for(int i = 0; i < _sessionChannelList.size(); i++)
		{
			SessionChannelInfo<T> sessionChannelInfo = _sessionChannelList.get(i);
			
			if( (sessionChannelInfo.state() & state) != 0)
				return true;
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
	
	void handleLoginStreamForChannelDown(List<LoginItem<T>>	loginItemList, ReactorChannel reactorChannel, int sessionListSize)
	{
		if (loginItemList == null)
			return;
		
		/* Checks whether this is the last channel being closed */
		boolean closedStream = sessionListSize == 1 ? true : false;
		
		populateStatusMsg();
		
		if(closedStream)
		{
			_rsslState.streamState(StreamState.CLOSED);
			
			_ommBaseImpl.ommImplState(OmmImplState.RSSLCHANNEL_CLOSED);
		}
		else
			_rsslState.streamState(StreamState.OPEN);
		
		
		if(_sendInitialLoginRefresh)
		{
			if(closedStream)
			{
				_rsslState.dataState(DataState.SUSPECT);
			}
			else
			{
				_rsslState.dataState(DataState.OK);
			}
			
		}
		else
		{
			_rsslState.dataState(DataState.SUSPECT);
		}

		_currentRsslDataState = _rsslState.dataState();

		_rsslState.code(StateCodes.NONE);
		_rsslState.text().data("session channel closed");
		_rsslStatusMsg.state(_rsslState);
		_rsslStatusMsg.applyHasState();
		
		_statusMsgImpl.decode(_rsslStatusMsg, reactorChannel.majorVersion(), reactorChannel.minorVersion(), null);
		
		for (int idx = 0; idx < loginItemList.size(); ++idx)
		{
			_eventImpl._item = loginItemList.get(idx);
			_eventImpl._channel = reactorChannel;
			
			((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
			((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
		}
	}
	
	public void processChannelEvent(SessionChannelInfo<T> sessionChannelInfo, ReactorChannelEvent event)
	{
		List<LoginItem<T>>	loginItemList = _ommBaseImpl.loginCallbackClient().loginItemList();
		
		if (loginItemList == null)
			return;
		
		_rsslState.clear();
		
		switch ( event.eventType() )
		{
		case ReactorChannelEventTypes.CHANNEL_UP:
			
			if(_sendInitialLoginRefresh)
				return;
			
			populateStatusMsg();
			
			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(DataState.SUSPECT);
			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("session channel up");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_currentRsslDataState = _rsslState.dataState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();
				
				((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}
			
			sessionChannelInfo.sendChannelUp = true;
			
			break;
		
		case ReactorChannelEventTypes.CHANNEL_READY:
			
			if(sessionChannelInfo.sendChannelUp)
				return;
			
			populateStatusMsg();
			
			_rsslState.streamState(StreamState.OPEN);
			
			if(_sendInitialLoginRefresh == true)
				_rsslState.dataState(DataState.OK);
			else
				_rsslState.dataState(DataState.SUSPECT);

			_currentRsslDataState = _rsslState.dataState();

			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("session channel up");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();
			
			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();
				
				((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}
			
			sessionChannelInfo.sendChannelUp = true;
			
			break;
			
		  case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			
			populateStatusMsg();
			
			_rsslState.streamState(StreamState.OPEN);
			
			
			if(_sendInitialLoginRefresh)
			{
				if(_sessionChannelList.size() == 1)
				{
					_rsslState.dataState(DataState.SUSPECT);
				}
				else
				{
					_rsslState.dataState(DataState.OK);
				}
				
			}
			else
			{
				_rsslState.dataState(DataState.SUSPECT);
			}

			_currentRsslDataState = _rsslState.dataState();

			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("session channel down reconnecting");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();
			
			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();
				
				((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}
			
			sessionChannelInfo.sendChannelUp = false;
			
			break;
		case ReactorChannelEventTypes.CHANNEL_DOWN:
			
			handleLoginStreamForChannelDown(loginItemList, event.reactorChannel(), _sessionChannelList.size());
			
			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK:

			populateStatusMsg();

			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(_currentRsslDataState);

			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("preferred host starting fallback");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);

			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();

				((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}

			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE:

			populateStatusMsg();

			_rsslState.streamState(StreamState.OPEN);
			_rsslState.dataState(_currentRsslDataState);

			_rsslState.code(StateCodes.NONE);
			_rsslState.text().data("preferred host complete");
			_rsslStatusMsg.state(_rsslState);
			_rsslStatusMsg.applyHasState();

			_statusMsgImpl.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);

			for (int idx = 0; idx < loginItemList.size(); ++idx)
			{
				_eventImpl._item = loginItemList.get(idx);
				_eventImpl._channel = event.reactorChannel();

				((OmmConsumerClient)_eventImpl._item.client()).onAllMsg(_statusMsgImpl, _eventImpl);
				((OmmConsumerClient) _eventImpl._item.client()).onStatusMsg(_statusMsgImpl, _eventImpl);
			}
			break;
		default:
			break;
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
	}

	public boolean hasActiveChannel()
	{
		for(SessionChannelInfo<T> sessionChannelInfo : _sessionChannelList)
		{
			if(sessionChannelInfo.reactorChannel() != null)
			{
				ReactorChannel.State state  = sessionChannelInfo.reactorChannel().state();
				if (state == ReactorChannel.State.READY || state == ReactorChannel.State.UP)
				{
					return true;
				}
			}
		}
		
		return false;
	}
}
