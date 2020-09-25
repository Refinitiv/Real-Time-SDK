package com.refinitiv.eta.valueadd.reactor;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;

/* The watchlist service cache. */
class WlServiceCache
{    
    Watchlist _watchlist;
    HashMap<String,WlService> _servicesByNameTable = new HashMap<String,WlService>();
    HashMap<WlInteger,WlService> _servicesByIdTable = new HashMap<WlInteger,WlService>();
    LinkedList<WlService> _serviceList = new LinkedList<WlService>();
    
    WlInteger _tempWlInteger = ReactorFactory.createWlInteger();
    
    WlServiceCache(Watchlist watchlist)
    {
        _watchlist = watchlist;
    }
    
    /* Processes a list of services that were received on the directory stream. */ 
    int processServiceList(List<Service> serviceList, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        WlService wlService = null;
        
        for (Service service : serviceList)
        {
            switch (service.action())
            {
                case MapEntryActions.ADD:
                    // add to cache
                    wlService = addToCache(service);
                    
                    // notify item handler service added
                    ret = _watchlist.itemHandler().serviceAdded(wlService);
                    break;
                case MapEntryActions.UPDATE:
                    _tempWlInteger.value(service.serviceId());
                    wlService = _servicesByIdTable.get(_tempWlInteger);
                    if (wlService != null)
                    {
                        // this is a change to an existing service
                        
                        // update service in table (this applies to both services by id and services by name tables)
                        service.applyUpdate(wlService.rdmService());
                        
                        // notify item handler service updated
                        ret = _watchlist.itemHandler().serviceUpdated(wlService, service.checkHasState());
                    }
                    else // service not in tables, this is the same as an add
                    {
                        // add to cache
                        wlService = addToCache(service);
                        
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
                        if (wlService.rdmService().checkHasInfo())
                        {
                            serviceName = wlService.rdmService().info().serviceName().toString();
                            _servicesByNameTable.remove(serviceName);
                        }
                        _serviceList.remove(wlService);
                        
                        // notify item handler service deleted
                        ret = _watchlist.itemHandler().serviceDeleted(wlService, false);
                        
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
            _servicesByNameTable.put(service.info().serviceName().toString(), wlService);
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
    }
    
    /* Clear service cache for re-use. */
    void clear()
    {
        _servicesByNameTable.clear();
        _servicesByIdTable.clear();
        _serviceList.clear();
    }
}
