///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.OmmBaseImpl.OmmImplState;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.ema.access.OmmState.DataState;
import com.thomsonreuters.ema.access.OmmState.StreamState;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginAttrib;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;


class LoginCallbackClient<T> extends CallbackClient<T> implements RDMLoginMsgCallback
{
	private static final String CLIENT_NAME 	= "LoginCallbackClient";
	private static final int REFRESH_MSG_SIZE 	= 512;
	
	private List<ChannelInfo> 		_loginChannelList;
	private List<LoginItem<T>>		_loginItemList;
	private Buffer 					_rsslEncBuffer;
	private ReentrantLock 			_loginItemLock = new java.util.concurrent.locks.ReentrantLock();
	private boolean					_notifyChannelDownReconnecting;
	private State	_rsslState;
	
	LoginCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		 super(baseImpl, CLIENT_NAME);
		 _loginChannelList = new ArrayList<ChannelInfo>();
		 _notifyChannelDownReconnecting = false;
	}

	void initialize()
	{
		_baseImpl.activeConfig().rsslRDMLoginRequest.streamId(1);
		
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			temp.append("RDMLogin request message was populated with this info: ")
										.append(OmmLoggerClient.CR)
										.append(_baseImpl.activeConfig().rsslRDMLoginRequest.toString());
										
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, 
					temp.toString(),Severity.TRACE).toString());
		}
	}
	
	@Override
	public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
	{
		Msg msg = event.msg();
		LoginMsg loginMsg = event.rdmLoginMsg();
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		ReactorChannel rsslReactorChannel  = event.reactorChannel();

		if (loginMsg == null)
		{
			_baseImpl.closeRsslChannel(rsslReactorChannel);

			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = event.errorInfo().error();
				
	        	StringBuilder temp = _baseImpl.strBuilder();
				
	        	temp.append("Received an event without RDMLogin message").append(OmmLoggerClient.CR)
					.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel != null ? rsslReactorChannel.reactor().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append("@").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
					.append("Error text ").append(error.text());

	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
			
			 return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		switch (loginMsg.rdmMsgType())
		{
			case REFRESH :
			{
				if (!_loginChannelList.contains(chnlInfo))
					_loginChannelList.add(chnlInfo);
					
				if (_rsslRefreshMsg == null)
				{
					_rsslRefreshMsg = (RefreshMsg)CodecFactory.createMsg();
					_rsslRefreshMsg.msgClass(MsgClasses.REFRESH);
					msg.copy(_rsslRefreshMsg, CopyMsgFlags.ALL_FLAGS);
				}
				else
				{
					_rsslRefreshMsg.clear();
					_rsslRefreshMsg.msgClass(MsgClasses.REFRESH);
					msg.copy(_rsslRefreshMsg, CopyMsgFlags.ALL_FLAGS);
				}
				
				com.thomsonreuters.upa.codec.State state = ((LoginRefresh)loginMsg).state();
	
				boolean closeChannel = false;
	
				if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
				{
					closeChannel = true;
	
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMLogin stream was closed with refresh message")
			        		.append(OmmLoggerClient.CR);
			        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
			        	temp.append(OmmLoggerClient.CR)
			    			.append("State: ").append(state.toString());
			        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
				}
				else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMLogin stream state was changed to suspect with refresh message")
			        		.append(OmmLoggerClient.CR);
			        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
			        	temp.append(OmmLoggerClient.CR)
			    			.append("State: ").append(state.toString());
			        	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					if (_baseImpl.ommImplState() >= OmmImplState.RSSLCHANNEL_UP )
						_baseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
				}
				else
				{
					_baseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_OK);
	
					if (_baseImpl.loggerClient().isTraceEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMLogin stream was open with refresh message").append(OmmLoggerClient.CR)
			        		.append(loginMsg.toString()).append(OmmLoggerClient.CR)
							.append("State: ").append(state.toString());
			        	
			        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		        	}
				}
	
				processRefreshMsg(msg, rsslReactorChannel, loginMsg);
	
				if (closeChannel)
					_baseImpl.closeRsslChannel(rsslReactorChannel);
	
				break;
			}
			case STATUS:
			{
				boolean closeChannel = false;
	
				if (((LoginStatus)loginMsg).checkHasState())
		    	{
					com.thomsonreuters.upa.codec.State state =((LoginStatus)loginMsg).state();
	
					if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
					{
						closeChannel = true;
						
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("RDMLogin stream was closed with status message")
				        		.append(OmmLoggerClient.CR);
				        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
				        	temp.append(OmmLoggerClient.CR)
				    			.append("State: ").append(state.toString());
				        	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
					}
					else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
					{
						if (_baseImpl.loggerClient().isWarnEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("RDMLogin stream state was changed to suspect with status message")
				        		.append(OmmLoggerClient.CR);
				        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
				        	temp.append(OmmLoggerClient.CR)
				    			.append("State: ").append(state.toString());
				        	
				        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
			        	}
					
						if (_baseImpl.ommImplState() >= OmmImplState.RSSLCHANNEL_UP )
							_baseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
					}
					else
					{
						if (_baseImpl.loggerClient().isTraceEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("RDMLogin stream was open with status message").append(OmmLoggerClient.CR)
				        		.append(loginMsg.toString()).append(OmmLoggerClient.CR)
								.append("State: ").append(state.toString());
				        	
				        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
			        	}
						
						_baseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_OK);
					}
				}
				else
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("Received RDMLogin status message without the state").append(OmmLoggerClient.CR)
			        		.append(loginMsg.toString());
			        	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
				}
	
				processStatusMsg(msg, rsslReactorChannel, loginMsg);
	
				if (closeChannel)
					_baseImpl.closeRsslChannel(rsslReactorChannel);
	
				break;
			}
			default:
			{
				if (msg.msgClass() == MsgClasses.GENERIC)
				{
					processGenericMsg(msg, rsslReactorChannel, event);
					break;
				}
				
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _baseImpl.strBuilder();
					
		        	temp.append("Received unknown RDMLogin message type")
		        		.append(OmmLoggerClient.CR)
						.append("Message type value ").append(loginMsg.rdmMsgType());
	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
				break;
			}
		
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	int processRefreshMsg(Msg rsslMsg, ReactorChannel rsslReactorChannel, LoginMsg loginMsg)
	{
		if (_loginItemList == null)
			return ReactorCallbackReturnCodes.SUCCESS;
		
		if (rsslMsg != null)
		{
			if (_refreshMsg == null)
				_refreshMsg = new RefreshMsgImpl(_baseImpl._objManager);
				
			_refreshMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
		}
		else
		{
			if (convertRdmLoginToRsslBuffer(rsslReactorChannel, loginMsg) != ReactorCallbackReturnCodes.SUCCESS)
				return ReactorCallbackReturnCodes.SUCCESS;

			_refreshMsg.decode(_rsslEncBuffer, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null, null);
		}
		
		_loginItemLock.lock();
		
		int itemSize = _loginItemList.size();
		for (int idx = 0; idx < itemSize; ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);
			
			notifyOnAllMsg(_refreshMsg);
			notifyOnRefreshMsg();
		}

		if (((LoginRefresh)loginMsg).state().streamState() != StreamStates.OPEN)
		{
			for (int idx = 0; idx < itemSize; ++idx)
				_loginItemList.get(idx).remove();
			
			_loginItemList.clear();
		}
			
		_loginItemLock.unlock();
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	int processStatusMsg(Msg rsslMsg, ReactorChannel rsslReactorChannel, LoginMsg loginMsg)
	{
		if (_loginItemList == null)
			return ReactorCallbackReturnCodes.SUCCESS;
		
		if (rsslMsg != null)
		{
			if (_statusMsg == null)
				_statusMsg = new StatusMsgImpl(_baseImpl._objManager);
				
			_statusMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
		}
		else
		{
			if (convertRdmLoginToRsslBuffer(rsslReactorChannel, loginMsg) != ReactorCallbackReturnCodes.SUCCESS)
				return ReactorCallbackReturnCodes.SUCCESS;

			_statusMsg.decode(_rsslEncBuffer, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null, null);
		}
		
		_loginItemLock.lock();
		
		int itemSize = _loginItemList.size();
		for (int idx = 0; idx < itemSize; ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);
			
			notifyOnAllMsg(_statusMsg);
			notifyOnStatusMsg();
		}

		if (((LoginStatus)loginMsg).checkHasState() && ((LoginStatus)loginMsg).state().streamState() != StreamStates.OPEN)
		{
			for (int idx = 0; idx < itemSize; ++idx)
				_loginItemList.get(idx).remove();
			
			_loginItemList.clear();
		}
		
		_loginItemLock.unlock();
			
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	int processGenericMsg(Msg rsslMsg, ReactorChannel rsslReactorChannel, RDMLoginMsgEvent event)
	{
		if (_loginItemList == null)
			return ReactorCallbackReturnCodes.SUCCESS;
		
		if (_genericMsg == null)
			_genericMsg = new GenericMsgImpl();
		_genericMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(),
							((ChannelInfo)event.reactorChannel().userSpecObj()).rsslDictionary());
		
		_loginItemLock.lock();
		
		int itemSize = _loginItemList.size();
		for (int idx = 0; idx < itemSize; ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);
			
			notifyOnAllMsg(_genericMsg);
			notifyOnGenericMsg();
		}

		_loginItemLock.unlock();
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	int convertRdmLoginToRsslBuffer(ReactorChannel rsslChannel, LoginMsg loginMsg)
	{
	    if (_rsslEncBuffer == null)
        {
	    	_rsslEncBuffer = CodecFactory.createBuffer();
	    	_rsslEncBuffer.data(ByteBuffer.allocate(REFRESH_MSG_SIZE));
        }
        else
        {
        	ByteBuffer byteBuf = _rsslEncBuffer.data();
        	byteBuf.clear();
        	_rsslEncBuffer.data(byteBuf, 0, byteBuf.capacity()); 
        }
	     
	    EncodeIterator rsslEncIter = _baseImpl.rsslEncIter();
        rsslEncIter.clear();
        if (rsslEncIter.setBufferAndRWFVersion(_rsslEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion()) != CodecReturnCodes.SUCCESS)
        {
        	if (_baseImpl.loggerClient().isErrorEnabled())
        	{
        		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, 
	        			"Internal error. Failed to set encode iterator buffer in LoginCallbackClient.convertRdmLoginToRsslBuffer()",
	        			Severity.ERROR));
        	}
        	
        	return ReactorCallbackReturnCodes.FAILURE;
        }
        
        int ret = 0;
        if ((ret = loginMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
        {
        	if (_baseImpl.loggerClient().isErrorEnabled())
        	{
        		StringBuilder temp = _baseImpl.strBuilder();
				
	        	temp.append("Internal error: failed to encode LoginMsg in LoginCallbackClient.convertRdmLoginToRsslBuffer()")
	        		.append(OmmLoggerClient.CR)
	        		.append("Error num ").append(ret).append(OmmLoggerClient.CR)
	        		.append("Error Text ").append(CodecReturnCodes.toString(ret));
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, 
	        									temp.toString(), Severity.ERROR));
        	}
        
        	return ReactorCallbackReturnCodes.FAILURE;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	LoginRequest rsslLoginRequest()
	{
		return _baseImpl.activeConfig().rsslRDMLoginRequest;
	}
	
	List<ChannelInfo> 	loginChannelList()
	{
		return _loginChannelList;
	}
	
	List<LoginItem<T>> loginItemList()
	{
		return _loginItemList;
	}
	
	ChannelInfo activeChannelInfo()
	{
		if (_loginChannelList != null)
		{
			int numOfChannel = _loginChannelList.size();
			for ( int  idx = 0; idx < numOfChannel; ++idx )
			{
				ReactorChannel.State state  = _loginChannelList.get(idx).rsslReactorChannel().state();
				if (state == ReactorChannel.State.READY || state == ReactorChannel.State.UP)
					return _loginChannelList.get(idx);
			}
			
			return numOfChannel > 0 ? _loginChannelList.get(numOfChannel-1) : null;
		}
		
		return null;
	}
	
	RefreshMsg rsslRefreshMsg()
	{
		return _rsslRefreshMsg;
	}
	
	RefreshMsgImpl refreshMsg()
	{
		return _refreshMsg;
	}
	
	State rsslState()
	{
		if (_rsslState == null)
			_rsslState = CodecFactory.createState();
		else
			_rsslState.clear();
		
		return _rsslState;
	}
	
	void loginMsgToString(StringBuilder toString, LoginMsg msg, LoginMsgType loginType)
	{
		switch (loginType)
		{
			case REFRESH :
				LoginRefresh loginRefresh = (LoginRefresh)msg;
				LoginAttrib attrib = loginRefresh.checkHasAttrib() ? loginRefresh.attrib() : null;

				toString.append("username ").append(loginRefresh.checkHasUserName() ? loginRefresh.userName().toString() : "<not set>").append(OmmLoggerClient.CR)
						.append("usernameType ").append(loginRefresh.checkHasUserNameType() ? loginRefresh.userNameType() : "<not set>").append(OmmLoggerClient.CR);
				if (attrib == null)
					return;
				
				toString.append("position ").append(attrib.checkHasPosition() ? attrib.position() : "<not set>").append(OmmLoggerClient.CR)
				.append("appId ").append(attrib.checkHasApplicationId() ? attrib.applicationId() : "<not set>").append(OmmLoggerClient.CR)
				.append("applicationName ").append(attrib.checkHasApplicationName() ? attrib.applicationName().toString() : "<not set>").append(OmmLoggerClient.CR)
				.append("singleOpen ").append(attrib.checkHasSingleOpen() ? attrib.singleOpen() : "<not set>").append(OmmLoggerClient.CR)
				.append("allowSuspect ").append(attrib.checkHasAllowSuspectData() ? attrib.allowSuspectData() : "<not set>").append(OmmLoggerClient.CR)
				//.append("optimizedPauseResume ").append(attrib.c _pauseResume).append(OmmLoggerClient.CR)
				.append("permissionExpressions ").append(attrib.checkHasProvidePermissionExpressions() ? attrib.providePermissionExpressions() : "<not set>").append(OmmLoggerClient.CR)
				.append("permissionProfile ").append(attrib.checkHasProvidePermissionProfile()? attrib.providePermissionProfile() : "<not set>").append(OmmLoggerClient.CR);
				//.append("supportBatchRequest ").append(loginRefresh.c.c _supportBatchRequest).append(OmmLoggerClient.CR)
				//.append("supportEnhancedSymbolList ").append(attrib.c _supportEnhancedSymbolList).append(OmmLoggerClient.CR)
				//.append("supportPost ").append(_supportPost).append(OmmLoggerClient.CR)
				//.append("supportViewRequest ").append(_supportViewRequest);
				
				break;
			case STATUS :
				LoginStatus loginStatus = (LoginStatus)msg;
				
				toString.append("username ").append(loginStatus.checkHasUserName() ? loginStatus.userName().toString() : "<not set>").append(OmmLoggerClient.CR)
						.append("usernameType ").append(loginStatus.checkHasUserNameType() ? loginStatus.userNameType() : "<not set>").append(OmmLoggerClient.CR);
				break;
			default:  
				break;
		}
	}

	int sendLoginClose()
	{
		CloseMsg rsslCloseMsg = rsslCloseMsg();

		rsslCloseMsg.streamId(1);
		rsslCloseMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		rsslCloseMsg.domainType(DomainTypes.LOGIN);

		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
	    ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
	    rsslSubmitOptions.serviceName(null);
		rsslSubmitOptions.requestMsgOptions().clear();
	    
		for (ChannelInfo entry : _loginChannelList)
			entry.rsslReactorChannel().submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo);
		
		return _loginChannelList.size();
	}
	
	@SuppressWarnings("unchecked")
	LoginItem<T> loginItem(ReqMsg reqMsg , T client , Object closure)
	{
		if (_loginItemList == null)
			_loginItemList = new ArrayList<>();
		
		LoginItem<T> item;
		if ((item = (LoginItem<T>)_baseImpl._objManager._loginItemPool.poll()) == null)
		{
			item = new LoginItem<T>(_baseImpl, client, closure);
			_baseImpl._objManager._loginItemPool.updatePool(item);
		}
		else
		{
			item.reset(_baseImpl, client, closure, null);
		}
		
		item.loginChannelList(_loginChannelList);
		_loginItemList.add(item);
		
		_baseImpl.addTimeoutEvent(10, item);
		
		return item;
	}

	int processAckMsg(Msg rsslMsg ,  ChannelInfo channelInfo)
	{
		if (_ackMsg == null)
			_ackMsg = new AckMsgImpl(_baseImpl._objManager);
		
		_ackMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
	
		for (Item<T> loginItem : _loginItemList)
		{
			_eventImpl._item = loginItem;
			
			notifyOnAllMsg(_ackMsg);
			notifyOnAckMsg();
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	int processGenericMsg(Msg rsslMsg ,  ChannelInfo channelInfo)
	{
		if (_genericMsg == null)
			_genericMsg = new GenericMsgImpl(_baseImpl._objManager);
		
		_genericMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
	
		for (Item<T> loginItem : _loginItemList)
		{
			_eventImpl._item = loginItem;
			
			notifyOnAllMsg(_genericMsg);
			notifyOnGenericMsg();
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	
	void populateStatusMsg()
	{
		rsslStatusMsg(); 
		
		_rsslStatusMsg.streamId(1);
		_rsslStatusMsg.domainType(DomainTypes.LOGIN);
		
		if(_rsslRefreshMsg != null)
		{
			_rsslStatusMsg.applyHasMsgKey();
			MsgKey msgKey = _rsslStatusMsg.msgKey();
			msgKey.clear();
			msgKey.applyHasNameType();
			msgKey.nameType(_rsslRefreshMsg.msgKey().nameType());
			msgKey.applyHasName();
			msgKey.name(_rsslRefreshMsg.msgKey().name());
		}
	}
	
	void processChannelEvent(ReactorChannelEvent event)
	{
		if (_loginItemList == null)
			return;
		

		State state = rsslState();
		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl(_baseImpl._objManager);
		
		switch ( event.eventType() )
		{
		case ReactorChannelEventTypes.CHANNEL_READY:

			_baseImpl.reLoadDirectory();
			
			if(!_notifyChannelDownReconnecting)
				break;

			populateStatusMsg();
			
			state.streamState(StreamState.OPEN);
			state.dataState(DataState.OK);
			state.code(StateCodes.NONE);
			state.text().data("channel up");
			_rsslStatusMsg.state(state);
			_rsslStatusMsg.applyHasState();
			
			_statusMsg.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < _loginItemList.size(); ++idx)
			{
				_eventImpl._item = _loginItemList.get(idx);
				
				notifyOnAllMsg(_statusMsg);
				notifyOnStatusMsg();
			}
			
			_notifyChannelDownReconnecting = false;
			
			break;
			
		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			
			if(_notifyChannelDownReconnecting)
				break;
			
			populateStatusMsg();
			
			state.streamState(StreamState.OPEN);
			state.dataState(DataState.SUSPECT);
			state.code(StateCodes.NONE);
			state.text().data("channel down");
			_rsslStatusMsg.state(state);
			_rsslStatusMsg.applyHasState();
			
			_statusMsg.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < _loginItemList.size(); ++idx)
			{
				_eventImpl._item = _loginItemList.get(idx);
				
				notifyOnAllMsg(_statusMsg);
				notifyOnStatusMsg();
			}
			
			_notifyChannelDownReconnecting = true;
			
			break;
		case ReactorChannelEventTypes.CHANNEL_DOWN:
			populateStatusMsg();
			
			state.streamState(StreamState.CLOSED);
			state.dataState(DataState.SUSPECT);
			state.code(StateCodes.NONE);
			state.text().data("channel closed");
			_rsslStatusMsg.state(state);
			_rsslStatusMsg.applyHasState();
			
			_statusMsg.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);
			
			for (int idx = 0; idx < _loginItemList.size(); ++idx)
			{
				_eventImpl._item = _loginItemList.get(idx);
				
				notifyOnAllMsg(_statusMsg);
				notifyOnStatusMsg();
			}
			
			break;
		default:
			break;
		}
	}
}

class LoginCallbackClientConsumer extends LoginCallbackClient<OmmConsumerClient>
{
	LoginCallbackClientConsumer(OmmBaseImpl<OmmConsumerClient> baseImpl) {
		super(baseImpl);
	}
	
	@Override
	void notifyOnAllMsg(com.thomsonreuters.ema.access.Msg msg)
	{
		_eventImpl._item.client().onAllMsg(msg, _eventImpl);
	}
	
	@Override
    void notifyOnRefreshMsg()
	{
		_eventImpl._item.client().onRefreshMsg(_refreshMsg, _eventImpl);
	}
	
	@Override
	void notifyOnStatusMsg() 
	{
		_eventImpl._item.client().onStatusMsg(_statusMsg, _eventImpl);
	}
	
	@Override
	void notifyOnGenericMsg()
	{
		_eventImpl._item.client().onGenericMsg(_genericMsg, _eventImpl);
	} 
	
	@Override
	void notifyOnAckMsg()
	{
		_eventImpl._item.client().onAckMsg(_ackMsg, _eventImpl);
	}
}

class LoginCallbackClientProvider extends LoginCallbackClient<OmmProviderClient>
{
	LoginCallbackClientProvider(OmmBaseImpl<OmmProviderClient> baseImpl) {
		super(baseImpl);
	}
	
	@Override
	void notifyOnAllMsg(com.thomsonreuters.ema.access.Msg msg)
	{
		_eventImpl._item.client().onAllMsg(msg, _eventImpl);
	}
	
	@Override
    void notifyOnRefreshMsg()
	{
		_eventImpl._item.client().onRefreshMsg(_refreshMsg, _eventImpl);
	}
	
	@Override
	void notifyOnStatusMsg() 
	{
		_eventImpl._item.client().onStatusMsg(_statusMsg, _eventImpl);
	}
	
	@Override
	void notifyOnGenericMsg()
	{
		_eventImpl._item.client().onGenericMsg(_genericMsg, _eventImpl);
	} 	
}

class LoginItem<T> extends SingleItem<T> implements TimeoutClient
{
	private static final String 	CLIENT_NAME = "LoginItem";
	private List<ChannelInfo> 		_loginChannelList;
	
	LoginItem(OmmBaseImpl<T> baseImpl , T client , Object closure)
	{
		super(baseImpl, client, closure, null);
		_streamId = 1;
	}

	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> batchItem)
	{
		super.reset(baseImpl, client, closure, batchItem);
		_streamId = 1;
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		return submit(((ReqMsgImpl) reqMsg).rsslMsg());
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		return submit(((PostMsgImpl) postMsg).rsslMsg());
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return submit(((GenericMsgImpl) genericMsg).rsslMsg());
	}
	
	@Override
	boolean close()
	{
		remove();
		return true;
	}
	
	@Override
	public void handleTimeoutEvent()
	{
		LoginCallbackClient<T> loginCallbackClient = _baseImpl.loginCallbackClient();
		
		if (_loginChannelList.isEmpty())
			return;
		
		ReactorChannel rsslReactorChannel = _loginChannelList.get(0).rsslReactorChannel();
		
		RefreshMsgImpl refreshMsg = loginCallbackClient.refreshMsg();
		refreshMsg.decode(loginCallbackClient.rsslRefreshMsg(), rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);

		loginCallbackClient._eventImpl._item = this;
		
		loginCallbackClient.notifyOnAllMsg(refreshMsg);
		loginCallbackClient.notifyOnRefreshMsg();

		if (refreshMsg.state().streamState() != OmmState.StreamState.OPEN)
		{
			loginCallbackClient.loginItemList().remove(this);
			remove();
		}
	}
	
	boolean submit(RequestMsg rsslRequestMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		 ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		 rsslRequestMsg.streamId(_streamId);
		 //TODO Workaround
		 if ( rsslRequestMsg.msgKey().nameType() == 0)
		 {
			 rsslRequestMsg.msgKey().nameType(Login.UserIdTypes.NAME);
			 rsslRequestMsg.msgKey().applyHasNameType();
		 }
		 
		 int ret;
		for (ChannelInfo entry : _loginChannelList)
		{
			rsslSubmitOptions.serviceName(null);
			rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
			rsslErrorInfo.clear();

			if (ReactorReturnCodes.SUCCESS > (ret = entry.rsslReactorChannel().submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
		    {
				StringBuilder temp = _baseImpl.strBuilder();
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
					
		        	temp.append("Internal error: rsslChannel.submit() failed in LoginItem.submit(RequestMsg)")
		        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
		    			.append(OmmLoggerClient.CR)
		    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
		    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
		    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
		    			.append("Error Text ").append(error.text());
		        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	
		        	temp.setLength(0);
	        	}
				
				temp.append("Failed to open or modify item request. Reason: ")
					.append(ReactorReturnCodes.toString(ret))
					.append(". Error text: ")
					.append(rsslErrorInfo.error().text());
					
				_baseImpl.handleInvalidUsage(temp.toString());
	
				return false;
		    }
		}
		
		return true;
	}

	boolean submit(com.thomsonreuters.upa.codec.PostMsg rsslPostMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslPostMsg.streamId(_streamId);
		int ret;
		
		rsslPostMsg.streamId(_streamId);
		
	    for (ChannelInfo entry : _loginChannelList)
		{
	    	rsslSubmitOptions.serviceName(null);
			rsslSubmitOptions.requestMsgOptions().clear();
			rsslErrorInfo.clear();
			if (ReactorReturnCodes.SUCCESS > (ret = entry.rsslReactorChannel().submit(rsslPostMsg, rsslSubmitOptions, rsslErrorInfo)))
		    {
				StringBuilder temp = _baseImpl.strBuilder();
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
					
		        	temp.append("Internal error: rsslChannel.submit() failed in LoginItem.submit(PostMsg)")
		        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
		    			.append(OmmLoggerClient.CR)
		    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
		    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
		    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
		    			.append("Error Text ").append(error.text());
		        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	
		        	temp.setLength(0);
	        	}
				
				temp.append("Failed to submit PostMsg on item stream. Reason: ")
					.append(ReactorReturnCodes.toString(ret))
					.append(". Error text: ")
					.append(rsslErrorInfo.error().text());
					
				_baseImpl.handleInvalidUsage(temp.toString());
	
				return false;
		    }
		}
	    
		return true;
	}
	
	boolean submit(com.thomsonreuters.upa.codec.GenericMsg rsslGenericMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslGenericMsg.streamId(_streamId);
		int ret;
		
		rsslGenericMsg.streamId(_streamId);
		
	    for (ChannelInfo entry : _loginChannelList)
		{
	    	rsslSubmitOptions.serviceName(null);
			rsslSubmitOptions.requestMsgOptions().clear();
	    	rsslErrorInfo.clear();
			if (ReactorReturnCodes.SUCCESS > (ret = entry.rsslReactorChannel().submit(rsslGenericMsg, rsslSubmitOptions, rsslErrorInfo)))
		    {
				StringBuilder temp = _baseImpl.strBuilder();
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
					
		        	temp.append("Internal error: rsslChannel.submit() failed in LoginItem.submit(GenericMsg)")
		        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
		    			.append(OmmLoggerClient.CR)
		    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
		    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
		    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
		    			.append("Error Text ").append(error.text());
		        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	
		        	temp.setLength(0);
	        	}
				
				temp.append("Failed to submit GenericMsg on item stream. Reason: ")
					.append(ReactorReturnCodes.toString(ret))
					.append(". Error text: ")
					.append(rsslErrorInfo.error().text());
					
				_baseImpl.handleInvalidUsage(temp.toString());
	
				return false;
		    }
		}
	    
		return true;
	}
	
	void loginChannelList(List<ChannelInfo> loginChannelList)
	{
		_loginChannelList = loginChannelList;
	}
}
	
	
