///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.reactor.DefaultMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

class MarketItemHandler implements DefaultMsgCallback
{
	private static final String CLIENT_NAME = "MarketItemHandler";
	
	protected OmmServerBaseImpl _ommServerBaseImpl;
	private EncodeIterator _encodeIterator = CodecFactory.createEncodeIterator();
	private com.thomsonreuters.upa.codec.Msg _rsslMsg = CodecFactory.createMsg();
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
		com.thomsonreuters.upa.codec.Msg msg = msgEvent.msg();
		ReactorChannel reactorChannel = msgEvent.reactorChannel();
		ClientSession clientSession = (ClientSession)msgEvent.reactorChannel().userSpecObj();
		
		if( msg == null )
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
			
			sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
			
			if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
        	{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msgEvent.msg().streamId())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.TRACE));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		switch(msg.msgClass())
		{
			case MsgClasses.REQUEST:
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Received request message.")
                    .append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
                       
                    _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
                    		temp.toString(), Severity.TRACE).toString());
                }
				
				_streamId.value(msg.streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
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
							
							sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
							
							if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
				        	{
								temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
			                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
			                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
								
								_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
					        			temp.toString(), Severity.TRACE));
				        	}
							
							if ( itemInfo != null )
							{
								notifyOnClose(reactorChannel, msg, itemInfo);
							}
							
							break;
						}
						
						if ( !_ommServerBaseImpl.activeConfig().acceptMessageWithoutQosInRange && _isDirectoryApiControl 
								&& !iProviderServcieStore.isValidQosRange(msg.msgKey().serviceId(), (RequestMsg)msg) )
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
							
							sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
							
							if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
				        	{
								temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
			                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
			                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
								
								_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
					        			temp.toString(), Severity.TRACE));
				        	}
							
							if ( itemInfo != null )
							{
								notifyOnClose(reactorChannel, msg, itemInfo);
							}
							
							break;
						}
					}
					else
					{
						StringBuilder temp = _ommServerBaseImpl.strBuilder();
						temp.append("Request Message rejected - the service Id = ").append(msg.msgKey().serviceId())
						.append(" does not exist in the source directory.");
						
						sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
						
						if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
			        	{
							temp.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
		                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
		                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
							
							_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
				        			temp.toString(), Severity.TRACE));
			        	}
						
						if ( itemInfo != null )
						{
							notifyOnClose(reactorChannel, msg, itemInfo);
						}
						
						break;
					}
				}
				else
				{
					reqMsg.decode(msg, reactorChannel.majorVersion(),
							reactorChannel.minorVersion(), null);
				}
				
				_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
				_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
				_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
				
				if( itemInfo == null )
				{
					itemInfo = ServerPool.getItemInfo();
				
					itemInfo.setRequestMsg((RequestMsg)msgEvent.msg());
					itemInfo.clientSession(clientSession);
					
					
					if ( !_ommServerBaseImpl.activeConfig().acceptMessageSameKeyButDiffStream )
					{
						if ( clientSession.checkingExistingReq(itemInfo) )
						{
							StringBuilder temp = _ommServerBaseImpl.strBuilder();
							temp.append("Request Message rejected - Item already open with exact same message key on another stream.");
							
							sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
							
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
				
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
					_ommServerBaseImpl.ommProviderEvent()._channel = msgEvent.reactorChannel();
				
					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onReqMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				}
				else
				{
					boolean setMessageKey = false;
					
					if ( msg.msgKey().checkHasServiceId() && itemInfo.msgKey().checkHasServiceId() )
					{
						if ( itemInfo.msgKey().serviceId() !=  msg.msgKey().serviceId() )
						{
							if (!_ommServerBaseImpl.activeConfig().acceptMessageThatChangesService)
							{
								StringBuilder temp = _ommServerBaseImpl.strBuilder();
								temp.append("Request Message rejected - Attempt to reissue the service Id from ")
									.append(itemInfo.serviceId()).append(" to ").append(msg.msgKey().serviceId())
									.append(" while this is not supported.");
							
								sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
							
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
									
									itemInfo.setRequestMsg((RequestMsg)msgEvent.msg());
									
									setMessageKey = true;
									
									_ommServerBaseImpl.addItemGroup(itemInfo, itemInfo.itemGroup());
								}
							}
						}
					}
					
					if(!setMessageKey)
					{
						itemInfo.setRequestMsg((RequestMsg)msgEvent.msg());
					}
					
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
					_ommServerBaseImpl.ommProviderEvent()._channel = msgEvent.reactorChannel();
					
					_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onReissue(reqMsg, _ommServerBaseImpl.ommProviderEvent());
				}
			}	
			break;
				
			case MsgClasses.CLOSE:			
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received close message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				_streamId.value(msg.streamId());
				_ommServerBaseImpl.ommProviderEvent()._channel = msgEvent.reactorChannel();

				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
				if ( itemInfo != null )
				{
					notifyOnClose(reactorChannel, msg, itemInfo);
				}
			}
			break;			
			case MsgClasses.POST:
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received post message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
					.append(OmmLoggerClient.CR).append("Post message is not support for this release.");
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
			}
			break;	
			case MsgClasses.GENERIC:
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received generic message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(msg.streamId())
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				_streamId.value(msg.streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
				if ( itemInfo != null )
				{
					GenericMsgImpl genericMsg = _ommServerBaseImpl.genericMsg();
					
					DataDictionary dataDictionary = null;
					
					if ( itemInfo.msgKey().checkHasServiceId() )
					{
						dataDictionary = _ommServerBaseImpl.dictionaryHandler().getDictionaryByServiceId(itemInfo.msgKey().serviceId());
					}
					
					genericMsg.decode(msg, msgEvent.reactorChannel().majorVersion(),
							msgEvent.reactorChannel().minorVersion(), dataDictionary);
					
					_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
					_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
					_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
					_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
					_ommServerBaseImpl.ommProviderEvent()._channel = msgEvent.reactorChannel();
					
					_ommServerBaseImpl.ommProviderClient().onAllMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
					_ommServerBaseImpl.ommProviderClient().onGenericMsg(genericMsg, _ommServerBaseImpl.ommProviderEvent());
				}
			}
			break;
			default:
			{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Rejected unhandled message type ").append(MsgClasses.toString(msg.msgClass()));
				
				_streamId.value(msgEvent.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
				if( itemInfo == null )
				{
					sendRejectMessage(msgEvent.reactorChannel(), msg, StateCodes.USAGE_ERROR , temp.toString() );
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
	
	void notifyOnClose(ReactorChannel reactorChannel, com.thomsonreuters.upa.codec.Msg msg, ItemInfo itemInfo)
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
	
	void sendRejectMessage(ReactorChannel reactorChannel, com.thomsonreuters.upa.codec.Msg msg, int statusCode, String text)
	{
		ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj();
		
		com.thomsonreuters.upa.codec.StatusMsg statusMsg = (StatusMsg)_rsslMsg;
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
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
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
}
