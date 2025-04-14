///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019-2025 LSEG. All rights reserved.         		--
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.OmmBaseImpl.OmmImplState;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.OmmState.DataState;
import com.refinitiv.ema.access.OmmState.StreamState;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginAttrib;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgCallback;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;


class LoginCallbackClient<T> extends CallbackClient<T> implements RDMLoginMsgCallback
{
	private static final String CLIENT_NAME 	= "LoginCallbackClient";
	private static final int REFRESH_MSG_SIZE 	= 512;
	
	private final List<ChannelInfo> 		_loginChannelList;
	private List<LoginItem<T>>		_loginItemList;
	private Buffer 					_rsslEncBuffer;
	private final ReentrantLock 	_loginItemLock = new java.util.concurrent.locks.ReentrantLock();
	private boolean					_notifyChannelDownReconnecting;
	private State	_rsslState;
	private final DecodeIterator	_decIter = CodecFactory.createDecodeIterator();
	private final EncodeIterator 	_encIter = CodecFactory.createEncodeIterator();
	private final Buffer			_tempBuffer = CodecFactory.createBuffer();
    private ByteBuffer 				_tempByteBuffer;
	private ByteBuffer 				_tempUserNameByteBuffer;
    private final Msg				_tempMsg = CodecFactory.createMsg();
	private final LoginRequest		_tempLoginReq = (LoginRequest) LoginMsgFactory.createMsg();
	private final OmmBaseImpl<T>	_ommBaseImpl;
	private LoginRefresh			_loginRefresh;
	private String					_loginFailureMsg;
    
	LoginCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		 super(baseImpl, CLIENT_NAME);
		 _ommBaseImpl = baseImpl;
		 _loginChannelList = new ArrayList<>();
		 _notifyChannelDownReconnecting = false;
	}

	void initialize()
	{
		_ommBaseImpl.activeConfig().rsslRDMLoginRequest.streamId(1);
		
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			temp.append("RDMLogin request message was populated with this info: ")
										.append(OmmLoggerClient.CR)
										.append(_ommBaseImpl.activeConfig().rsslRDMLoginRequest.toString());
										
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, 
					temp.toString(),Severity.TRACE));
		}
		
		_tempByteBuffer = ByteBuffer.allocate(8192);
		_tempBuffer.data(_tempByteBuffer);
		_tempUserNameByteBuffer = ByteBuffer.allocate(8192);
	}
	
	
	public com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh loginRefreshMsg()
	{
		if (_loginRefresh == null)
		{
			_loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
			_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		}
		
		return _loginRefresh;
	}
	
	String loginFailureMessage()
	{
		return _loginFailureMsg;
	}

	@SuppressWarnings("unchecked")
	@Override
	public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
	{
		Msg msg = event.msg();
		LoginMsg loginMsg = event.rdmLoginMsg();
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		if (chnlInfo.getParentChannel() != null)
			chnlInfo = chnlInfo.getParentChannel();
		ReactorChannel rsslReactorChannel  = event.reactorChannel();
		SessionChannelInfo<T> sessionChannelInfo =  chnlInfo.sessionChannelInfo() != null ? (SessionChannelInfo<T>) chnlInfo.sessionChannelInfo() : null;
		ConsumerSession<T> consumerSession = sessionChannelInfo != null ? sessionChannelInfo.consumerSession() : null;

		_baseImpl.eventReceived();

		if (loginMsg == null)
		{
			_ommBaseImpl.closeRsslChannel(rsslReactorChannel);

			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = event.errorInfo().error();
				
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
				boolean notifyRefreshMsg = true;
				
				if (_rsslRefreshMsg == null)
				{
					_rsslRefreshMsg = (RefreshMsg)CodecFactory.createMsg();
                }
				else
				{
					_rsslRefreshMsg.clear();
					_rsslRefreshMsg.msgClass(MsgClasses.REFRESH);
					msg.copy(_rsslRefreshMsg, CopyMsgFlags.ALL_FLAGS);
				}
				
				LoginRefresh loginRefresh;
				
				if(consumerSession != null)
				{
					loginRefresh = sessionChannelInfo.loginRefresh();
					
					if(!consumerSession.sessionChannelList().contains(sessionChannelInfo))
					{
						consumerSession.sessionChannelList().add(sessionChannelInfo);
					}
					
					sessionChannelInfo.receivedLoginRefresh(true);
				}
				else
				{
					loginRefresh = loginRefreshMsg();
					
					if (!_loginChannelList.contains(chnlInfo))
					{
						removeChannelInfo(event.reactorChannel());
						_loginChannelList.add(chnlInfo);
					}
				}
				
				loginRefresh.clear();
				
				((LoginRefresh)loginMsg).copy(loginRefresh);

				com.refinitiv.eta.codec.State state = ((LoginRefresh)loginMsg).state();
	
				boolean closeChannel = false;
	
				if (state.streamState() != com.refinitiv.eta.codec.StreamStates.OPEN)
				{
					closeChannel = true;
					
					if(consumerSession != null)
					{
						sessionChannelInfo.state(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN);
						
						consumerSession.increaseNumOfLoginClose();
						
						if(consumerSession.checkAllSessionChannelHasState(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN))
						{
							_ommBaseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN);
						}
					}
					else
					{
						_ommBaseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN);
					}
					
					
					StringBuilder temp = _baseImpl.strBuilder();
					
		        	temp.append("RDMLogin stream was closed with refresh message")
		        		.append(OmmLoggerClient.CR);
		        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
		        	temp.append(OmmLoggerClient.CR)
		    			.append(state);
		        	
		        	_loginFailureMsg = temp.toString();
	
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{			        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, _loginFailureMsg, Severity.ERROR));
		        	}
				}
				else if (state.dataState() == com.refinitiv.eta.codec.DataStates.SUSPECT)
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMLogin stream state was changed to suspect with refresh message")
			        		.append(OmmLoggerClient.CR);
			        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
			        	temp.append(OmmLoggerClient.CR)
		    			.append(state);
			        	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					if (_ommBaseImpl.ommImplState() >= OmmImplState.RSSLCHANNEL_UP )
					{
						if(sessionChannelInfo != null)
						{
							sessionChannelInfo.state(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
						}
						else
						{
							_ommBaseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
						}
					}
				}
				else
				{
					if(consumerSession != null)
					{
						sessionChannelInfo.state(OmmImplState.LOGIN_STREAM_OPEN_OK);
						
						consumerSession.increaseNumOfLoginOk();

						if(consumerSession.checkAllSessionChannelHasState(OmmImplState.LOGIN_STREAM_OPEN_OK))
						{
							consumerSession.aggregateLoginResponse();
							
							/* Swap to send with the aggregated login refresh */
							loginMsg = consumerSession.loginRefresh();
							msg = null;
							
							consumerSession.sendInitialLoginRefresh(true);
						}
						else
						{
							/* Wait until EMA's receives login refresh message from all channels. */
							notifyRefreshMsg = false;
						}
					}
					
					if(notifyRefreshMsg)
					{
						_ommBaseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_OK);
						
						_ommBaseImpl.setActiveRsslReactorChannel(chnlInfo);
						_ommBaseImpl.reLoadDirectory();
						
						if (_baseImpl.loggerClient().isTraceEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("RDMLogin stream was open with refresh message").append(OmmLoggerClient.CR)
				        		.append(loginMsg.toString()).append(OmmLoggerClient.CR);
		
				        	
				        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
			        	}
					}
				}
	
				if(notifyRefreshMsg)
				{
					processRefreshMsg(msg, rsslReactorChannel, loginMsg);
				}
	
				if (closeChannel)
				{
					if(sessionChannelInfo != null)
					{
						_ommBaseImpl.closeSessionChannel(sessionChannelInfo);
					}
					else
					{
						_ommBaseImpl.unsetActiveRsslReactorChannel(chnlInfo);
						_ommBaseImpl.closeRsslChannel(rsslReactorChannel);
					}
				}
	
				break;
			}
			case STATUS:
			{
				boolean closeChannel = false;
				boolean notifyStatusMsg = true;
	
				if (((LoginStatus)loginMsg).checkHasState())
		    	{
					com.refinitiv.eta.codec.State state =((LoginStatus)loginMsg).state();
	
					if (state.streamState() != com.refinitiv.eta.codec.StreamStates.OPEN)
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMLogin stream was closed with status message")
			        		.append(OmmLoggerClient.CR);
			        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
			        	temp.append(OmmLoggerClient.CR)
		    				.append(state);
			        	
			        	_loginFailureMsg = temp.toString();
						
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, _loginFailureMsg, Severity.ERROR));
			        	}
						
						closeChannel = true;
						
						if(consumerSession != null)
						{
							sessionChannelInfo.state(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN);
							
							consumerSession.increaseNumOfLoginClose();
							
							if(consumerSession.checkAllSessionChannelHasState(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN))
							{
								_ommBaseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN);
							}
							else
							{
								notifyStatusMsg = false;
								
								LoginStatus loginStatus = (LoginStatus)loginMsg;
								
								int streamState = loginStatus.state().streamState();
								
								loginStatus.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
								
								processStatusMsg(null, rsslReactorChannel, loginMsg);
								
								loginStatus.state().streamState(streamState);
							}
						}
						else
						{
							_ommBaseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN);
						}
					}
					else if (state.dataState() == com.refinitiv.eta.codec.DataStates.SUSPECT)
					{
						if (_baseImpl.loggerClient().isWarnEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("RDMLogin stream state was changed to suspect with status message")
				        		.append(OmmLoggerClient.CR);
				        	loginMsgToString(temp, loginMsg, loginMsg.rdmMsgType());
				        	temp.append(OmmLoggerClient.CR)
			    				.append(state);
				        	
				        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
			        	}
						
						if(consumerSession != null)
						{
							if(sessionChannelInfo.state() >= OmmImplState.RSSLCHANNEL_UP)
							{
								sessionChannelInfo.state(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
							}
							
							// Checks whether all session channel has the OmmImplState.LOGIN_STREAM_OPEN_SUSPECT state.
							boolean result = consumerSession.checkAllSessionChannelHasState(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
							
							if(result)
							{
								_ommBaseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
							}
							else
							{
								notifyStatusMsg = false;
							}
						}
						else
						{
							if (_ommBaseImpl.ommImplState() >= OmmImplState.RSSLCHANNEL_UP )
								_ommBaseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
						}
					}
					else
					{
						_ommBaseImpl.setActiveRsslReactorChannel(chnlInfo);
						_ommBaseImpl.reLoadDirectory();
						
						if (_baseImpl.loggerClient().isTraceEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("RDMLogin stream was open with status message").append(OmmLoggerClient.CR)
				        		.append(loginMsg).append(OmmLoggerClient.CR);
				        	
				        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
			        	}
						
						if(consumerSession != null)
						{
							sessionChannelInfo.state(OmmImplState.LOGIN_STREAM_OPEN_OK);
							
							consumerSession.increaseNumOfLoginOk();
							
							if(consumerSession.checkAllSessionChannelHasState(OmmImplState.LOGIN_STREAM_OPEN_OK))
							{
								_ommBaseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_OK);
								
								consumerSession.aggregateLoginResponse();
								
								processRefreshMsg(null, sessionChannelInfo.reactorChannel(), consumerSession.loginRefresh());
								
								consumerSession.sendInitialLoginRefresh(true);
								notifyStatusMsg = false;
							}
						}
						else
						{
							_ommBaseImpl.ommImplState(OmmImplState.LOGIN_STREAM_OPEN_OK);
						}
					}
				}
				else
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("Received RDMLogin status message without the state").append(OmmLoggerClient.CR)
			        		.append(loginMsg);
			        	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(LoginCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
				}
	
				if(notifyStatusMsg)
				{
					processStatusMsg(msg, rsslReactorChannel, loginMsg);
				}
	
				if (closeChannel)
				{
					if(sessionChannelInfo != null)
					{
						_ommBaseImpl.closeSessionChannel(sessionChannelInfo);
					}
					else
					{
						_ommBaseImpl.unsetActiveRsslReactorChannel(chnlInfo);
						_ommBaseImpl.closeRsslChannel(rsslReactorChannel);
					}
				}
	
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
		if (rsslMsg != null)
		{
			if (_refreshMsg == null)
				_refreshMsg = new RefreshMsgImpl(_baseImpl.objManager());
				
			_refreshMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
		}
		else
		{
			if (convertRdmLoginToRsslBuffer(rsslReactorChannel, loginMsg) != ReactorCallbackReturnCodes.SUCCESS)
				return ReactorCallbackReturnCodes.SUCCESS;

			_refreshMsg.decode(_rsslEncBuffer, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null, null);
		}
		
		if (_loginItemList == null)
			return ReactorCallbackReturnCodes.SUCCESS;
		
		_loginItemLock.lock();
		
		int itemSize = _loginItemList.size();
		for (int idx = 0; idx < itemSize; ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);
			_eventImpl._channel = rsslReactorChannel;
			
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
		if (rsslMsg != null)
		{
			if (_statusMsg == null)
				_statusMsg = new StatusMsgImpl(_baseImpl.objManager());
				
			_statusMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
		}
		else
		{
			if (convertRdmLoginToRsslBuffer(rsslReactorChannel, loginMsg) != ReactorCallbackReturnCodes.SUCCESS)
				return ReactorCallbackReturnCodes.SUCCESS;

			if (_statusMsg == null)
				_statusMsg = new StatusMsgImpl(_baseImpl.objManager());
			
			_statusMsg.decode(_rsslEncBuffer, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null, null);
		}
		
		if (_loginItemList == null)
			return ReactorCallbackReturnCodes.SUCCESS;
		
		_loginItemLock.lock();
		
		int itemSize = _loginItemList.size();
		for (int idx = 0; idx < itemSize; ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);
			_eventImpl._channel = rsslReactorChannel;
			
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
			_genericMsg = new GenericMsgImpl(_baseImpl.objManager());
		_genericMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(),
							((ChannelInfo)event.reactorChannel().userSpecObj()).rsslDictionary());
		
		_loginItemLock.lock();
		
		int itemSize = _loginItemList.size();
		for (int idx = 0; idx < itemSize; ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);
			_eventImpl._channel = rsslReactorChannel;
			
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
	     
	    EncodeIterator rsslEncIter = _ommBaseImpl.rsslEncIter();
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
        
        int ret;
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
		return _ommBaseImpl.activeConfig().rsslRDMLoginRequest;
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
	
	void removeChannelInfo(ReactorChannel rsslChannel)
	{
		if (_loginChannelList != null)
		{
			Iterator<ChannelInfo> iter = _loginChannelList.iterator();
			ChannelInfo eachOne;
        	
        	while(iter.hasNext())
        	{
        		eachOne = iter.next();
				if (eachOne.rsslReactorChannel() == rsslChannel)
					iter.remove();
			}
		}
	}
	
	void removeSessionChannelInfo(SessionChannelInfo<T> sessionChannelInfo)
	{
		if(sessionChannelInfo != null)
		{
			sessionChannelInfo.consumerSession().removeSessionChannelInfo(sessionChannelInfo);
		}
	}
	
	public RefreshMsg rsslRefreshMsg()
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
				.append("permissionExpressions ").append(attrib.checkHasProvidePermissionExpressions() ? attrib.providePermissionExpressions() : "<not set>").append(OmmLoggerClient.CR)
				.append("permissionProfile ").append(attrib.checkHasProvidePermissionProfile()? attrib.providePermissionProfile() : "<not set>").append(OmmLoggerClient.CR);

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
		rsslCloseMsg.containerType(com.refinitiv.eta.codec.DataTypes.NO_DATA);
		rsslCloseMsg.domainType(DomainTypes.LOGIN);

		ReactorErrorInfo rsslErrorInfo = _ommBaseImpl.rsslErrorInfo();
	    ReactorSubmitOptions rsslSubmitOptions = _ommBaseImpl.rsslSubmitOptions();
	    rsslSubmitOptions.serviceName(null);
		rsslSubmitOptions.requestMsgOptions().clear();
	    
		if(_ommBaseImpl.consumerSession() != null)
		{
			List<SessionChannelInfo<T>> sessionChannelList = _ommBaseImpl.consumerSession().sessionChannelList();
			
			for(SessionChannelInfo<T> entry : sessionChannelList)
			{
				if(entry.receivedLoginRefresh()) 
				{
					entry.reactorChannel().submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo);
				}
			}
			
			return sessionChannelList.size();
		}
		else
		{
			for (ChannelInfo entry : _loginChannelList)
				entry.rsslReactorChannel().submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo);
		
			return _loginChannelList.size();
		}
	}
	
	@SuppressWarnings("unchecked")
	LoginItem<T> loginItem(ReqMsg reqMsg , T client , Object closure)
	{
		if (_loginItemList == null)
			_loginItemList = new ArrayList<>();
		
		LoginItem<T> item;
		if ((item = (LoginItem<T>)_baseImpl.objManager()._loginItemPool.poll()) == null)
		{
			item = new LoginItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure);
			_baseImpl.objManager()._loginItemPool.updatePool(item);
		}
		else
		{
			item.reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
		}
		
		item.loginChannelList(_loginChannelList);
		_loginItemList.add(item);
		
		/* Do not give a refresh msg to the user if one is not present */
		if(_refreshMsg != null)
			_ommBaseImpl.addTimeoutEvent(10, item);
		
		return item;
	}

	int processAckMsg(Msg rsslMsg ,  ChannelInfo channelInfo)
	{
		if (_ackMsg == null)
			_ackMsg = new AckMsgImpl(_baseImpl.objManager());
		
		_ackMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);


		if(channelInfo.sessionChannelInfo() != null && _ackMsg.hasServiceId())
		{
			Directory<OmmConsumerClient> directory = channelInfo.sessionChannelInfo().getDirectoryById(_ackMsg.serviceId());
			
			if(directory != null && directory.hasGeneratedServiceId())
			{
				_ackMsg.serviceId(directory.generatedServiceId());
				_ackMsg.serviceName(directory.serviceName());
			}
		}
	
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
			_genericMsg = new GenericMsgImpl(_baseImpl.objManager());
		
		_genericMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		if (_loginItemList != null) {
			for (Item<T> loginItem : _loginItemList)
			{
				_eventImpl._item = loginItem;

				notifyOnAllMsg(_genericMsg);
				notifyOnGenericMsg();
			}
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	
	int overlayLoginRequest(Msg request)
	{
        // clear temp buffer
        _tempBuffer.clear();
        _tempByteBuffer.clear();
        _tempBuffer.data(_tempByteBuffer);
        
        // encode Codec message into buffer
        _encIter.clear();
        _encIter.setBufferAndRWFVersion(_tempBuffer, Codec.majorVersion(), Codec.minorVersion());

		int ret;
        while ((ret = request.encode(_encIter)) == CodecReturnCodes.BUFFER_TOO_SMALL) {
        	_tempByteBuffer = ByteBuffer.allocate(_tempByteBuffer.capacity() * 2);
			_tempBuffer.clear();
			_tempBuffer.data(_tempByteBuffer);
			_encIter.clear();
			_encIter.setBufferAndRWFVersion(_tempBuffer, Codec.majorVersion(), Codec.minorVersion());
		}

        if (ret >= CodecReturnCodes.SUCCESS)
        {
            // decode encoded Codec message into RDM message
            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_tempBuffer, Codec.majorVersion(), Codec.minorVersion());
            _tempMsg.clear();
			_tempLoginReq.clear();
			_tempLoginReq.rdmMsgType(LoginMsgType.REQUEST);
            _tempMsg.decode(_decIter);
            if((ret = _tempLoginReq.decode(_decIter, _tempMsg)) < CodecReturnCodes.SUCCESS)
            {
            	return ret;
	        }
        }
        else
        	return ret;
        
        /* Apply the following changed fields to the cached login request:
         * userName
         * Attrib
         * applicationName
         * Position
         * InstanceId
         * Password
         * Pause
         * No refresh
         * 
         * Note: Pause and no refresh should be removed from the cached request after submitting to the reactor
         */
        
        if(!_tempLoginReq.userName().isBlank() && !_tempLoginReq.userName().equals(rsslLoginRequest().userName()))
        {
			_tempUserNameByteBuffer.clear();
			if (_tempLoginReq.userName().copy(_tempUserNameByteBuffer) < CodecReturnCodes.SUCCESS) {
				_tempUserNameByteBuffer = ByteBuffer.allocate(_tempLoginReq.userName().length() * 2);
				_tempLoginReq.userName().copy(_tempUserNameByteBuffer);
			}
			rsslLoginRequest().userName().data(_tempUserNameByteBuffer, 0, _tempLoginReq.userName().length());
        }
        
        if(_tempLoginReq.checkHasAttrib())
        {
        	if(_tempLoginReq.attrib().checkHasApplicationId())
        	{
	        	if(!rsslLoginRequest().attrib().checkHasApplicationId() || 
	        		!_tempLoginReq.attrib().applicationId().equals(rsslLoginRequest().attrib().applicationId()))
				{
	        		rsslLoginRequest().applyHasAttrib();
	        		ByteBuffer byteBuffer = ByteBuffer.allocate(_tempLoginReq.attrib().applicationId().length());
	            	_tempLoginReq.attrib().applicationId().copy(byteBuffer);
	            	rsslLoginRequest().attrib().applyHasApplicationId();
	            	rsslLoginRequest().attrib().applicationId().data(byteBuffer);
				}
        	}
        	
        	if(_tempLoginReq.attrib().checkHasApplicationName())
        	{
	        	if(!rsslLoginRequest().attrib().checkHasApplicationName() || 
	        		!_tempLoginReq.attrib().applicationName().equals(rsslLoginRequest().attrib().applicationName()))
				{
	        		rsslLoginRequest().applyHasAttrib();
	        		ByteBuffer byteBuffer = ByteBuffer.allocate(_tempLoginReq.attrib().applicationName().length());
	            	_tempLoginReq.attrib().applicationName().copy(byteBuffer);
	            	rsslLoginRequest().attrib().applyHasApplicationName();
	            	rsslLoginRequest().attrib().applicationName().data(byteBuffer);
				}
        	}
        	
        	if(_tempLoginReq.attrib().checkHasPosition())
        	{
	        	if(!rsslLoginRequest().attrib().checkHasPosition() || 
	        			!_tempLoginReq.attrib().position().equals(rsslLoginRequest().attrib().position()))
				{
	        		ByteBuffer byteBuffer = ByteBuffer.allocate(_tempLoginReq.attrib().position().length());
	            	_tempLoginReq.attrib().position().copy(byteBuffer);
	            	rsslLoginRequest().attrib().applyHasPosition();
	            	rsslLoginRequest().attrib().position().data(byteBuffer);
				}
        	}
        }
        
        if(_tempLoginReq.checkHasInstanceId())
        {
	        if(!rsslLoginRequest().checkHasInstanceId() || 
	        	!_tempLoginReq.instanceId().equals(rsslLoginRequest().instanceId()))
			{
	        	ByteBuffer byteBuffer = ByteBuffer.allocate(_tempLoginReq.instanceId().length());
	        	_tempLoginReq.instanceId().copy(byteBuffer);
	        	rsslLoginRequest().applyHasInstanceId();
	        	rsslLoginRequest().instanceId().data(byteBuffer);
			}
        }
        
        if(_tempLoginReq.checkHasPassword())
        {
	        if(!rsslLoginRequest().checkHasPassword() || 
	        	!_tempLoginReq.password().equals(rsslLoginRequest().password()))
			{
	        	ByteBuffer byteBuffer = ByteBuffer.allocate(_tempLoginReq.password().length());
	        	_tempLoginReq.password().copy(byteBuffer);
	        	rsslLoginRequest().applyHasPassword();
	        	rsslLoginRequest().password().data(byteBuffer);
			}
        }
        
        if(_tempLoginReq.checkPause())
        {
        	rsslLoginRequest().applyPause();
        }
        
        if(_tempLoginReq.checkNoRefresh())
        {
        	rsslLoginRequest().applyNoRefresh();
        }
        
        return CodecReturnCodes.SUCCESS;
	}
	
	void processChannelEvent(ReactorChannelEvent event)
	{
		if (_loginItemList == null)
			return;
		
		State state = rsslState();
		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl(_baseImpl.objManager());
		
		switch ( event.eventType() )
		{
		case ReactorChannelEventTypes.CHANNEL_READY:

			if(!_notifyChannelDownReconnecting)
				break;

			state.streamState(StreamState.OPEN);
			state.dataState(DataState.OK);
			state.code(OmmState.StatusCode.NONE);
			state.text().data("channel up");

			prepareAndSendStatusMsg(event, state);

			_notifyChannelDownReconnecting = false;
			
			break;
		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			
			if(_notifyChannelDownReconnecting)
				break;

			state.streamState(StreamState.OPEN);
			state.dataState(DataState.SUSPECT);
			state.code(OmmState.StatusCode.NONE);
			state.text().data("channel down");

			prepareAndSendStatusMsg(event, state);

			_notifyChannelDownReconnecting = true;
			
			break;
		case ReactorChannelEventTypes.CHANNEL_DOWN:

			state.streamState(StreamState.CLOSED);
			state.dataState(DataState.SUSPECT);
			state.code(OmmState.StatusCode.NONE);
			state.text().data("channel closed");

			prepareAndSendStatusMsg(event, state);

			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE:
			state.streamState(StreamState.OPEN);
			state.dataState(DataState.OK);
			state.code(OmmState.StatusCode.PREFERRED_HOST_COMPLETE);
			state.text().data("preferred host complete");

			prepareAndSendStatusMsg(event, state);

			break;
		default:
			break;
		}
	}

	private void populateStatusMsg()
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

	private void prepareAndSendStatusMsg(ReactorChannelEvent event, State state) {
		populateStatusMsg();

		_rsslStatusMsg.state(state);
		_rsslStatusMsg.applyHasState();

		_statusMsg.decode(_rsslStatusMsg, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion(), null);

		for (int idx = 0; idx < _loginItemList.size(); ++idx)
		{
			_eventImpl._item = _loginItemList.get(idx);

			notifyOnAllMsg(_statusMsg);
			notifyOnStatusMsg();
		}
	}
}

class LoginCallbackClientConsumer extends LoginCallbackClient<OmmConsumerClient>
{
	LoginCallbackClientConsumer(OmmBaseImpl<OmmConsumerClient> baseImpl) {
		super(baseImpl);
	}
	
	@Override
	void notifyOnAllMsg(com.refinitiv.ema.access.Msg msg)
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
		
		_eventImpl._ommProvider = (OmmProvider) baseImpl;
	}
	
	@Override
	void notifyOnAllMsg(com.refinitiv.ema.access.Msg msg)
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
	LoginRequest						_loginReq;
	
	LoginItem(OmmBaseImpl<T> baseImpl , T client , Object closure)
	{
		super(baseImpl, client, closure, null);
		_loginReq = baseImpl._loginCallbackClient.rsslLoginRequest();
		_streamId = 1;
	}

	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> batchItem)
	{
		super.reset(baseImpl, client, closure, batchItem);
		_loginReq = baseImpl._loginCallbackClient.rsslLoginRequest();
		_streamId = 1;
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		int ret;
		if((ret = _baseImpl._loginCallbackClient.overlayLoginRequest(((ReqMsgImpl)reqMsg).rsslMsg()))
				!= CodecReturnCodes.SUCCESS)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
	        	temp.append("Internal error: Error caching login reissue.");
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(LoginItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed cache login reissue. Reason: ")
				.append(ReactorReturnCodes.toString(ret));
				
			_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
			
		}
		
		boolean submitRet = submit(_baseImpl.loginCallbackClient().rsslLoginRequest());
		
		/* Unset the pause all and no refresh flags on the stored request. */
		_baseImpl.loginCallbackClient().rsslLoginRequest().flags(_baseImpl.loginCallbackClient().rsslLoginRequest().flags() & ~LoginRequestFlags.PAUSE_ALL);
		_baseImpl.loginCallbackClient().rsslLoginRequest().flags(_baseImpl.loginCallbackClient().rsslLoginRequest().flags() & ~LoginRequestFlags.NO_REFRESH);

		return submitRet;
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		ConsumerSession<T> consumerSession = _baseImpl.consumerSession();
		
		if(consumerSession == null)
		{
			return submit(((PostMsgImpl) postMsg).rsslMsg(), postMsg.hasServiceName() ? postMsg.serviceName() : null );
		}
		else
		{
			boolean supportPosting = true;
			if(consumerSession.loginRefresh().checkHasFeatures() && consumerSession.loginRefresh().features().checkHasSupportPost()) 
			{
				if(consumerSession.loginRefresh().features().supportOMMPost() == 0)
				{
					supportPosting = false;
				}
			}
			else
			{
				supportPosting = false;
			}
			
			if(!supportPosting)
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Invalid attempt to submit PostMsg while posting not supported by provider.");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(LoginItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

				return false;
			}
				
			return submit(consumerSession,((PostMsgImpl) postMsg).rsslMsg(), postMsg.hasServiceName() ? postMsg.serviceName() : null );
		}
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		ConsumerSession<T> consumerSession = _baseImpl.consumerSession();
		
		if(consumerSession == null)
		{
			return submit(((GenericMsgImpl) genericMsg).rsslMsg());
		}
		else
		{
			return submit(consumerSession, ((GenericMsgImpl) genericMsg).rsslMsg());
		}
	}
	
	@Override
	boolean close()
	{
		remove();
		
		_baseImpl.loginCallbackClient().loginItemList().remove(this);
		
		return true;
	}
	
	@Override
	public void handleTimeoutEvent()
	{
		LoginCallbackClient<T> loginCallbackClient = _baseImpl.loginCallbackClient();
		ConsumerSession<T> consumerSession = _baseImpl.consumerSession();
		List<SessionChannelInfo<T>> sessionChannelList = (consumerSession != null && !consumerSession.sessionChannelList().isEmpty())
				? consumerSession.sessionChannelList() : null;
		
		if ( _loginChannelList.isEmpty() && ( consumerSession == null || !consumerSession.checkAllSessionChannelHasState(OmmImplState.LOGIN_STREAM_OPEN_OK)))
			return;
		
		ReactorChannel rsslReactorChannel;
		RefreshMsgImpl refreshMsg = loginCallbackClient.refreshMsg();
		
		if(consumerSession == null)
		{
			rsslReactorChannel = _loginChannelList.get(0).rsslReactorChannel();
			refreshMsg.decode(loginCallbackClient.rsslRefreshMsg(), rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
		}
		else
		{
			rsslReactorChannel = sessionChannelList.get(0).reactorChannel();
		}

		loginCallbackClient._eventImpl._item = this;
		loginCallbackClient._eventImpl._channel = rsslReactorChannel;
		
		loginCallbackClient.notifyOnAllMsg(refreshMsg);
		loginCallbackClient.notifyOnRefreshMsg();

		if (refreshMsg.state().streamState() != OmmState.StreamState.OPEN)
		{
			loginCallbackClient.loginItemList().remove(this);
			remove();
		}
	}

	@Override
	public ReentrantLock userLock() {
		return _baseImpl.userLock();
	}
	
	private int submitLoginRequest(ReactorChannel reactorChannel, ReactorSubmitOptions rsslSubmitOptions, LoginRequest rdmRequestMsg)
	{
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = reactorChannel.submit(rdmRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
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
				
			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return ret;
	    }
		
		return ret;
	}

	boolean submit(LoginRequest rdmRequestMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		 ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		 rdmRequestMsg.streamId(_streamId);
		 //TODO Workaround
		 if ( !rdmRequestMsg.checkHasUserNameType() )
		 {
			 rdmRequestMsg.applyHasUserNameType();
			 rdmRequestMsg.userNameType(Login.UserIdTypes.NAME);
		 }
		 
		int ret;
		for (ChannelInfo entry : _loginChannelList)
		{
			rsslSubmitOptions.serviceName(null);
			rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
			rsslErrorInfo.clear();
			
			ret = submitLoginRequest(entry.rsslReactorChannel(), rsslSubmitOptions, rdmRequestMsg);
			
			if(ReactorReturnCodes.SUCCESS > ret)
			{
				return false;
			}
		}
		
		return true;
	}
	
	boolean submit(ConsumerSession<T> consumerSession, LoginRequest rdmRequestMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		 ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		 rdmRequestMsg.streamId(_streamId);
		 //TODO Workaround
		 if ( !rdmRequestMsg.checkHasUserNameType() )
		 {
			 rdmRequestMsg.applyHasUserNameType();
			 rdmRequestMsg.userNameType(Login.UserIdTypes.NAME);
		 }
		 
	    int ret;
		for (SessionChannelInfo<T> entry : consumerSession.sessionChannelList())
		{
			rsslSubmitOptions.serviceName(null);
			rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
			rsslErrorInfo.clear();
			
			/* Ensure that the ReactorChannel is ready to submit the message */
			if(entry.reactorChannel().state() == ReactorChannel.State.UP || entry.reactorChannel().state() ==  ReactorChannel.State.READY)
			{	
				ret = submitLoginRequest(entry.reactorChannel(), rsslSubmitOptions, rdmRequestMsg);
				if(ReactorReturnCodes.SUCCESS > ret)
				{
					return false;
				}
			}
		}

		return true;
	}

	boolean submit( com.refinitiv.eta.codec.PostMsg rsslPostMsg, String serviceName )
	{
		if(!validateServiceName(serviceName)){
			return false;
		}
		
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		int ret;
		
		rsslPostMsg.streamId(_streamId);
		
	    for (ChannelInfo entry : _loginChannelList)
		{
			rsslSubmitOptions.serviceName( serviceName );
			rsslSubmitOptions.requestMsgOptions().clear();
			rsslErrorInfo.clear();
			if (ReactorReturnCodes.SUCCESS > (ret = entry.rsslReactorChannel().submit(rsslPostMsg, rsslSubmitOptions, rsslErrorInfo)))
		    {
				StringBuilder temp = _baseImpl.strBuilder();
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
					
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
					
				_baseImpl.handleInvalidUsage(temp.toString(), ret);
	
				return false;
		    }
		}
	    
		return true;
	}
	
	boolean submit(ConsumerSession<T> consumerSession, com.refinitiv.eta.codec.PostMsg rsslPostMsg, String serviceName )
	{		
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslPostMsg.streamId(_streamId);
		int ret;
		
	    for (SessionChannelInfo<T> entry : consumerSession.sessionChannelList())
		{
	    	/* Validate whether the service name is valid for SessionChannelInfo before submitting it. */
	    	boolean result = consumerSession.validateServiceName(entry, rsslPostMsg, serviceName);
	    	
	    	if(!result)
	    	{
	    		/* The PostMsg is dropped from this session channel */
	    		continue;
	    	}
	    	
			rsslSubmitOptions.serviceName( serviceName );
			rsslSubmitOptions.requestMsgOptions().clear();
			rsslErrorInfo.clear();
			
			/* Ensure that the ReactorChannel is ready to submit the message */
			if(entry.reactorChannel().state() == ReactorChannel.State.UP || entry.reactorChannel().state() ==  ReactorChannel.State.READY)
			{
				if (ReactorReturnCodes.SUCCESS > (ret = entry.reactorChannel().submit(rsslPostMsg, rsslSubmitOptions, rsslErrorInfo)))
			    {
					StringBuilder temp = _baseImpl.strBuilder();
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
						com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
						
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
						
					_baseImpl.handleInvalidUsage(temp.toString(), ret);
		
					return false;
			    }
			}
		}
	    
		return true;
	}
	
	boolean submit(com.refinitiv.eta.codec.GenericMsg rsslGenericMsg)
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
					com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
					
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
					
				_baseImpl.handleInvalidUsage(temp.toString(), ret);
	
				return false;
		    }
		}
	    
		return true;
	}
	
	boolean submit(ConsumerSession<T> consumerSession, com.refinitiv.eta.codec.GenericMsg rsslGenericMsg)
	{		
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslGenericMsg.streamId(_streamId);
		int ret;
		
	    for (SessionChannelInfo<T> entry : consumerSession.sessionChannelList())
		{
	    	/* Translate from generated service Id to the actual service Id */
	    	consumerSession.checkServiceId(entry, rsslGenericMsg);
	    	
			rsslSubmitOptions.serviceName( null );
			rsslSubmitOptions.requestMsgOptions().clear();
			rsslErrorInfo.clear();
			
			/* Ensure that the ReactorChannel is ready to submit the message */
			if(entry.reactorChannel().state() == ReactorChannel.State.UP || entry.reactorChannel().state() ==  ReactorChannel.State.READY)
			{
				if (ReactorReturnCodes.SUCCESS > (ret = entry.reactorChannel().submit(rsslGenericMsg, rsslSubmitOptions, rsslErrorInfo)))
			    {
					StringBuilder temp = _baseImpl.strBuilder();
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
						com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
						
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
						
					_baseImpl.handleInvalidUsage(temp.toString(), ret);
		
					return false;
			    }
			}
		}
	    
		return true;
	}

	private boolean validateServiceName(String serviceName)
	{
		if (serviceName == null || _baseImpl.directoryCallbackClient().directory(serviceName) != null)
		{
			return true;
		}
		StringBuilder temp = _baseImpl.strBuilder();
		if (_baseImpl.loggerClient().isErrorEnabled())
		{
			temp.append("Internal error: rsslChannel.submit() failed in LoginItem.submit(PostMsg)")
					.append(OmmLoggerClient.CR)
					.append("Error Id ").append(ReactorReturnCodes.INVALID_USAGE).append(OmmLoggerClient.CR)
					.append("Error Location ").append("LoginCallbackClient.rsslSubmit(PostMsg,String)").append(OmmLoggerClient.CR)
					.append("Error Text ").append("Message submitted with unknown service name ").append(serviceName);

			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));

			temp.setLength(0);
		}

		temp.append("Failed to submit PostMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ReactorReturnCodes.INVALID_USAGE))
				.append(". Error text: ")
				.append("Message submitted with unknown service name ").append(serviceName);

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		return false;
	}
	
	void loginChannelList(List<ChannelInfo> loginChannelList)
	{
		_loginChannelList = loginChannelList;
	}
}
	
	
