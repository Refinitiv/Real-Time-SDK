///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.ConfigReader.XMLnode;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceGroup;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceLink;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceState;

class DirectoryCache
{
	String						directoryName;
	private DirectoryRefresh 	directoryRefresh;
	
	DirectoryCache()
	{
		directoryName = "";
		directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
		clear();
	}
	
	void clear()
	{
		directoryName = "";
		directoryRefresh.clear();
	
	    directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
	       
	    directoryRefresh.streamId(0);
	    
	    Buffer stateText = CodecFactory.createBuffer();
	    stateText.data("Source Directory Refresh Completed");
	        
	    directoryRefresh.state().streamState(StreamStates.OPEN);
	    directoryRefresh.state().dataState(DataStates.OK);
	    directoryRefresh.state().code(StateCodes.NONE);
	    directoryRefresh.state().text(stateText);
	}
	
	void addService(Service service)
	{
		directoryRefresh.serviceList().add(service);
	}
	
	Service getService(int serviceId)
	{
		for(int index = 0; index < directoryRefresh.serviceList().size(); index++ )
		{
			if ( directoryRefresh.serviceList().get(index).serviceId() == serviceId )
			{
				return directoryRefresh.serviceList().get(index);
			}
		}
		
		return null;
	}
	
	void removeService(int serviceId)
	{
		for(int index = 0; index < directoryRefresh.serviceList().size(); index++ )
		{
			if ( directoryRefresh.serviceList().get(index).serviceId() == serviceId )
			{
				directoryRefresh.serviceList().remove(index);
				break;
			}
		}
	}
	
	List<Service> serviceList()
	{
		return directoryRefresh.serviceList();
	}
	
	DirectoryRefresh getDirectoryRefresh()
	{		
		return directoryRefresh;
	}
}

interface DirectoryServiceStoreClient
{
	void onServiceDelete(ClientSession clientSession,int serviceId);
	
	void onServiceStateChange(ClientSession clientSession, int serviceId, ServiceState serviceState);
	
	void onServiceGroupChange(ClientSession clientSession, int serviceId, List<ServiceGroup> serviceGroupList);
}

abstract class DirectoryServiceStore 
{
	protected HashMap<String, ServiceIdInteger> _servicesNameAndIdTable;
	protected HashMap<ServiceIdInteger, String> _servicesIdAndNameTable;
	protected ServiceIdInteger 				_tempServiceIdInteger;
	protected DirectoryCache	 			_directoryCache;
	protected boolean						_bUsingDefaultService = false;
	
    private static ReentrantLock 			_cacheLock = new java.util.concurrent.locks.ReentrantLock();
    private EmaObjectManager 				_objectManager;
    private int				 				_providerRole;
    private OmmCommonImpl	 				_ommCommonImpl;
    private BaseConfig		 				_activeConfig;
    private DecodeIterator 					_userStoreDecodeIt = null;
    private DirectoryMsg	 				_submittedDirectoryMsg;
    private DirectoryServiceStoreClient		_directoryServiceStoreClient = null;
    private static ArrayDeque<Service>		_servicePool = new ArrayDeque<>(5);
    
    abstract boolean checkExistingServiceId(int serviceId, StringBuilder errorText);
    abstract boolean addServiceIdAndNamePair(int serviceId, String serviceName, StringBuilder errorText);
    
    DirectoryServiceStore(EmaObjectManager objectManager, int providerRole, OmmCommonImpl ommCommonImpl,
    		BaseConfig baseConfig )
    {   
    	_objectManager = objectManager;
    	_tempServiceIdInteger = createServiceIdInteger();
    	_directoryCache = new DirectoryCache();
    	_providerRole = providerRole;
    	_ommCommonImpl = ommCommonImpl;
    	_activeConfig = baseConfig;
    	_submittedDirectoryMsg = DirectoryMsgFactory.createMsg();
    	
    	_servicesNameAndIdTable = new HashMap<String, ServiceIdInteger>(baseConfig.serviceCountHint);
    	_servicesIdAndNameTable = new HashMap<ServiceIdInteger, String>(baseConfig.serviceCountHint);
    }
    
    DirectoryMsg getSubmittedDirectoryMsg()
    {
    	return _submittedDirectoryMsg;
    }
    
	private static Service getService()
	{
		Service service;
		
		if ( _servicePool.size() == 0)
		{
			service = DirectoryMsgFactory.createService();
		}
		else
		{
			service = _servicePool.pop();
		}
		
		service.clear();
		
		return service;
	}
	
	private static void returnService(Service service)
	{
		_servicePool.push(service);
	}
    
    void addToMap(int serviceID, String serviceName)
    {
    	_cacheLock.lock();
    	
    	ServiceIdInteger serviceIdInteger = createServiceIdInteger().value(serviceID);
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
        
    void remove(int serviceId)
    {
       	_tempServiceIdInteger.value(serviceId);
    	_cacheLock.lock();

    	String serviceName =  _servicesIdAndNameTable.get(_tempServiceIdInteger);
     			
    	_servicesIdAndNameTable.remove(_tempServiceIdInteger);
   	
    	if ( serviceName != null)
    	{
    		ServiceIdInteger serviceIdInteger = _servicesNameAndIdTable.get(serviceName);
    		_servicesNameAndIdTable.remove(serviceName);
    		_objectManager._ommServiceIdIntegerPool.add(serviceIdInteger);
    	}
                
        _cacheLock.unlock();
    }
    
    void setClient(DirectoryServiceStoreClient directoryServiceStoreClient)
    {
    	_directoryServiceStoreClient = directoryServiceStoreClient;
    }
    
    void notifyOnServiceStateChange(ClientSession clientSession, Service service)
    {
    	if ( _directoryServiceStoreClient != null )
    	{
    		if ( service.state().action() !=  FilterEntryActions.CLEAR )
    		{
    			_directoryServiceStoreClient.onServiceStateChange(clientSession, service.serviceId(), service.state());
    		}
    	}
    }
    
    void notifyOnServiceGroupChange(ClientSession clientSession, Service service)
    {
    	if ( _directoryServiceStoreClient != null )
    	{
    		if ( !service.groupStateList().isEmpty() )
    		{
    			_directoryServiceStoreClient.onServiceGroupChange(clientSession, service.serviceId(), service.groupStateList());
    		}
    	}
    }
    
    void notifyOnServiceDelete(ClientSession clientSession, Service service)
    {
    	if ( _directoryServiceStoreClient != null )
    	{
    		_directoryServiceStoreClient.onServiceDelete(clientSession, service.serviceId());
    	}
    }
    
    void loadConfigDirectory(DirectoryCache directoryCache, EmaConfigBaseImpl config, List<ServiceDictionaryConfig> serviceDictionaryConfigList)
    {
    	String directoryName;
    	
    	if ( _providerRole == OmmProviderConfig.ProviderRole.INTERACTIVE )
    	{
    		directoryName =  (String)config.xmlConfig().getIProviderAttributeValue(
    				_activeConfig.configuredName, ConfigManager.IProviderDirectoryName);
    	}
    	else
    	{
    		directoryName =  (String)config.xmlConfig().getNiProviderAttributeValue(
    				_activeConfig.configuredName, ConfigManager.NiProviderDirectoryName);
    	}
				
		if ( directoryName == null || directoryName.isEmpty() )
			directoryName = (String)config.xmlConfig().getDefaultDirectoryName();
		
		if ( directoryName == null || directoryName.isEmpty() )
			directoryName = (String)config.xmlConfig().getFirstDirectory();
		
		if (directoryName == null || directoryName.isEmpty() )
		{
			if ( directoryCache != null)
			{
				directoryCache.directoryName = directoryName;
				useDefaultService(config, directoryCache);
			}
		}
		else
		{
			if ( directoryCache != null)
			{
				directoryCache.directoryName = directoryName;
			}
			
			XMLnode directoryNode = config.xmlConfig().getDirectory(directoryName);
			
			if (directoryNode != null)
			{
				int numberofservice = 0;
				Set<String> serviceNameSet = new LinkedHashSet<>();
				Set<Integer> serviceIdSet = new LinkedHashSet<>();
				List<Service> unspecifiedIdList = new ArrayList<>();
				boolean result = false;
				Service service = null;
				String serviceName = null;
				
				for(int i = 0; i < directoryNode.children().size() ; i++ )
				{
					XMLnode childNode = directoryNode.children().get(i);
					
					if( childNode != null && childNode.tagId() == ConfigManager.Service )
					{	
						if ( childNode.attributeList() != null )
						{
							serviceName = (String)childNode.attributeList().getValue(ConfigManager.ServiceName);
						}
						
						if ( serviceName != null && !serviceName.isEmpty() )
						{
							if( serviceNameSet.contains(serviceName))
							{
								config.errorTracker().append("service[").append(serviceName)
								.append("] is already specified by another service. Will drop this service.").create(Severity.ERROR);
								continue;
							}
							
							if ( ++numberofservice > ConfigManager.MAX_UINT16 )
							{
								config.errorTracker().append("Number of configured services is greater than allowed maximum(")
								.append(ConfigManager.MAX_UINT16).append("). Some services will be dropped.").create(Severity.ERROR);
								break;
							}
							
							service = DirectoryMsgFactory.createService();
							service.applyHasInfo();
							service.info().action(FilterEntryActions.SET);
							service.info().serviceName().data(serviceName);
							
							XMLnode infoFilterNode = childNode.getChild(ConfigManager.ServiceInfoFilter);
							
							if ( infoFilterNode != null)
							{
								result = readServiceInfoFilter(config, serviceIdSet, unspecifiedIdList, service, infoFilterNode, serviceDictionaryConfigList);
							}
							
							XMLnode stateFilterNode = childNode.getChild(ConfigManager.ServiceStateFilter);
							
							if (result && stateFilterNode != null)
							{
								result = readServiceStateFilter(config, service,stateFilterNode);
							}
							
							if ( result )
							{
								serviceNameSet.add(serviceName);
								
								if ( directoryCache != null )
								{
									addToMap(service);
									directoryCache.addService(service);
								}
							}
						}
					}
				}
				
				if ( ( directoryCache != null) && directoryCache.serviceList().size() == 0 )
				{					
					useDefaultService(config, directoryCache);
				}
				
				if( unspecifiedIdList.size() > 0 )
				{
					int serviceId = 0;
					
					for(int index = 0; index < unspecifiedIdList.size(); ++index )
					{
						while(serviceIdSet.contains(serviceId))
						{
							++serviceId;
						}
						
						if( serviceId > ConfigManager.MAX_UINT16 )
						{
							config.errorTracker().append("EMA ran out of assignable service ids. Will drop rest of the services").create(Severity.ERROR);
							break;
						}
						
						unspecifiedIdList.get(index).serviceId(serviceId);
						serviceIdSet.add(serviceId);
						++serviceId;
					}
				}
			}
			else
			{
				useDefaultService(config, directoryCache);
			}
		}
    }
    
    boolean readServiceInfoFilter(EmaConfigBaseImpl config, Set<Integer> serviceIdSet, List<Service> unspecifiedIdList, Service service, XMLnode infoFilterNode,
    		List<ServiceDictionaryConfig> serviceDictionaryConfigList)
	{
    	ServiceDictionaryConfig serviceDictionaryConfig = null;
		ConfigAttributes infoAttributes =  infoFilterNode.attributeList();
		
		ConfigElement element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterServiceId);
		
		if (element != null)
		{
			int serviceId = element.intLongValue();
			
			if (serviceId > ConfigManager.MAX_UINT16)
			{
				config.errorTracker().append("service[").append(service.info().serviceName().toString())
				.append("] specifies out of range ServiceId (value of ").append(serviceId)
				.append("). Will drop this service.").create(Severity.ERROR);
				return false;
			}
			
			if (serviceIdSet.contains(serviceId))
			{
				config.errorTracker().append("service[").append(service.info().serviceName().toString())
				.append("] specifies the same ServiceId (value of ").append(serviceId)
				.append(") as already specified by another service. Will drop this service.").create(Severity.ERROR);
				return false;
			}
			
			service.serviceId(serviceId);
			serviceIdSet.add(serviceId);
		}
		else
		{
			unspecifiedIdList.add(service);
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterVendor);
		
		if (element != null)
		{
			service.info().applyHasVendor();
			service.info().vendor().data(element.asciiValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterIsSource);
		
		if (element != null)
		{			
			 service.info().applyHasIsSource();
		     service.info().isSource(element.intLongValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterSupportsQoSRange);
		
		if (element != null)
		{
			 service.info().applyHasSupportsQosRange();
		     service.info().supportsQosRange(element.intLongValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterItemList);
		
		if (element != null)
		{			
			service.info().applyHasItemList();
		    service.info().itemList().data(element.asciiValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterAcceptingConsumerStatus);
		
		if (element != null)
		{
		     service.info().applyHasAcceptingConsumerStatus();
		     service.info().acceptingConsumerStatus(element.intLongValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterSupportsOutOfBandSnapshots);
		
		if (element != null)
		{
			service.info().applyHasSupportsOutOfBandSnapshots();
	        service.info().supportsOutOfBandSnapshots(element.intLongValue());
		}
		
		for(int i = 0; i < infoFilterNode.children().size() ; i++ )
		{
			XMLnode node = infoFilterNode.children().get(i);
			
			List<ConfigElement> configElementList;
			
			if ( node.tagId() == ConfigManager.ServiceInfoFilterCapabilities )
			{
				Integer domainTypeInt;
				
				configElementList =  node.attributeList().getConfigElementList(ConfigManager.ServiceInfoFilterCapabilitiesCapabilitiesEntry);
				
				for(int index = 0 ; index < configElementList.size(); ++index )
				{
					element = configElementList.get(index);
					
					if (element != null)
					{
						domainTypeInt = ConfigManager.convertDomainType(element.asciiValue());
						
						if ( domainTypeInt != null )
						{
							if ( domainTypeInt > ConfigManager.MAX_UINT16 )
							{
								config.errorTracker().append("specified service [")
								.append(service.info().serviceName().toString()).append("] contains out of range capability = ")
								.append(domainTypeInt).append(". Will drop this capability.").create(Severity.ERROR);
								continue;
							}
							
							service.info().capabilitiesList().add(domainTypeInt.longValue());
						}
						else
						{
							config.errorTracker().append("failed to read or convert a capability from the specified service [")
							.append(service.info().serviceName().toString())
							.append("]. Will drop this capability. Its value is = ").append(element.asciiValue()).create(Severity.ERROR);
						}
					}
				}
			}
					
			if ( (serviceDictionaryConfigList != null ) && (serviceDictionaryConfig == null) )
			{
				serviceDictionaryConfig = new ServiceDictionaryConfig();
				serviceDictionaryConfig.serviceId = service.serviceId();
				serviceDictionaryConfigList.add(serviceDictionaryConfig);
			}
			
		    readServiceInfoFilterDictionary(config, service, node, ConfigManager.ServiceInfoFilterDictionariesProvided, 
		    		ConfigManager.ServiceInfoFilterDictionariesProvidedDictionariesProvidedEntry, serviceDictionaryConfig);
		    
		    readServiceInfoFilterDictionary(config, service, node, ConfigManager.ServiceInfoFilterDictionariesUsed, 
		    		ConfigManager.ServiceInfoFilterDictionariesUsedDictionariesUsedEntry, serviceDictionaryConfig);
		   
		    if ( node.tagId() == ConfigManager.ServiceInfoFilterQoS )
		    {
		    	Long timeliness;
		    	Long rate;
		    
		    	if ( node.children().size() == 0 )
		    	{
		    		config.errorTracker().append("no configuration QoSEntry exists for service QoS [")
					.append(service.info().serviceName().toString())
					.append("|InfoFilter|QoS]. Will use default QoS.").create(Severity.WARNING);
		    		
		    		Qos qos = CodecFactory.createQos();
					Utilities.toRsslQos(OmmQos.Rate.TICK_BY_TICK, OmmQos.Timeliness.REALTIME, qos);
					service.info().applyHasQos();
					service.info().qosList().add(qos);
		    	}
		    	else
		    	{
			    	for(int index = 0; index < node.children().size() ; index++ )
					{
			    		timeliness = new Long(OmmQos.Timeliness.REALTIME);
				    	rate = new Long(OmmQos.Rate.TICK_BY_TICK);
			    		
						XMLnode childNode = node.children().get(index);
						
						ConfigAttributes qosAttributes = childNode.attributeList();
						
						element = (ConfigElement) qosAttributes.getElement(ConfigManager.ServiceInfoFilterQoSEntryTimeliness);
						
						if (element != null)
						{
							timeliness = ConfigManager.convertQosTimeliness(element.asciiValue());
							
							if( timeliness == null )
							{
								config.errorTracker().append("failed to read or convert a QoS Timeliness from the specified service [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Timeliness]. Will use default Timeliness.")
								.append(" Suspect Timeliness value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								timeliness = new Long(OmmQos.Timeliness.REALTIME);
							}
							else if ( timeliness > Integer.MAX_VALUE )
							{
								config.errorTracker().append("specified service QoS::Timeliness [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Timeliness] is greater than allowed maximum. Will use maximum Timeliness.")
								.append(" Suspect Timeliness value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								timeliness = new Long(OmmQos.Timeliness.INEXACT_DELAYED);
							}
						}
						else
						{
							config.errorTracker().append("no configuration exists for service QoS Timeliness [")
							.append(service.info().serviceName().toString())
							.append("|InfoFilter|QoS|QoSEntry|Timeliness]. Will use default Timeliness.").create(Severity.WARNING);
						}
						
						element = (ConfigElement) qosAttributes.getElement(ConfigManager.ServiceInfoFilterQoSEntryRate);
						
						if (element != null)
						{
							rate = ConfigManager.convertQosRate(element.asciiValue());
							
							if( rate == null )
							{
								config.errorTracker().append("failed to read or convert a QoS Rate from the specified service [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Rate]. Will use default Rate.")
								.append(" Suspect Rate value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								rate = new Long(OmmQos.Rate.TICK_BY_TICK);
							}
							else if ( rate > Integer.MAX_VALUE )
							{
								config.errorTracker().append("specified service QoS::Rate [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Rate] is greater than allowed maximum. Will use maximum Rate.")
								.append(" Suspect Rate value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								rate = new Long(OmmQos.Rate.JUST_IN_TIME_CONFLATED);
							}
						}
						else
						{
							config.errorTracker().append("no configuration exists for service QoS Rate [")
							.append(service.info().serviceName().toString())
							.append("|InfoFilter|QoS|QoSEntry|Timeliness]. Will use default Rate").create(Severity.WARNING);
						}
						
						Qos qos = CodecFactory.createQos();
						Utilities.toRsslQos(rate.intValue(), timeliness.intValue(), qos);
						service.info().applyHasQos();
						service.info().qosList().add(qos);
					}
		    	}
		    }
		}
		
		if ( service.info().capabilitiesList().size() == 0 )
		{
			config.errorTracker().append("specified service [")
			.append(service.info().serviceName().toString())
			.append("] contains no capabilities. Will drop this service.").create(Severity.ERROR);
			return false;
		}
		
		return true;
	}
	
	void readServiceInfoFilterDictionary(EmaConfigBaseImpl config, Service service, XMLnode node, int nodeId, int entryId,
			ServiceDictionaryConfig serviceDictionaryConfig)
	{
		String dictionaryName = null;
		List<ConfigElement> configElementList;
		ConfigElement element;
		String rdmEnumTypeItemName;
		String rdmFieldDictItemName;
		String rdmEnumTypeDefFileName = null;
		String rdmFieldDictionaryFileName = null;
		
		if ( node.tagId() == nodeId )
		{
			configElementList =  node.attributeList().getConfigElementList(entryId);
			
			for(int index = 0 ; index < configElementList.size(); ++index )
			{
				dictionaryName = configElementList.get(index).asciiValue();
				
				ConfigAttributes dictionaryAttributes = config.xmlConfig().getDictionaryAttributes(dictionaryName);
				
				if ( dictionaryAttributes != null)
				{
					element = (ConfigElement) dictionaryAttributes.getElement(ConfigManager.DictionaryEnumTypeDefItemName);
					
					if ( element == null || ( rdmEnumTypeItemName = element.asciiValue()).isEmpty() )
					{
						rdmEnumTypeItemName = DictionaryCallbackClient.DICTIONARY_RWFENUM;
						
						config.errorTracker().append("no configuration exists or unspecified name for EnumTypeDefItemName in dictionary [")
						.append(dictionaryName).append("]. Will use default value of ").append(rdmEnumTypeItemName)
						.create(Severity.WARNING);
					}
		
					element = (ConfigElement) dictionaryAttributes.getElement(ConfigManager.DictionaryRdmFieldDictionaryItemName);
					
					if ( element == null || ( rdmFieldDictItemName = element.asciiValue()).isEmpty() )
					{
						rdmFieldDictItemName = DictionaryCallbackClient.DICTIONARY_RWFFID;
						
						config.errorTracker().append("no configuration exists or unspecified name for RdmFieldDictionaryItemName in dictionary [")
						.append(dictionaryName).append("]. Will use default value of ").append(rdmFieldDictItemName)
						.create(Severity.WARNING);
					}
					
					element = (ConfigElement) dictionaryAttributes.getElement(ConfigManager.DictionaryEnumTypeDefFileName);
					
					if ( element == null || ( rdmEnumTypeDefFileName = element.asciiValue()).isEmpty() )
					{
						rdmEnumTypeDefFileName = "./enumtype.def";
						
						config.errorTracker().append("no configuration exists or unspecified name for EnumTypeDefFileName in dictionary [")
						.append(dictionaryName).append("]. Will use default value of ").append(rdmEnumTypeDefFileName)
						.create(Severity.WARNING);
					}
					
					element = (ConfigElement) dictionaryAttributes.getElement(ConfigManager.DictionaryRDMFieldDictFileName);
					
					if ( element == null || ( rdmFieldDictionaryFileName = element.asciiValue()).isEmpty() )
					{
						rdmFieldDictionaryFileName = "./RDMFieldDictionary";
						
						config.errorTracker().append("no configuration exists or unspecified name for RdmFieldDictionaryFileName in dictionary [")
						.append(dictionaryName).append("]. Will use default value of ").append(rdmFieldDictionaryFileName)
						.create(Severity.WARNING);
					}
				}
				else
				{
					config.errorTracker().append("no configuration exists for dictionary [")
					.append(dictionaryName).append("]. Will use dictionary defaults").create(Severity.WARNING);
					
					rdmEnumTypeItemName = DictionaryCallbackClient.DICTIONARY_RWFENUM;
					rdmFieldDictItemName = DictionaryCallbackClient.DICTIONARY_RWFFID;
					rdmEnumTypeDefFileName = "./enumtype.def";
					rdmFieldDictionaryFileName = "./RDMFieldDictionary";
				}
				
				if ( nodeId == ConfigManager.ServiceInfoFilterDictionariesProvided )
				{
					service.info().applyHasDictionariesProvided();
					service.info().dictionariesProvidedList().add(rdmEnumTypeItemName);
					service.info().dictionariesProvidedList().add(rdmFieldDictItemName);
					
					if( serviceDictionaryConfig != null )
					{
						DictionaryConfig dictionaryConfig = new DictionaryConfig(true);
						dictionaryConfig.dictionaryName = dictionaryName;
						dictionaryConfig.enumTypeDefItemName = rdmEnumTypeItemName;
						dictionaryConfig.rdmFieldDictionaryItemName = rdmFieldDictItemName;
						dictionaryConfig.enumtypeDefFileName = rdmEnumTypeDefFileName;
						dictionaryConfig.rdmfieldDictionaryFileName = rdmFieldDictionaryFileName;
						serviceDictionaryConfig.dictionaryProvidedList.add(dictionaryConfig);
					}
				}
				else if ( nodeId == ConfigManager.ServiceInfoFilterDictionariesUsed )
				{
					service.info().applyHasDictionariesUsed();
					service.info().dictionariesUsedList().add(rdmEnumTypeItemName);
					service.info().dictionariesUsedList().add(rdmFieldDictItemName);
					
					if( serviceDictionaryConfig != null )
					{
						DictionaryConfig dictionaryConfig = new DictionaryConfig(true);
						dictionaryConfig.dictionaryName = dictionaryName;
						dictionaryConfig.enumTypeDefItemName = rdmEnumTypeItemName;
						dictionaryConfig.rdmFieldDictionaryItemName = rdmFieldDictItemName;
						dictionaryConfig.enumtypeDefFileName = rdmEnumTypeDefFileName;
						dictionaryConfig.rdmfieldDictionaryFileName = rdmFieldDictionaryFileName;
						serviceDictionaryConfig.dictionaryUsedList.add(dictionaryConfig);
					}
				}
			}
		}
	}
	
	boolean readServiceStateFilter(EmaConfigBaseImpl config, Service service, XMLnode stateFilterNode)
	{
		ConfigAttributes stateAttributes =  stateFilterNode.attributeList();
		
		ConfigElement element = (ConfigElement) stateAttributes.getElement(ConfigManager.ServiceStateFilterServiceState);
		
		if (element != null)
		{
			service.applyHasState();
		    service.state().action(FilterEntryActions.SET);
			service.state().serviceState(element.intLongValue());
		}
		
		element = (ConfigElement) stateAttributes.getElement(ConfigManager.ServiceStateFilterAcceptingRequests);
		
		if (element != null)
		{
			service.state().applyHasAcceptingRequests();
			service.state().acceptingRequests(element.intLongValue());
		}
		
		for(int j = 0; j < stateFilterNode.children().size() ; j++ )
		{
			XMLnode node = stateFilterNode.children().get(j);
			
			ConfigAttributes statusAttributes =  node.attributeList();
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusStreamState);
			
			if (element != null)
			{			
				service.state().applyHasStatus();
	            service.state().status().streamState(element.intValue());
			}
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusDataState);
			
			if (element != null)
			{
				service.state().applyHasStatus();
				service.state().status().dataState(element.intValue());
			}
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusStatusCode);
			
			if (element != null)
			{
				service.state().applyHasStatus();
	            service.state().status().code(element.intValue());
			}
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusStatusText);
			
			if (element != null)
			{			
				service.state().applyHasStatus();
	            service.state().status().text().data(element.asciiValue());
			}
		}
		
		return true;
	}
    
    void useDefaultService(EmaConfigBaseImpl config, DirectoryCache directoryCache)
	{
    	if ( _providerRole == OmmProviderConfig.ProviderRole.NON_INTERACTIVE)
		{
			config.errorTracker().append("no configuration exists for ni provider directory [")
			.append(_activeConfig.instanceName).append("]. Will use directory defaults.").create(Severity.WARNING);
		}
		else
		{
			config.errorTracker().append("no configuration exists for interactive provider directory [")
			.append(_activeConfig.instanceName).append("]. Will use directory defaults.").create(Severity.WARNING);
		}
    	
		Service service = DirectoryMsgFactory.createService();
		populateDefaultService(service);
		addToMap(service);
		directoryCache.addService(service);
	}
    
    void populateDefaultService(Service service)
	{
    	_bUsingDefaultService = true;
    	
		service.clear();
		
		if ( _providerRole == OmmProviderConfig.ProviderRole.NON_INTERACTIVE)
		{
			service.info().serviceName().data(OmmNiProviderActiveConfig.DEFAULT_SERVICE_NAME);
			service.serviceId(OmmNiProviderActiveConfig.DEFAULT_SERVICE_ID);
			
	        service.state().applyHasAcceptingRequests();
	        service.state().acceptingRequests(OmmNiProviderActiveConfig.DEFAULT_ACCEPTING_REQUESTS);
	        
	        service.info().applyHasIsSource();
	        service.info().isSource(OmmNiProviderActiveConfig.DEFAULT_SERVICE_IS_SOURCE);
	        
	        service.info().applyHasSupportsQosRange();
	        service.info().supportsQosRange(OmmNiProviderActiveConfig.DEFAULT_SERVICE_SUPPORTS_QOS_RANGE);
		}
		else if ( _providerRole == OmmProviderConfig.ProviderRole.INTERACTIVE)
		{
			service.info().serviceName().data(OmmIProviderActiveConfig.DEFAULT_SERVICE_NAME);
			service.serviceId(OmmIProviderActiveConfig.DEFAULT_SERVICE_ID);
			
	        service.state().applyHasAcceptingRequests();
	        service.state().acceptingRequests(OmmIProviderActiveConfig.DEFAULT_ACCEPTING_REQUESTS);
	        
	        service.info().applyHasIsSource();
	        service.info().isSource(OmmIProviderActiveConfig.DEFAULT_SERVICE_IS_SOURCE);
	        
	        service.info().applyHasSupportsQosRange();
	        service.info().supportsQosRange(OmmIProviderActiveConfig.DEFAULT_SERVICE_SUPPORTS_QOS_RANGE);
	        
	        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_DICTIONARY);	        
	        
	        service.info().applyHasDictionariesProvided();
	        service.info().dictionariesProvidedList().add(DictionaryCallbackClient.DICTIONARY_RWFFID);
	        service.info().dictionariesProvidedList().add(DictionaryCallbackClient.DICTIONARY_RWFENUM);
		}
		
		service.action(MapEntryActions.ADD);
		
		service.applyHasInfo();
		service.info().action(FilterEntryActions.SET);

		service.info().applyHasVendor();
		service.info().vendor().data("");
      
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE);
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_BY_ORDER);
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_BY_PRICE);
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_MAKER);

        service.info().applyHasQos();
        Qos qos = CodecFactory.createQos();
        qos.rate(QosRates.TICK_BY_TICK);
        qos.timeliness(QosTimeliness.REALTIME);
        service.info().qosList().add(qos);

        service.info().applyHasDictionariesUsed();
        service.info().dictionariesUsedList().add(DictionaryCallbackClient.DICTIONARY_RWFFID);
        service.info().dictionariesUsedList().add(DictionaryCallbackClient.DICTIONARY_RWFENUM);
        
        service.info().applyHasItemList();
        service.info().itemList().data("");

        service.info().applyHasAcceptingConsumerStatus();
        service.info().acceptingConsumerStatus(OmmNiProviderActiveConfig.DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE);
        
        service.info().applyHasSupportsOutOfBandSnapshots();
        service.info().supportsOutOfBandSnapshots(OmmNiProviderActiveConfig.DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS);
  
        service.applyHasState();
        service.state().action(FilterEntryActions.SET);
        service.state().serviceState(OmmNiProviderActiveConfig.DEFAULT_SERVICE_STATE);
	}
    
    boolean decodeSourceDirectory(com.thomsonreuters.upa.codec.Msg rsslMsg, StringBuilder errorText)
	{
		int retCode = CodecReturnCodes.SUCCESS;
		DecodeIterator decodeIt = CodecFactory.createDecodeIterator();
		decodeIt.clear();
		
		Buffer inputBuffer = rsslMsg.encodedDataBody();
		
		retCode = decodeIt.setBufferAndRWFVersion(inputBuffer, Codec.majorVersion(), Codec.minorVersion());
		
		if( retCode !=  CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to set decode iterator buffer and version in OmmNiProviderImpl.decodeSourceDirectory(). Reason = ")
			.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
		
		com.thomsonreuters.upa.codec.Map map = CodecFactory.createMap();
		map.clear();
		
		if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
		{
			_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, "Begin decoding of SourceDirectory.", Severity.TRACE) );
		}
		
		retCode = map.decode(decodeIt);
		
		if( retCode <  CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to decode Map in OmmNiProviderImpl.decodeSourceDirectory(). Reason = ")
			.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
		else if ( retCode == CodecReturnCodes.NO_DATA )
		{
			if ( _ommCommonImpl.loggerClient().isWarnEnabled() )
			{
				_ommCommonImpl.loggerClient().warn( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, "Passed in SourceDirectory map contains no entries"
						+ " (e.g. there is no service specified).", Severity.WARNING) );
			}
			
			if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
			{
				_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, "End decoding of SourceDirectory.", Severity.TRACE) );
			}
			
			return true;
		}
		
		switch( map.keyPrimitiveType())
		{
		case com.thomsonreuters.upa.codec.DataTypes.UINT:
			if( !decodeSourceDirectoryKeyUInt(map, decodeIt, errorText) )
				return false;
			break;
		case com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING:
		{
			errorText.append("Attempt to specify SourceDirectory info with a Map using key DataType of ")
			.append( DataType.asString(Utilities.toEmaDataType[map.keyPrimitiveType()]))
			.append(" while the expected key DataType is ")
			.append( DataType.asString(DataType.DataTypes.UINT));
			
			if ( _ommCommonImpl.loggerClient().isErrorEnabled() )
			{
				_ommCommonImpl.loggerClient().error( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, errorText.toString(), Severity.ERROR) );
			}
			
			return false;
		}
		default:
			errorText.append("Attempt to specify SourceDirectory info with a Map using key DataType of  ")
			.append( DataType.asString(Utilities.toEmaDataType[map.keyPrimitiveType()]))
			.append(" while the expected key DataType is ")
			.append( DataType.asString(DataType.DataTypes.UINT) + " or " + DataType.asString(DataType.DataTypes.ASCII) );
			return false;
		}
		
		if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
		{
			_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, "End decoding of SourceDirectory.", Severity.TRACE) );
		}
		
		return true;
	}
    
    boolean decodeSourceDirectoryKeyUInt(com.thomsonreuters.upa.codec.Map map, DecodeIterator decodeIt, StringBuilder errorText)
	{
		int retCode = CodecReturnCodes.SUCCESS;
		com.thomsonreuters.upa.codec.UInt serviceId = CodecFactory.createUInt();
		com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
		StringBuilder text = new StringBuilder();
		com.thomsonreuters.upa.codec.FilterList filterList = CodecFactory.createFilterList();
		com.thomsonreuters.upa.codec.FilterEntry filterEntry = CodecFactory.createFilterEntry();
		com.thomsonreuters.upa.codec.ElementList elementList = CodecFactory.createElementList();
		com.thomsonreuters.upa.codec.ElementEntry elementEntry = CodecFactory.createElementEntry();
		
		while ( ( retCode = mapEntry.decode(decodeIt, serviceId) ) != CodecReturnCodes.END_OF_CONTAINER )
		{
			if ( retCode != CodecReturnCodes.SUCCESS )
			{
				errorText.append( "Internal error: Failed to Decode Map Entry. Reason = " )
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			text.setLength(0);
			text.append( "Begin decoding of Service with id of " );
			text.append( serviceId ).append(". Action= ");
			switch ( mapEntry.action() )
			{
			case com.thomsonreuters.upa.codec.MapEntryActions.UPDATE:
				text.append("Upate");
				break;
			case com.thomsonreuters.upa.codec.MapEntryActions.ADD:
				text.append("Add");
				break;
			case com.thomsonreuters.upa.codec.MapEntryActions.DELETE:
				text.append("Delete");
				break;
			}
			
			if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
			{
				_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
			}
			
			if ( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.DELETE )
			{
				String serviceName = serviceName(serviceId.toBigInteger().intValue());
				
				if ( serviceName != null )
				{
					remove(serviceId.toBigInteger().intValue());
				}
				
				text.setLength(0);
				text.append("End decoding of Service with id of ").append(serviceId);
				if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
				{
					_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				continue;
			}
			else if ( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.ADD )
			{
				if ( checkExistingServiceId((int)serviceId.toLong(), errorText) == false )
				{
					return false;
				}
			}
			
			if( map.containerType() != com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST )
			{
				errorText.append( "Attempt to specify Service with a container of " )
				.append(DataType.asString(Utilities.toEmaDataType[map.containerType()]))
				.append("  rather than the expected  ").append( DataType.asString(DataTypes.FILTER_LIST));
				return false;
			}
			
			filterList.clear();
			filterEntry.clear();
			
			retCode = filterList.decode(decodeIt);
			
			if ( retCode < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Decode FilterList. Reason")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			else if ( retCode == CodecReturnCodes.NO_DATA )
			{
				text.setLength(0);
				text.append("Service with id of ").append(serviceId)
				.append(" contains no FilterEntries. Skipping this service.");
				
				if ( _ommCommonImpl.loggerClient().isWarnEnabled() )
				{
					_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.WARNING) );
				}
				
				text.setLength(0);
				text.append("End decoding of Service with id of ").append(serviceId);
				
				if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
				{
					_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				continue;
			}
			
			while ( ( retCode = filterEntry.decode(decodeIt) ) != CodecReturnCodes.END_OF_CONTAINER )
			{	
				if ( retCode < CodecReturnCodes.SUCCESS )
				{
					errorText.append("Internal error: Failed to Decode Filter Entry. Reason = ");
					errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
					return false;
				}
				
				text.setLength(0);
				text.append("Begin decoding of FilterEntry with id of ").append(filterEntry.id());
				
				if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
				{
					_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				if ( filterEntry.id() == com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds.INFO )
				{
					if( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.UPDATE )
					{
						errorText.append("Attempt to update Infofilter of service with id of ").append(serviceId)
						.append(" while this is not allowed.");
						return false;
					}
					
					if ( filterEntry.checkHasContainerType() && ( filterEntry.containerType() != com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )
							&& filterList.containerType() != com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )
					{
						int containerType = filterEntry.checkHasContainerType() ? filterEntry.containerType() : filterList.containerType();
						errorText.append("Attempt to specify Service InfoFilter with a container of ");
						errorText.append(DataType.asString(Utilities.toEmaDataType[containerType]));
						errorText.append(" rather than the expected ").append(DataType.asString(DataTypes.ELEMENT_LIST));
						return false;
					}
					
					elementList.clear();
					elementEntry.clear();
					
					if ( ( retCode = elementList.decode(decodeIt, null) ) < CodecReturnCodes.SUCCESS )
					{
						errorText.append("Internal error: Failed to Decode Element List. Reason = ");
						errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
						return false;
					}
					
					boolean bServiceNameEntryFound = false;
					
					while ( ( retCode = elementEntry.decode(decodeIt)) != CodecReturnCodes.END_OF_CONTAINER )
					{
						if ( retCode < CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Decode ElementEntry. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						
						text.setLength(0);
						text.append("Decoding of ElementEntry with name of ");
						text.append(elementEntry.name().toString());
						
						if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
						{
							_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
						}
						
						if( !bServiceNameEntryFound && elementEntry.name().equals(com.thomsonreuters.upa.rdm.ElementNames.NAME) )
						{
							if ( elementEntry.dataType() != com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING )
							{
								errorText.append("Attempt to specify Service Name with a ")
								.append( DataType.asString(Utilities.toEmaDataType[elementEntry.dataType()]) )
								.append(" rather than the expected ").append( DataType.asString(DataTypes.ASCII));
								return false;
							}
							
							Buffer serviceNameBuffer = CodecFactory.createBuffer();
							serviceNameBuffer.clear();
							
							retCode = serviceNameBuffer.decode(decodeIt);
							if( retCode < CodecReturnCodes.SUCCESS)
							{
								errorText.append("Internal error: Failed to Decode Buffer. Reason = ");
								errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
								return false;
							}
							else if ( retCode == CodecReturnCodes.BLANK_DATA )
							{
								errorText.append("Attempt to specify Service Name with a blank ascii string for service id of ");
								errorText.append(serviceId);
								return false;
							}
							
							bServiceNameEntryFound = true;
							
							if ( addServiceIdAndNamePair(serviceId.toBigInteger().intValue(), serviceNameBuffer.toString(), errorText ) == false )
							{
								return false;
							}
						}
					}
					
					if( !bServiceNameEntryFound )
					{
						errorText.append("Attempt to specify service InfoFilter without required Service Name for service id of ")
						.append(serviceId);
						return false;
					}
				}
			}
			
			text.setLength(0);
			text.append("End decoding of FilterEntry with id of ");
			text.append(filterEntry.id());
			
			if ( _ommCommonImpl.loggerClient().isTraceEnabled() )
			{
				_ommCommonImpl.loggerClient().trace( _ommCommonImpl.formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
			}
		}
		
		return true;
	}
    
    boolean submitSourceDirectory(ClientSession clientSession, com.thomsonreuters.upa.codec.Msg msg, StringBuilder errorText, boolean storeUserSubmitted)
    {
    	if ( _userStoreDecodeIt == null)
    	{
    		_userStoreDecodeIt = CodecFactory.createDecodeIterator();
    	}
    	else
    	{
    		_userStoreDecodeIt.clear();
    	}
    	
    	int retCode = _userStoreDecodeIt.setBufferAndRWFVersion(msg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
    	
    	if( retCode !=  CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to set decode iterator buffer and version in DirectoryServiceStore.submitSourceDirectory(). Reason = ")
			.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
   
    	if ( msg.msgClass() == MsgClasses.REFRESH )
    	{
    		_submittedDirectoryMsg.clear();
    		_submittedDirectoryMsg.rdmMsgType(DirectoryMsgType.REFRESH);
    	}
    	else if ( msg.msgClass() == MsgClasses.UPDATE )
    	{
    		_submittedDirectoryMsg.clear();
    		_submittedDirectoryMsg.rdmMsgType(DirectoryMsgType.UPDATE);
    	}
    	
    	retCode = _submittedDirectoryMsg.decode(_userStoreDecodeIt, msg);
    	
		if ( retCode < CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error: Failed to decode Source Directory message in DirectoryServiceStore.submitSourceDirectory(). Reason")
			.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
		
		List<Service> serviceList = null;
		
		switch (_submittedDirectoryMsg.rdmMsgType())
		{
			case REFRESH:
			{
				DirectoryRefresh directoryRefresh = (DirectoryRefresh)_submittedDirectoryMsg;
				serviceList = directoryRefresh.serviceList();
			}
				break;
			case UPDATE:
			{
				DirectoryUpdate directoryUpdate = (DirectoryUpdate)_submittedDirectoryMsg;
				serviceList = directoryUpdate.serviceList();
			}
				break;
			default:
			{
				errorText.append("Received unexpected message type ").append(_submittedDirectoryMsg.rdmMsgType().toString()).append(".");
				return false;
			}
		}
		
		if(!storeUserSubmitted)
		{
			Service submittedService;
			
			for(int index = 0; index < serviceList.size(); index++ )
			{	
				submittedService = serviceList.get(index);
				
				switch(submittedService.action())
				{
				case MapEntryActions.ADD:
					{
						if ( submittedService.checkHasState() )
						{
							notifyOnServiceStateChange(clientSession, submittedService);
						}
						
						notifyOnServiceGroupChange(clientSession, submittedService);
					}
					break;
				case MapEntryActions.DELETE:
					{
						notifyOnServiceDelete(clientSession, submittedService);
					}
					break;
				case MapEntryActions.UPDATE:
					{
						if ( submittedService.checkHasState() )
						{
							notifyOnServiceStateChange(clientSession, submittedService);
						}
						
						notifyOnServiceGroupChange(clientSession, submittedService);
					}
					break;
				}
			}
			return true;
		}
		else
		{
			Service submittedService;
									
			for(int index = 0; index < serviceList.size(); index++ )
			{	
				submittedService = serviceList.get(index);
				
				switch(submittedService.action())
				{
				case MapEntryActions.ADD:
					{
						Service cacheService = _directoryCache.getService(submittedService.serviceId());
						
						if ( cacheService == null)
						{
							cacheService = DirectoryMsgFactory.createService();
							if ( !applyService(cacheService, submittedService, clientSession, errorText) )
							{
								return false;
							}
							
							_directoryCache.addService(cacheService);
						}
					}
					break;
				case MapEntryActions.DELETE:
					{
						_directoryCache.removeService(submittedService.serviceId());
						
						notifyOnServiceDelete(clientSession, submittedService);
					}
					break;
				case MapEntryActions.UPDATE:
				{
					Service cacheService = _directoryCache.getService(submittedService.serviceId());
					
					if ( cacheService != null)
					{
						if ( !applyService( cacheService, submittedService, clientSession, errorText) )
						{
							return false;
						}
					}
				}
				break;
				}
			}
		}
		
    	return true;
    }
    
    boolean applyService(Service cacheService, Service submittedService, ClientSession clientSession,  StringBuilder errorText)
    {
    	int retCode = CodecReturnCodes.SUCCESS;
    	int flags;

    	if ( submittedService.checkHasState() )
    	{
    		if ( submittedService.state().action() == FilterEntryActions.UPDATE)
    		{
    			flags = cacheService.state().flags() | submittedService.state().flags();
    			retCode = submittedService.state().update(cacheService.state());
    			cacheService.state().flags(flags);
    		}
    		else
    		{
    			retCode = submittedService.state().update(cacheService.state());
    		}
			
			if ( retCode < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Update State filter in DirectoryServiceStroe.applyService() for Service Id = ")
				.append(submittedService.serviceId()).append(OmmLoggerClient.CR).append("Reason = ")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			cacheService.applyHasState();
			
			notifyOnServiceStateChange(clientSession, submittedService);
		}
		
		if ( submittedService.checkHasInfo() )
		{
			if ( submittedService.info().action() == FilterEntryActions.UPDATE)
    		{
				flags = cacheService.info().flags() | submittedService.info().flags();
				retCode = submittedService.info().update(cacheService.info());
				cacheService.info().flags(flags);
    		}
			else
			{
				retCode = submittedService.info().update(cacheService.info());
			}
				
			if ( retCode < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Update Info filter in DirectoryServiceStroe.applyService() for Service Id = ")
				.append(submittedService.serviceId()).append(OmmLoggerClient.CR).append("Reason = ")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			cacheService.applyHasInfo();
		}
		
		if ( submittedService.checkHasLoad() )
		{
			if ( submittedService.load().action() == FilterEntryActions.UPDATE)
    		{
				flags = cacheService.load().flags() | submittedService.load().flags();
				retCode = submittedService.load().update(cacheService.load());
				cacheService.load().flags(flags);
    		}
			else
			{
				retCode = submittedService.load().update(cacheService.load());
			}
				
			if ( retCode < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Update Load filter in DirectoryServiceStroe.applyService() for Service Id = ")
				.append(submittedService.serviceId()).append(OmmLoggerClient.CR).append("Reason = ")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			cacheService.applyHasLoad();
		}
		
		if ( submittedService.checkHasLink() )
		{
			if ( submittedService.link().action() == FilterEntryActions.UPDATE)
    		{
				cacheService.link().action(FilterEntryActions.UPDATE);
				
				ServiceLink submittedLink, cacheLink = null;
				
				for(int submittedIndex = 0; submittedIndex < submittedService.link().linkList().size(); submittedIndex++)
				{
					submittedLink = submittedService.link().linkList().get(submittedIndex);
					boolean foundLink = false;
					
					for(int cacheIndex = 0; cacheIndex < cacheService.link().linkList().size(); cacheIndex++)
					{
						cacheLink = cacheService.link().linkList().get(cacheIndex);
						
						if ( submittedLink.name().equals(cacheLink.name()))
						{
							foundLink = true;
							break;
						}
					}
					
					if(foundLink)
					{
						if (submittedLink.checkHasCode())
			            {
						  cacheLink.applyHasCode();
						  cacheLink.linkCode(submittedLink.linkCode());
			            }

			            if (submittedLink.checkHasText())
			            {
			            	cacheLink.applyHasText();
			                ByteBuffer byteBuffer = ByteBuffer.allocate(submittedLink.text().length());
			                retCode = submittedLink.text().copy(byteBuffer);
			                cacheLink.text().data(byteBuffer);
			            }

			            if (submittedLink.checkHasType())
			            {
			            	cacheLink.applyHasType();
			            	cacheLink.type(submittedLink.type());
			            }

			            cacheLink.linkState(submittedLink.linkState());
					}
					else
					{
						cacheLink = new Service.ServiceLink();
						retCode = submittedLink.copy(cacheLink);
						cacheService.link().linkList().add(cacheLink);
					}
				}
    		}
			else
			{
				retCode = submittedService.link().update(cacheService.link());
			}
				
			if ( retCode < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Update Link filter in DirectoryServiceStroe.applyService() for Service Id = ")
				.append(submittedService.serviceId()).append(OmmLoggerClient.CR).append("Reason = ")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			cacheService.applyHasLink();
		}
		
		notifyOnServiceGroupChange(clientSession, submittedService);
		
    	return true;
    }
    
    DirectoryCache getDirectoryCache()
    {
    	return _directoryCache;
    }
    
    static long applyDirectoryService(long directoryServiceFilter, boolean initialResp, Service service, Service respService)
    {
    	long filter = 0;
    	
    	if ( service.checkHasInfo() && ( (directoryServiceFilter & Directory.ServiceFilterFlags.INFO) == Directory.ServiceFilterFlags.INFO ) )
    	{
    		respService.applyHasInfo();
    		respService.info(service.info());
    		
    		if ( initialResp )
    		{
    			respService.info().action(FilterEntryActions.SET);
    		}
    		
    		filter |= directoryServiceFilter & Directory.ServiceFilterFlags.INFO;
    	}
    	
    	if ( service.checkHasState() && ( (directoryServiceFilter & Directory.ServiceFilterFlags.STATE) == Directory.ServiceFilterFlags.STATE ) )
    	{
    		respService.applyHasState();
    		respService.state(service.state());
    		
    		if ( initialResp )
    		{
    			respService.state().action(FilterEntryActions.SET);
    		}
    		
    		filter |= directoryServiceFilter & Directory.ServiceFilterFlags.STATE;
    	}
    	
    	if ( service.checkHasLoad() && ( (directoryServiceFilter & Directory.ServiceFilterFlags.LOAD) == Directory.ServiceFilterFlags.LOAD ) )
    	{
    		respService.applyHasLoad();
    		respService.load(service.load());
    		
    		if ( initialResp )
    		{
    			respService.load().action(FilterEntryActions.SET);
    		}
    		
    		filter |= directoryServiceFilter & Directory.ServiceFilterFlags.LOAD;
    	}
    	
    	if ( service.checkHasLink() && ( (directoryServiceFilter & Directory.ServiceFilterFlags.LINK) == Directory.ServiceFilterFlags.LINK ) )
    	{
    		respService.applyHasLink();
    		respService.link(service.link());
    		
    		if ( initialResp )
    		{
    			respService.link().action(FilterEntryActions.SET);
    		}
    		
    		filter |= directoryServiceFilter & Directory.ServiceFilterFlags.LINK;
    	}
    	
    	if ( service.checkHasData() && ( (directoryServiceFilter & Directory.ServiceFilterFlags.DATA) == Directory.ServiceFilterFlags.DATA ) )
    	{
    		respService.applyHasData();
    		respService.data(service.data());
    		
    		if ( initialResp )
    		{
    			respService.data().action(FilterEntryActions.SET);
    		}
    		
    		filter |= directoryServiceFilter & Directory.ServiceFilterFlags.DATA;
    	}
    	
    	if ( ( service.groupStateList().size() != 0 ) && ( (directoryServiceFilter & Directory.ServiceFilterFlags.GROUP) == Directory.ServiceFilterFlags.GROUP ) )
    	{
    		respService.groupStateList(service.groupStateList());
    		
    		filter |= directoryServiceFilter & Directory.ServiceFilterFlags.GROUP;
    	}
    	
    	if (filter != 0)
    	{
    		if ( initialResp )
    		{
    			respService.action(MapEntryActions.ADD);
    		}
    		else
    		{
    			respService.action(service.action());
    		}
    		
    		respService.serviceId(service.serviceId());
    	}
    
    	return filter;
    }
    
    static long encodeDirectoryMsg(List<Service> inputServiceList, List<Service> outputServiceList, long directoryServiceFilter, boolean initialResp, 
    		boolean specifiedServiceId, int serviceId)
    {
    	_cacheLock.lock();
    	
    	Service service = null;
    	Service respService = null;
    	long filters = 0;
    	
    	
    	if( specifiedServiceId)
    	{
    		for(int index = 0; index < inputServiceList.size(); index ++)
    		{
    			if ( inputServiceList.get(index).serviceId() == serviceId )
    			{
    				service = inputServiceList.get(index);
    				break;
    			}
    		}
    		
    		if ( service == null )
    		{
    			outputServiceList.clear();
    			return filters;
    		}
    	}
    	
    	if(service != null)
    	{
    		if ( service.action() == MapEntry.MapAction.DELETE )
			{
    			respService = getService();
	    		respService.clear();
	    		
	    		service.copy(respService);
    			
				outputServiceList.add(respService);
			}
			else
			{
	    		respService = getService();
	    		respService.clear();
	    		
	    		filters = applyDirectoryService(directoryServiceFilter, initialResp, service, respService);
	    		
	    		if ( filters != 0 )
	    		{
	    			outputServiceList.add(respService);
	    		}
	    		else
	    		{
	    			 returnService(respService);
	    		}
			}
    	}
    	else
    	{
    		long applyFilters = 0;
    		
    		for(int index = 0; index < inputServiceList.size(); index++)
    		{
    			if ( inputServiceList.get(index).action() == MapEntry.MapAction.DELETE )
    			{
    				respService = getService();
    	    		respService.clear();
    	    		
    	    		inputServiceList.get(index).copy(respService);
        			
    				outputServiceList.add(respService);
    			}
    			else
    			{
    				respService = getService();
        			respService.clear();
    				
	    			applyFilters = applyDirectoryService(directoryServiceFilter, initialResp, inputServiceList.get(index), respService);
	    			
	    			if( applyFilters != 0)
	    			{
	    				filters |= applyFilters;
	    			
	    				outputServiceList.add(respService);
	    			}
	    			else
	        		{
	        			 returnService(respService);
	        		}
    			}
    		}
    	}
    	
    	_cacheLock.unlock();
    	
		return filters;
    }
    
    static void returnServiceToPool(List<Service> serviceList)
    {
    	_cacheLock.lock();
    	
    	for(int index = 0; index < serviceList.size(); index++)
		{
    		returnService(serviceList.get(index));
		}
    	
    	serviceList.clear();
    	
    	_cacheLock.unlock();
    }
    
    static DirectoryRefresh getDirectoryRefreshMsg(DirectoryCache directoryCache, boolean clearCache)
    {
    	_cacheLock.lock();
    	
    	Service service;
    	DirectoryRefresh directoryRefresh = directoryCache.getDirectoryRefresh();
   
    	if ( clearCache )
    	{
    		directoryRefresh.applyClearCache();
    	}
    	
    	int filters = 0;
    	
    	List<Service> serviceList = directoryCache.serviceList();
    	for (int index = 0; index < serviceList.size(); index++)
		{
    		service = serviceList.get(index);
    		if ( service.checkHasInfo() )
    			filters |= com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO;
    		
    		if( service.checkHasState() )
    			filters |= com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.STATE;
    		
    		if( service.checkHasData() )
    			filters |= com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LOAD;
    		
    		if( service.checkHasLink() )
    			filters |= com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.LINK;
		}
    	
    	directoryRefresh.filter(filters);
  
	    _cacheLock.unlock();
    	
    	return directoryRefresh;
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
        
        ServiceIdInteger value(int value)
        {
            _value = value;
            return this;
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

        ServiceIdInteger clear()
        {
            _value = 0;
            return this;
        }
    }
}

class OmmIProviderDirectoryStore extends DirectoryServiceStore
{
	OmmIProviderDirectoryStore(EmaObjectManager objectManager, OmmIProviderImpl ommIProviderImpl, OmmIProviderActiveConfig ommIProviderActiveConfig)
	{
		super(objectManager, ommIProviderImpl.providerRole() , ommIProviderImpl, ommIProviderActiveConfig);
		
		_ommIProviderImpl = ommIProviderImpl;
		_ommIProviderActiveConfig = ommIProviderActiveConfig;
	}
	
	boolean isAcceptingRequests(int serviceId)
	{
		_ommIProviderImpl.userLock().lock();
		
		boolean acceptingRequests = true;
		
		Service service = _directoryCache.getService(serviceId);
		
		if ( service != null )
		{
			if ( service.checkHasState() )
			{
				if ( service.state().checkHasAcceptingRequests() )
				{
					if ( service.state().acceptingRequests() == 0 )
					{
						acceptingRequests = false;
					}
				}
			}
		}
		else
		{
			acceptingRequests = false;
		}
		
		_ommIProviderImpl.userLock().unlock();
		
		return acceptingRequests;
	}
	
	boolean isValidQosRange(int serviceId, RequestMsg requestMsg)
	{
		_ommIProviderImpl.userLock().lock();
		
		boolean result = false;
		
		Service service = _directoryCache.getService(serviceId);
		
		if ( service != null )
		{
			if ( service.checkHasInfo() )
			{
				if ( requestMsg.checkHasQos() && requestMsg.checkHasWorstQos() )
				{
					if ( service.info().checkHasSupportsQosRange() )
					{
						if ( service.info().checkHasQos() )
						{
							List<Qos> qosList = service.info().qosList();
							
							for(int index = 0; index < qosList.size(); index++ )
							{
								if ( qosList.get(index).isInRange(requestMsg.qos(), requestMsg.worstQos()))
								{
									result = true;
									break;
								}
							}
						}
						else
						{
							 Qos qos = CodecFactory.createQos();
							 qos.rate(QosRates.TICK_BY_TICK);
							 qos.timeliness(QosTimeliness.REALTIME);
							 
							 if ( qos.isInRange(requestMsg.qos(), requestMsg.worstQos()))
							 {
								result = true;
							 }
						}
					}
				}
				else if ( requestMsg.checkHasQos() )
				{
					if ( service.info().checkHasQos() )
					{
						List<Qos> qosList = service.info().qosList();
						
						for(int index = 0; index < qosList.size(); index++ )
						{
							if ( qosList.get(index).equals(requestMsg.qos()) )
							{
								result = true;
								break;
							}
						}
					}
					else
					{
						 Qos qos = CodecFactory.createQos();
						 qos.rate(QosRates.TICK_BY_TICK);
						 qos.timeliness(QosTimeliness.REALTIME);
						 
						 if ( qos.equals(requestMsg.qos()) )
						 {
							result = true;
						 }
					}
				}
				else
				{
					Qos qos = CodecFactory.createQos();
					qos.rate(QosRates.TICK_BY_TICK);
					qos.timeliness(QosTimeliness.REALTIME);
					
					if ( service.info().checkHasQos() )
					{
						List<Qos> qosList = service.info().qosList();
						
						for(int index = 0; index < qosList.size(); index++ )
						{
							if ( qosList.get(index).equals(qos) )
							{
								result = true;
								break;
							}
						}
					}
					else
					{
						result = true;
					}
				}
			}
			else
			{
				result = true;
			}
		}
		
		_ommIProviderImpl.userLock().unlock();
		
		return result;
	}
	
	void loadConfigDirectory(EmaConfigBaseImpl configBaseImpl)
	{
		DirectoryCache directoryCache = null;
		List<ServiceDictionaryConfig> serviceDictionaryConfigList = null;
		
		if ( _ommIProviderActiveConfig.directoryAdminControl == OmmIProviderConfig.AdminControl.API_CONTROL)
		{
			directoryCache = _directoryCache;
		}
		
		if ( _ommIProviderActiveConfig.dictionaryAdminControl == OmmIProviderConfig.AdminControl.API_CONTROL)
		{
			serviceDictionaryConfigList = new ArrayList<>();
		}
		
		if ( (directoryCache == null) && (serviceDictionaryConfigList == null) )
		{
			return;
		}
		
		super.loadConfigDirectory(directoryCache, configBaseImpl, serviceDictionaryConfigList);
		
		if (serviceDictionaryConfigList != null)
		{
			if(_bUsingDefaultService )
			{
				ServiceDictionaryConfig serviceDictionaryConfig = new ServiceDictionaryConfig();
				serviceDictionaryConfig.serviceId = 1;

				DictionaryConfig dictionaryProvided = new DictionaryConfig(true);
				dictionaryProvided.rdmfieldDictionaryFileName = "./RDMFieldDictionary";
				dictionaryProvided.rdmFieldDictionaryItemName = "RWFFld";
				dictionaryProvided.enumtypeDefFileName = "./enumtype.def";
				dictionaryProvided.enumTypeDefItemName = "RWFEnum";
				serviceDictionaryConfig.dictionaryProvidedList.add(dictionaryProvided);
				serviceDictionaryConfigList.add(serviceDictionaryConfig);
			}
		
			_ommIProviderActiveConfig.setServiceDictionaryConfigCollection(serviceDictionaryConfigList);
		
			serviceDictionaryConfigList.clear();
		}
	}

	@Override
	boolean checkExistingServiceId(int serviceId, StringBuilder errorText)
	{
		if ( _ommIProviderActiveConfig.directoryAdminControl == OmmIProviderConfig.AdminControl.API_CONTROL)
		{
			String serviceName = _servicesIdAndNameTable.get(_tempServiceIdInteger.value(serviceId));
			
			if(serviceName != null)
			{
				errorText.setLength(0);
				errorText.append("Attempt to add a service with name of ");
				errorText.append(serviceName).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
				return false;
			}
		}
		
		return true;
	}

	@Override
	boolean addServiceIdAndNamePair(int serviceId, String serviceName, StringBuilder errorText)
	{
		if ( _ommIProviderActiveConfig.directoryAdminControl == OmmIProviderConfig.AdminControl.API_CONTROL)
		{
			if ( _servicesNameAndIdTable.get(serviceName) != null )
			{
				if ( errorText != null )
				{
					errorText.setLength(0);
					errorText.append("Attempt to add a service with name of ");
					errorText.append(serviceName).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
				}
				
				return false;
			}
			
			addToMap(serviceId, serviceName);
		}
		else
		{
			if ( _servicesNameAndIdTable.get(serviceName) == null )
			{
				addToMap(serviceId, serviceName);
			}
		}
		
		if ( (errorText != null) && _ommIProviderImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder text = _ommIProviderImpl.strBuilder();
			text.append("Detected Service with name of ").append(serviceName).append(" and Id of ").append(serviceId);
			_ommIProviderImpl.loggerClient().trace( _ommIProviderImpl.formatLogMessage(_ommIProviderActiveConfig.instanceName, text.toString(), Severity.TRACE) );
		}
		
		return true;
	}
	
	private OmmIProviderImpl			_ommIProviderImpl;
	private OmmIProviderActiveConfig	_ommIProviderActiveConfig;
}

class OmmNiProviderDirectoryStore extends DirectoryServiceStore
{
	OmmNiProviderDirectoryStore(EmaObjectManager objectManager, OmmNiProviderImpl ommNiProviderImpl, OmmNiProviderActiveConfig ommNiProviderActiveConfig)
	{
		super(objectManager, ommNiProviderImpl.providerRole() , ommNiProviderImpl, ommNiProviderActiveConfig);
		
		_ommNiProviderImpl = ommNiProviderImpl;
		_ommNiProviderActiveConfig = ommNiProviderActiveConfig;
		_directoryCacheApiControl = null;
	}
	
	void loadConfigDirectory(EmaConfigBaseImpl configBaseImpl)
	{
		if ( _ommNiProviderActiveConfig.directoryAdminControl == OmmNiProviderConfig.AdminControl.API_CONTROL )
		{
			_directoryCacheApiControl = new DirectoryCache();
			
			super.loadConfigDirectory(_directoryCacheApiControl, configBaseImpl, null);
		}
	}

	@Override
	protected boolean checkExistingServiceId(int serviceId, StringBuilder errorText)
	{
		String serviceName = _servicesIdAndNameTable.get(_tempServiceIdInteger.value(serviceId));
		
		if(serviceName != null)
		{
			errorText.setLength(0);
			errorText.append("Attempt to add a service with name of ");
			errorText.append(serviceName).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
			return false;
		}
		
		return true;
	}

	@Override
	protected boolean addServiceIdAndNamePair(int serviceId, String serviceName, StringBuilder errorText)
	{
		if ( _servicesNameAndIdTable.get(serviceName) != null )
		{
			if ( errorText != null )
			{
				errorText.setLength(0);
				errorText.append("Attempt to add a service with name of ");
				errorText.append(serviceName).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
			}
			
			return false;
		}
		
		addToMap(serviceId, serviceName);
		
		if ( (errorText != null) && _ommNiProviderImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder text = _ommNiProviderImpl.strBuilder();
			text.append("Detected Service with name of ").append(serviceName).append(" and Id of ").append(serviceId);
			_ommNiProviderImpl.loggerClient().trace( _ommNiProviderImpl.formatLogMessage(_ommNiProviderActiveConfig.instanceName, text.toString(), Severity.TRACE) );
		}
		
		return true;
	}
	
	DirectoryCache getApiControlDirectory()
	{
		return _directoryCacheApiControl;
	}
	
	private OmmNiProviderImpl			_ommNiProviderImpl;
	private OmmNiProviderActiveConfig 	_ommNiProviderActiveConfig;
	private DirectoryCache				_directoryCacheApiControl;
}

