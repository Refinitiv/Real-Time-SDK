///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.thomsonreuters.ema.access.OmmBaseImpl.OmmImplState;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceInfo;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

class DirectoryCallbackClient<T> extends CallbackClient<T> implements RDMDirectoryMsgCallback
{
	private static final String CLIENT_NAME = "DirectoryCallbackClient";
	
	private Map<Integer, Directory>					_serviceById;
	private Map<String, Directory>					_serviceByName;

	DirectoryCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		super(baseImpl, CLIENT_NAME);
		 
		int initialHashSize =  (int)(_baseImpl.activeConfig().serviceCountHint/ 0.75 + 1);
		_serviceById = new HashMap<Integer, Directory>(initialHashSize);
		_serviceByName = new HashMap<String, Directory>(initialHashSize);
	}

	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
	{
		_baseImpl.eventReceived();
		
		DirectoryMsg directoryMsg = event.rdmDirectoryMsg();
		ReactorChannel rsslReactorChannel = event.reactorChannel();
		ChannelInfo channelInfo = (ChannelInfo)rsslReactorChannel.userSpecObj();
		
		if (directoryMsg == null)
		{
			_baseImpl.closeRsslChannel(rsslReactorChannel);

			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = event.errorInfo().error();
				
	        	StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Received event without RDMDirectory message").append(OmmLoggerClient.CR)
	        		.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode())).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());

	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		Object item = event.streamInfo() != null ? event.streamInfo().userSpecObject() : null;
		if (item != null)
			return processCallback(event, rsslReactorChannel, (SingleItem<T>)item);

		switch (directoryMsg.rdmMsgType())
		{
			case REFRESH:
			{
				com.thomsonreuters.upa.codec.State state = ((DirectoryRefresh)directoryMsg).state();
	
				if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
				{
					_baseImpl.closeRsslChannel(rsslReactorChannel);
	
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDirectory stream was closed with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
					
					_baseImpl.directoryCallbackClient().processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), rsslReactorChannel);
	
					break;
				}
				else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDirectory stream state was changed to suspect with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					_baseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
	
					_baseImpl.directoryCallbackClient().processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), rsslReactorChannel);
					break;
				}
	
				_baseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
	
				if (_baseImpl.loggerClient().isTraceEnabled())
	        	{
		        	StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("RDMDirectory stream state was open with refresh message ").append(OmmLoggerClient.CR)
		        		.append("State: ").append(state.toString());
	
		        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
	        	}
				
	
				_baseImpl.directoryCallbackClient().processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), rsslReactorChannel);
				break;
			}
			case STATUS:
			{
				if (((DirectoryStatus)directoryMsg).checkHasState())
		    	{
					com.thomsonreuters.upa.codec.State state = ((DirectoryStatus)directoryMsg).state();
	
					if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
					{
						_baseImpl.closeRsslChannel(rsslReactorChannel);
	
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("RDMDirectory stream was closed with status message ").append(OmmLoggerClient.CR)
				        		.append("State: ").append(state.toString());
	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						
						return ReactorCallbackReturnCodes.SUCCESS;
					}
					else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
					{
						if (_baseImpl.loggerClient().isWarnEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("RDMDirectory stream state was changed to suspect with status message ").append(OmmLoggerClient.CR)
				        		.append("State: ").append(state.toString());
	
				        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
			        	}
	
						_baseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
						break;
					}
					
					if (_baseImpl.loggerClient().isTraceEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMDirectory stream was open with status message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		        	}
	
					_baseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
				}
				else
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMDirectory status message without the state", Severity.WARNING));
		        	}
				}
				break;
			}
			case UPDATE:
			{
				if (_baseImpl.loggerClient().isTraceEnabled())
	        	{
		        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMDirectory update message", Severity.TRACE));
	        	}
	
				_baseImpl.directoryCallbackClient().processDirectoryPayload(((DirectoryUpdate)directoryMsg).serviceList(), rsslReactorChannel);
				break;
			}
			default:
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _baseImpl.strBuilder();
					
		        	temp.append("Received unknown RDMDirectory message type")
		        		.append(OmmLoggerClient.CR)
		        		.append("message type value ").append(directoryMsg.rdmMsgType());
	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
				break;
			}
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	void processDirectoryPayload(List<Service> serviceList, ReactorChannel channel)
	{
		ChannelInfo chnlInfo = (ChannelInfo)channel.userSpecObj();
		
		if (serviceList == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMDirectory message without a service list", Severity.ERROR));
        	}
			return;
		}

		if (chnlInfo == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Internal error: no reactorChannel().userSpecObj()", Severity.ERROR));
        	}
			return;
		}


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
							_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMService with Add action but no Service Info", Severity.ERROR));
			        	}
						break;
					}

					String serviceName = oneService.info().serviceName().toString();
		            if(serviceName == null)
		            {
		            	if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
							_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMService with Add action but no Service Info", Severity.ERROR));
			        	}
						break;
		            }

		            Service existService = null;
		            Directory existDirectory = null;
		            if (_serviceByName.size() > 0)
		            {
		            	existDirectory = _serviceByName.get(serviceName);
		            	existService = existDirectory != null ? existDirectory.service() : null;
		            }
		            if (existService != null)
		            {
		            	if (existService.serviceId() != oneService.serviceId())
						{
		            		_serviceById.remove(existService.serviceId());
		            		existService.serviceId(oneService.serviceId());
		            		_serviceById.put(existService.serviceId(), existDirectory);
		            		
						}
		            	if( existDirectory.channelInfo() != chnlInfo )
		            	{
		            		chnlInfo.rsslDictionary(existDirectory.channelInfo().rsslDictionary());
		            	}
		            }
		            else
		            {    
		            	Service newService = DirectoryMsgFactory.createService();
		            	oneService.copy(newService);
		            	
		            	Directory directory = new Directory(newService); 
		            	directory.channelInfo(chnlInfo);
		            	directory.serviceName(serviceName);
		            	
		            	_serviceById.put(oneService.serviceId(), directory);
		            	_serviceByName.put(serviceName, directory);

						if (newService.state().acceptingRequests() == 1 && newService.state().serviceState() == 1)
							_baseImpl.dictionaryCallbackClient().downloadDictionary(directory);
						
		            }
	
					break;
				}
				case MapEntryActions.UPDATE :
				{
					Service existService = null;
					Directory existDirectory = null; 
			        if (_serviceById.size() > 0)
			        {
			        	existDirectory = _serviceById.get((Integer)oneService.serviceId());
			        	existService = existDirectory.service();
			        }
			        
			        if (existService == null)
					{
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("Received Update action for unknown Service with service id ")
				        		.append(oneService.serviceId());
				        	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						break;
					}
					else if (_baseImpl.loggerClient().isTraceEnabled())
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
						temp.append("Received Update action for RDMService").append(OmmLoggerClient.CR)
							.append("Service name ").append(existService.info().serviceName().toString()).append(OmmLoggerClient.CR)
							.append("Service id ").append(existService.serviceId());
						
						_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
					}
	            	if((existDirectory != null) && existDirectory.channelInfo() != chnlInfo )
	            	{
	            		chnlInfo.rsslDictionary(existDirectory.channelInfo().rsslDictionary());
	            	}

					if (oneService.checkHasInfo())
					{
	
						ServiceInfo  existInfo = existService.info();
						if (!(existInfo.serviceName().equals(oneService.info().serviceName())))
						{
							if (_baseImpl.loggerClient().isErrorEnabled())
				        	{
					        	StringBuilder temp = _baseImpl.strBuilder();
								
					        	temp.append("Received Update action for RDMService").append(OmmLoggerClient.CR)
					        		.append("Service name ").append(existInfo.serviceName().toString()).append(OmmLoggerClient.CR)
									.append("Service id ").append(existService.serviceId()).append(OmmLoggerClient.CR)
									.append("attempting to change service name to ").append(oneService.info().serviceName().toString());
					        	
					        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
				        	}
							break;
						}
						
						oneService.info().copy(existInfo);
					}
					
					if (oneService.checkHasState())
					{
						oneService.state().copy(existService.state());
						if (oneService.state().acceptingRequests() == 1 && oneService.state().serviceState() == 1)
							_baseImpl.dictionaryCallbackClient().downloadDictionary(existDirectory);
					}
					
					existService.action(MapEntryActions.UPDATE);
	
					break;
				}
				case MapEntryActions.DELETE :
				{
					Service existService = null;
			        if (_serviceById.size() > 0)
			        	existService = _serviceById.get((Integer)oneService.serviceId()).service();
					
					if (existService == null)
					{
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
							
				        	temp.append("Received Delete action for unknown RDMService with service id ")
				        		.append(oneService.serviceId());
				        	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						break;
					}
					else if (_baseImpl.loggerClient().isTraceEnabled())
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
						temp.append("Received Delete action for RDMService").append(OmmLoggerClient.CR)
							.append("Service name ").append(existService.info().serviceName().toString()).append(OmmLoggerClient.CR)
							.append("Service id ").append(existService.serviceId());
						
						_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
					}
	
					existService.action(MapEntryActions.DELETE);
					break;
				}
				default :
				{
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("Received unknown action for RDMService. Action value ")
			        		.append(oneService.action());
			        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
					break;
				}
			}
		}
	}

	int processCallback(RDMDirectoryMsgEvent event, ReactorChannel rsslReactorChannel, SingleItem<T> item)
	{
		Msg rsslMsg = event.msg();
		switch (event.rdmDirectoryMsg().rdmMsgType())
		{
			case REFRESH:
				{
					_refreshMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), 
					((ChannelInfo)rsslReactorChannel.userSpecObj()).rsslDictionary());
	
					_eventImpl._item = item;
					
					notifyOnAllMsg(_refreshMsg);
					notifyOnRefreshMsg();
	
					int rsslStreamState = ((com.thomsonreuters.upa.codec.RefreshMsg)rsslMsg).state().streamState();
					if (rsslStreamState == StreamStates.NON_STREAMING)
					{
						if (((com.thomsonreuters.upa.codec.RefreshMsg)rsslMsg).checkRefreshComplete())
							_eventImpl._item.remove();
					}
					else if (rsslStreamState != StreamStates.OPEN)
					{
						_eventImpl._item.remove();
					}
				}
				break;
			case UPDATE :
				{
					if (_updateMsg == null)
						_updateMsg = new UpdateMsgImpl(_baseImpl._objManager);
					
					_updateMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), 
							((ChannelInfo)rsslReactorChannel.userSpecObj()).rsslDictionary());
	
					_eventImpl._item = item;
					
					notifyOnAllMsg(_updateMsg);
					notifyOnUpdateMsg();
				}
				break;
			case STATUS :
				{
					if (_statusMsg == null)
						_statusMsg = new StatusMsgImpl(_baseImpl._objManager);
					
					_statusMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
	
					_eventImpl._item = item;
					
					notifyOnAllMsg(_statusMsg);
					notifyOnStatusMsg();
	
					if (((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).checkHasState() &&  
							((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).state().streamState() != StreamStates.OPEN) 
						_eventImpl._item.remove();
				}
				break;
			case CONSUMER_STATUS :
				{
					if (_genericMsg == null)
						_genericMsg = new GenericMsgImpl();
					
					_genericMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
	
					_eventImpl._item = item;
					
					notifyOnAllMsg(_genericMsg);
					notifyOnGenericMsg();
				}
				break;
			default :
				{
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME,
			        			"Internal error. Received unexpected type of RDMDirectoryMsg in DirectoryCallbackClient.processCallback()",
			        			Severity.ERROR));
		        	}
					break;
				}
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	void initialize()
	{
		if (_baseImpl.activeConfig().rsslDirectoryRequest == null)
		{
			_baseImpl.activeConfig().rsslDirectoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
			_baseImpl.activeConfig().rsslDirectoryRequest.clear();
			_baseImpl.activeConfig().rsslDirectoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
			_baseImpl.activeConfig().rsslDirectoryRequest.streamId(2);
			_baseImpl.activeConfig().rsslDirectoryRequest.applyStreaming();
			_baseImpl.activeConfig().rsslDirectoryRequest.filter(
									 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO |
									 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE |
									 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.GROUP |
									 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LOAD |
									 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.DATA |
									 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LINK);
		}
		else
		{
			_baseImpl.activeConfig().rsslDirectoryRequest.streamId(2);
			long filter = _baseImpl.activeConfig().rsslDirectoryRequest.filter();
			if (filter == 0)
			{			
				if (_baseImpl.loggerClient().isWarnEnabled())
				{
					_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(CLIENT_NAME,
													"Configured source directory request message contains no filter. Will request all filters",
													Severity.WARNING).toString());
				}
				
				_baseImpl.activeConfig().rsslDirectoryRequest.filter(
										 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO |
										 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE |
										 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.GROUP |
										 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LOAD |
										 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.DATA |
										 com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LINK);
			}

			if (!_baseImpl.activeConfig().rsslDirectoryRequest.checkStreaming())
			{
				_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(CLIENT_NAME, 
						                  		"Configured source directory request message contains no streaming flag. Will request streaming",
												Severity.WARNING).toString());
				
				_baseImpl.activeConfig().rsslDirectoryRequest.applyStreaming();
			}
		}

		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			
			temp.append("RDMDirectoryRequest message was populated with Filter(s)");
			int filter = (int)_baseImpl.activeConfig().rsslDirectoryRequest.filter();
			
			if ((filter & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_INFO_FILTER");
			if ((filter & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_STATE_FILTER");
			if ((filter & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.GROUP) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_GROUP_FILTER");
			if ((filter & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LOAD) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_LOAD_FILTER");
			if ((filter & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.DATA) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_DATA_FILTER"); 
			if ((filter & com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LINK) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_LINK_FILTER");

			if (_baseImpl.activeConfig().rsslDirectoryRequest.checkHasServiceId())
				temp.append(OmmLoggerClient.CR).append("requesting serviceId ").append(_baseImpl.activeConfig().rsslDirectoryRequest.serviceId());
			else
				temp.append(OmmLoggerClient.CR).append("requesting all services");

			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
											Severity.TRACE));
		}
	}
	
	DirectoryRequest rsslDirectoryRequest()
	{
		return _baseImpl.activeConfig().rsslDirectoryRequest;
	}
	
	Directory directory(String serviceName)
	{
		return _serviceByName.get(serviceName);
	}
	
	Directory directory(int serviceId)
	{
		return _serviceById.get(serviceId);
	}
}

class DirectoryCallbackClientConsumer extends DirectoryCallbackClient<OmmConsumerClient>
{
	DirectoryCallbackClientConsumer(OmmBaseImpl<OmmConsumerClient> baseImpl) {
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
	void notifyOnUpdateMsg()
	{
		_eventImpl._item.client().onUpdateMsg(_updateMsg, _eventImpl);
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

class DirectoryCallbackClientProvider extends DirectoryCallbackClient<OmmProviderClient>
{
	DirectoryCallbackClientProvider(OmmBaseImpl<OmmProviderClient> baseImpl) {
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


class Directory
{
	private ChannelInfo		_channelInfo;
	private Service			_service;
	private String 			_serviceName;
	
	Directory(Service service)
	{
		_service = service;
	}
	
	ChannelInfo channelInfo()
	{
		return _channelInfo;
	}
	
	Directory channelInfo(ChannelInfo channelInfo)
	{
		_channelInfo = channelInfo;
		return this;
	}
	
	Service service()
	{
		return _service;
	}

	Directory service(Service service)
	{
		_service = service;
		return this;
	}
	
	Directory serviceName(String serviceName)
	{
		_serviceName = serviceName;
		return this;
	}
	
	String serviceName()
	{
		return _serviceName;
	}
}

class DirectoryItem<T> extends SingleItem<T>
{
	private static final String 	CLIENT_NAME = "DirectoryItem";
	private ChannelInfo	_channelInfo;

	DirectoryItem(OmmBaseImpl<T> baseImpl, T client, Object closure)
	{
		super(baseImpl, client, closure, null);
	}
	
	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> item)
	{
		super.reset(baseImpl, client, closure, item);
		
		_channelInfo = null;
	}
	
	void channelInfo(ChannelInfo channelInfo)
	{
		_channelInfo = channelInfo;
	}
	
	@Override
	boolean open(ReqMsg reqMsg)
	{
		Directory directory = null;

		if (reqMsg.hasServiceName())
		{
			directory = _baseImpl.directoryCallbackClient().directory(reqMsg.serviceName());

			if (directory == null)
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service name of '")
					.append(reqMsg.serviceName()).append("' is not found.");

				TimeoutClient client = closedStatusClient(_baseImpl.directoryCallbackClient(),
															this, ((ReqMsgImpl)reqMsg).rsslMsg(),
															temp.toString(), reqMsg.serviceName());
				_baseImpl.addTimeoutEvent(1000, client);
				
				return true;
			}
		}
		else
		{
			if (reqMsg.hasServiceId())
			{
				directory = _baseImpl.directoryCallbackClient().directory(reqMsg.serviceId());

				if (directory == null)
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Service id of '")
						.append(reqMsg.serviceId()).append("' is not found.");

					TimeoutClient client = closedStatusClient(_baseImpl.directoryCallbackClient(),
							this, ((ReqMsgImpl)reqMsg).rsslMsg(), temp.toString(), null);
					_baseImpl.addTimeoutEvent(1000, client);

					return true;
				}
			}
		}

		_directory = directory;
		
		return submit(((ReqMsgImpl)reqMsg).rsslMsg());
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		return submit(((ReqMsgImpl) reqMsg).rsslMsg());
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit PostMsg on directory stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString());

		return false;
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return submit(((GenericMsgImpl) genericMsg).rsslMsg());
	}
	
	boolean submit(com.thomsonreuters.upa.codec.GenericMsg rsslGenericMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		rsslSubmitOptions.requestMsgOptions().clear();
		
		rsslGenericMsg.streamId(_streamId);
		rsslGenericMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelInfo.rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslGenericMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in DirectoryItem.submit(GenericMsg rsslGenericMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit GenericMsg on directory stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			_baseImpl.handleInvalidUsage(temp.toString());

			return false;
	    }
        
		return true;
	}
	
	@Override
	boolean close()
	{
		CloseMsg rsslCloseMsg = _baseImpl.directoryCallbackClient().rsslCloseMsg();
		rsslCloseMsg.containerType(DataTypes.NO_DATA);
		rsslCloseMsg.domainType(_domainType);

		boolean retCode = submit(rsslCloseMsg);

		remove();
		return retCode;
	}
	
	@Override
	void remove()
	{
		_baseImpl.itemCallbackClient().removeFromMap(this);
		this.returnToPool();
	}
	
	boolean submit(RequestMsg rsslRequestMsg)
	{	
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		
		int rsslFlags = rsslRequestMsg.msgKey().flags();
		rsslRequestMsg.msgKey().flags(rsslFlags & ~MsgKeyFlags.HAS_SERVICE_ID);

		if (!rsslRequestMsg.checkHasQos())
		{
			rsslRequestMsg.applyHasQos();
			rsslRequestMsg.applyHasWorstQos();
			rsslRequestMsg.qos().dynamic(false);
			rsslRequestMsg.qos().timeliness(QosTimeliness.REALTIME);
			rsslRequestMsg.qos().rate(QosRates.TICK_BY_TICK);
			rsslRequestMsg.worstQos().rate(QosRates.TIME_CONFLATED);
			rsslRequestMsg.worstQos().timeliness(QosTimeliness.DELAYED_UNKNOWN);
			rsslRequestMsg.worstQos().rateInfo(65535);
		}	

		if (_baseImpl.activeConfig().channelConfigSet.get(0).msgKeyInUpdates)
			rsslRequestMsg.applyMsgKeyInUpdates();
		
		if (_directory != null)
			rsslSubmitOptions.serviceName(_directory.serviceName());
		
		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);

		if (_streamId == 0)
		{
			rsslRequestMsg.streamId(_channelInfo.nextStreamId(0));
			_streamId = rsslRequestMsg.streamId();
		}
		else
			rsslRequestMsg.streamId(_streamId);

		if (_domainType == 0)
			_domainType = rsslRequestMsg.domainType();
		else
			rsslRequestMsg.domainType(_domainType);

		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelInfo.rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: ReactorChannel.submit() failed in DirectoryItem.submit(RequestMsg rsslRequestMsg, Directory directory)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to open or modify directory request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			_baseImpl.handleInvalidUsage(temp.toString());

			return false;
	    }

		return true;
	}

	boolean submit(CloseMsg rsslCloseMsg)
	{	
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);

		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
	
		if (_streamId == 0)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryItem.CLIENT_NAME,
	        									"Invalid streamId for this item in in DirectoryItem.submit(CloseMsg rsslCloseMsg)",
	        									Severity.ERROR));
		}
		else
			rsslCloseMsg.streamId(_streamId);
	
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelInfo.rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			
			if (_baseImpl.loggerClient().isErrorEnabled())
	    	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: ReactorChannel.submit() failed in DirectoryItem.submit(CloseMsg rsslCloseMsg)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
	    	}
			
			temp.append("Failed to close directory stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			_baseImpl.handleInvalidUsage(temp.toString());
	
			return false;
	    }
	
		return true;
	}
}
