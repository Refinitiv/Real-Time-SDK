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

import com.thomsonreuters.ema.access.OmmConsumerImpl.OmmConsumerState;
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
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
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

class DirectoryCallbackClient extends ConsumerCallbackClient implements RDMDirectoryMsgCallback
{
	private static final String CLIENT_NAME = "DirectoryCallbackClient";
	
	private Map<Integer, Directory>					_serviceById;
	private Map<String, Directory>					_serviceByName;
	private com.thomsonreuters.upa.codec.CloseMsg	_rsslCloseMsg;

	DirectoryCallbackClient(OmmConsumerImpl consumer)
	{
		super(consumer, CLIENT_NAME);
		 
		int initialHashSize =  (int)(_consumer.activeConfig().serviceCountHint/ 0.75 + 1);
		_serviceById = new HashMap<Integer, Directory>(initialHashSize);
		_serviceByName = new HashMap<String, Directory>(initialHashSize);
	}

	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
	{
		DirectoryMsg directoryMsg = event.rdmDirectoryMsg();
		ReactorChannel rsslReactorChannel = event.reactorChannel();
		ChannelInfo channelInfo = (ChannelInfo)rsslReactorChannel.userSpecObj();
		
		if (directoryMsg == null)
		{
			_consumer.closeRsslChannel(rsslReactorChannel);

			if (_consumer.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = event.errorInfo().error();
				
	        	StringBuilder temp = _consumer.consumerStrBuilder();
	        	temp.append("Received event without RDMDirectory message").append(OmmLoggerClient.CR)
	        		.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode())).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());

	        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}

		Object item = event.streamInfo() != null ? event.streamInfo().userSpecObject() : null;
		if (item != null)
			return processCallback(event, rsslReactorChannel, (SingleItem)item);

		switch (directoryMsg.rdmMsgType())
		{
			case REFRESH:
			{
				com.thomsonreuters.upa.codec.State state = ((DirectoryRefresh)directoryMsg).state();
	
				if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
				{
					_consumer.closeRsslChannel(rsslReactorChannel);
	
					if (_consumer.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
			        	temp.append("RDMDirectory stream was closed with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
					
					_consumer.directoryCallbackClient().processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), rsslReactorChannel);
	
					break;
				}
				else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
				{
					if (_consumer.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
			        	temp.append("RDMDirectory stream state was changed to suspect with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_consumer.loggerClient().warn(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					_consumer.ommConsumerState(OmmConsumerState.DIRECTORY_STREAM_OPEN_SUSPECT);
	
					_consumer.directoryCallbackClient().processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), rsslReactorChannel);
					break;
				}
	
				_consumer.ommConsumerState(OmmConsumerState.DIRECTORY_STREAM_OPEN_OK);
	
				if (_consumer.loggerClient().isTraceEnabled())
	        	{
		        	StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("RDMDirectory stream state was open with refresh message ").append(OmmLoggerClient.CR)
		        		.append("State: ").append(state.toString());
	
		        	_consumer.loggerClient().trace(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
	        	}
				
	
				_consumer.directoryCallbackClient().processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), rsslReactorChannel);
				break;
			}
			case STATUS:
			{
				if (((DirectoryStatus)directoryMsg).checkHasState())
		    	{
					com.thomsonreuters.upa.codec.State state = ((DirectoryStatus)directoryMsg).state();
	
					if (state.streamState() != com.thomsonreuters.upa.codec.StreamStates.OPEN)
					{
						_consumer.closeRsslChannel(rsslReactorChannel);
	
						if (_consumer.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
				        	temp.append("RDMDirectory stream was closed with status message ").append(OmmLoggerClient.CR)
				        		.append("State: ").append(state.toString());
	
				        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						
						return ReactorCallbackReturnCodes.SUCCESS;
					}
					else if (state.dataState() == com.thomsonreuters.upa.codec.DataStates.SUSPECT)
					{
						if (_consumer.loggerClient().isWarnEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
				        	temp.append("RDMDirectory stream state was changed to suspect with status message ").append(OmmLoggerClient.CR)
				        		.append("State: ").append(state.toString());
	
				        	_consumer.loggerClient().warn(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
			        	}
	
						_consumer.ommConsumerState(OmmConsumerState.DIRECTORY_STREAM_OPEN_SUSPECT);
						break;
					}
					
					if (_consumer.loggerClient().isTraceEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("RDMDirectory stream was open with status message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_consumer.loggerClient().trace(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		        	}
	
					_consumer.ommConsumerState(OmmConsumerState.DIRECTORY_STREAM_OPEN_OK);
				}
				else
				{
					if (_consumer.loggerClient().isWarnEnabled())
		        	{
			        	_consumer.loggerClient().warn(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMDirectory status message without the state", Severity.WARNING));
		        	}
				}
				break;
			}
			case UPDATE:
			{
				if (_consumer.loggerClient().isTraceEnabled())
	        	{
		        	_consumer.loggerClient().trace(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMDirectory update message", Severity.TRACE));
	        	}
	
				_consumer.directoryCallbackClient().processDirectoryPayload(((DirectoryUpdate)directoryMsg).serviceList(), rsslReactorChannel);
				break;
			}
			default:
			{
				if (_consumer.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _consumer.consumerStrBuilder();
					
		        	temp.append("Received unknown RDMDirectory message type")
		        		.append(OmmLoggerClient.CR)
		        		.append("message type value ").append(directoryMsg.rdmMsgType());
	
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
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
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMDirectory message without a service list", Severity.ERROR));
        	}
			return;
		}

		if (chnlInfo == null)
		{
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Internal error: no reactorChannel().userSpecObj()", Severity.ERROR));
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
						if (_consumer.loggerClient().isErrorEnabled())
			        	{
							_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMService with Add action but no Service Info", Severity.ERROR));
			        	}
						break;
					}

					String serviceName = oneService.info().serviceName().toString();
		            if(serviceName == null)
		            {
		            	if (_consumer.loggerClient().isErrorEnabled())
			        	{
							_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, "Received RDMService with Add action but no Service Info", Severity.ERROR));
			        	}
						break;
		            }

		            Service existService = null;
		            if (_serviceByName.size() > 0)
		            {
		            	Directory directory = _serviceByName.get(serviceName);
		            	existService = directory != null ? directory.service() : null;
		            }
		            if (existService != null)
		            {
		            	if (existService.serviceId() != oneService.serviceId())
						{
		            		existService.serviceId(oneService.serviceId());
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
							_consumer.dictionaryCallbackClient().downloadDictionary(directory);
						
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
						if (_consumer.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
							
				        	temp.append("Received Update action for unknown Service with service id ")
				        		.append(oneService.serviceId());
				        	
				        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						break;
					}
					else if (_consumer.loggerClient().isTraceEnabled())
					{
						StringBuilder temp = _consumer.consumerStrBuilder();
						
						temp.append("Received Update action for RDMService").append(OmmLoggerClient.CR)
							.append("Service name ").append(existService.info().serviceName().toString()).append(OmmLoggerClient.CR)
							.append("Service id ").append(existService.serviceId());
						
						_consumer.loggerClient().trace(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
					}
	
					if (oneService.checkHasInfo())
					{
	
						ServiceInfo  existInfo = existService.info();
						if (!(existInfo.serviceName().equals(oneService.info().serviceName())))
						{
							if (_consumer.loggerClient().isErrorEnabled())
				        	{
					        	StringBuilder temp = _consumer.consumerStrBuilder();
								
					        	temp.append("Received Update action for RDMService").append(OmmLoggerClient.CR)
					        		.append("Service name ").append(existInfo.serviceName().toString()).append(OmmLoggerClient.CR)
									.append("Service id ").append(existService.serviceId()).append(OmmLoggerClient.CR)
									.append("attempting to change service name to ").append(oneService.info().serviceName().toString());
					        	
					        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
				        	}
							break;
						}
						
						oneService.info().copy(existInfo);
					}
					
					if (oneService.checkHasState())
					{
						oneService.state().copy(existService.state());
						if (oneService.state().acceptingRequests() == 1 && oneService.state().serviceState() == 1)
							_consumer.dictionaryCallbackClient().downloadDictionary(existDirectory);
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
						if (_consumer.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _consumer.consumerStrBuilder();
							
				        	temp.append("Received Delete action for unknown RDMService with service id ")
				        		.append(oneService.serviceId());
				        	
				        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						break;
					}
					else if (_consumer.loggerClient().isTraceEnabled())
					{
						StringBuilder temp = _consumer.consumerStrBuilder();
						
						temp.append("Received Delete action for RDMService").append(OmmLoggerClient.CR)
							.append("Service name ").append(existService.info().serviceName().toString()).append(OmmLoggerClient.CR)
							.append("Service id ").append(existService.serviceId());
						
						_consumer.loggerClient().trace(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
					}
	
					existService.action(MapEntryActions.DELETE);
					break;
				}
				default :
				{
					if (_consumer.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _consumer.consumerStrBuilder();
						
			        	temp.append("Received unknown action for RDMService. Action value ")
			        		.append(oneService.action());
			        	
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
					break;
				}
			}
		}
	}

	int processCallback(RDMDirectoryMsgEvent event, ReactorChannel rsslReactorChannel, SingleItem item)
	{
		Msg rsslMsg = event.msg();
		switch (event.rdmDirectoryMsg().rdmMsgType())
		{
			case REFRESH:
				{
					_refreshMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), 
					((ChannelInfo)rsslReactorChannel.userSpecObj()).rsslDictionary());
	
					_event._item = item;
					_event._item.client().onAllMsg(_refreshMsg, _event);
					_event._item.client().onRefreshMsg(_refreshMsg, _event);
	
					int rsslStreamState = ((com.thomsonreuters.upa.codec.RefreshMsg)rsslMsg).state().streamState();
					if (rsslStreamState == StreamStates.NON_STREAMING)
					{
						if (((com.thomsonreuters.upa.codec.RefreshMsg)rsslMsg).checkRefreshComplete())
							_event._item.remove();
					}
					else if (rsslStreamState != StreamStates.OPEN)
					{
						_event._item.remove();
					}
				}
				break;
			case UPDATE :
				{
					if (_updateMsg == null)
						_updateMsg = new UpdateMsgImpl(true);
					
					_updateMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), 
							((ChannelInfo)rsslReactorChannel.userSpecObj()).rsslDictionary());
	
					_event._item = item;
					_event._item.client().onAllMsg(_updateMsg, _event);
					_event._item.client().onUpdateMsg(_updateMsg, _event);
				}
				break;
			case STATUS :
				{
					if (_statusMsg == null)
						_statusMsg = new StatusMsgImpl(true);
					
					_statusMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
	
					_event._item = item;
					_event._item.client().onAllMsg(_statusMsg, _event);
					_event._item.client().onStatusMsg(_statusMsg, _event);
	
					if (((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).checkHasState() &&  
							((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).state().streamState() != StreamStates.OPEN) 
						_event._item.remove();
				}
				break;
			case CONSUMER_STATUS :
				{
					if (_genericMsg == null)
						_genericMsg = new GenericMsgImpl();
					
					_genericMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
	
					_event._item = item;
					_event._item.client().onAllMsg(_genericMsg, _event);
					_event._item.client().onGenericMsg(_genericMsg, _event);
				}
				break;
			default :
				{
					if (_consumer.loggerClient().isErrorEnabled())
		        	{
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME,
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
		_consumer.activeConfig().intializeDirReq(CLIENT_NAME);
	}
	
	DirectoryRequest rsslDirectoryRequest()
	{
		return _consumer.activeConfig().rsslDirectoryRequest;
	}
	
	Directory directory(String serviceName)
	{
		return _serviceByName.get(serviceName);
	}
	
	Directory directory(int serviceId)
	{
		return _serviceById.get(serviceId);
	}
	
	com.thomsonreuters.upa.codec.CloseMsg rsslCloseMsg()
	{
		if (_rsslCloseMsg == null)
			_rsslCloseMsg = (CloseMsg)CodecFactory.createMsg();
		else
			_rsslCloseMsg.clear();
		
		return _rsslCloseMsg;
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

class DirectoryItem extends SingleItem
{
	private static final String 	CLIENT_NAME = "DirectoryItem";
	private ChannelInfo				_channelInfo;

	DirectoryItem(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure)
	{
		super(consumer, consumerClient, closure, null);
	}
	
	@Override
	void reset(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure, Item item)
	{
		super.reset(consumer, consumerClient, closure, item);
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
			directory = _consumer.directoryCallbackClient().directory(reqMsg.serviceName());

			if (directory == null)
			{
				StringBuilder temp = _consumer.consumerStrBuilder();
				temp.append("Service name of '")
					.append(reqMsg.serviceName()).append("' is not found.");

				TimeoutClient client = closedStatusClient(_consumer.directoryCallbackClient(),
															this, ((ReqMsgImpl)reqMsg).rsslMsg(),
															temp.toString(), reqMsg.serviceName());
				_consumer.addTimeoutEvent(1000, client);
				
				return true;
			}
		}
		else
		{
			if (reqMsg.hasServiceId())
			{
				directory = _consumer.directoryCallbackClient().directory(reqMsg.serviceId());

				if (directory == null)
				{
					StringBuilder temp = _consumer.consumerStrBuilder();
					temp.append("Service id of '")
						.append(reqMsg.serviceId()).append("' is not found.");

					TimeoutClient client = closedStatusClient(_consumer.directoryCallbackClient(),
							this, ((ReqMsgImpl)reqMsg).rsslMsg(), temp.toString(), null);
					_consumer.addTimeoutEvent(1000, client);

					return true;
				}
			}
		}

		return submit(((ReqMsgImpl)reqMsg).rsslMsg(), directory);
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		return submit(((ReqMsgImpl) reqMsg).rsslMsg());
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		StringBuilder temp = _consumer.consumerStrBuilder();
		temp.append("Invalid attempt to submit PostMsg on directory stream. ").append("OmmConsumer name='")
				.append(_consumer.consumerName()).append("'.");

		if (_consumer.loggerClient().isErrorEnabled())
			_consumer.loggerClient()
					.error(_consumer.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		if (_consumer.hasConsumerErrorClient())
			_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
		else
			throw (_consumer.ommIUExcept().message(temp.toString()));

		return false;
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return submit(((GenericMsgImpl) genericMsg).rsslMsg());
	}
	
	boolean submit(com.thomsonreuters.upa.codec.GenericMsg rsslGenericMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();
		
		rsslGenericMsg.streamId(_streamId);
		rsslGenericMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelInfo.rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslGenericMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in DirectoryItem.submit(GenericMsg rsslGenericMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit GenericMsg on directory stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));

			return false;
	    }
        
		return true;
	}
	
	@Override
	boolean close()
	{
		CloseMsg rsslCloseMsg = _consumer.directoryCallbackClient().rsslCloseMsg();
		rsslCloseMsg.msgClass(MsgClasses.CLOSE);
		rsslCloseMsg.containerType(DataTypes.NO_DATA);
		rsslCloseMsg.domainType(_domainType);

		boolean retCode = submit(rsslCloseMsg);

		remove();
		return retCode;
	}
	
	@Override
	void remove()
	{
		_consumer.itemCallbackClient().removeFromMap(this);
		this.returnToPool();
	}
	
	boolean submit(RequestMsg rsslRequestMsg, Directory directory)
	{	
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();
		
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

		if (_consumer.activeConfig().channelConfig.msgKeyInUpdates)
			rsslRequestMsg.applyMsgKeyInUpdates();
		
		if (directory != null)
			rsslSubmitOptions.serviceName(directory.serviceName());
		
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

		ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelInfo.rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: ReactorChannel.submit() failed in DirectoryItem.submit(RequestMsg rsslRequestMsg, Directory directory)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to open or modify directory request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));

			return false;
	    }

		return true;
	}

	boolean submit(CloseMsg rsslCloseMsg)
	{	
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		rsslSubmitOptions.clear();

		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
	
		if (_streamId == 0)
		{
			if (_consumer.loggerClient().isErrorEnabled())
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryItem.CLIENT_NAME,
	        									"Invalid streamId for this item in in DirectoryItem.submit(CloseMsg rsslCloseMsg)",
	        									Severity.ERROR));
		}
		else
			rsslCloseMsg.streamId(_streamId);
	
		ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelInfo.rsslReactorChannel();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _consumer.consumerStrBuilder();
			
			if (_consumer.loggerClient().isErrorEnabled())
	    	{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: ReactorChannel.submit() failed in DirectoryItem.submit(CloseMsg rsslCloseMsg)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(DirectoryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
	    	}
			
			temp.append("Failed to close directory stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));
	
			return false;
	    }
	
		return true;
	}
}
