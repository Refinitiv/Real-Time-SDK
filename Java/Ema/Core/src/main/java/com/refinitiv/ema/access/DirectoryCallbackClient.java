/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.OmmBaseImpl.OmmImplState;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceInfo;
import com.refinitiv.eta.valueadd.reactor.RDMDirectoryMsgCallback;
import com.refinitiv.eta.valueadd.reactor.RDMDirectoryMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

class DirectoryCallbackClient<T> extends CallbackClient<T> implements RDMDirectoryMsgCallback
{
	private static final String CLIENT_NAME = "DirectoryCallbackClient";
	
	private Map<Integer, Directory<T>>					_serviceById;
	private Map<String, Directory<T>>					_serviceByName;
	private OmmBaseImpl<T>							_ommBaseImpl;
	
	// This is used only for request routing to fan out source directory aggregation 
	private List<DirectoryItem<T>>				    _directoryItemList;

	DirectoryCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		super(baseImpl, CLIENT_NAME);
		
		_ommBaseImpl = baseImpl;
		 
		int initialHashSize =  (int)(_ommBaseImpl.activeConfig().serviceCountHint/ 0.75 + 1);
		_serviceById = new HashMap<Integer, Directory<T>>(initialHashSize);
		_serviceByName = new HashMap<String, Directory<T>>(initialHashSize);
		
		_directoryItemList = new ArrayList<DirectoryItem<T>>();
	}

	@SuppressWarnings("unchecked")
	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
	{
		_baseImpl.eventReceived();
		
		DirectoryMsg directoryMsg = event.rdmDirectoryMsg();
		ReactorChannel rsslReactorChannel = event.reactorChannel();
		ChannelInfo channelInfo = (ChannelInfo)rsslReactorChannel.userSpecObj();
		
		if (channelInfo.getParentChannel() != null)
			channelInfo = channelInfo.getParentChannel();
		
		SessionChannelInfo<OmmConsumerClient> sessionChannelInfo = channelInfo.sessionChannelInfo();
		ConsumerSession<OmmConsumerClient> consumerSession = sessionChannelInfo!= null ? sessionChannelInfo.consumerSession() : null;
		
		if (directoryMsg == null)
		{
			if(consumerSession != null)
			{
				_ommBaseImpl.closeSessionChannel((SessionChannelInfo<T>) sessionChannelInfo);
			}
			else
			{
				_ommBaseImpl.closeRsslChannel(rsslReactorChannel);
			}

			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = event.errorInfo().error();
				
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
			return processCallback(event, rsslReactorChannel, (SingleItem<T>)item, channelInfo);

		switch (directoryMsg.rdmMsgType())
		{
			case REFRESH:
			{
				com.refinitiv.eta.codec.State state = ((DirectoryRefresh)directoryMsg).state();
	
				if (state.streamState() != StreamStates.OPEN)
				{	
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDirectory stream was closed with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}
					
					if(consumerSession != null)
					{
						consumerSession.processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), channelInfo);
						
						_ommBaseImpl.closeSessionChannel((SessionChannelInfo<T>) sessionChannelInfo);
					}
					else
					{
						_ommBaseImpl.closeRsslChannel(rsslReactorChannel);
						
						processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), channelInfo);
					}
	
					break;
				}
				else if (state.dataState() == com.refinitiv.eta.codec.DataStates.SUSPECT)
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDirectory stream state was changed to suspect with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					if(consumerSession != null)
					{
						sessionChannelInfo.state(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
						
						if(consumerSession.checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT))
						{
							_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
						}
						
						consumerSession.processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), channelInfo);
					}
					else
					{
						_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);						
						processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), channelInfo);
					}
					break;
				}
				
				boolean changeToOpenOk = false;
	
				if(consumerSession != null)
				{
					sessionChannelInfo.state(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
					
					if(consumerSession.checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_OK))
					{
						changeToOpenOk = true;
					}
					
					consumerSession.processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), channelInfo);
				}
				else
				{
					processDirectoryPayload(((DirectoryRefresh)directoryMsg).serviceList(), channelInfo);
					changeToOpenOk = true;
				}
				
				if(changeToOpenOk)
				{
					_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
		
					if (_baseImpl.loggerClient().isTraceEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDirectory stream state was open with refresh message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
		
			        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		        	}
				}
				
				
				break;
			}
			case STATUS:
			{
				if (((DirectoryStatus)directoryMsg).checkHasState())
		    	{
					com.refinitiv.eta.codec.State state = ((DirectoryStatus)directoryMsg).state();
	
					if (state.streamState() != StreamStates.OPEN)
					{
						if(consumerSession != null)
						{
							_ommBaseImpl.closeSessionChannel((SessionChannelInfo<T>) sessionChannelInfo);
						}
						else
						{
							_ommBaseImpl.closeRsslChannel(rsslReactorChannel);
						}
	
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("RDMDirectory stream was closed with status message ").append(OmmLoggerClient.CR)
				        		.append("State: ").append(state.toString());
	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}
						
						return ReactorCallbackReturnCodes.SUCCESS;
					}
					else if (state.dataState() == com.refinitiv.eta.codec.DataStates.SUSPECT)
					{
						if (_baseImpl.loggerClient().isWarnEnabled())
			        	{
				        	StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("RDMDirectory stream state was changed to suspect with status message ").append(OmmLoggerClient.CR)
				        		.append("State: ").append(state.toString());
	
				        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
			        	}
						
						if(consumerSession != null)
						{
							sessionChannelInfo.state(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
							
							if(consumerSession.checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT))
							{
								_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
							}
						}
						else
						{
							_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);						
						}
						
						break;
					}
					
					if (_baseImpl.loggerClient().isTraceEnabled())
		        	{
			        	StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("RDMDirectory stream was open with status message ").append(OmmLoggerClient.CR)
			        		.append("State: ").append(state.toString());
	
			        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(DirectoryCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		        	}
					
					if(consumerSession != null)
					{
						sessionChannelInfo.state(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
						
						if(consumerSession.checkAllSessionChannelHasState(OmmImplState.DIRECTORY_STREAM_OPEN_OK))
						{
							_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);
						}
					}
					else
					{
						_ommBaseImpl.ommImplState(OmmImplState.DIRECTORY_STREAM_OPEN_OK);						
					}
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
	
				if(consumerSession != null)
				{
					consumerSession.processDirectoryPayload(((DirectoryUpdate)directoryMsg).serviceList(), channelInfo);
					
					consumerSession.fanoutSourceDirectoryResponse(DirectoryMsgType.UPDATE);
				}
				else
				{
					processDirectoryPayload(((DirectoryUpdate)directoryMsg).serviceList(), channelInfo);
				}
				
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
	
	void processDirectoryPayload(List<Service> serviceList, ChannelInfo chnlInfo)
	{
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

		if (chnlInfo.getParentChannel() != null)
			chnlInfo = chnlInfo.getParentChannel();
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
		            Directory<T> existDirectory = null;
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
		            		existDirectory.channelInfo(chnlInfo);
		            	}
		            }
		            else
		            {    
		            	Service newService = DirectoryMsgFactory.createService();
		            	oneService.copy(newService);
		            	
		            	Directory<T> directory = new Directory<T>(newService); 
		            	directory.channelInfo(chnlInfo);
		            	directory.serviceName(serviceName);
		            	
		            	_serviceById.put(oneService.serviceId(), directory);
		            	_serviceByName.put(serviceName, directory);

						if (_ommBaseImpl.activeConfig().dictionaryConfig.isLocalDictionary ||
						(newService.state().acceptingRequests() == 1 && newService.state().serviceState() == 1))
							_ommBaseImpl.dictionaryCallbackClient().downloadDictionary(directory, 
									_ommBaseImpl.dictionaryCallbackClient().pollChannelDict(_ommBaseImpl));
		            }
	
					break;
				}
				case MapEntryActions.UPDATE :
				{
					Service existService = null;
					Directory<T> existDirectory = null; 
			        if (_serviceById.size() > 0 && _serviceById.containsKey(oneService.serviceId()))
			        {
			        	existDirectory = _serviceById.get(oneService.serviceId());
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
	            		existDirectory.channelInfo(chnlInfo);
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
							_ommBaseImpl.dictionaryCallbackClient().downloadDictionary(existDirectory,
									_ommBaseImpl.dictionaryCallbackClient().pollChannelDict(_ommBaseImpl));
					}
					
					existService.action(MapEntryActions.UPDATE);
	
					break;
				}
				case MapEntryActions.DELETE :
				{
					Service existService = null;
			        if (_serviceById.size() > 0 && _serviceById.containsKey(oneService.serviceId()))
					{
						existService = _serviceById.get(oneService.serviceId()).service();
					}
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

	int processCallback(RDMDirectoryMsgEvent event, ReactorChannel rsslReactorChannel, SingleItem<T> item, ChannelInfo channelInfo)
	{
		Msg rsslMsg = event.msg();
		ChannelInfo channelInfo = (ChannelInfo)rsslReactorChannel.userSpecObj();
		if (channelInfo.getParentChannel() != null)
			channelInfo = channelInfo.getParentChannel();

		switch (event.rdmDirectoryMsg().rdmMsgType())
		{
			case REFRESH:
				{
					_refreshMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), channelInfo.rsslDictionary());

					_eventImpl._item = item;
					
					notifyOnAllMsg(_refreshMsg);
					notifyOnRefreshMsg();
	
					int rsslStreamState = ((com.refinitiv.eta.codec.RefreshMsg)rsslMsg).state().streamState();
					if (rsslStreamState == StreamStates.NON_STREAMING)
					{
						if (((com.refinitiv.eta.codec.RefreshMsg)rsslMsg).checkRefreshComplete())
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
						_updateMsg = new UpdateMsgImpl(_baseImpl.objManager());
					
					_updateMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(),
							channelInfo.rsslDictionary());
	
					_eventImpl._item = item;
					
					notifyOnAllMsg(_updateMsg);
					notifyOnUpdateMsg();
				}
				break;
			case STATUS :
				{
					if (_statusMsg == null)
						_statusMsg = new StatusMsgImpl(_baseImpl.objManager());
					
					_statusMsg.decode(rsslMsg, rsslReactorChannel.majorVersion(), rsslReactorChannel.minorVersion(), null);
	
					_eventImpl._item = item;
					
					notifyOnAllMsg(_statusMsg);
					notifyOnStatusMsg();
	
					if (((com.refinitiv.eta.codec.StatusMsg)rsslMsg).checkHasState() &&  
							((com.refinitiv.eta.codec.StatusMsg)rsslMsg).state().streamState() != StreamStates.OPEN) 
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
		if (_ommBaseImpl.activeConfig().rsslDirectoryRequest == null)
		{
			_ommBaseImpl.activeConfig().rsslDirectoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
			_ommBaseImpl.activeConfig().rsslDirectoryRequest.clear();
			_ommBaseImpl.activeConfig().rsslDirectoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
			_ommBaseImpl.activeConfig().rsslDirectoryRequest.streamId(2);
			_ommBaseImpl.activeConfig().rsslDirectoryRequest.applyStreaming();
			_ommBaseImpl.activeConfig().rsslDirectoryRequest.filter(
									 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.INFO |
									 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.STATE |
									 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.GROUP |
									 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LOAD |
									 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.DATA |
									 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LINK);
		}
		else
		{
			_ommBaseImpl.activeConfig().rsslDirectoryRequest.streamId(2);
			long filter = _ommBaseImpl.activeConfig().rsslDirectoryRequest.filter();
			if (filter == 0)
			{			
				if (_baseImpl.loggerClient().isWarnEnabled())
				{
					_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(CLIENT_NAME,
													"Configured source directory request message contains no filter. Will request all filters",
													Severity.WARNING).toString());
				}
				
				_ommBaseImpl.activeConfig().rsslDirectoryRequest.filter(
										 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.INFO |
										 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.STATE |
										 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.GROUP |
										 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LOAD |
										 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.DATA |
										 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LINK);
			}

			if (!_ommBaseImpl.activeConfig().rsslDirectoryRequest.checkStreaming())
			{
				_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(CLIENT_NAME, 
						                  		"Configured source directory request message contains no streaming flag. Will request streaming",
												Severity.WARNING).toString());
				
				_ommBaseImpl.activeConfig().rsslDirectoryRequest.applyStreaming();
			}
		}

		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			
			temp.append("RDMDirectoryRequest message was populated with Filter(s)");
			int filter = (int)_ommBaseImpl.activeConfig().rsslDirectoryRequest.filter();
			
			if ((filter & com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.INFO) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_INFO_FILTER");
			if ((filter & com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.STATE) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_STATE_FILTER");
			if ((filter & com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.GROUP) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_GROUP_FILTER");
			if ((filter & com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LOAD) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_LOAD_FILTER");
			if ((filter & com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.DATA) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_DATA_FILTER"); 
			if ((filter & com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LINK) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_LINK_FILTER");

			if (_ommBaseImpl.activeConfig().rsslDirectoryRequest.checkHasServiceId())
				temp.append(OmmLoggerClient.CR).append("requesting serviceId ").append(_ommBaseImpl.activeConfig().rsslDirectoryRequest.serviceId());
			else
				temp.append(OmmLoggerClient.CR).append("requesting all services");

			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
											Severity.TRACE));
		}
	}
	
	DirectoryRequest rsslDirectoryRequest()
	{
		return _ommBaseImpl.activeConfig().rsslDirectoryRequest;
	}
	
	Directory<T> directory(String serviceName)
	{
		return _serviceByName.get(serviceName);
	}
	
	Directory<T> directory(int serviceId)
	{
		return _serviceById.get(serviceId);
	}
	
	@SuppressWarnings("unchecked")
	DirectoryItem<T> directoryItem(ConsumerSession<T> consumerSession, ReqMsg reqMsg, T client, Object closure)
	{		
		if(_updateMsg == null)
			_updateMsg = new UpdateMsgImpl(_baseImpl.objManager());
		
		DirectoryItem<T> item;
		if( (item = (DirectoryItem<T>)_ommBaseImpl.objManager()._directoryItemPool.poll()) == null)
		{
			item = new DirectoryItem<T>(_ommBaseImpl, client, closure);
			_ommBaseImpl.objManager()._directoryItemPool.updatePool(item);
		}
		else
		{
			item.reset(_ommBaseImpl, client, closure, item);
		}
		
		// Checks for service ID or service name and filter ID
		if(reqMsg.hasServiceName())
		{
			String serviceName = reqMsg.serviceName();
			
			SessionDirectory<T> sessionDir = consumerSession.sessionDirectoryByName().get(serviceName);
			
			if(sessionDir == null && (!consumerSession.loginRefresh().attrib().checkHasSingleOpen() || consumerSession.loginRefresh().attrib().singleOpen() == 0))
			{
				/* This ensures that the user will get a valid handle.  The callback should clean it up after. */
				_ommBaseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), item);
				
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service name of '")
					.append(reqMsg.serviceName()).append("' is not found.");

				item.scheduleItemClosedStatus(_ommBaseImpl.directoryCallbackClient(),
															item, ((ReqMsgImpl)reqMsg).rsslMsg(),
															temp.toString(), serviceName);
				
				return item;
			}
			
			item.serviceName(serviceName);
			
		}
		else if (reqMsg.hasServiceId())
		{
			int serviceId = reqMsg.serviceId();
			
			SessionDirectory<T> sessionDir = consumerSession.sessionDirectoryById().get(serviceId);
			
			if(sessionDir == null && (!consumerSession.loginRefresh().attrib().checkHasSingleOpen() || consumerSession.loginRefresh().attrib().singleOpen() == 0))
			{
				/* This ensures that the user will get a valid handle.  The callback should clean it up after. */
				_ommBaseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), item);
				
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service Id of '")
					.append(serviceId).append("' is not found.");

				item.scheduleItemClosedStatus(_ommBaseImpl.directoryCallbackClient(),
															item, ((ReqMsgImpl)reqMsg).rsslMsg(),
															temp.toString(), null);
				
				return item;
			}
			
			if(sessionDir != null)
				item.serviceName(sessionDir.serviceName());
			else
				item.serviceId(serviceId);
		}
		
		if(reqMsg.hasFilter())
		{
			item.filterId(reqMsg.filter());
		}
		
		_directoryItemList.add(item);
		
		_ommBaseImpl.addTimeoutEvent(10, item);
		
		return item;
	}
	
	List<DirectoryItem<T>> directoryItemList()
	{
		return _directoryItemList;
	}
}

class DirectoryCallbackClientConsumer extends DirectoryCallbackClient<OmmConsumerClient>
{
	DirectoryCallbackClientConsumer(OmmBaseImpl<OmmConsumerClient> baseImpl) {
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
		
		_eventImpl._ommProvider = (OmmProvider)baseImpl;
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


class Directory<T>
{
	/* The generated service Id is used for the request routing feature only.*/
	private ChannelInfo		_channelInfo;
	private Service			_service;
	private String 			_serviceName;
	private boolean         _hasGenServiceId;
	private int				_genServiceId;
	private SessionDirectory<T> _sessionDirectory;
	
	Directory(Service service)
	{
		_service = service;
	}
	
	Directory(String serviceName)
	{
		_serviceName = serviceName;
	}
	
	void generatedServiceId(int genServiceId)
	{
		_hasGenServiceId = true;
		_genServiceId = genServiceId;
	}
	
	/* Checks whether there is a generated service ID */
	boolean hasGeneratedServiceId()
	{
		return _hasGenServiceId;
	}
	
	/* Gets generated service Id */
	int generatedServiceId()
	{
		return _genServiceId;
	}
	
	ChannelInfo channelInfo()
	{
		return _channelInfo;
	}
	
	Directory<T> channelInfo(ChannelInfo channelInfo)
	{
		_channelInfo = channelInfo;
		return this;
	}
	
	Service service()
	{
		return _service;
	}

	Directory<T> service(Service service)
	{
		_service = service;
		return this;
	}
	
	Directory<T> serviceName(String serviceName)
	{
		_serviceName = serviceName;
		return this;
	}
	
	String serviceName()
	{
		return _serviceName;
	}

	void sessionDirectory(SessionDirectory<T> sessionDirectory)
	{
		_sessionDirectory = sessionDirectory;
	}
	
	SessionDirectory<T>  sessionDirectory()
	{
		return _sessionDirectory;
	}
	
	@Override
	public String toString()
	{
		return "Name: " + _serviceName + ",\tId: " + _service.serviceId() 
		+ ",\tGeneratedServiceId: " + _genServiceId;
	}
}

class DirectoryItem<T> extends SingleItem<T> implements TimeoutClient
{
	private static final String 	CLIENT_NAME = "DirectoryItem";
	private ChannelInfo	_channelInfo;
	
	protected int _serviceId;
	protected long _filterId;
	protected boolean _hasServiceId;

	DirectoryItem(OmmBaseImpl<T> baseImpl, T client, Object closure)
	{
		super(baseImpl, client, closure, null);
		
		_filterId = com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.INFO |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.STATE |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.GROUP |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LOAD |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.DATA |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LINK;
		
		_serviceName = "";
		_hasServiceId = false;
	}
	
	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> item)
	{
		super.reset(baseImpl, client, closure, item);
		
		_channelInfo = null;
		
		_filterId = com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.INFO |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.STATE |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.GROUP |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LOAD |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.DATA |
				 com.refinitiv.eta.rdm.Directory.ServiceFilterFlags.LINK;
		
		_serviceName = "";
		_hasServiceId = false;
	}
	
	void channelInfo(ChannelInfo channelInfo)
	{
		_channelInfo = channelInfo;
	}
	
	void serviceName(String serviceName)
	{
		_serviceName = serviceName;
	}
	
	String serviceName()
	{
		return _serviceName;
	}
	
	void serviceId(int serviceId)
	{
		_hasServiceId = true;
		_serviceId = serviceId;
	}
	
	int serviceId()
	{
		return _serviceId;
	}
	
	void filterId(long filterId)
	{
		_filterId = filterId;
	}
	
	long filterId()
	{
		return _filterId;
	}
	
	boolean hasServiceId()
	{
		return _hasServiceId;
	}
	
	@Override
	boolean open(ReqMsg reqMsg)
	{
		Directory<T> directory = null;

		if (reqMsg.hasServiceName())
		{
			directory = _baseImpl.directoryCallbackClient().directory(reqMsg.serviceName());

			if (directory == null && (!_baseImpl.loginCallbackClient().loginRefreshMsg().attrib().checkHasSingleOpen() || _baseImpl.loginCallbackClient().loginRefreshMsg().attrib().singleOpen() == 0))
			{
				/* This ensures that the user will get a valid handle.  The callback should clean it up after. */
				_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
				
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service name of '")
					.append(reqMsg.serviceName()).append("' is not found.");

				scheduleItemClosedStatus(_baseImpl.directoryCallbackClient(),
															this, ((ReqMsgImpl)reqMsg).rsslMsg(),
															temp.toString(), reqMsg.serviceName());
				
				return true;
			}
		}
		else
		{
			if (reqMsg.hasServiceId())
			{
				directory = _baseImpl.directoryCallbackClient().directory(reqMsg.serviceId());

				if (directory == null && (!_baseImpl.loginCallbackClient().loginRefreshMsg().attrib().checkHasSingleOpen() || _baseImpl.loginCallbackClient().loginRefreshMsg().attrib().singleOpen() == 0))
				{
					/* This ensures that the user will get a valid handle.  The callback should clean it up after. */
					_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);

					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Service id of '")
						.append(reqMsg.serviceId()).append("' is not found.");

					scheduleItemClosedStatus(_baseImpl.directoryCallbackClient(),
							this, ((ReqMsgImpl)reqMsg).rsslMsg(), temp.toString(), null);
				
					return true;
				}
			}
		}

		_directory = directory;
		
		String serviceName = reqMsg.hasServiceName() ? reqMsg.serviceName() : null;
		return submit(((ReqMsgImpl)reqMsg).rsslMsg(), serviceName);
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		String serviceName = reqMsg.hasServiceName() ? reqMsg.serviceName() : null;
		return submit(((ReqMsgImpl) reqMsg).rsslMsg(), serviceName);
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

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return submit(((GenericMsgImpl) genericMsg).rsslMsg());
	}
	
	boolean submit(com.refinitiv.eta.codec.GenericMsg rsslGenericMsg)
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
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
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
				
			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
        
		return true;
	}
	
	@Override
	boolean close()
	{
		if(_baseImpl.consumerSession() == null)
		{
			CloseMsg rsslCloseMsg = _baseImpl.directoryCallbackClient().rsslCloseMsg();
			rsslCloseMsg.containerType(DataTypes.NO_DATA);
			rsslCloseMsg.domainType(_domainType);
	
			boolean retCode = submit(rsslCloseMsg);
	
			remove();
			return retCode;
		}
		else
		{
			remove();
			
			_baseImpl.directoryCallbackClient().directoryItemList().remove(this);
			
			return true;
		}
	}
	
	@Override
	void remove()
	{
		_baseImpl.itemCallbackClient().removeFromMap(this, true);
	}
	
	boolean submit(RequestMsg rsslRequestMsg, String serviceName)
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

		if (_baseImpl.activeConfig().msgKeyInUpdates)
			rsslRequestMsg.applyMsgKeyInUpdates();
		
		rsslSubmitOptions.serviceName(serviceName);
		
		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);

		if (_streamId == 0)
		{
			rsslRequestMsg.streamId(_baseImpl._itemCallbackClient.nextStreamId(0));
			_streamId = rsslRequestMsg.streamId();
			_baseImpl._itemCallbackClient.addToMap(_baseImpl.nextLongId(), this);
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
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
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
				
			_baseImpl.handleInvalidUsage(temp.toString(), ret);

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
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
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
				
			_baseImpl.handleInvalidUsage(temp.toString(), ret);
	
			return false;
	    }
	
		return true;
	}

	@Override
	public void handleTimeoutEvent()
	{
		if(_baseImpl.directoryCallbackClient().directoryItemList().isEmpty())
			return;
		
		_baseImpl.consumerSession().fanoutSourceDirectoryResponsePerItem(this, DirectoryMsgType.REFRESH, true);
	}

	@Override
	public ReentrantLock userLock() {
		return _baseImpl.userLock();
	}
}
