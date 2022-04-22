///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.valueadd.reactor.DefaultMsgCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

class MarketItemHandler implements DefaultMsgCallback
{
	private static final String CLIENT_NAME = "MarketItemHandler";
	
	protected OmmServerBaseImpl _ommServerBaseImpl;
	private EncodeIterator _encodeIterator = CodecFactory.createEncodeIterator();
	private com.refinitiv.eta.codec.Msg _rsslMsg = CodecFactory.createMsg();
	private Buffer _msgBuffer = CodecFactory.createBuffer();
	private Buffer _textBuffer = CodecFactory.createBuffer();
	private ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	private LongObject _streamId = new LongObject();
	private boolean _isDirectoryApiControl;
	
	MarketItemHandler(OmmServerBaseImpl ommServerBaseImpl)
	{
		_ommServerBaseImpl = ommServerBaseImpl;
		_isDirectoryApiControl = ommServerBaseImpl._activeServerConfig.directoryAdminControl() == OmmIProviderConfig.AdminControl.API_CONTROL ? true : false;
	}

	public void initialize()
	{
		_msgBuffer.data(ByteBuffer.allocate(1024));
		_textBuffer.data(ByteBuffer.allocate(256));
	}

	@Override
	public int defaultMsgCallback(ReactorMsgEvent msgEvent)
	{
		_ommServerBaseImpl.eventReceived();
		com.refinitiv.eta.codec.Msg msg = msgEvent.msg();
		ReactorChannel reactorChannel = msgEvent.reactorChannel();
		ClientSession clientSession = (ClientSession) msgEvent.reactorChannel().userSpecObj();
		
		if (msg == null)
		{
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Received error message.")
				.append(OmmLoggerClient.CR).append("ErrorText ").append(msgEvent.errorInfo().error().text())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				
				_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, 
						temp.toString(), Severity.ERROR));
        	}
				
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		if ( !_ommServerBaseImpl.activeConfig().acceptMessageWithoutBeingLogin && !clientSession.isLogin() )
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Message rejected - there is no logged in user for this session.");
			
			sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());
			
			if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
			{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());

				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
						temp.toString(), Severity.TRACE));
			}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		ItemInfo itemInfo = clientSession.getItemInfo(_streamId.value(msg.streamId()));
		switch (msg.msgClass())
		{
			case MsgClasses.REQUEST:
			{
				logReceivedMessage(msg, clientSession, "request");
				
				ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
				int flags = msg.msgKey().flags();
				
				if ( (flags & MsgKeyFlags.HAS_SERVICE_ID) == MsgKeyFlags.HAS_SERVICE_ID )
				{
					reqMsg.decode(msg, reactorChannel.majorVersion(),
							reactorChannel.minorVersion(),  _ommServerBaseImpl.dictionaryHandler().getDictionaryByServiceId(msg.msgKey().serviceId()));
					
					String serviceName = _ommServerBaseImpl.directoryServiceStore().serviceName(msg.msgKey().serviceId());
					
					if (serviceName != null)
					{
						flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
				
						reqMsg._rsslMsg.msgKey().flags(flags);
				
						reqMsg.msgServiceName(serviceName);
				
						reqMsg._rsslMsg.msgKey().flags( flags | MsgKeyFlags.HAS_SERVICE_ID);
						
						OmmIProviderDirectoryStore iProviderServcieStore = (OmmIProviderDirectoryStore)_ommServerBaseImpl.directoryServiceStore();
						
						if ( !_ommServerBaseImpl.activeConfig().acceptMessageWithoutAcceptingRequests && _isDirectoryApiControl 
								&& !iProviderServcieStore.isAcceptingRequests(msg.msgKey().serviceId()) )
						{
							StringBuilder temp = _ommServerBaseImpl.strBuilder();
							temp.append("Request message rejected - the service Id = ").append(msg.msgKey().serviceId())
							.append(" does not accept any requests.");

							sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());

							if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
							{
								temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
										.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
										.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());

								_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
										temp.toString(), Severity.TRACE));
							}

							if (itemInfo != null)
							{
								notifyOnClose(reactorChannel, msg, itemInfo);
							}

							break;
						}

						if (!_ommServerBaseImpl.activeConfig().acceptMessageWithoutQosInRange && _isDirectoryApiControl && !iProviderServcieStore.isValidQosRange(msg.msgKey().serviceId(), (RequestMsg) msg))
						{
							StringBuilder temp = _ommServerBaseImpl.strBuilder();
							temp.append("Request message rejected - the service Id = ").append(msg.msgKey().serviceId())
							.append(" does not support the specified QoS(");
							
							RequestMsg requestMsg = (RequestMsg)msg;
							
							temp.append(requestMsg.qos()).append(")");
							
							if ( requestMsg.checkHasWorstQos() )
							{
								temp.append(" and Worst QoS(").append(requestMsg.worstQos()).append(").");
							}
							else
							{
								temp.append(".");
							}
							
							sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());
							
							if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
							{
								temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
										.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
										.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
								
								_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
										temp.toString(), Severity.TRACE));
							}
							
							if (itemInfo != null)
							{
								notifyOnClose(reactorChannel, msg, itemInfo);
							}
							
							break;
						}
					}
					else
					{
						handleNonExistentServiceId(msg, reactorChannel, clientSession, itemInfo);
						
						break;
					}
				}
				else
				{
					reqMsg.decode(msg, reactorChannel.majorVersion(),
							reactorChannel.minorVersion(), null);
				}
				
				if (itemInfo == null)
				{
					itemInfo = ServerPool.getItemInfo();
					
					itemInfo.setRequestMsg((RequestMsg) msg);
					itemInfo.clientSession(clientSession);
					
					
					if (!_ommServerBaseImpl.activeConfig().acceptMessageSameKeyButDiffStream)
					{
						if (clientSession.checkingExistingReq(itemInfo))
						{
							StringBuilder temp = _ommServerBaseImpl.strBuilder();
							temp.append("Request Message rejected - Item already open with exact same message key on another stream.");
							
							sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());
							
							if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
							{
								temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
								.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
								.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
								
								_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
										temp.toString(), Severity.TRACE));
							}
							
							itemInfo.returnToPool();
							
							return ReactorCallbackReturnCodes.SUCCESS;
						}
					}
					
					_ommServerBaseImpl.addItemInfo(clientSession, itemInfo);
					
					setCommonProviderEventAttributes(reactorChannel, itemInfo.handle(), clientSession.clientHandle());
					
					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onReqMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				}
				else
				{
					boolean setMessageKey = false;
					
					if (msg.msgKey().checkHasServiceId() && itemInfo.msgKey().checkHasServiceId())
					{
						if (itemInfo.msgKey().serviceId() != msg.msgKey().serviceId())
						{
							if (!_ommServerBaseImpl.activeConfig().acceptMessageThatChangesService)
							{
								StringBuilder temp = _ommServerBaseImpl.strBuilder();
								temp.append("Request Message rejected - Attempt to reissue the service Id from ")
									.append(itemInfo.serviceId()).append(" to ").append(msg.msgKey().serviceId())
									.append(" while this is not supported.");
								
								sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());
								
								if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
								{
									temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
									.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
									.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
								
									_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
											temp.toString(), Severity.TRACE));
								}
							
								notifyOnClose(reactorChannel, msg, itemInfo);
							
								break;
							}
							else
							{
								if ( itemInfo.hasItemGroup() )
								{
									_ommServerBaseImpl.removeItemGroup(itemInfo);
									
									itemInfo.setRequestMsg((RequestMsg) msg);
									
									setMessageKey = true;
									
									_ommServerBaseImpl.addItemGroup(itemInfo, itemInfo.itemGroup());
								}
							}
						}
					}
					
					if (!setMessageKey)
					{
						itemInfo.setRequestMsg((RequestMsg) msg);
					}
					
					setCommonProviderEventAttributes(reactorChannel, itemInfo.handle(), clientSession.clientHandle());
					
					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onReissue(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				}
				break;
			}
			case MsgClasses.CLOSE:
			{
				logReceivedMessage(msg, clientSession, "close");
				
				_ommServerBaseImpl.ommProviderEvent()._channel = reactorChannel;
				if (itemInfo != null)
				{
					notifyOnClose(reactorChannel, msg, itemInfo);
				}
				break;
			}
			case MsgClasses.POST:
			{
				processPost(msg, reactorChannel, clientSession, itemInfo);
				break;
			}
			case MsgClasses.GENERIC:
			{
				logReceivedMessage(msg, clientSession, "generic");
				if (itemInfo != null)
				{
					GenericMsgImpl genericMsg = _ommServerBaseImpl.genericMsg();

					genericMsg.decode(msg, reactorChannel.majorVersion(),
							reactorChannel.minorVersion(), getDataDictionary(itemInfo, msg));

					setCommonProviderEventAttributes(reactorChannel, itemInfo.handle(), clientSession.clientHandle());

					_ommServerBaseImpl.ommProviderClient().onAllMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onGenericMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
				}
				break;
			}
			default:
			{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Rejected unhandled message type ").append(MsgClasses.toString(msg.msgClass()));

				if (itemInfo == null)
				{
					sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());
				}

				if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
				{
					temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
							.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
							.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());

					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}
			}
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	void notifyOnClose(ReactorChannel reactorChannel, com.refinitiv.eta.codec.Msg msg, ItemInfo itemInfo)
	{
		RequestMsg rsslReqMsg = _ommServerBaseImpl.rsslRequestMsg();
		
		rsslReqMsg.applyNoRefresh();
		
		rsslReqMsg.streamId(msg.streamId());
		
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
		
		rsslReqMsg.domainType(msg.domainType());
	
		ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
							
		if( itemInfo.msgKey().checkHasServiceId())
		{
			rsslReqMsg.msgKey().applyHasServiceId();
			rsslReqMsg.msgKey().serviceId(itemInfo.msgKey().serviceId());
			
			reqMsg.decode(rsslReqMsg, reactorChannel.majorVersion(),
					reactorChannel.minorVersion(), null);
			
			String serviceName = _ommServerBaseImpl.directoryServiceStore().serviceName(itemInfo.msgKey().serviceId());
			
			int flags = reqMsg._rsslMsg.msgKey().flags();
			
			if (serviceName != null)
			{
				flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
		
				reqMsg._rsslMsg.msgKey().flags(flags);
		
				reqMsg.msgServiceName(serviceName);
		
				reqMsg._rsslMsg.msgKey().flags( flags | MsgKeyFlags.HAS_SERVICE_ID);
			}
		}
		else
		{
			reqMsg.decode(rsslReqMsg, reactorChannel.majorVersion(),
					reactorChannel.minorVersion(), null);
		}
		
		int flags = reqMsg._rsslMsg.flags();
		flags &= ~RequestMsgFlags.STREAMING;
		rsslReqMsg.flags(flags);
		
		_ommServerBaseImpl.ommProviderEvent()._clientHandle = itemInfo.clientSession().clientHandle();
		_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
		_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
		_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
		
		_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
		_ommServerBaseImpl.ommProviderClient().onClose(reqMsg, _ommServerBaseImpl.ommProviderEvent());
		
		_ommServerBaseImpl.removeItemInfo(itemInfo, true);
	}
	
	void sendRejectMessage(ReactorChannel reactorChannel, com.refinitiv.eta.codec.Msg msg, int statusCode, String text)
	{
		ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj();
		
		com.refinitiv.eta.codec.StatusMsg statusMsg = (StatusMsg)_rsslMsg;
		statusMsg.clear();
		
		statusMsg.msgClass(MsgClasses.STATUS);
		statusMsg.streamId(msg.streamId());
		statusMsg.domainType(msg.domainType());
		statusMsg.containerType(DataTypes.NO_DATA);
		statusMsg.applyHasState();
		statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
		statusMsg.state().dataState(DataStates.SUSPECT);
		statusMsg.state().code(statusCode);
		_textBuffer.data(text);
		statusMsg.state().text(_textBuffer);
		
		if ( msg.msgClass() == MsgClasses.REQUEST)
		{
			if ( ((RequestMsg)msg).checkPrivateStream()  )
			{
				statusMsg.applyPrivateStream();
			}
		}
		
		_encodeIterator.clear();
		
		int retCode = _encodeIterator.setBufferAndRWFVersion(_msgBuffer, Codec.majorVersion(), Codec.minorVersion());
		
		if( retCode !=  CodecReturnCodes.SUCCESS )
		{
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Internal error. Failed to set decode iterator buffer and version in MarketPriceHandler.sendRejectMessage().")
            	.append(OmmLoggerClient.CR).append("Client Handle ").append(clientSession.clientHandle().value())
            	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
						temp.toString(), Severity.ERROR));
        	}
			
			return;
		}
		
		while ((retCode = statusMsg.encode(_encodeIterator)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		{
			_msgBuffer = Utilities.realignBuffer(_encodeIterator, _msgBuffer.capacity() * 2);
		}
		
		if (retCode < CodecReturnCodes.SUCCESS)
		{
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Internal error. Failed to encode status message in MarketPriceHandler.sendRejectMessage().")
            	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
            	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
						temp.toString(), Severity.ERROR));
        	}
				
			return;
		}
		
		_rsslErrorInfo.clear();
		if (ReactorReturnCodes.SUCCESS > (retCode = clientSession.channel().submit(statusMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {			
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = _rsslErrorInfo.error();
				
				_ommServerBaseImpl.strBuilder().append("Internal error: rsslChannel.submit() failed in MarketItemHandler.sendRejectMessage().")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
					.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
					.append(OmmLoggerClient.CR).append("Error Id ").append(error.errorId())
					.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.sysError())
					.append(OmmLoggerClient.CR).append("Error Location ").append(_rsslErrorInfo.location())
	    			.append("Error Text ").append(error.text());
	        	
				_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, _ommServerBaseImpl._strBuilder.toString(),
						Severity.ERROR));
        	}
			
			return;
	    }
	}

	private void processPost(Msg msg, ReactorChannel reactorChannel, ClientSession clientSession, ItemInfo itemInfo)
	{
		logReceivedMessage(msg, clientSession, "post");
		if (itemInfo == null)
		{
			return;
		}
		PostMsgImpl postMsg = _ommServerBaseImpl.postMsg();
		postMsg.decode(msg, reactorChannel.majorVersion(),
				reactorChannel.minorVersion(), getDataDictionary(itemInfo, msg));

		if (((PostMsg) msg).checkHasMsgKey() && msg.msgKey().checkHasServiceId())
		{
			String serviceName = _ommServerBaseImpl.directoryServiceStore().serviceName(msg.msgKey().serviceId());
			if (serviceName == null)
			{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Post Message invalid - the service Id = ").append(msg.msgKey().serviceId())
						.append(" does not exist in the source directory.");

				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
				{
					temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
							.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
							.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
		
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
							temp.toString(), Severity.TRACE));
				}
			}
			else
			{
				postMsg.serviceName(serviceName);
			}
		}

		setCommonProviderEventAttributes(reactorChannel, itemInfo.handle(), clientSession.clientHandle());
		if(_ommServerBaseImpl.activeConfig().enforceAckIDValidation) {
			itemInfo.addPostId(postMsg.postId());
		}
		
		_ommServerBaseImpl.ommProviderClient().onAllMsg(postMsg, _ommServerBaseImpl.ommProviderEvent());
		_ommServerBaseImpl.ommProviderClient().onPostMsg(postMsg, _ommServerBaseImpl.ommProviderEvent());
	}

	private void logReceivedMessage(Msg msg, ClientSession clientSession, String messageName)
	{
		if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Received ").append(messageName).append(" message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
					.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
					.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());

			_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
					temp.toString(), Severity.TRACE));
		}
	}

	private void setCommonProviderEventAttributes(ReactorChannel reactorChannel, LongObject handle, LongObject clientHandle)
	{
		_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientHandle;
		_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
		_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
		_ommServerBaseImpl.ommProviderEvent()._channel = reactorChannel;
		_ommServerBaseImpl.ommProviderEvent()._handle = handle;
	}

	private void handleNonExistentServiceId(Msg msg, ReactorChannel reactorChannel, ClientSession clientSession, ItemInfo itemInfo)
	{
		StringBuilder temp = _ommServerBaseImpl.strBuilder();
		temp.append("Request Message rejected - the service Id = ").append(msg.msgKey().serviceId())
				.append(" does not exist in the source directory.");

		sendRejectMessage(reactorChannel, msg, StateCodes.USAGE_ERROR, temp.toString());

		if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
		{
			temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
					.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
					.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());

			_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
					temp.toString(), Severity.TRACE));
		}

		if (itemInfo != null)
		{
			notifyOnClose(reactorChannel, msg, itemInfo);
		}
	}


	private DataDictionary getDataDictionary(ItemInfo itemInfo, Msg msg)
	{
		if (itemInfo.msgKey().checkHasServiceId())
		{
			return _ommServerBaseImpl.dictionaryHandler().getDictionaryByServiceId(itemInfo.msgKey().serviceId());
		} else if (msg.msgKey().checkHasServiceId())
		{
		return _ommServerBaseImpl.dictionaryHandler().getDictionaryByServiceId(msg.msgKey().serviceId());
		}
		return null;
	}
}
