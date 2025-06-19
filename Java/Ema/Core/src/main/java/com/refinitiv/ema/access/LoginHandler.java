/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;

import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgCallback;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

class LoginHandler implements RDMLoginMsgCallback
{
	protected OmmServerBaseImpl _ommServerBaseImpl;
	
	private static final String CLIENT_NAME = "LoginHandler";
	
	private LoginStatus _loginStatus = (LoginStatus)LoginMsgFactory.createMsg();
	private ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	private ArrayList<ItemInfo> _itemInfoList = new ArrayList<>();
	private LongObject _streamId = new LongObject();
	
	LoginHandler(OmmServerBaseImpl ommServerBaseImpl)
	{
		_ommServerBaseImpl = ommServerBaseImpl;
		 _loginStatus.rdmMsgType(LoginMsgType.STATUS);
	}

	public void initialize()
	{
	}
	
	ArrayList<ItemInfo> getItemInfoList()
	{
		return _itemInfoList;
	}

	@Override
	public int rdmLoginMsgCallback(RDMLoginMsgEvent loginMsgEvent)
	{
		_ommServerBaseImpl.eventReceived();
		ClientSession clientSession = (ClientSession)loginMsgEvent.reactorChannel().userSpecObj(); 
		LoginMsg loginMsg = loginMsgEvent.rdmLoginMsg();
		
		if ( loginMsg == null )
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Login message rejected - Invalid login domain message.");
			
			sendLoginReject(loginMsgEvent.reactorChannel(), loginMsgEvent.msg().streamId(), StateCodes.USAGE_ERROR ,
					temp.toString() );
			
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(loginMsgEvent.msg().streamId())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
				_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.ERROR));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		switch(loginMsg.rdmMsgType())
		{
		case REQUEST:
			{	
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received login request message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(loginMsgEvent.msg().streamId())
					.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
					.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
				
				reqMsg.decode(loginMsgEvent.msg(), loginMsgEvent.reactorChannel().majorVersion(),
						loginMsgEvent.reactorChannel().minorVersion(), null);
				
				_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
				_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
				_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
				_ommServerBaseImpl.ommProviderEvent()._channel = loginMsgEvent.reactorChannel();
				
				_streamId.value(loginMsgEvent.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				RequestMsg requestMsg = (RequestMsg)loginMsgEvent.msg();
				
				if( itemInfo == null )
				{
					itemInfo = ServerPool.getItemInfo();
				
					itemInfo.setRequestMsg(requestMsg);
					itemInfo.clientSession(clientSession);
					clientSession.setLoginHandle(itemInfo.handle().value());

					_ommServerBaseImpl.addItemInfo(clientSession, itemInfo);
					
					_itemInfoList.add(itemInfo);
				
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();

					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onReqMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				}
				else
				{	
					itemInfo.setRequestMsg(requestMsg);
					
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();

					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onReissue(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				}
		
				break;
			}
		case CONSUMER_CONNECTION_STATUS:
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received Consumer Connection Status message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(loginMsgEvent.msg().streamId())
					.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
					.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				_streamId.value(loginMsgEvent.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
				if( itemInfo != null )
				{
					GenericMsgImpl genericMsg = _ommServerBaseImpl.genericMsg();
					
					genericMsg.decode(loginMsgEvent.msg(), loginMsgEvent.reactorChannel().majorVersion(),
							loginMsgEvent.reactorChannel().minorVersion(), null);
					
					_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
					_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
					_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
					_ommServerBaseImpl.ommProviderEvent()._channel = loginMsgEvent.reactorChannel();
					
					_ommServerBaseImpl.ommProviderClient().onAllMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onGenericMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
				}
			
			break;
			}
		case RTT:
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
				{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received RoundTripLatency message.")
							.append(OmmLoggerClient.CR).append("Stream Id ").append(loginMsgEvent.msg().streamId())
							.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
							.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());

					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
							temp.toString(), Severity.TRACE));
				}

				_streamId.value(loginMsgEvent.msg().streamId());

				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);

				if( itemInfo != null )
				{
					GenericMsgImpl genericMsg = _ommServerBaseImpl.genericMsg();

					genericMsg.decode(loginMsgEvent.msg(), loginMsgEvent.reactorChannel().majorVersion(),
							loginMsgEvent.reactorChannel().minorVersion(), null);

					_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
					_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
					_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
					_ommServerBaseImpl.ommProviderEvent()._channel = loginMsgEvent.reactorChannel();

					_ommServerBaseImpl.ommProviderClient().onAllMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onGenericMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
				}

				break;
			}
		case CLOSE:
			{		
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received login close message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(loginMsgEvent.msg().streamId())
					.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
					.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				_streamId.value(loginMsgEvent.msg().streamId());
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
				if ( itemInfo != null )
				{
					RequestMsg rsslReqMsg = _ommServerBaseImpl.rsslRequestMsg();
					
					rsslReqMsg.applyNoRefresh();
					
					rsslReqMsg.streamId(loginMsgEvent.msg().streamId());
					
					if( itemInfo.msgKey().checkHasName() )
					{
						rsslReqMsg.msgKey().applyHasName();
						rsslReqMsg.msgKey().name(itemInfo.msgKey().name());
					}
					
					if ( itemInfo.msgKey().checkHasNameType())
					{
						rsslReqMsg.msgKey().applyHasNameType();
						rsslReqMsg.msgKey().nameType(itemInfo.msgKey().nameType());
					}
					
					rsslReqMsg.domainType(loginMsgEvent.msg().domainType());
					
					ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
					
					reqMsg.decode(rsslReqMsg, loginMsgEvent.reactorChannel().majorVersion(),
							loginMsgEvent.reactorChannel().minorVersion(), null);
					
					int flags = reqMsg._rsslMsg.flags();
					flags &= ~RequestMsgFlags.STREAMING;
					rsslReqMsg.flags(flags);
					
					_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
					_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
					_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
					_ommServerBaseImpl.ommProviderEvent()._channel = loginMsgEvent.reactorChannel();
					
					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onClose(reqMsg, _ommServerBaseImpl.ommProviderEvent());

					clientSession.resetLoginHandle();
					_ommServerBaseImpl.serverChannelHandler().closeChannel(loginMsgEvent.reactorChannel());
				}
			
				break;
			}
		default:
			
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Rejected unhandled login message type ").append(loginMsg.rdmMsgType().toString() + ".");
			
			_streamId.value(loginMsgEvent.msg().streamId());
			
			ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
			
			if( itemInfo == null )
			{
				sendLoginReject(loginMsgEvent.reactorChannel(), loginMsgEvent.msg().streamId(), StateCodes.USAGE_ERROR , temp.toString() );
			}
			
			if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
        	{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(loginMsgEvent.msg().streamId())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.TRACE));
        	}
			
			break;
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	void sendLoginReject(ReactorChannel reactorChannel, int streamId, int statusCode, String text )
	{
		_loginStatus.clear();
		_loginStatus.streamId(streamId);
		_loginStatus.applyHasState();
		_loginStatus.state().streamState(StreamStates.CLOSED_RECOVER);
		_loginStatus.state().dataState(DataStates.SUSPECT);
		_loginStatus.state().code(statusCode);
		Buffer buffer = CodecFactory.createBuffer();
		buffer.data(text);
		_loginStatus.state().text(buffer);
		
		_rsslSubmitOptions.clear();
		_rsslErrorInfo.clear();
		int retCode = reactorChannel.submit(_loginStatus, _rsslSubmitOptions, _rsslErrorInfo);
		
		if ( retCode != CodecReturnCodes.SUCCESS)
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = _rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in LoginHandler.sendLoginReject().")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.ERROR));
        	}
		}	
	}
	
	void notifyChannelDown(ClientSession clientSession)
	{
		_ommServerBaseImpl.userLock().lock();
		
		for(int index = 0; index < _itemInfoList.size(); index++)
		{
			ItemInfo itemInfo = _itemInfoList.get(index);
			
			if( clientSession == itemInfo.clientSession() )
			{
				RequestMsg rsslReqMsg = _ommServerBaseImpl.rsslRequestMsg();
				
				rsslReqMsg.applyNoRefresh();
				
				rsslReqMsg.streamId((int)itemInfo.streamId().value());
				
				if( itemInfo.msgKey().checkHasName() )
				{
					rsslReqMsg.msgKey().applyHasName();
					rsslReqMsg.msgKey().name(itemInfo.msgKey().name());
				}
				
				if ( itemInfo.msgKey().checkHasNameType())
				{
					rsslReqMsg.msgKey().applyHasNameType();
					rsslReqMsg.msgKey().nameType(itemInfo.msgKey().nameType());
				}
				
				rsslReqMsg.domainType(EmaRdm.MMT_LOGIN);
				
				ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
				
				reqMsg.decode(rsslReqMsg, clientSession.channel().majorVersion(),
						clientSession.channel().minorVersion(), null);
				
				int flags = reqMsg._rsslMsg.flags();
				flags &= ~RequestMsgFlags.STREAMING;
				rsslReqMsg.flags(flags);
				
				_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
				_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
				_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
				_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
				
				_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				_ommServerBaseImpl.ommProviderClient().onClose(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				clientSession.resetLoginHandle();
			}
		}
		
		_ommServerBaseImpl.userLock().unlock();
	}
}
