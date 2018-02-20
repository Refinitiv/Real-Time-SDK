///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayList;

import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

class DirectoryHandler implements RDMDirectoryMsgCallback
{
	private static final String CLIENT_NAME = "DirectoryHandler";

	protected OmmServerBaseImpl _ommServerBaseImpl;
	
	private DirectoryStatus _directoryStatus = (DirectoryStatus)DirectoryMsgFactory.createMsg();
	private DirectoryRefresh _directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
	private ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	private ArrayList<ItemInfo> _itemInfoList = new ArrayList<>();
	private LongObject _streamId = new LongObject();
	private Buffer _statusText = CodecFactory.createBuffer();
	
	private boolean _apiAdminControl;
	
	DirectoryHandler(OmmServerBaseImpl ommServerBaseImpl)
	{
		_ommServerBaseImpl = ommServerBaseImpl;
		_directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
		_directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
		_statusText.data("Source Directory Refresh Completed");
	}
	
	ArrayList<ItemInfo> getItemInfoList()
	{
		return _itemInfoList;
	}

	public void initialize()
	{
		_apiAdminControl = _ommServerBaseImpl.activeConfig().directoryAdminControl() == OmmIProviderConfig.AdminControl.API_CONTROL ? true : false;
	}

	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
	{	
		_ommServerBaseImpl.eventReceived();
		ClientSession clientSession = (ClientSession)directoryMsgEvent.reactorChannel().userSpecObj(); 
		DirectoryMsg directoryMsg = directoryMsgEvent.rdmDirectoryMsg();
		
		if ( directoryMsg == null )
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Directory message rejected - Invalid directory domain message.");
			
			sendDirectoryReject(directoryMsgEvent.reactorChannel(), directoryMsgEvent.msg().streamId(), StateCodes.USAGE_ERROR ,
					temp.toString() );
			
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(directoryMsgEvent.msg().streamId())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.ERROR));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		if ( !_ommServerBaseImpl.activeConfig().acceptMessageWithoutBeingLogin && !clientSession.isLogin() )
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Directory message rejected - there is no logged in user for this session.");
			
			sendDirectoryReject(directoryMsgEvent.reactorChannel(), directoryMsgEvent.msg().streamId(), StateCodes.USAGE_ERROR ,
					temp.toString() );
			
			if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
        	{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(directoryMsgEvent.msg().streamId())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.TRACE));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		switch(directoryMsg.rdmMsgType())
		{
		case REQUEST:
			{	
			   if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Received directory request message.")
                    	.append(OmmLoggerClient.CR).append("Stream Id ").append(directoryMsgEvent.msg().streamId())
                    	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                    	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
                    
                    _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
                }
			   
			   _streamId.value(directoryMsgEvent.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
			   
			   DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsg;
			   
			   if ( _ommServerBaseImpl.activeConfig().acceptDirMessageWithoutMinFilters || 
					   ( ( directoryRequest.filter() & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO ) == com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO )
					   && ( ( directoryRequest.filter() & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE ) == com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE ) )
			   {
					
					if( itemInfo == null )
					{
						itemInfo = ServerPool.getItemInfo();
					
						itemInfo.setRequestMsg((RequestMsg)directoryMsgEvent.msg());
						itemInfo.clientSession(clientSession);
					
						_ommServerBaseImpl.addItemInfo(clientSession, itemInfo);
					
						_itemInfoList.add(itemInfo);
					
						if ( _apiAdminControl == false )
						{
							ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
						
							reqMsg.decode(directoryMsgEvent.msg(), directoryMsgEvent.reactorChannel().majorVersion(),
								directoryMsgEvent.reactorChannel().minorVersion(), null);
						
							_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
							_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
							_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
							_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
						
							_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
							_ommServerBaseImpl.ommProviderClient().onReqMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
						}
						else
						{
							handleDirectoryRequest( directoryMsgEvent.reactorChannel(), directoryRequest);
						}
					}
					else
					{
						itemInfo.setRequestMsg((RequestMsg)directoryMsgEvent.msg());
						
						if ( _apiAdminControl == false )
						{
							ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
							
							reqMsg.decode(directoryMsgEvent.msg(), directoryMsgEvent.reactorChannel().majorVersion(),
									directoryMsgEvent.reactorChannel().minorVersion(), null);
					
							_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
							_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
							_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
							_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
							
							_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
							_ommServerBaseImpl.ommProviderClient().onReissue(reqMsg, _ommServerBaseImpl.ommProviderEvent());
						}
						else
						{
							handleDirectoryRequest( directoryMsgEvent.reactorChannel(), directoryRequest);
						}
					}
			   }
			   else
			   {
				   StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Source directory request rejected - request message must minimally have SERVICE_INFO_FILTER and SERVICE_STATE_FILTER filters");
					
					sendDirectoryReject(directoryMsgEvent.reactorChannel(), directoryMsgEvent.msg().streamId(), StateCodes.USAGE_ERROR ,
							temp.toString() );
					
					if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
		        	{
						temp.append(OmmLoggerClient.CR).append("stream Id ").append(directoryMsgEvent.msg().streamId())
						.append(OmmLoggerClient.CR).append("client handle ").append(clientSession.clientHandle().value());
						_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
			        			temp.toString(), Severity.TRACE));
		        	}
					
					if ( itemInfo != null )
					{
						if ( _apiAdminControl == false )
						{
							notifyOnClose(directoryMsgEvent.reactorChannel(), directoryMsgEvent.msg(), itemInfo);
						}
						
						_itemInfoList.remove(itemInfo);
						_ommServerBaseImpl.removeItemInfo(itemInfo, false);
					}
			   }
		
				break;
			}
		case CONSUMER_STATUS:
		{
		   if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
           {
               StringBuilder temp = _ommServerBaseImpl.strBuilder();
               temp.append("Received directory consumer status message.")
               	.append(OmmLoggerClient.CR).append("Stream Id ").append(directoryMsgEvent.msg().streamId())
               	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
               	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
               
               _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
           }
			
			_streamId.value(directoryMsgEvent.msg().streamId());
			
			ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
			
			if( itemInfo != null )
			{
				GenericMsgImpl genericMsg = _ommServerBaseImpl.genericMsg();
				
				genericMsg.decode(directoryMsgEvent.msg(), directoryMsgEvent.reactorChannel().majorVersion(),
						directoryMsgEvent.reactorChannel().minorVersion(), null);
				
				_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
				_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
				_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
				_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
				
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
		               temp.append("Received directory close message.")
		               	.append(OmmLoggerClient.CR).append("Stream Id ").append(directoryMsgEvent.msg().streamId())
		               	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
		               	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
		               
		               _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
		        }
				
				_streamId.value(directoryMsgEvent.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
				
				if ( itemInfo != null )
				{
					if ( _apiAdminControl == false )
					{
						notifyOnClose(directoryMsgEvent.reactorChannel(), directoryMsgEvent.msg(), itemInfo);
					}
					
					_itemInfoList.remove(itemInfo);
					_ommServerBaseImpl.removeItemInfo(itemInfo, false);
				}
			
				break;
			}
		default:
			
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			temp.append("Rejected unhandled directory message type ").append(directoryMsg.rdmMsgType().toString());
			
			_streamId.value(directoryMsgEvent.msg().streamId());
			
			ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
			
			if( itemInfo == null )
			{
				sendDirectoryReject(directoryMsgEvent.reactorChannel(), directoryMsgEvent.msg().streamId(), StateCodes.USAGE_ERROR , temp.toString() );
			}
			
			if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
        	{
				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(directoryMsgEvent.msg().streamId())
               	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
               	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.TRACE));
        	}
			
			break;
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
	}
	
	void sendDirectoryReject(ReactorChannel reactorChannel, int streamId, int statusCode, String text )
	{
		_directoryStatus.clear();
		_directoryStatus.streamId(streamId);
		_directoryStatus.applyHasState();
		_directoryStatus.state().streamState(StreamStates.CLOSED_RECOVER);
		_directoryStatus.state().dataState(DataStates.SUSPECT);
		_directoryStatus.state().code(statusCode);
		Buffer buffer = CodecFactory.createBuffer();
		buffer.data(text);
		_directoryStatus.state().text(buffer);
		
		_rsslSubmitOptions.clear();
		_rsslErrorInfo.clear();
		
		int retCode = reactorChannel.submit(_directoryStatus, _rsslSubmitOptions, _rsslErrorInfo);
		
		if ( retCode != CodecReturnCodes.SUCCESS)
		{
			StringBuilder temp = _ommServerBaseImpl.strBuilder();
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj(); 
				
	        	temp.append("Internal error: rsslChannel.submit() failed in DirectoryHandler.sendDirectoryReject().")
	        		.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
	        		.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
	        		.append(OmmLoggerClient.CR).append("Error Id ").append(error.errorId())
	        		.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.sysError())
	        		.append(OmmLoggerClient.CR).append("Error Location ").append(_rsslErrorInfo.location())
	        		.append(OmmLoggerClient.CR).append("Error Text ").append(error.text());
	        	
	        	_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.ERROR));
        	}
		}	
	}
	
	void handleDirectoryRequest(ReactorChannel reactorChannel, DirectoryRequest directoryRequest)
	{
		_directoryRefresh.clear();
		_directoryRefresh.state().streamState(StreamStates.OPEN);
	    _directoryRefresh.state().dataState(DataStates.OK);
	    _directoryRefresh.state().code(StateCodes.NONE);
	    _directoryRefresh.state().text(_statusText);
		
		long filters = DirectoryServiceStore.encodeDirectoryMsg(_ommServerBaseImpl.directoryServiceStore().getDirectoryCache().serviceList() , _directoryRefresh.serviceList(),
				directoryRequest.filter(), true, directoryRequest.checkHasServiceId(), directoryRequest.serviceId());
		
		_directoryRefresh.filter(filters);
		_directoryRefresh.applyClearCache();
		_directoryRefresh.streamId(directoryRequest.streamId());
		_directoryRefresh.applySolicited();
		
		_rsslSubmitOptions.clear();
		_rsslErrorInfo.clear();
		
		int retCode = reactorChannel.submit(_directoryRefresh, _rsslSubmitOptions, _rsslErrorInfo);
		
		if ( retCode != CodecReturnCodes.SUCCESS)
		{
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
				ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj(); 
				
	        	temp.append("Internal error: rsslChannel.submit() failed in DirectoryHandler.handleDirectoryRequest().")
	        		.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
	        		.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
	        		.append(OmmLoggerClient.CR).append("Error Id ").append(error.errorId())
	        		.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.sysError())
	        		.append(OmmLoggerClient.CR).append("Error Location ").append(_rsslErrorInfo.location())
	        		.append(OmmLoggerClient.CR).append("Error Text ").append(error.text());
	        	
	        	_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
		}
		
		DirectoryServiceStore.returnServiceToPool(_directoryRefresh.serviceList());
	}
}
