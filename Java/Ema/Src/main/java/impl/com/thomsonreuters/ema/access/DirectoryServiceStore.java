package com.thomsonreuters.ema.access;

import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;


public class DirectoryServiceStore 
{
    HashMap<String, ServiceIdInteger> _servicesNameAndIdTable = new HashMap<String, ServiceIdInteger>();
    HashMap<ServiceIdInteger, String> _servicesIdAndNameTable = new HashMap<ServiceIdInteger, String>();
    
    HashMap<ServiceIdInteger, Service> _servicesByIdStore = new HashMap<ServiceIdInteger, Service>();

    static ReentrantLock _cacheLock = new java.util.concurrent.locks.ReentrantLock();
    ServiceIdInteger _tempServiceIdInteger;
    DirectoryRefresh _directoryRefreshMsg = null;
    EmaObjectManager _objectManager;
    
    DirectoryServiceStore(EmaObjectManager objectManager)
    {    
    	_objectManager = objectManager;
    	_tempServiceIdInteger = createServiceIdInteger();
    }
    
    void addToMap(int serviceID, String serviceName)
    {
    	_cacheLock.lock();
    	
    	ServiceIdInteger serviceIdInteger = createServiceIdInteger();
        serviceIdInteger.value(serviceID);
 
        _servicesIdAndNameTable.put(serviceIdInteger, serviceName);
        _servicesNameAndIdTable.put(serviceName, serviceIdInteger);
       
        _cacheLock.unlock();
    }
    
    void addToMap(Service service)
    {
        if (service.checkHasInfo())
        {
        	addToMap(service.serviceId(),service.info().serviceName().toString());
        }
    }
    
    void clearMap()
    {
    	_cacheLock.lock();
    	
    	Set<Entry<ServiceIdInteger,String> > entrySet =  _servicesIdAndNameTable.entrySet();
    	
    	for( Entry<ServiceIdInteger,String> entry : entrySet)
    	{
    		_objectManager._ommServiceIdIntegerPool.add(entry.getKey());
    	}
    	
    	_servicesIdAndNameTable.clear();
    	_servicesNameAndIdTable.clear();
    	
    	 _cacheLock.unlock();
    }
    
    ServiceIdInteger serviceId(String serviceName)
    {
        return _servicesNameAndIdTable.get(serviceName);
    }

    String serviceName(int serviceId)
    {
    	_tempServiceIdInteger.value(serviceId);
        
        return _servicesIdAndNameTable.get(_tempServiceIdInteger);
    }
 
    Service addToStore(Service service)
    {
    	_cacheLock.lock();
    	
    	ServiceIdInteger serviceIdInteger = createServiceIdInteger();
        serviceIdInteger.value(service.serviceId());
        
        Service existingService = null;
 
        if ( ( existingService =_servicesByIdStore.get(serviceIdInteger) )  != null )
        {
        	existingService.applyUpdate(service);
        }
        else
        {
        	_servicesByIdStore.put(serviceIdInteger, service);
        	service = null;
        }
    	
    	_cacheLock.unlock();
    	
    	return service;
    }
        
    void remove(int serviceId)
    {
       	_tempServiceIdInteger.value(serviceId);
    	_cacheLock.lock();

    	String serviceName =  _servicesIdAndNameTable.get(_tempServiceIdInteger);
     			
    	_servicesIdAndNameTable.remove(_tempServiceIdInteger);
    	_servicesByIdStore.remove(_tempServiceIdInteger);
    	
    	if ( serviceName != null)
    	{
    		ServiceIdInteger serviceIdInteger = _servicesNameAndIdTable.get(serviceName);
    		_servicesNameAndIdTable.remove(serviceName);
    		_objectManager._ommServiceIdIntegerPool.add(serviceIdInteger);
    	}
                
        _cacheLock.unlock();
    }
    
    DirectoryRefresh getDirectoryRefreshMsg()
    {
    	Service service;
    	if ( _directoryRefreshMsg == null )
    	{
    		_directoryRefreshMsg = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
    		_directoryRefreshMsg.clear();
    		_directoryRefreshMsg.rdmMsgType(DirectoryMsgType.REFRESH);
    	}
    	else
    	{
    		_directoryRefreshMsg.clear();
    		_directoryRefreshMsg.rdmMsgType(DirectoryMsgType.REFRESH);
    	}
    	
    	int flags = 0;
    	
    	_cacheLock.lock();
    	
    	Set<Entry<ServiceIdInteger,Service>> entrySet = _servicesByIdStore.entrySet();
    	for (Entry<ServiceIdInteger,Service> entry : entrySet)
		{
    		service = entry.getValue();
    		if ( service.checkHasInfo() )
    			flags |= com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO;
    		
    		if( service.checkHasState() )
    			flags |= com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE;
    		
    		_directoryRefreshMsg.serviceList().add(service);
		}
    	
    	_directoryRefreshMsg.flags(flags);
    	_directoryRefreshMsg.streamId(0);
	    
	    Buffer stateText = CodecFactory.createBuffer();
	    stateText.data("Refresh Complete");
	    _directoryRefreshMsg.state().text(stateText);
	    _directoryRefreshMsg.state().streamState(StreamStates.OPEN);
	    _directoryRefreshMsg.state().dataState(DataStates.OK);
	    _directoryRefreshMsg.state().code(StateCodes.NONE);
	    
	    _cacheLock.unlock();
    	
    	return _directoryRefreshMsg;
    }
     
    ServiceIdInteger createServiceIdInteger()
    {    
    	ServiceIdInteger serviceIdInteger = (ServiceIdInteger)_objectManager._ommServiceIdIntegerPool.poll();
    	if (serviceIdInteger == null)
    	{
    		serviceIdInteger = new ServiceIdInteger();
    		_objectManager._ommServiceIdIntegerPool.updatePool(serviceIdInteger);
    	}
    	else
    	{
    		serviceIdInteger.clear();
    	}
    	return serviceIdInteger;

    }
     
    class ServiceIdInteger extends VaNode
    {
        int _value;
        
        int value()
        {
            return _value;
        }
        
        void value(int value)
        {
            _value = value;
        }
        
        public int hashCode()
        {
            return _value;
        }

        public boolean equals(Object obj)
        {
            if (obj == this)
            {
                return true;
            }
            
            ServiceIdInteger intObj;
            
            try
            {
                intObj = (ServiceIdInteger)obj;
            }
            catch (ClassCastException e)
            {
                return false;
            }
            
            return (intObj._value == _value);
        }

        void clear()
        {
            _value = 0;
        }
    }
}

