///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.impl.OmmConsumerImpl.OmmConsumerState;
import com.thomsonreuters.ema.access.impl.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
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
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;


class LoginCallbackClient extends ConsumerCallbackClient implements RDMLoginMsgCallback
{
	private static final String CLIENT_NAME 	= "LoginCallbackClient";
	private static final int REFRESH_MSG_SIZE 	= 512;
	
	private List<ChannelInfo> 		_loginChannelList;
	private List<LoginItem>			_loginItemList;
	private Buffer 					_rsslEncBuffer;
	private RefreshMsg	 			_rsslRefreshMsg;  
	private CloseMsg				_rsslCloseMsg;
	private ReentrantLock 			_loginItemLock = new java.util.concurrent.locks.ReentrantLock();
	
	LoginCallbackClient(OmmConsumerImpl consumer)
	{
		 super(consumer, CLIENT_NAME);
		 _loginChannelList = new ArrayList<ChannelInfo>();
	}

	void initialize()
	{
		_consumer.activeConfig().intializeLoginReq(CLIENT_NAME);
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
			_consumer.closeRsslChannel(rsslReactorChannel);

			if (_consumer.loggerClient().isErrorEnabled())
        	{
				ReactorErrorInfo errorInfo = event.errorInfo();
				
	        	StringBuilder temp = _consumer.consumerStrBuilder();
				
	        	temp.append("Received an event without RDMLogin message").append(OmmLoggerClient.CR)
					.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.hashCode())).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append("@").append(Integer.toHexString(errorInfo.error().channel().hashCode())).append(OmmLoggerClient.CR)
					.append("Error Id ").append(errorInfo.error().errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(errorInfo.error().sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(errorInfo.location()).append(OmmLoggerClient.CR)
					.append("Error text ").append(errorInfo.error().text());

	        	_consumer.loggerClient().error(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
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
					msg.copy(_rsslRefreshMsg, CopyMsgFlags.ALL_FLAGS);
				}
				else
				{
					_rsslRefreshMsg.clear();
					msg.copy(_rsslRefreshMsg, CopyMsgFlags.ALL_FLAGS);
				}
				
				com.thomsonreuters.upa.codec.State state = ((LoginRefresh)loginMsg).state();
	
				boolean closeChannel = false;
	
				if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
				{
					closeChannel = true;
	
					if (_consumer.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("RDMLogin stream was closed with refresh message")
			        		.append(OmmLoggerClient.CR);
			        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
			        	temp.append(OmmLoggerClient.CR)
			    			.append("State: ").append(state.toString());
			        	
		        		_consumer.loggerClient().error(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
				}
				else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
				{
					if (_consumer.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("RDMLogin stream state was changed to suspect with refresh message")
			        		.append(OmmLoggerClient.CR);
			        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
			        	temp.append(OmmLoggerClient.CR)
			    			.append("State: ").append(state.toString());
			        	
		        		_consumer.loggerClient().warn(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					_consumer.ommConsumerState(OmmConsumerState.LOGIN_STREAM_OPEN_SUSPECT);
				}
				else
				{
					_consumer.ommConsumerState(OmmConsumerState.LOGIN_STREAM_OPEN_OK);
	
					if (_consumer.loggerClient().isTraceEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("RDMLogin stream was open with refresh message").append(OmmLoggerClient.CR)
			        		.append(loginMsg.toString()).append(OmmLoggerClient.CR)
							.append("State: ").append(state.toString());
			        	
			        	_consumer.loggerClient().trace(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		        	}
				}
	
				processRefreshMsg(msg, rsslReactorChannel, loginMsg);
	
				if (closeChannel)
					_consumer.closeRsslChannel(rsslReactorChannel);
	
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
						
						if (_consumer.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
							
				        	temp.append("RDMLogin stream was closed with status message")
				        		.append(OmmLoggerClient.CR);
				        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
				        	temp.append(OmmLoggerClient.CR)
				    			.append("State: ").append(state.toString());
				        	
			        		_consumer.loggerClient().error(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
					}
					else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
					{
						if (_consumer.loggerClient().isWarnEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
							
				        	temp.append("RDMLogin stream state was changed to suspect with status message")
				        		.append(OmmLoggerClient.CR);
				        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
				        	temp.append(OmmLoggerClient.CR)
				    			.append("State: ").append(state.toString());
				        	
			        		_consumer.loggerClient().warn(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
			        	}
					
						_consumer.ommConsumerState(OmmConsumerState.LOGIN_STREAM_OPEN_SUSPECT);
					}
					else
					{
						if (_consumer.loggerClient().isTraceEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
							
				        	temp.append("RDMLogin stream was open with status message").append(OmmLoggerClient.CR)
				        		.append(loginMsg.toString()).append(OmmLoggerClient.CR)
								.append("State: ").append(state.toString());
				        	
				        	_consumer.loggerClient().trace(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
			        	}
						
						_consumer.ommConsumerState(OmmConsumerState.LOGIN_STREAM_OPEN_OK);
					}
				}
				else
				{
					if (_consumer.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("Received RDMLogin status message without the state").append(OmmLoggerClient.CR)
			        		.append(loginMsg.toString());
			        	
			        	_consumer.loggerClient().warn(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
				}
	
				processStatusMsg(msg, rsslReactorChannel, loginMsg);
	
				if (closeChannel)
					_consumer.closeRsslChannel(rsslReactorChannel);
	
				break;
			}
			default:
			{
				if (msg.msgClass() == MsgClasses.GENERIC)
				{
					processGenericMsg(msg, rsslReactorChannel, event);
					break;
				}
				
				if (_consumer.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _consumer.consumerStrBuilder();
					
		        	temp.append("Received unknown RDMLogin message type")
		        		.append(OmmLoggerClient.CR)
						.append("Message type value ").append(loginMsg.rdmMsgType());
	
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
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
				_refreshMsg = new RefreshMsgImpl();
				
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
			_event._item = _loginItemList.get(idx);
			_event._item.client().onAllMsg(_refreshMsg, _event);
			_event._item.client().onRefreshMsg(_refreshMsg, _event);
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
				_statusMsg = new StatusMsgImpl();
				
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
			_event._item = _loginItemList.get(idx);
			_event._item.client().onAllMsg(_statusMsg, _event);
			_event._item.client().onStatusMsg(_statusMsg, _event);
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
			_event._item = _loginItemList.get(idx);
			_event._item.client().onAllMsg(_genericMsg, _event);
			_event._item.client().onGenericMsg(_genericMsg, _event);
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
        	_rsslEncBuffer.data(byteBuf, 0, 0); 
        }
	     
	    EncodeIterator rsslEncIter = _consumer.rsslEncIter();
        rsslEncIter.clear();
        if (rsslEncIter.setBufferAndRWFVersion(_rsslEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion()) != CodecReturnCodes.SUCCESS)
        {
        	if (_consumer.loggerClient().isErrorEnabled())
        	{
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, 
	        			"Internal error. Failed to set encode iterator buffer in LoginCallbackClient.convertRdmLoginToRsslBuffer()",
	        			Severity.ERROR));
        	}
        	
        	return ReactorCallbackReturnCodes.FAILURE;
        }
        
        int ret = 0;
        if ((ret = loginMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
        {
        	if (_consumer.loggerClient().isErrorEnabled())
        	{
        		StringBuilder temp = _consumer.consumerStrBuilder();
				
	        	temp.append("Internal error: failed to encode LoginMsg in LoginCallbackClient.convertRdmLoginToRsslBuffer()")
	        		.append(OmmLoggerClient.CR)
	        		.append("Error num ").append(ret).append(OmmLoggerClient.CR)
	        		.append("Error Text ").append(CodecReturnCodes.toString(ret));
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(LoginCallbackClient.CLIENT_NAME, 
	        									temp.toString(), Severity.ERROR));
        	}
        
        	return ReactorCallbackReturnCodes.FAILURE;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	LoginRequest rsslLoginRequest()
	{
		return _consumer.activeConfig().rsslRDMLoginRequest;
	}
	
	List<ChannelInfo> 	loginChannelList()
	{
		return _loginChannelList;
	}
	
	List<LoginItem> loginItemList()
	{
		return _loginItemList;
	}
	
	RefreshMsg rsslRefreshMsg()
	{
		return _rsslRefreshMsg;
	}
	
	RefreshMsgImpl refreshMsg()
	{
		return _refreshMsg;
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
		if (_rsslCloseMsg == null)
		{
			_rsslCloseMsg = (CloseMsg)CodecFactory.createMsg();
		}
		else
			_rsslCloseMsg.clear();
		
		_rsslCloseMsg.streamId(1);
		_rsslCloseMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		_rsslCloseMsg.domainType(DomainTypes.LOGIN);

		ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
	    ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
	    rsslSubmitOptions.clear();
	    
		for (ChannelInfo entry : _loginChannelList)
			entry.rsslReactorChannel().submit(_rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo);
		
		return _loginChannelList.size();
	}
	
	LoginItem loginItem(ReqMsg reqMsg , OmmConsumerClient consumerClient , Object closure)
	{
		if (_loginItemList == null)
			_loginItemList = new ArrayList<LoginItem>();
		
		LoginItem item;
		if ((item = (LoginItem)GlobalPool._loginItemPool.poll()) == null)
		{
			item = new LoginItem(_consumer, consumerClient, closure);
			GlobalPool._loginItemPool.updatePool(item);
		}
		else
		{
			item.reset(_consumer, consumerClient, closure, null);
		}
		
		_loginItemList.add(item);
		
		_consumer.addTimeoutEvent(10, item);
		
		return item;
	}

	int processAckMsg(Msg rsslMsg , ReactorChannel rsslChannel , RDMLoginMsgEvent rsslEvent)
	{
		return 0;
	}
}

class LoginItem extends SingleItem implements TimeoutClient
{
	private static final String 	CLIENT_NAME = "LoginItem";
	
	LoginItem(OmmConsumerImpl consumer , OmmConsumerClient consumerClient , Object closure)
	{
		super(consumer, consumerClient, closure, null);
		_streamId = 1;
	}

	@Override
	void reset(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure, Item batchItem)
	{
		super.reset(consumer, consumerClient, closure, batchItem);
		_streamId = 1;
	}
	
	boolean open(LoginRequest loginReq , List<LoginItem> loginChannelList)
	{
		return true;
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		return true;
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		return true;
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return true;
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
		LoginCallbackClient loginCallbackClient = _consumer.loginCallbackClient();
		
		if (loginCallbackClient.loginChannelList().isEmpty())
			return;
		
		ReactorChannel rsslReactorChannel = loginCallbackClient.loginChannelList().get(0).rsslReactorChannel();
		
		RefreshMsgImpl refreshMsg = loginCallbackClient.refreshMsg();
		refreshMsg.decode(loginCallbackClient.rsslRefreshMsg(), rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);

		loginCallbackClient._event._item = this;
		client().onAllMsg(refreshMsg, loginCallbackClient._event);
		client().onRefreshMsg(refreshMsg, loginCallbackClient._event);

		if (refreshMsg.state().streamState() != OmmState.StreamState.OPEN)
		{
			loginCallbackClient.loginItemList().remove(this);
			remove();
		}
	}
}
	
	