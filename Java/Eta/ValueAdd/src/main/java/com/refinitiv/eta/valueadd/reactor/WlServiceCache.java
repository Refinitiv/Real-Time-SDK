/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.rdm.Directory.WarmStandbyDirectoryServiceTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.ConsumerStatusServiceFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceFlags;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel.State;

/* The watchlist service cache. */
class WlServiceCache
{    
    Watchlist _watchlist;
    HashMap<String,WlService> _servicesByNameTable = new HashMap<String,WlService>();
    HashMap<WlInteger,WlService> _servicesByIdTable = new HashMap<WlInteger,WlService>();
    LinkedList<WlService> _serviceList = new LinkedList<WlService>();
    
    WlInteger _tempWlInteger = ReactorFactory.createWlInteger();
    
    boolean initDirectory = false;
    
    WlServiceCache(Watchlist watchlist)
    {
        _watchlist = watchlist;
    }
    
    /* Processes a list of services that were received on the directory stream. */ 
	int processServiceList(List<Service> serviceList, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        WlService wlService = null;
        long serviceState;
        
        boolean wsbActive = _watchlist.reactor().reactorHandlesWarmStandby(_watchlist.reactorChannel());

        
        for (Service service : serviceList)
        {
            switch (service.action())
            {
                case MapEntryActions.ADD:
                    // add to cache
                    wlService = addToCache(service);
                    if(initDirectory && wsbActive)
                    	wsbUpdateCachedService(wlService, MapEntryActions.ADD, errorInfo);
                    
                    // notify item handler service added
                    ret = _watchlist.itemHandler().serviceAdded(wlService);
                    break;
                case MapEntryActions.UPDATE:
                    _tempWlInteger.value(service.serviceId());
                    wlService = _servicesByIdTable.get(_tempWlInteger);
                    if (wlService != null)
                    {
                        // this is a change to an existing service
                        serviceState = wlService.rdmService().state().serviceState();
                        // update service in table (this applies to both services by id and services by name tables)
                        service.applyUpdate(wlService.rdmService());
                        
                        if(initDirectory && wsbActive)
                        {
                        	/* Chceck to see if the old state is the same as the new state after the service has been applied to cache */
                         	if(serviceState != wlService.rdmService().state().serviceState())
                         	{
                         		wsbServiceStateChange(wlService._tableKey, (int)wlService.rdmService().state().serviceState(), MapEntryActions.UPDATE, errorInfo);
                         	}
                         	
                         	wsbUpdateCachedService(wlService, MapEntryActions.UPDATE, errorInfo);
                         	
                        }
                        
                        
                        // notify item handler service updated
                        ret = _watchlist.itemHandler().serviceUpdated(wlService, service.checkHasState());
                    }
                    else // service not in tables, this is the same as an add
                    {
                        // add to cache
                        wlService = addToCache(service);
                        
                        if(initDirectory && wsbActive)
                        	wsbUpdateCachedService(wlService, MapEntryActions.ADD, errorInfo);

                        // notify item handler service added
                        ret = _watchlist.itemHandler().serviceAdded(wlService);
                    }
                    break;
                case MapEntryActions.DELETE:
                    // remove service from _servicesByIdTable, _servicesByNameTable and _serviceList
                    _tempWlInteger.value(service.serviceId());
                    wlService = _servicesByIdTable.remove(_tempWlInteger);

                    if (wlService != null)
                    {
                        String serviceName = null;
                    	serviceState = wlService.rdmService().state().serviceState();

                        if (wlService.rdmService().checkHasInfo())
                        {
                            serviceName = wlService.rdmService().info().serviceName().toString();
                            _servicesByNameTable.remove(serviceName);
                        }
                        _serviceList.remove(wlService);
                        
                        // notify item handler service deleted
                        ret = _watchlist.itemHandler().serviceDeleted(wlService, false);
                        
                        if(initDirectory && wsbActive)
                        {
                        	/* Chceck to see if the old state is the same as the new state after the service has been applied to cache */
                         	if(serviceState != 0)
                         	{
                         		wsbServiceStateChange(wlService._tableKey, 0, MapEntryActions.DELETE, errorInfo);
                         	}
                         	
                        	wsbUpdateCachedService(wlService, MapEntryActions.DELETE, errorInfo);
                        	
                        }
                        
                        // return table key to pool
                        wlService.tableKey().returnToPool();
                        
                        // return WlService to pool
                        wlService.returnToPool();
                    }
                    break;
                default:
                    ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                 ReactorReturnCodes.FAILURE,
                                                                 "WlServiceCache.processServiceList",
                                                                 "Invalid map entry action (" + service.action() + ") received on directory service.");
                    break;
            }
            
            // break out of loop when error encountered
            if (ret < ReactorReturnCodes.SUCCESS)
            {
                break;
            }
        }
        
        if(_watchlist._reactorChannel.reactor().reactorHandlesWarmStandby(_watchlist._reactorChannel))
        {
	        ReactorChannel channel = _watchlist._reactorChannel;
        	ReactorWarmStandbyHandler wsbHandler = _watchlist._reactorChannel.warmStandByHandlerImpl;
        	if(!initDirectory)
	        {
	        	if((wsbHandler.warmStandbyHandlerState() & ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_DIRECTORY_RESPONSE) == 0)
	        	{
	        		/* Update the full service cache */
	        		ret = wsbUpdateFullServiceCache(errorInfo);
		        	
		        	/*
		    		 * We have made the first connection, so we can now connect the secondaries...
		    		 */
		    		wsbHandler.setPrimaryDirectoryResponseState();
		    		
		    		// Handle generic messages for service-based
					_watchlist.reactor().reactorWSBHandleServiceActiveStandby(_watchlist.reactorChannel(), 
						wsbHandler.currentWarmStandbyGroupImpl(), true, errorInfo);
					
					// Submit requests now only for the first connection... every one after this will be handled by the watchlist.
					if(wsbHandler.currentWarmStandbyGroupIndex() == 0)
					{
						if (_watchlist.reactor().submitWSBRequestQueue(_watchlist.reactorChannel().warmStandByHandlerImpl,
								_watchlist.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl(),
								_watchlist.reactorChannel(), errorInfo) != ReactorReturnCodes.SUCCESS)
						{
							if (_watchlist.reactorChannel().server() == null && !_watchlist.reactorChannel().recoveryAttemptLimitReached()) // client
								// channel
							{
								// Do not return failure here because it's in a dispatch call
								_watchlist.reactorChannel().state(State.DOWN_RECONNECTING);
								_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
								ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, _watchlist.reactorChannel(), errorInfo);
							} else // server channel or no more retries
							{
								// Do not return failure here because it's in a dispatch call
								_watchlist.reactorChannel().state(State.DOWN);
								_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
								ReactorChannelEventTypes.CHANNEL_DOWN, _watchlist.reactorChannel(), errorInfo);
							}
						}
					}


		    		ReactorWarmStandbyEvent reactorWarmStandbyEvent = _watchlist._reactorChannel.reactor().reactorWarmStandbyEventPool.getEvent(errorInfo);
					reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.CONNECT_SECONDARY_SERVER;
	
		    		_watchlist._reactorChannel.reactor().sendWarmStandbyEvent(channel, reactorWarmStandbyEvent, errorInfo);
		    		
	        	}
	        	else
	        	{
		    		wsbHandler.setSecondaryDirectoryResponseState();
	        		
		    		/* Check to see if this directory response is correct */
	        		if(wsbCompareRDMServiceInfo() == false)
	        		{
	        			ReactorWarmStandbyEvent reactorWarmStandbyEvent = _watchlist._reactorChannel.reactor().reactorWarmStandbyEventPool.getEvent(errorInfo);
	        			_watchlist._reactorChannel.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
								"processServiceList", "The source directory response from standby server does not match with the primary server.");
						reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;
		
			    		_watchlist._reactorChannel.reactor().sendWarmStandbyEvent(channel, reactorWarmStandbyEvent, errorInfo);
	        		}
	        		else
	        		{
	        			ret = wsbUpdateFullServiceCache(errorInfo);
	        			/* Update has gone through, now update the service(s) if they're new for Service-based */
						_watchlist.reactor().reactorWSBHandleServiceActiveStandby(_watchlist.reactorChannel(), 
								wsbHandler.currentWarmStandbyGroupImpl(), false, errorInfo);
						
						// Submit requests now
						if (_watchlist.reactor().submitWSBRequestQueue(_watchlist.reactorChannel().warmStandByHandlerImpl,
								_watchlist.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl(),
								_watchlist.reactorChannel(), errorInfo) != ReactorReturnCodes.SUCCESS)
						{
							if (_watchlist.reactorChannel().server() == null && !_watchlist.reactorChannel().recoveryAttemptLimitReached()) // client
								// channel
							{
								// Do not return failure here because it's in a dispatch call
								_watchlist.reactorChannel().state(State.DOWN_RECONNECTING);
								_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
								ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, _watchlist.reactorChannel(), errorInfo);
							} else // server channel or no more retries
							{
								// Do not return failure here because it's in a dispatch call
								_watchlist.reactorChannel().state(State.DOWN);
								_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
								ReactorChannelEventTypes.CHANNEL_DOWN, _watchlist.reactorChannel(), errorInfo);
							}
						}
	        		}
	        	}
	        	
	        	initDirectory = true;
	        }
        	else
        	{
	    		wsbHandler.setSecondaryDirectoryResponseState();
        		
	    		/* Check to see if this directory response is correct */
        		if(wsbCompareRDMServiceInfo() == false)
        		{
        			ReactorWarmStandbyEvent reactorWarmStandbyEvent = _watchlist._reactorChannel.reactor().reactorWarmStandbyEventPool.getEvent(errorInfo);
        			_watchlist._reactorChannel.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
							"processServiceList", "The source directory response from standby server does not match with the primary server.");
					reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;
	
		    		_watchlist._reactorChannel.reactor().sendWarmStandbyEvent(channel, reactorWarmStandbyEvent, errorInfo);
        		}
        	}
        }
        
        return ret;        
    }
    
    /* Adds a service to the service cache. */
    WlService addToCache(Service service)
    {
        // add to _serviceList, _servicesByIdTable and _servicesByNameTable
        WlService wlService = ReactorFactory.createWlService();
        service.copy(wlService.rdmService());
        _serviceList.add(wlService);
        WlInteger wlInteger = ReactorFactory.createWlInteger();
        wlInteger.value(service.serviceId());
        wlService.tableKey(wlInteger);
        _servicesByIdTable.put(wlInteger, wlService);
        if (service.checkHasInfo())
        {
            _servicesByNameTable.put(wlService.rdmService().info().serviceName().toString(), wlService);
        } 
        
        return wlService;
    }
    
    /* Retrieve service id from service name. */
    int serviceId(String serviceName)
    {
        int serviceId = ReactorReturnCodes.PARAMETER_INVALID;
        
        WlService wlService = service(serviceName);
        
        if (wlService != null)
        {
            serviceId = wlService.rdmService().serviceId();
        }
        
        return serviceId;
    }

    /* Retrieve service name from service id. */
    String serviceName(int serviceId)
    {
        String serviceName = null;
        
        WlService wlService = service(serviceId);
        
        if (wlService != null && wlService.rdmService().checkHasInfo())
        {
            serviceName = wlService.rdmService().info().serviceName().toString();
        }
        
        return serviceName;
    }
    
    /* Retrieve service by service name. */
	WlService service(String serviceName)
    {
        return _servicesByNameTable.get(serviceName);
    }

    /* Retrieve service by service id. */
    WlService service(int serviceId)
    {
        _tempWlInteger.value(serviceId);
        
        return _servicesByIdTable.get(_tempWlInteger);
    }
    
    /* Clear the service cache. */
    void clearCache(boolean channelIsDown)
    {
        WlService wlService = null;
        
        // clear service list
        while ((wlService = _serviceList.poll()) != null)
        {
            // Handle items associated with this service.
        	_watchlist.itemHandler().serviceDeleted(wlService, channelIsDown);
        	// Can set the errorInfo to null here because DELETE actions will not trigger sending any generic messages
        	if(_watchlist.reactor().reactorHandlesWarmStandby(_watchlist.reactorChannel()))
        		wsbUpdateCachedService(wlService, MapEntryActions.DELETE, null);
        	
            // clear service
            wlService.rdmService().clear();
            
            // return table key to pool
            wlService.tableKey().returnToPool();
            
            //  return wlService to pool
            wlService.returnToPool();
        }
        
        // clear service tables
        _servicesByNameTable.clear();
        _servicesByIdTable.clear();
        initDirectory = false;
    }
    
    /* Clear service cache for re-use. */
    void clear()
    {
        _servicesByNameTable.clear();
        _servicesByIdTable.clear();
        _serviceList.clear();
        initDirectory = false;
    }
    
    /* Update the warm standby service cache with information from this connection */
    int wsbUpdateFullServiceCache(ReactorErrorInfo errorInfo)
    {
    	WlService newService = null;
    	Iterator<WlService> iter = _serviceList.iterator();

    	
    	while(iter.hasNext())
    	{
    		newService = iter.next();
    		
    		wsbUpdateCachedService(newService, newService.rdmService().action(), errorInfo);
    	}
    	
    	
    	return ReactorReturnCodes.SUCCESS;
    }

    /* Verifies that the service(s) in the watchlist cache match the ones in the wsb cache. */
    private boolean wsbCompareRDMServiceInfo()
	{
    	WlService wlService = null;
    	ReactorWarmStandbyGroupImpl wsbGroup = _watchlist._reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
		ReactorWSBService wsbService = null;

    	// Iterate through the warmstandby group's services
		Iterator<Map.Entry<WlInteger, ReactorWSBService>> iter = wsbGroup._perServiceById.entrySet().iterator();

    	while(iter.hasNext())
    	{
    		wsbService = iter.next().getValue();
    		wlService = _servicesByIdTable.get(wsbService.serviceId);
    		
    		if(wlService != null)
    		{
    			if(!wlService.rdmService().info().serviceName().equals(wsbService.serviceInfo.info().serviceName()))
    			{
    				return false;
    			}
    			
    			if(wlService.rdmService().info().capabilitiesList().size() != wsbService.serviceInfo.info().capabilitiesList().size())
    			{
    				return false;
    			}
    			else
    			{
    				for(int i = 0; i < wlService.rdmService().info().capabilitiesList().size(); i++)
    				{
    					if(wlService.rdmService().info().capabilitiesList().get(i).longValue() != wsbService.serviceInfo.info().capabilitiesList().get(i).longValue())
    					{
    						return false;
    					}
    				}
    			}
    			
    			if(wlService.rdmService().info().dictionariesProvidedList().size() != wsbService.serviceInfo.info().dictionariesProvidedList().size())
    			{
    				return false;
    			}
    			else
    			{
    				for(int i = 0; i < wlService.rdmService().info().dictionariesProvidedList().size(); i++)
    				{
    					if(!wlService.rdmService().info().dictionariesProvidedList().get(i).equals(wsbService.serviceInfo.info().dictionariesProvidedList().get(i)))
    					{
    						return false;
    					}
    				}
    			}
    			
    			if(wlService.rdmService().info().dictionariesUsedList().size()!= wsbService.serviceInfo.info().dictionariesUsedList().size())
    			{
    				return false;
    			}
    			else
    			{
    				for(int i = 0; i < wlService.rdmService().info().dictionariesUsedList().size(); i++)
    				{
    					if(!wlService.rdmService().info().dictionariesUsedList().get(i).equals(wsbService.serviceInfo.info().dictionariesUsedList().get(i)))
    					{
    						return false;
    					}
    				}
    			}
    			
    			if(wlService.rdmService().info().qosList().size() != wsbService.serviceInfo.info().qosList().size())
    			{
    				return false;
    			}
    			else
    			{
    				for(int i = 0; i < wlService.rdmService().info().qosList().size(); i++)
    				{
    					if(!wlService.rdmService().info().qosList().get(i).equals(wsbService.serviceInfo.info().qosList().get(i)))
    					{
    						return false;
    					}
    				}
    			}
    			
    			if(wlService.rdmService().info().checkHasSupportsQosRange() != wsbService.serviceInfo.info().checkHasSupportsQosRange())
    			{
    				return false;
    			}
    			else
    			{	
	    			if(wlService.rdmService().info().supportsQosRange() != wsbService.serviceInfo.info().supportsQosRange())
	    			{
	    				return false;
	    			}
    			}
    			
    			if(wlService.rdmService().info().checkHasItemList() != wsbService.serviceInfo.info().checkHasItemList())
    			{
    				return false;
    			}
    			else
    			{
    				if(wlService.rdmService().info().itemList().length() != 0 && wsbService.serviceInfo.info().itemList().length() != 0)
    				{
		    			if(!wlService.rdmService().info().itemList().equals(wsbService.serviceInfo.info().itemList()))
		    			{
		    				return false;
		    			}
    				}
    			}
    			
    			if(wlService.rdmService().info().supportsOutOfBandSnapshots() != wsbService.serviceInfo.info().supportsOutOfBandSnapshots())
    			{
    				return false;
    			}
    			
    			if(wlService.rdmService().info().acceptingConsumerStatus() != wsbService.serviceInfo.info().acceptingConsumerStatus())
    			{
    				return false;
    			}
    			
    		}
    		else
    		{
    			wlService = _servicesByNameTable.get(wsbService.serviceInfo.info().serviceName().toString());
    			if(wlService != null)
    			{
    				return false;
    			}
    		}
    	}
		return true;
	}
    
    /* Updates the service cache for a single given service */
	void wsbUpdateCachedService(WlService newService, int entryAction, ReactorErrorInfo errorInfo)
	{
		ReactorWarmStandbyGroupImpl wsbGroup = _watchlist._reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
		ReactorWSBService wsbService = wsbGroup._perServiceById.get(newService.tableKey());
		boolean addToUpdateList = false;
		boolean hasService = false;
		boolean sendWsbMsg = false;
		
		
		if(wsbService != null)
		{
			/* Found, update the service in the wsb cache. */
			
			/* Aggregate the serviceState.  */
			boolean containsChannel = wsbService.channels.contains(_watchlist._reactorChannel);
			
			if(newService.rdmService().state().serviceState() != wsbService.serviceState.serviceState())
			{
				/* Service is from down to up */
				if(newService.rdmService().state().serviceState() == 1)
				{
					wsbService.serviceState.serviceState(1);
					if(!containsChannel)
						wsbService.channels.add(_watchlist._reactorChannel);
					
					wsbService.updateServiceFilter |= ServiceFlags.HAS_STATE;
					wsbService.serviceAction = MapEntryActions.UPDATE;
					addToUpdateList = true;
					sendWsbMsg = true;
					
				}
				else if(newService.rdmService().state().serviceState() == 0)
				{
					if(containsChannel)
					{
						wsbService.channels.remove(_watchlist._reactorChannel);
					}
					
					if(wsbService.channels.size() == 0)
					{
						wsbService.serviceState.serviceState(0);
					
						wsbService.updateServiceFilter |= ServiceFlags.HAS_STATE;
						wsbService.serviceAction = MapEntryActions.UPDATE;
						addToUpdateList = true;
					}
				}
			}
			else if(entryAction != MapEntryActions.DELETE && !containsChannel && (newService.rdmService().state().serviceState() == 1))
			{
				wsbService.channels.add(_watchlist._reactorChannel);
				sendWsbMsg = true;
			}
			
			if(entryAction == MapEntryActions.DELETE)
			{
				/* Check to see other channels have this service */
				
				if((containsChannel && wsbService.channels.size() > 1) || (!containsChannel && wsbService.channels.size() > 0))
				{
					hasService = true;
				}
				// Remove the channel from the wsbService here.  The channels are handled here and when sendDirectoryMsgCallback is called.
				if(containsChannel)
				{
					wsbService.channels.remove(_watchlist._reactorChannel);
					
					sendWsbMsg = true;
				}
				
				if(hasService == false)
				{
					wsbService.serviceAction = MapEntryActions.DELETE;
					addToUpdateList = true;
				}
			}
			else if(wsbService.serviceAction == MapEntryActions.DELETE && entryAction == MapEntryActions.ADD)
			{
				wsbService.serviceAction = MapEntryActions.ADD;
				wsbService.updateServiceFilter = newService.rdmService().flags();
				addToUpdateList = true;
			}
			
			if(addToUpdateList)
			{
				wsbGroup._updateServiceList.add(wsbService);
			}
			
		}
		else
		{
			/* New Service for the cache */
			if(entryAction == MapEntryActions.DELETE)
			{
				return;
			}
						
			wsbService = ReactorFactory.createWsbService();
			
			newService.rdmService().copy(wsbService.serviceInfo);
			newService.rdmService().state().copy(wsbService.serviceState);

			wsbService.serviceAction = newService.rdmService().action();
			wsbService.updateServiceFilter = newService.rdmService().flags();
			
			wsbService.serviceId.value(newService.tableKey().value());
			/* If the service is active on this current connection, set the Active channel here */
			if(wsbService.serviceState.serviceState() == 1)
			{
				wsbService.channels.add(_watchlist.reactorChannel());
				// Determine the active channel later for the service... only used with service-based.
			}
			
			wsbGroup._updateServiceList.add(wsbService);
			wsbGroup._perServiceById.put(wsbService.serviceId, wsbService);
			sendWsbMsg = true;
		}
		
		/* If we have received an initial response with services and this a SERVICE mode WSB, and this is an ADD action
		 * send out the generic message to the provider indicating that this is either the new active
		 * (if an active does not exist for the service) or new standby 
		 */
		if(initDirectory && sendWsbMsg && wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
		{
			if(entryAction == MapEntryActions.ADD)
			{
			
				Reactor reactor = _watchlist.reactor();
				ReactorChannel reactorChannel = _watchlist.reactorChannel();
				boolean activeService = _watchlist.reactor().wsbServiceInStartupList(wsbGroup, newService,
						reactorChannel);
				
				if (wsbService.activeChannel == null && newService._rdmService.state().serviceState() == 1 && activeService)
				{
					reactorChannel._directoryConsumerStatus.clear();
					reactorChannel._directoryConsumerStatus
							.streamId(reactorChannel.watchlist().directoryHandler()._directoryStreamId);
					reactorChannel._serviceConsumerStatus.clear();
					reactorChannel._serviceConsumerStatus
							.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
					reactorChannel._serviceConsumerStatus
							.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
					reactorChannel._serviceConsumerStatus.serviceId(newService._rdmService.serviceId());
					reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
							.add(reactorChannel._serviceConsumerStatus);
	
					/*
					 * Write directly to the channel without involving the watchlist or wsb message
					 * queues
					 */
					if (reactor.submitChannel(reactorChannel, reactorChannel._directoryConsumerStatus,
							reactor.reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
					{
						if (reactorChannel.server() == null
								&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							reactor.sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
									errorInfo);
							return;
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							reactor.sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
									errorInfo);
							return;
						}
					}
	
					wsbService.activeChannel = reactorChannel;
				} else
				{
					if(wsbService.activeChannel != reactorChannel)
					{
						reactorChannel._directoryConsumerStatus.clear();
						reactorChannel._directoryConsumerStatus
								.streamId(reactorChannel.watchlist().directoryHandler()._directoryStreamId);
						reactorChannel._serviceConsumerStatus.clear();
						reactorChannel._serviceConsumerStatus
								.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
						reactorChannel._serviceConsumerStatus
								.warmStandbyMode(WarmStandbyDirectoryServiceTypes.STANDBY);
						reactorChannel._serviceConsumerStatus.serviceId(newService._rdmService.serviceId());
						reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
								.add(reactorChannel._serviceConsumerStatus);
		
						/*
						 * Write directly to the channel without involving the watchlist or wsb message
						 * queues
						 */
						if (reactor.submitChannel(reactorChannel, reactorChannel._directoryConsumerStatus,
								reactor.reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
						{
							if (reactorChannel.server() == null
									&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
							{
								reactorChannel.state(State.DOWN_RECONNECTING);
								reactor.sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
										ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
										errorInfo);
								return;
							} else // server channel or no more retries
							{
								reactorChannel.state(State.DOWN);
								reactor.sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
										ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
										errorInfo);
								return;
							}
						}
					}
				}
			}
		}
	}
	
	/* Send a ACTIVE_TO_STANDBY or STANDBY_TO_ACTIVE message if the service state has changed.  This will only be sent on a update, not on
	 * any initial directory response for either the active or standby server connections.  This function does not aggregate the state of the
	 * wsb cache, that is handled in wsbUpdateCachedService
	 */
	private void wsbServiceStateChange(WlInteger serviceId, int newServiceState, int entryAction, ReactorErrorInfo errorInfo)
	{
		ReactorWarmStandbyGroupImpl wsbGroup = _watchlist.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
		if(wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
		{
			ReactorWSBService wsbService = wsbGroup._perServiceById.get(serviceId);
			
			if(newServiceState == 0 && wsbService.activeChannel == _watchlist.reactorChannel())
			{
				ReactorChannel reactorChannel = _watchlist.reactorChannel();
				if(entryAction != MapEntryActions.DELETE)
				{
					/*
					 * Send a service consumer status message indicating that this is now the standby.
					 */
					reactorChannel._directoryConsumerStatus.clear();
					reactorChannel._directoryConsumerStatus
							.streamId(reactorChannel.watchlist().directoryHandler()._directoryStreamId);
					reactorChannel._serviceConsumerStatus.clear();
					reactorChannel._serviceConsumerStatus
							.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
					reactorChannel._serviceConsumerStatus
							.warmStandbyMode(WarmStandbyDirectoryServiceTypes.STANDBY);
					reactorChannel._serviceConsumerStatus.serviceId(serviceId.value());
					reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
							.add(reactorChannel._serviceConsumerStatus);
	
					/*
					 * Write directly to the channel without involving the watchlist or wsb message
					 * queues
					 */
					if (_watchlist.reactor().submitChannel(reactorChannel, reactorChannel._directoryConsumerStatus,
							_watchlist.reactor().reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
					{
						if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
									errorInfo);
							return;
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
							return;
						}
					}
				}


				wsbService.activeChannel = null;

				// Find a new active for this service, if it exists
				for (int i = 0; i < wsbService.channels.size(); i++)
				{
					ReactorChannel processReactorChannel = wsbService.channels.get(i);
					if (processReactorChannel != reactorChannel)
					{
						WlService processChannelService = processReactorChannel.watchlist().directoryHandler()
								.service(wsbService.serviceId.value());
						if (processChannelService == null)
							continue;

						if (processChannelService.rdmService().state().serviceState() == 0)
							continue;

						if (processChannelService.rdmService().state().serviceState() == 1)
						{
							/*
							 * Send a service consumer status message indicating that this is the new active
							 * This should trigger the upstream provider to start sending data.
							 */
							reactorChannel._directoryConsumerStatus.clear();
							reactorChannel._directoryConsumerStatus.streamId(
									processReactorChannel.watchlist().directoryHandler()._directoryStreamId);
							reactorChannel._serviceConsumerStatus.clear();
							reactorChannel._serviceConsumerStatus
									.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
							reactorChannel._serviceConsumerStatus
									.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
							reactorChannel._serviceConsumerStatus
									.serviceId(processChannelService.tableKey().value());
							reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
									.add(reactorChannel._serviceConsumerStatus);

							/*
							 * Write directly to the channel without involving the watchlist or wsb message
							 * queues
							 */
							if (_watchlist.reactor().submitChannel(processReactorChannel, reactorChannel._directoryConsumerStatus,
									_watchlist.reactor().reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
							{
								if (processReactorChannel.server() == null
										&& !processReactorChannel.recoveryAttemptLimitReached()) // client
																									// channel
								{
									processReactorChannel.state(State.DOWN_RECONNECTING);
									_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
											ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
											processReactorChannel, errorInfo);
									return;
								} else // server channel or no more retries
								{
									processReactorChannel.state(State.DOWN);
									_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
											ReactorChannelEventTypes.CHANNEL_DOWN, processReactorChannel,
											errorInfo);
									return;
								}
							}

							wsbService.activeChannel = processReactorChannel;
							return;
						}
					}
				}
			}
			else
			{
				/* There isn't an active, and the new service is up, so we've got a new active for this service */
				if(newServiceState == 1 && wsbService.activeChannel == null)
				{
					ReactorChannel reactorChannel = _watchlist.reactorChannel();
					/*
					 * Send a service consumer status message indicating that this is the new active
					 * This should trigger the upstream provider to start sending data.
					 */
					reactorChannel._directoryConsumerStatus.clear();
					reactorChannel._directoryConsumerStatus.streamId(reactorChannel.watchlist()._directoryHandler._directoryStreamId);
					reactorChannel._serviceConsumerStatus.clear();
					reactorChannel._serviceConsumerStatus.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
					reactorChannel._serviceConsumerStatus.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
					reactorChannel._serviceConsumerStatus.serviceId(serviceId.value());
					reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
							.add(reactorChannel._serviceConsumerStatus);

					/*
					 * Write directly to the channel without involving the watchlist or wsb message
					 * queues
					 */
					if (_watchlist.reactor().submitChannel(reactorChannel, reactorChannel._directoryConsumerStatus, _watchlist.reactor().reactorSubmitOptions,
							errorInfo) < ReactorReturnCodes.SUCCESS)
					{
						if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
							return;
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							_watchlist.reactor().sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
							return;
						}
					}

					wsbService.activeChannel = reactorChannel;

				}
			}
		}
		else
		{
			// LOGIN based
			if(newServiceState == 1 && _watchlist.reactorChannel().isActiveServer)
			{
				
				ReactorWarmStandbyEvent reactorWarmStandbyEvent = _watchlist.reactor().reactorWarmStandbyEventPool
						.getEvent(errorInfo);
				reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP;
				reactorWarmStandbyEvent.reactorChannel = _watchlist.reactorChannel();
				
				_watchlist.reactor().sendWarmStandbyEvent(_watchlist.reactorChannel(), reactorWarmStandbyEvent, errorInfo);
			}
		}
		
	}
	

}

