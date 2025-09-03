/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayDeque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import com.refinitiv.ema.access.OmmBaseImpl.OmmImplState;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelType;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

class WatchlistResult
{
	public static class Code
	{
		public final static int SUCCESS = 0;
		
		public final static int SERVICE_NOT_UP = -1;
		
		public final static int CAPABILITY_NOT_FOUND = -2;
		
		public final static int MATCHING_QOS_NOT_FOUND = -3;
	}
	
	public WatchlistResult()
	{
		clear();
	}
	
	public void clear()
	{
		resultCode = Code.SUCCESS;
		resultText = "";
	}
	
	int resultCode;
	String resultText;
}


class SessionWatchlist<T>
{
	private Map<LongObject, Item<T>> _itemHandleMap;
	private Map<IntObject, Item<T>>  _streamIdMap;
	
	/* The message queue is used to queue item when there is no suitable ReactorChannel to submit a request message */
	private ArrayDeque<RequestMsg> _recoveryItemQueue;
	private ArrayDeque<SingleItem<T>> _closingItemQueue;

	private Buffer 			_statusText =  CodecFactory.createBuffer();
	
	private boolean _sessionEnhancedItemRecovery;
	
	ConsumerSession<T> _consumerSession;
	CallbackClient<T> callbackClient;
	
	LongObject tmpLongObject = new LongObject();
	IntObject tmpIntObject = new IntObject();
	
	Qos _defaultQos = CodecFactory.createQos();
	Qos _defaultWorstQos = CodecFactory.createQos();
	Qos _matchedQos = CodecFactory.createQos();
	
		
	SessionWatchlist(ConsumerSession<T> consumerSession, int itemCountHint)
	{
		_recoveryItemQueue = new ArrayDeque<RequestMsg>(100000);
		_closingItemQueue = new ArrayDeque<SingleItem<T>>(1000);
		
		_consumerSession = consumerSession;
		
		if(consumerSession.ommBaseImpl().activeConfig().userDispatch == OmmConsumerConfig.OperationModel.API_DISPATCH)
		{
			_itemHandleMap = new ConcurrentHashMap<>( itemCountHint == 0 ? 1024 : itemCountHint);
			_streamIdMap = new ConcurrentHashMap<>( itemCountHint == 0 ? 1024 : itemCountHint);
		}
		else
		{
			_itemHandleMap = new HashMap<>( itemCountHint == 0 ? 1024 : itemCountHint);
			_streamIdMap = new HashMap<>( itemCountHint == 0 ? 1024 : itemCountHint);
		}
		
		 _sessionEnhancedItemRecovery = consumerSession.ommBaseImpl().activeConfig().sessionEnhancedItemRecovery;
		
		_defaultQos.clear();
		_defaultQos.dynamic(false);
	    _defaultQos.timeliness(QosTimeliness.REALTIME);
	    _defaultQos.rate(QosRates.TICK_BY_TICK);
	    
	    _defaultWorstQos.clear();
	    _defaultWorstQos.rate(QosRates.TIME_CONFLATED);
	    _defaultWorstQos.timeliness(QosTimeliness.DELAYED_UNKNOWN);
	    _defaultWorstQos.rateInfo(65535);
	}
	
	ArrayDeque<RequestMsg> recoverItemQueue()
	{
		return _recoveryItemQueue;
	}
	
	void callbackClient(CallbackClient<T> client)
	{
		callbackClient = client;
	}
	
	Map<LongObject, Item<T>> itemHandleMap()
	{
		return _itemHandleMap;
	}
	
	Map<IntObject, Item<T>> streamIdMap()
	{
		return _streamIdMap;
	}
	
	void sendItemStatus(SingleItem<T> item, RequestMsg requestMsg, int streamState, int dataState, int statusCode, String statusText)
	{
		com.refinitiv.eta.codec.StatusMsg rsslStatusMsg = callbackClient.rsslStatusMsg();

		rsslStatusMsg.streamId(item._streamId);
		rsslStatusMsg.domainType(requestMsg.domainType());
		rsslStatusMsg.containerType(DataTypes.NO_DATA);
	
		rsslStatusMsg.applyHasState();
		rsslStatusMsg.state().streamState(streamState);
		rsslStatusMsg.state().dataState(dataState);
		rsslStatusMsg.state().code(statusCode);
		
		_statusText.data(statusText);
		rsslStatusMsg.state().text(_statusText);
		    
		rsslStatusMsg.applyHasMsgKey();
		requestMsg.msgKey().copy(rsslStatusMsg.msgKey()); 
		
		if (requestMsg.checkPrivateStream())
			rsslStatusMsg.applyPrivateStream();

		if (callbackClient._statusMsg == null)
			callbackClient._statusMsg = new StatusMsgImpl(callbackClient._baseImpl.objManager());
		
		callbackClient._statusMsg.decode(rsslStatusMsg, Codec.majorVersion(), Codec.majorVersion(), null);

		if(item._serviceList != null)
		{
			callbackClient._statusMsg.service(item._serviceList.name());
			callbackClient._statusMsg.serviceId(item._serviceList.serviceId());
		}
		else if(item.directory() != null && item.directory().serviceName() != null)
		{
			callbackClient._statusMsg.service(item.directory().serviceName());
			callbackClient._statusMsg.serviceId(item.directory().generatedServiceId());
		}
		else if (item._serviceName != null)
		{
			callbackClient._statusMsg.service(item._serviceName);
		}

		callbackClient._eventImpl._item = item;
		callbackClient.notifyOnAllMsg(callbackClient._statusMsg);
		callbackClient.notifyOnStatusMsg();
	}

	public void submitItemRecovery() 
	{	
		int count = _recoveryItemQueue.size();
		RequestMsg rsslRequestMsg = _recoveryItemQueue.poll();
		
		while(rsslRequestMsg != null)
		{
			tmpIntObject.value(rsslRequestMsg.streamId());
			
			SingleItem<T> item = (SingleItem<T>) _streamIdMap.get(tmpIntObject);
			
			/* Checks to ensure that the item exists. */
			if(item != null && (item.state() == SingleItem.ItemStates.RECOVERING || item.state() == SingleItem.ItemStates.RECOVERING_NO_MATHCING))
			{
				ServiceList serviceList = item._serviceList;
				
				if(serviceList == null)
				{
					ChannelInfo channelInfo = item.directory().channelInfo();
					
					// Gets a Directory from SessionDirectory
					Directory<T> directory = item.directory().sessionDirectory().updateSessionChannelInfo(rsslRequestMsg, channelInfo.rsslReactorChannel(), true, item._itemClosedDirHash);
					
					if(directory == null)
					{
						if(item.state() == SingleItem.ItemStates.RECOVERING)
						{
							if(item._itemClosedDirHash == null)
							{
								sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.OPEN,
										OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "No matching service present.");
								
								item.state(SingleItem.ItemStates.RECOVERING_NO_MATHCING);
								
								_recoveryItemQueue.addLast(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
							}
							else
							{
								/* Sends the closed status message and removes this item as there is no session channel to retry */
								sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.CLOSED,
										item._lastDataState, item._lastStatusCode, item._lastStatusText);
								
								item.remove();
							}
						}
						else
						{
							_recoveryItemQueue.addLast(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
							
							_consumerSession.nextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
						}
					}
					else
					{
						item._directory = directory;
						
						if(item._itemClosedDirHash != null)
						{
							/* Notify the application with the last status message from the provider */
							sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.OPEN,
									item._lastDataState, item._lastStatusCode, item._lastStatusText);
						}
						
						if(item.rsslSubmit(rsslRequestMsg, false))
						{
							/* The item state is changed to normal item stream */
							item.state(SingleItem.ItemStates.NORMAL);
						}
						else
						{
							_recoveryItemQueue.addLast(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
							_consumerSession.nextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
						}
					}
				}
				else
				{
					ChannelInfo channelInfo = item.directory().channelInfo();
					SessionDirectory<T> currentSessionDir = item.directory().sessionDirectory();
					
					// Gets the same service from others connection if any.
					Directory<T> directory = currentSessionDir.updateSessionChannelInfo(rsslRequestMsg, channelInfo.rsslReactorChannel(), item._retrytosameChannel, item._itemClosedDirHash);
					
					if(directory == null)
					{
						String currentServiceName = item.directory().serviceName();
						for(String serviceName : serviceList.concreteServiceList())
						{
							/* Try to recover with the next service name in the list as the current service name is not available from others connection. */
							if(serviceName.equals(currentServiceName))
								continue;
							
							SessionDirectory<T> sessionDirectory = item.session().sessionDirectoryByName(serviceName);
							
							if(sessionDirectory != null)
							{
								directory = sessionDirectory.directory(item._requestMsg);
								
								if(directory == null)
								{	
									continue;
								}
								else
								{
									if(item._itemClosedDirHash != null)
									{	/* Moves to next directory if the directory is requested */
										if(item._itemClosedDirHash.contains(directory))
										{
											directory = null;
											continue;
										}
									}
									
									item._directory = directory;
									item._serviceName = directory.serviceName();
									
									if(item._itemClosedDirHash != null)
									{
										/* Notify the application with the last status message from the provider */
										sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.OPEN,
												item._lastDataState, item._lastStatusCode, item._lastStatusText);
									}
									
									if(item.rsslSubmit(rsslRequestMsg, false))
									{
										/* The item state is changed to normal item stream */
										item.state(SingleItem.ItemStates.NORMAL);
										item._retrytosameChannel = false;
										break;
									}
								}
							}
						}
						
						/* Try again with the same service name on the same ReactorChannel */
						if(directory == null)
						{
							directory = currentSessionDir.updateSessionChannelInfo(rsslRequestMsg, channelInfo.rsslReactorChannel(), true, item._itemClosedDirHash);
							
							if(directory != null)
							{
								item._directory = directory;
								
								if(item._itemClosedDirHash != null)
								{
									/* Notify the application with the last status message from the provider */
									sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.OPEN,
											item._lastDataState, item._lastStatusCode, item._lastStatusText);
								}
								
								if(item.rsslSubmit(rsslRequestMsg, false))
								{
									/* The item state is changed to normal item stream */
									item.state(SingleItem.ItemStates.NORMAL);
									item._retrytosameChannel = false;
								}
							}
						}
						
						if(item.state() != SingleItem.ItemStates.NORMAL)
						{
							item._retrytosameChannel = true;
							
							if(item.state() == SingleItem.ItemStates.RECOVERING)
							{
								if(item._itemClosedDirHash == null)
								{
									sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.OPEN,
											OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "No matching service present.");
								
									item.state(SingleItem.ItemStates.RECOVERING_NO_MATHCING);
									
									_recoveryItemQueue.addLast(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
								}
								else
								{
									/* Sends the closed status message and removes this item as there is no session channel to retry */
									sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.CLOSED,
											item._lastDataState, item._lastStatusCode, item._lastStatusText);

									item.remove();
								}
							}
							else
							{
								_recoveryItemQueue.addLast(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
								
								_consumerSession.nextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
							}
						}
					}
					else
					{
						item._directory = directory;
						
						if(item._itemClosedDirHash != null)
						{
							/* Notify the application with the last status message from the provider */
							sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.OPEN,
									item._lastDataState, item._lastStatusCode, item._lastStatusText);
						}
						
						if(item.rsslSubmit(rsslRequestMsg, false))
						{
							/* The item state is changed to normal item stream */
							item.state(SingleItem.ItemStates.NORMAL);
							item._retrytosameChannel = false;
						}
						else
						{
							_recoveryItemQueue.addLast(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
							_consumerSession.nextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
						}
					}
				}
			}
			
			if( (--count) == 0)
				break;
			
			rsslRequestMsg = _recoveryItemQueue.poll();
		}
	}
	
	/* This must be called only after the connection is recovered for a ReactorChannel */
	public void submitItemCloseForChannel(ReactorChannel reactorChannel) 
	{
		int count = _closingItemQueue.size();
		SingleItem<T> singleItem = _closingItemQueue.poll();
		
		while(singleItem != null)
		{
			if(singleItem.directory().channelInfo().rsslReactorChannel() == reactorChannel)
			{
				singleItem.close();
			
				/* Sets the state that the item is being recovered */
				singleItem.state(SingleItem.ItemStates.RECOVERING);
			
				/* Add item to recovery queue to retry with another connection if any */
				_recoveryItemQueue.addLast(singleItem._requestMsg);
			}
			else
			{
				_closingItemQueue.addLast(singleItem);
			}
			
			if( (--count) == 0)
				break;
				
			singleItem = _closingItemQueue.poll();
		}
	}
	
	public int handleItemStatus(com.refinitiv.eta.codec.StatusMsg rsslStatusMsg, StatusMsgImpl statusMsg, Item<T> item)
	{
		SingleItem<T> singleItem = null;
		if(item instanceof SingleItem)
		{
			singleItem = (SingleItem<T>)item;
		}
		else
		{
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		boolean handleConnectionRecovering = false;
		
		if (item.directory() != null)
		{
			if(singleItem._serviceList != null)
			{
				statusMsg.service(singleItem._serviceList.name());
				statusMsg.serviceId(singleItem._serviceList.serviceId());
			}
			else if(item.directory().hasGeneratedServiceId()) /* Gets the mapping alternate service Id if any */
				statusMsg.serviceId(item.directory().generatedServiceId());
			
			
			if(!_sessionEnhancedItemRecovery && item.directory().channelInfo().rsslReactorChannel() != null)
			{
				if(item.directory().channelInfo().rsslReactorChannel().state() != ReactorChannel.State.UP &&
						item.directory().channelInfo().rsslReactorChannel().state() != ReactorChannel.State.READY)
				{
					handleConnectionRecovering = true;
				}
			}
		}
		
		State state = rsslStatusMsg.state();
		int originalStreamState = state.streamState();
		boolean notifyStatusMsg = true;
		
		while(true && singleItem._requestMsg != null)
		{
			RequestMsg requestMsg = singleItem._requestMsg;
			
			if(item.type() == Item.ItemType.SINGLE_ITEM && rsslStatusMsg.checkHasState())
			{
				if (state.streamState() == StreamStates.CLOSED_RECOVER)
				{					
					/* Recover item only for non-private item stream. */
					 if(requestMsg.checkPrivateStream() == false)
					 {
						if(item.directory().channelInfo().getReactorChannelType() == ReactorChannelType.NORMAL)
						{
							state.streamState(StreamStates.OPEN);
							
							/* Sets the state that the item is being recovered */
							singleItem.state(SingleItem.ItemStates.RECOVERING);
						
							/* Add item to recovery queue to retry with another connection if any */
							_recoveryItemQueue.addLast(requestMsg);
						}
						else if (item.directory().channelInfo().getReactorChannelType() == ReactorChannelType.WARM_STANDBY)
						{
							@SuppressWarnings("unchecked")
							SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) item.directory().channelInfo().sessionChannelInfo();
							
							state.streamState(StreamStates.OPEN);
							
							singleItem.state(SingleItem.ItemStates.RECOVERING);
							_consumerSession.watchlist().recoverItemQueue().add(singleItem._requestMsg);
							
							int wsbMode = sessionChannelInfo.getWarmStandbyMode(sessionChannelInfo.reactorChannel());
							
							if(wsbMode == ReactorWarmStandbyMode.LOGIN_BASED)
							{
								sessionChannelInfo.removeWSBStaleItem(singleItem);
							}
							else if (wsbMode == ReactorWarmStandbyMode.SERVICE_BASED)
							{
								sessionChannelInfo.removeWSBServiceBasedStaleItem(singleItem.directory().serviceName(), singleItem);
							}
						}
					 }
				}
				else if(state.streamState() == StreamStates.OPEN && state.dataState() == DataStates.SUSPECT)
				{					
					/* Recover item only for non-private item stream. */
					if(requestMsg.checkPrivateStream() == false)
					{
						if(item.directory().channelInfo().getReactorChannelType() == ReactorChannelType.NORMAL)
						{
							@SuppressWarnings("unchecked")
							SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) item.directory().channelInfo().sessionChannelInfo();
							
							if(sessionChannelInfo.phOperationInProgress() == false)
							{
								/* This is used to close this stream with the watchlist only */
								singleItem.state(SingleItem.ItemStates.CLOSING_STREAM);
								
								try
								{
									singleItem.close();
								}
								catch(OmmInvalidUsageException iue)
								{
									/* This will be closed later when the ReactorChannel is operational  */
									if (singleItem.directory().channelInfo().rsslReactorChannel().state() == ReactorChannel.State.CLOSED)
									{
										_closingItemQueue.addLast(singleItem);
										break;
									}
									else if(iue.errorCode() == OmmInvalidUsageException.ErrorCode.SHUTDOWN)
									{
										/* Remove this item as Reactor is shutdown */
										singleItem.remove();
										
										state.streamState(StreamStates.CLOSED);
										break;
									}
								}
							}
							
							if(handleConnectionRecovering || sessionChannelInfo.phOperationInProgress())
							{
								/* Sets the state that the item is being recovered */
								singleItem.state(SingleItem.ItemStates.RECOVERING);
								
								/* Waiting to recover this item with the same channel */
								singleItem._retrytosameChannel = true;
								
								/* Added this item into the recovering queue of SessionDirectory to recover once the requested service is ready. */
								singleItem.directory().sessionDirectory().addRecoveringQueue(singleItem);
								
							}
							else
							{	
								/* Sets the state that the item is being recovered */
								singleItem.state(SingleItem.ItemStates.RECOVERING);
								
								/* Add item to recovery queue to retry with another connection if any */
								_recoveryItemQueue.addLast(requestMsg);
							}
							
						}
						else if (item.directory().channelInfo().getReactorChannelType() == ReactorChannelType.WARM_STANDBY)
						{
							@SuppressWarnings("unchecked")
							SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) item.directory().channelInfo().sessionChannelInfo();
							if(sessionChannelInfo != null)
							{
								singleItem.state(SingleItem.ItemStates.RECOVERING);
								
								int wsbMode = sessionChannelInfo.getWarmStandbyMode(sessionChannelInfo.reactorChannel());
								
								/* Checks whether the WSB channel in the DOWN_RECONNECTING/DOWN state in order to recover with another connection if any for both login and service based.*/
								if(sessionChannelInfo.state() == OmmImplState.RSSLCHANNEL_DOWN && !sessionChannelInfo.phOperationInProgress())
								{	
									/* This is used to close this stream with the watchlist only */
									singleItem.state(SingleItem.ItemStates.CLOSING_STREAM);
									
									try
									{
										singleItem.close();
									}
									catch(OmmInvalidUsageException iue)
									{
										/* This will be closed later when the ReactorChannel is operational  */
										if (singleItem.directory().channelInfo().rsslReactorChannel().state() == ReactorChannel.State.CLOSED)
										{
											_closingItemQueue.addLast(singleItem);
											break;
										}
										else if(iue.errorCode() == OmmInvalidUsageException.ErrorCode.SHUTDOWN)
										{
											/* Remove this item as Reactor is shutdown */
											singleItem.remove();
											
											state.streamState(StreamStates.CLOSED);
											break;
										}
									}
									
									/* Sets the state that the item is being recovered */
									singleItem.state(SingleItem.ItemStates.RECOVERING);
									
									/* Add item to recovery queue to retry with another connection if any */
									_recoveryItemQueue.addLast(requestMsg);
								}
								else if(wsbMode == ReactorWarmStandbyMode.LOGIN_BASED)
								{
									sessionChannelInfo.addWSBStaleItem(singleItem);
								}
								else if (wsbMode == ReactorWarmStandbyMode.SERVICE_BASED)
								{
									sessionChannelInfo.addWSBServiceBasedStaleItem(singleItem.directory().serviceName(), singleItem);
								}
							}
							
						}
					}
				}
				else if (state.streamState() == StreamStates.CLOSED)
				{
					/* Recover item only for non-private item stream. */
					if(requestMsg.checkPrivateStream() == false)
					{
						state.streamState(StreamStates.OPEN);
						
						/* Sets the state that the item is being recovered */
						singleItem.state(SingleItem.ItemStates.RECOVERING);
						
						if(singleItem._itemClosedDirHash == null)
							singleItem._itemClosedDirHash = new HashSet<Directory<T>>();
						
						singleItem._itemClosedDirHash.add(item.directory());
						
						singleItem._lastDataState = state.dataState();
						singleItem._lastStatusCode = state.code();
						singleItem._lastStatusText = state.text().toString();
						
						/* Add item to recovery queue to retry with another connection if any */
						_recoveryItemQueue.addLast(requestMsg);
						
						notifyStatusMsg = false;
					}
				}
			}
			break;
		}
		
		if(notifyStatusMsg)
		{
			callbackClient.notifyOnAllMsg(statusMsg);
			callbackClient.notifyOnStatusMsg();
		}
		
		if (rsslStatusMsg.checkHasState() && state.streamState() != StreamStates.OPEN)
		{
			item.remove();
		}
		
		/* Restore the original stream state */
		state.streamState(originalStreamState);
		
		_consumerSession.nextDispatchTime(1000); // Wait for 1 millisecond to recover
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	/* Checks whether service's capability and QoS match with the request message */
	public boolean checkMatchingService(com.refinitiv.eta.codec.RequestMsg rsslRequestMsg, Service service, 
			WatchlistResult result)
	{
		/* Check to ensure that the service is up and accepting request */
		if(!service.checkHasState() || service.state().serviceState() == 0 || service.state().acceptingRequests() == 0)
		{
			result.resultCode = WatchlistResult.Code.SERVICE_NOT_UP;
			result.resultText = "Service not up";
			return false;
		}
		
		if(!isCapabilitySupported(rsslRequestMsg.domainType(), service))
		{
			result.resultCode = WatchlistResult.Code.CAPABILITY_NOT_FOUND;
			result.resultText = "Capability not supported";
			return false;
		}
		
		_matchedQos.clear();
		
		Qos qos;
		Qos worstQos;
		
		if(!rsslRequestMsg.checkHasQos())
		{
			qos = _defaultQos;
			worstQos = _defaultWorstQos;
		}
		else
		{
			qos = rsslRequestMsg.qos();
			worstQos = rsslRequestMsg.worstQos();
		}
		
		if(!isQosSupported(qos, worstQos, service, _matchedQos))
		{
			result.resultCode = WatchlistResult.Code.MATCHING_QOS_NOT_FOUND;
			result.resultText = "Service does not provide a matching QoS";
			return false;
		}
		
		result.resultCode = WatchlistResult.Code.SUCCESS;
		result.resultText = "";
		return true;
	}
	
	/* Determines if a service supports a capability. */
    static boolean isCapabilitySupported(int domainType, Service service)
    {
        boolean ret = false;
        
        if (service.checkHasInfo())
        {
            for (int i = 0; i < service.info().capabilitiesList().size(); i++)
            {
                if(service.info().capabilitiesList().get(i) == domainType)
                {
                    ret = true;
                    break;
                }
            }
        }
        
        return ret;
    }
    
    /* Determines if a service supports a qos or qos range. */
    boolean isQosSupported(Qos qos, Qos worstQos, Service service, Qos matchedQos)
    {
        boolean ret = false;
        
        if (service.checkHasInfo())
        {
            if (service.info().checkHasQos())
            {
                // service has qos
                for (int i = 0; i < service.info().qosList().size(); i++)
                {
                    Qos serviceQos = service.info().qosList().get(i);
                    if (worstQos == null)
                    {
                        // no worst qos, determine if request qos supported by service
                        if (serviceQos.equals(qos))
                        {
                            ret = true;
                            serviceQos.copy(matchedQos);
                            break;
                        }
                    }
                    else // worstQos specified
                    {
                        if (serviceQos.isInRange(qos, worstQos))
                        {
                            if (serviceQos.isBetter(matchedQos))
                            {
                                ret = true;
                                serviceQos.copy(matchedQos);
                            }
                        }
                    }
                }
            }
            else // service has no qos
            {
                // determine if qos matches default of Realtime, Tick-By-Tick
                if (worstQos == null)
                {
                    ret = _defaultQos.equals(qos);
                }
                else // worstQos specified
                {
                    ret = _defaultQos.isInRange(qos, worstQos);
                }
                
                if (ret == true)
                {
                    _defaultQos.copy(matchedQos);
                }
            }
        }
        
        // set isDynamic flag to that of requested qos before returning
        if (ret == true)
        {
            matchedQos.dynamic(qos.isDynamic());
        }
        
        return ret;
    }

	public void recoverStaleWSBItems(SessionChannelInfo<T> sessionChannelInfo, HashSet<SingleItem<T>> wsbStaleItems, boolean forceToRecoveryQueue)
	{
		Iterator<SingleItem<T>> singleItemIt =  wsbStaleItems.iterator();
		
		SingleItem<T> singleItem;
		while(singleItemIt.hasNext())
		{
			singleItem  = singleItemIt.next();
			
			/* Checks whether this item has been removed */
			if(singleItem._requestMsg == null)
			{
				/* Removes from the recovery queue */
				singleItemIt.remove();
				continue;
			}
			
			singleItem.state(SingleItem.ItemStates.CLOSING_STREAM);
			
			try
			{
				singleItem.close();
			}
			catch(OmmInvalidUsageException iue)
			{
				/* This will be closed later when the ReactorChannel is operational  */
				if (singleItem.directory().channelInfo().rsslReactorChannel().state() == ReactorChannel.State.CLOSED)
				{
					_closingItemQueue.addLast(singleItem);
					break;
				}
				else if(iue.errorCode() == OmmInvalidUsageException.ErrorCode.SHUTDOWN)
				{
					/* Remove this item as Reactor is shutdown */
					singleItem.remove();
					break;
				}
			}
			
			Directory<T> directory;
			directory = singleItem.directory().sessionDirectory().directory(singleItem._requestMsg);
			
			if(directory != null && !forceToRecoveryQueue)
			{
				singleItem._directory = directory;
				singleItem._serviceName = directory.serviceName();
				singleItem.state(SingleItem.ItemStates.NORMAL);
				if(singleItem.rsslSubmit(singleItem._requestMsg, false) == false)
				{
					singleItem.state(SingleItem.ItemStates.RECOVERING);
					_consumerSession.watchlist().recoverItemQueue().add(singleItem._requestMsg);
					_consumerSession.nextDispatchTime(1000); // Wait for 1 millisecond to recover
				}
			}
			else
			{
				singleItem.state(SingleItem.ItemStates.RECOVERING);
				_consumerSession.watchlist().recoverItemQueue().add(singleItem._requestMsg);
				_consumerSession.nextDispatchTime(1000); // Wait for 1 millisecond to recover
			}
			
			/* Removes from the recovery queue */
			singleItemIt.remove();
			
		}
	}
	
	/* This is used to fan-out item closed for remaining recovery items */
	public void close()
	{
		RequestMsg rsslRequestMsg = _recoveryItemQueue.poll();
		
		while(rsslRequestMsg != null)
		{
			tmpIntObject.value(rsslRequestMsg.streamId());
			
			SingleItem<T> item = (SingleItem<T>) _streamIdMap.get(tmpIntObject);
			
			if(item != null)
			{
				sendItemStatus(item, rsslRequestMsg, OmmState.StreamState.CLOSED,
					OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Consumer session is closed.");
			}
			
			rsslRequestMsg = _recoveryItemQueue.poll();
		}
	}
}
