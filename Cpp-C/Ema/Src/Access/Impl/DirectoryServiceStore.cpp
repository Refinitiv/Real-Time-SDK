/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright Thomson Reuters 2016. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "DirectoryServiceStore.h"
#include "OmmBaseImplMap.h"
#include "EmaConfigImpl.h"
#include "EmaRdm.h"
#include "OmmQosDecoder.h"
#include "OmmIProviderImpl.h"
#include "OmmNiProviderImpl.h"
#include "ExceptionTranslator.h"

#include <ctype.h>
#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

#define DEFAULT_SERVICE_ID										0
#define DEFAULT_SERVICE_IS_SOURCE								0
#define DEFAULT_SERVICE_SUPPORTS_QOS_RANGE						1
#define DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS			1
#define DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE				1
#define DEFAULT_SERVICE_STATE									1
#define DEFAULT_ACCEPTING_REQUESTS								1
#define DEFAULT_IS_STATUS_CONFIGURED							false
#define DEFAULT_LINK_TYPE										1
#define DEFAULT_LINK_CODE										0

InfoFilter::InfoFilter() :
	serviceName(),
	vendorName(),
	isSource(DEFAULT_SERVICE_IS_SOURCE),
	capabilities(),
	dictionariesProvided(),
	dictionariesUsed(),
	qos(),
	supportsQosRange(DEFAULT_SERVICE_SUPPORTS_QOS_RANGE),
	itemList(),
	supportsOutOfBandSnapshots(DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS),
	acceptingConsumerStatus(DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE),
	action(RSSL_FTEA_CLEAR_ENTRY),
	flags(0)
{
}

InfoFilter::~InfoFilter()
{
}

InfoFilter::InfoFilter(const InfoFilter& other) :
	serviceName(other.serviceName),
	vendorName(other.vendorName),
	isSource(other.isSource),
	capabilities(other.capabilities),
	dictionariesProvided(other.dictionariesProvided),
	dictionariesUsed(other.dictionariesUsed),
	qos(other.qos),
	supportsQosRange(other.supportsQosRange),
	itemList(other.itemList),
	supportsOutOfBandSnapshots(other.supportsOutOfBandSnapshots),
	acceptingConsumerStatus(other.acceptingConsumerStatus),
	action(other.action),
	flags(other.flags)
{
}

InfoFilter& InfoFilter::operator=(const InfoFilter& other)
{
	if (&other == this) return *this;

	serviceName = other.serviceName;
	vendorName = other.vendorName;
	isSource = other.isSource;
	capabilities = other.capabilities;
	dictionariesProvided = other.dictionariesProvided;
	dictionariesUsed = other.dictionariesUsed;
	qos = other.qos;
	supportsQosRange = other.supportsQosRange;
	itemList = other.itemList;
	supportsOutOfBandSnapshots = other.supportsOutOfBandSnapshots;
	acceptingConsumerStatus = other.acceptingConsumerStatus;
	action = other.action;
	flags = other.flags;

	return *this;
}

void InfoFilter::clear()
{
	serviceName.clear();
	vendorName.clear();
	isSource = DEFAULT_SERVICE_IS_SOURCE;
	capabilities.clear();
	dictionariesProvided.clear();
	dictionariesUsed.clear();
	qos.clear();
	supportsQosRange = DEFAULT_SERVICE_SUPPORTS_QOS_RANGE;
	itemList.clear();
	supportsOutOfBandSnapshots = DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS;
	acceptingConsumerStatus = DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE;
	action = RSSL_FTEA_CLEAR_ENTRY;
	flags = 0;
}

void InfoFilter::apply(RsslRDMServiceInfo& rsslRDMServiceInfo)
{
	action = rsslRDMServiceInfo.action;

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS)
	{
		flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
		acceptingConsumerStatus = rsslRDMServiceInfo.acceptingConsumerStatus;
	}

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_IS_SOURCE)
	{
		flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
		isSource = rsslRDMServiceInfo.isSource;
	}

	for (UInt32 i = 0; i < rsslRDMServiceInfo.capabilitiesCount; ++i)
	{
		capabilities.push_back(*(rsslRDMServiceInfo.capabilitiesList++));
	}

	for (UInt32 i = 0; i < rsslRDMServiceInfo.dictionariesProvidedCount; ++i)
	{
		flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

		dictionariesProvided.push_back(EmaString(rsslRDMServiceInfo.dictionariesProvidedList->data, rsslRDMServiceInfo.dictionariesProvidedList->length));
		rsslRDMServiceInfo.dictionariesProvidedList++;
	}

	for (UInt32 i = 0; i < rsslRDMServiceInfo.dictionariesUsedCount; ++i)
	{
		flags |= RDM_SVC_IFF_HAS_DICTS_USED;

		dictionariesUsed.push_back(EmaString(rsslRDMServiceInfo.dictionariesUsedList->data, rsslRDMServiceInfo.dictionariesUsedList->length));
		rsslRDMServiceInfo.dictionariesUsedList++;
	}

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_ITEM_LIST)
	{
		flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
		itemList.set(rsslRDMServiceInfo.itemList.data, rsslRDMServiceInfo.itemList.length);
	}

	serviceName.set(rsslRDMServiceInfo.serviceName.data, rsslRDMServiceInfo.serviceName.length);

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_QOS)
	{
		flags |= RDM_SVC_IFF_HAS_QOS;
		for (UInt32 i = 0; i < rsslRDMServiceInfo.qosCount; ++i)
			qos.push_back(*(rsslRDMServiceInfo.qosList++));
	}

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS)
	{
		flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;

		supportsOutOfBandSnapshots = rsslRDMServiceInfo.supportsOutOfBandSnapshots;
	}

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
	{
		flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;

		supportsQosRange = rsslRDMServiceInfo.supportsQosRange;
	}

	if (rsslRDMServiceInfo.flags & RDM_SVC_IFF_HAS_VENDOR)
	{
		vendorName.set(rsslRDMServiceInfo.vendor.data, rsslRDMServiceInfo.vendor.length);
	}

	serviceName.set(rsslRDMServiceInfo.serviceName.data, rsslRDMServiceInfo.serviceName.length);
}

StateFilter::StateFilter() :
	serviceState(DEFAULT_SERVICE_STATE),
	acceptingRequests(DEFAULT_ACCEPTING_REQUESTS),
	status(),
	statusText(),
	action(RSSL_FTEA_CLEAR_ENTRY),
	flags(0)
{
	rsslClearState(&status);
}

StateFilter::~StateFilter()
{
}

StateFilter::StateFilter(const StateFilter& other) :
	serviceState(other.serviceState),
	acceptingRequests(other.acceptingRequests),
	status(other.status),
	statusText(other.statusText),
	action(other.action),
	flags(other.flags)
{
	status.text.data = (char*)statusText.c_str();
	status.text.length = statusText.length();
}

StateFilter& StateFilter::operator=(const StateFilter& other)
{
	if (&other == this) return *this;

	serviceState = other.serviceState;
	acceptingRequests = other.acceptingRequests;
	status = other.status;
	statusText = other.statusText;
	action = other.action;
	flags = other.flags;

	return *this;
}

void StateFilter::clear()
{
	serviceState = DEFAULT_SERVICE_STATE;
	acceptingRequests = DEFAULT_ACCEPTING_REQUESTS;
	rsslClearState(&status);
	statusText.clear();
	action = RSSL_FTEA_CLEAR_ENTRY;
	flags = 0;
}

void StateFilter::apply(RsslRDMServiceState& rsslRDMServiceState)
{
	action = rsslRDMServiceState.action;

	if (rsslRDMServiceState.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS)
	{
		flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;

		acceptingRequests = rsslRDMServiceState.acceptingRequests;
	}

	serviceState = rsslRDMServiceState.serviceState;

	if (rsslRDMServiceState.flags & RDM_SVC_STF_HAS_STATUS)
	{
		flags |= RDM_SVC_STF_HAS_STATUS;

		status = rsslRDMServiceState.status;
		statusText.set(rsslRDMServiceState.status.text.data, rsslRDMServiceState.status.text.length);
		status.text.data = (char*)statusText.c_str();
		status.text.length = statusText.length();
	}
}

GroupFilter::GroupFilter() :
	groupId(),
	mergedToGroupId(),
	status(),
	statusText(),
	action(RSSL_FTEA_CLEAR_ENTRY),
	flags(0)
{

}

GroupFilter::GroupFilter(const GroupFilter& other)
{
	groupId = other.groupId;
	mergedToGroupId = other.mergedToGroupId;
	status = other.status;
	statusText = other.statusText;
	action = other.action;
	flags = other.flags;
}

GroupFilter& GroupFilter::operator=(const GroupFilter& other)
{
	if (&other == this) return *this;

	groupId = other.groupId;
	mergedToGroupId = other.mergedToGroupId;
	status = other.status;
	statusText = other.statusText;
	action = other.action;
	flags = other.flags;

	return *this;
}

GroupFilter::~GroupFilter()
{
}

void GroupFilter::clear()
{
	groupId.clear();
	mergedToGroupId.clear();
	rsslClearState(&status);
	statusText.clear();
	action = RSSL_FTEA_CLEAR_ENTRY;
	flags = 0;
}

LoadFilter::LoadFilter() :
	openLimit(0),
	openWindow(0),
	loadFactor(0),
	action(RSSL_FTEA_CLEAR_ENTRY),
	flags(0)
{
}

LoadFilter::LoadFilter(const LoadFilter& other)
{
	openLimit = other.openLimit;
	openWindow = other.openWindow;
	loadFactor = other.loadFactor;
	action = other.action;
	flags = other.flags;
}

LoadFilter& LoadFilter::operator=(const LoadFilter& other)
{
	if (&other == this) return *this;

	openLimit = other.openLimit;
	openWindow = other.openWindow;
	loadFactor = other.loadFactor;
	action = other.action;
	flags = other.flags;

	return *this;
}

LoadFilter::~LoadFilter()
{
}

void LoadFilter::clear()
{
	openLimit = 0;
	openWindow = 0;
	loadFactor = 0;
	action = RSSL_FTEA_CLEAR_ENTRY;
	flags = 0;
}

void LoadFilter::apply(RsslRDMServiceLoad& rsslRDMServiceLoad)
{
	action = rsslRDMServiceLoad.action;

	if (rsslRDMServiceLoad.flags &  RDM_SVC_LDF_HAS_OPEN_LIMIT)
	{
		flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;

		openLimit = rsslRDMServiceLoad.openLimit;
	}

	if (rsslRDMServiceLoad.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW)
	{
		flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;

		openWindow = rsslRDMServiceLoad.openWindow;
	}

	if (rsslRDMServiceLoad.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR)
	{
		flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;

		loadFactor = rsslRDMServiceLoad.loadFactor;
	}
}

Link::Link() :
	type(DEFAULT_LINK_TYPE),
	linkState(0),
	linkCode(DEFAULT_LINK_CODE),
	text(),
	name(),
	action(RSSL_MPEA_ADD_ENTRY),
	flags(0)
{
}

Link::Link(const Link& other)
{
	type = other.type;
	linkState = other.linkState;
	linkCode = other.linkCode;
	text = other.text;
	name = other.name;
	action = other.action;
	flags = other.flags;
}

Link& Link::operator=(const Link& other)
{
	type = other.type;
	linkState = other.linkState;
	linkCode = other.linkCode;
	text = other.text;
	name = other.name;
	action = other.action;
	flags = other.flags;
	
	return *this;
}

Link::~Link()
{
}

void Link::clear()
{
	type = DEFAULT_LINK_TYPE;
	linkState = 0;
	linkCode = DEFAULT_LINK_CODE;
	text = "";
	name = "";
	action = RSSL_MPEA_ADD_ENTRY;
	flags = 0;
}

void Link::apply(RsslRDMServiceLink& rsslRdmServiceLink)
{
	action = rsslRdmServiceLink.action;

	name.set(rsslRdmServiceLink.name.data, rsslRdmServiceLink.name.length);

	linkState = rsslRdmServiceLink.linkState;

	if (rsslRdmServiceLink.flags & RDM_SVC_LKF_HAS_TYPE)
	{
		flags |= RDM_SVC_LKF_HAS_TYPE;

		type = rsslRdmServiceLink.type;
	}

	if (rsslRdmServiceLink.flags & RDM_SVC_LKF_HAS_CODE)
	{
		flags |= RDM_SVC_LKF_HAS_CODE;

		linkCode = rsslRdmServiceLink.linkCode;
	}

	if (rsslRdmServiceLink.flags & RDM_SVC_LKF_HAS_TEXT)
	{
		flags |= RDM_SVC_LKF_HAS_TEXT;

		text.set(rsslRdmServiceLink.text.data, rsslRdmServiceLink.text.length);
	}
}

LinkFilter::LinkFilter() :
	action(RSSL_FTEA_CLEAR_ENTRY)
{
}

LinkFilter::~LinkFilter()
{
	clear();
}

void LinkFilter::clear()
{
	Link* link = _linkList.front();
	Link* pTemp;

	while (link)
	{
		pTemp = link;
		link = link->next();
		delete pTemp;
	}

	_linkList.clear();
	action = RSSL_FTEA_CLEAR_ENTRY;
}

void LinkFilter::addLink(const Link& link)
{
	try
	{
		Link* pLink = new Link(link);

		_linkList.push_back(pLink);
	}
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in LinkFilter::addLink()");
	}
}

void LinkFilter::removeLink(const EmaString& linkName)
{
	Link* link = _linkList.front();

	while (link)
	{
		if (link->name == linkName)
		{
			Link* pTemp = link;
			_linkList.remove(link);
			delete pTemp;
			break;
		}

		link = link->next();
	}
}

Link* LinkFilter::getLink(const EmaString& linkName)
{
	Link* link = _linkList.front();

	while (link)
	{
		if (link->name == linkName)
		{
			return link;
		}

		link = link->next();
	}

	return 0;
}

const EmaList< Link* >& LinkFilter::getLinkList() const
{
	return _linkList;
}

Service::Service() :
serviceId(DEFAULT_SERVICE_ID),
infoFilter(),
stateFilter(),
loadFilter(),
linkFilter(),
flags(0)
{
}

Service::~Service()
{
}

void Service::clear()
{
	serviceId = DEFAULT_SERVICE_ID;
	flags = 0;
	infoFilter.clear();
	stateFilter.clear();
	loadFilter.clear();
	linkFilter.clear();
}

Service::Service(const Service& other) :
serviceId(other.serviceId),
infoFilter(other.infoFilter),
stateFilter(other.stateFilter),
loadFilter(other.loadFilter),
linkFilter(other.linkFilter),
flags(other.flags)
{
}

Service& Service::operator=(const Service& other)
{
	if (&other == this) return *this;

	serviceId = other.serviceId;
	infoFilter = other.infoFilter;
	stateFilter = other.stateFilter;
	loadFilter = other.loadFilter;
	linkFilter = other.linkFilter;
	flags = other.flags;

	return *this;
}

DirectoryCache::DirectoryCache() :
directoryName(),
_serviceHash()
{
}

DirectoryCache::~DirectoryCache()
{
	clear();
}

void DirectoryCache::clear()
{
	Service* service = _serviceList.front();
	Service* pTemp;

	while (service)
	{
		pTemp = service;
		service = service->next();
		delete pTemp;
	}

	_serviceList.clear();
	_serviceHash.clear();
	directoryName.clear();
}

const EmaList< Service* >& DirectoryCache::getServiceList() const
{
	return _serviceList;
}

void DirectoryCache::addService(const Service& service)
{
	try
	{
		Service* pService = new Service(service);

		_serviceList.push_back(pService);
		_serviceHash.insert(service.serviceId, pService);
	}
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in DirectoryCache::addService()");
	}
}

void DirectoryCache::removeService(UInt64 serviceId)
{
	Service** servicePtr = _serviceHash.find(serviceId);

	if (servicePtr)
	{
		Service* pTemp = *servicePtr;
		_serviceList.remove(pTemp);
		_serviceHash.erase(serviceId);
		delete pTemp;
	}
}

Service* DirectoryCache::getService(UInt64 serviceId) const
{
	Service** servicePtr = _serviceHash.find(serviceId);

	if (servicePtr)
	{
		return *servicePtr;
	}

	return 0;
}

size_t DirectoryCache::UInt64rHasher::operator()(const UInt64& value) const
{
	return value;
}

bool DirectoryCache::UInt64Equal_To::operator()(const UInt64& x, const UInt64& y) const
{
	return x == y ? true : false;
}

DirectoryServiceStore::DirectoryServiceStore(OmmProviderConfig::ProviderRole providerRole, OmmCommonImpl& ommCommonImpl,
	BaseConfig& baseConfig) :
	_providerRole(providerRole),
	_ommCommonImpl(ommCommonImpl),
	_baseConfig(baseConfig),
	_pDirectoryServiceStoreClient(0),
	_bUsingDefaultService(false)
{
	_serviceNameToServiceId.rehash(baseConfig.serviceCountHint);
	_serviceIdToServiceName.rehash(baseConfig.serviceCountHint);
}

DirectoryServiceStore::~DirectoryServiceStore()
{
	clearServiceNamePair();
}

void DirectoryServiceStore::setClient(DirectoryServiceStoreClient* pDirectoryServiceStoreClient)
{
	_pDirectoryServiceStoreClient = pDirectoryServiceStoreClient;
}

void DirectoryServiceStore::populateDefaultService(Service& service)
{
	_bUsingDefaultService = true;

	service.infoFilter.action = RSSL_FTEA_SET_ENTRY;
	service.stateFilter.action = RSSL_FTEA_SET_ENTRY;

	if (_providerRole == OmmProviderConfig::NonInteractiveEnum)
	{
		service.infoFilter.serviceName = "NI_PUB";
		service.serviceId = 0;
		service.stateFilter.acceptingRequests = 0;
		service.stateFilter.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
		service.infoFilter.dictionariesProvided.clear();
                service.infoFilter.supportsQosRange = 0;
        	service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
	}
	else if (_providerRole == OmmProviderConfig::InteractiveEnum)
	{
		service.infoFilter.serviceName = "DIRECT_FEED";
		service.serviceId = 1;
		service.stateFilter.acceptingRequests = 1;
		service.stateFilter.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
		// Provider Implementation supports Dictionary Domain
		service.infoFilter.capabilities.push_back( MMT_DICTIONARY );
		service.infoFilter.dictionariesProvided.push_back("RWFFld");
		service.infoFilter.dictionariesProvided.push_back("RWFEnum");
		service.infoFilter.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
                service.infoFilter.supportsQosRange = 1;
                service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
	}

	const EmaString* pServiceName = new EmaString(service.infoFilter.serviceName);

	_serviceNameToServiceId.insert(pServiceName, service.serviceId);
	_serviceIdToServiceName.insert(service.serviceId, pServiceName);
	_serviceNameList.push_back(pServiceName);
	
	service.infoFilter.isSource = 0;
	service.infoFilter.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;

	service.infoFilter.acceptingConsumerStatus = 0;
	service.infoFilter.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;

	service.infoFilter.capabilities.push_back( MMT_MARKET_PRICE );
	service.infoFilter.capabilities.push_back( MMT_MARKET_BY_ORDER );
	service.infoFilter.capabilities.push_back( MMT_MARKET_BY_PRICE );
	service.infoFilter.capabilities.push_back( MMT_MARKET_MAKER );

	service.infoFilter.dictionariesUsed.push_back("RWFFld");
	service.infoFilter.dictionariesUsed.push_back("RWFEnum");
	service.infoFilter.flags |= RDM_SVC_IFF_HAS_DICTS_USED;

	RsslQos rsslQos;
	rsslClearQos(&rsslQos);
	rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
	service.infoFilter.qos.push_back(rsslQos);
	service.infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;

	service.infoFilter.supportsOutOfBandSnapshots = 0;
	service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;

	service.infoFilter.vendorName.clear();
	service.infoFilter.itemList.clear();

	service.stateFilter.serviceState = 1;

	service.flags |= RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;
}

void DirectoryServiceStore::loadConfigDirectory(DirectoryCache* pDirectoryCache, EmaConfigBaseImpl *pConfigImpl, EmaList<ServiceDictionaryConfig*>* serviceDictionaryConfigList)
{
	EmaString directoryName;

	pConfigImpl->getDirectoryName(_baseConfig.configuredName, directoryName);

	if (directoryName.empty())
		pConfigImpl->get<EmaString>("DirectoryGroup|DefaultDirectory", directoryName);

	if (directoryName.empty())
		pConfigImpl->get< EmaString >("DirectoryGroup|DirectoryList|Directory|Name", directoryName);

	if (directoryName.empty() && pDirectoryCache)
	{
		Service service;
		populateDefaultService(service);
		pDirectoryCache->addService(service);
	}
	else
	{
		EmaString directoryNodeName("DirectoryGroup|DirectoryList|Directory.");
		directoryNodeName.append(directoryName).append("|");

		EmaString name;
		if (pDirectoryCache && !pConfigImpl->get< EmaString >(directoryNodeName + "Name", name))
		{
			EmaString errorMsg("no configuration exists for ni provider directory [");
			errorMsg.append(directoryNodeName).append("]. Will use directory defaults");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

			Service service;
			populateDefaultService(service);
			addServiceIdAndNamePair(service.serviceId, new EmaString(service.infoFilter.serviceName), 0);
			pDirectoryCache->addService(service);
		}
		else
		{
			if (pDirectoryCache)
			{
				pDirectoryCache->directoryName = directoryName;
			}

			EmaVector< EmaString > serviceNames;
			ServiceDictionaryConfig* serviceDictionaryConfig = 0;

			pConfigImpl->getServiceNames(directoryName, serviceNames);

			if (pDirectoryCache && serviceNames.empty())
			{
				EmaString errorMsg("specified directory [");
				errorMsg.append(directoryNodeName).append("] contains no services. Will use directory defaults");
				pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

				Service service;
				populateDefaultService(service);
				addServiceIdAndNamePair(service.serviceId, new EmaString(service.infoFilter.serviceName), 0);
				pDirectoryCache->addService(service);
			}
			else
			{
				const UInt16 maxUInt16 = 0xFFFF;

				if (serviceNames.size() > maxUInt16)
				{
					EmaString errorMsg("Number of configured services is greater than allowed maximum. Some services will be dropped.");
					pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
				}

				EmaVector< UInt16 > validConfiguredServiceIds;
				for (UInt32 idx = 0; idx < serviceNames.size() && idx < maxUInt16; ++idx)
				{
					UInt64 tempUInt64 = 0;
					EmaString serviceNodeName(directoryNodeName + "Service." + serviceNames[idx] + "|");
					if (pConfigImpl->get< UInt64 >(serviceNodeName + "InfoFilter|ServiceId", tempUInt64))
						if (tempUInt64 <= maxUInt16)
						{
							if (-1 == validConfiguredServiceIds.getPositionOf((UInt16)tempUInt64))
								validConfiguredServiceIds.push_back((UInt16)tempUInt64);
						}
				}

				EmaVector< UInt64 > usedServiceIds;
				UInt32 emaAssignedServiceId = 0;
				for (UInt32 idx = 0; idx < serviceNames.size() && idx < maxUInt16; ++idx)
				{
					Service service;

					service.infoFilter.serviceName = serviceNames[idx];

					EmaString serviceNodeName(directoryNodeName + "Service." + serviceNames[idx] + "|");

					UInt64 tempUInt64 = 0;
					if (pConfigImpl->get< UInt64 >(serviceNodeName + "StateFilter|ServiceState", tempUInt64))
						service.stateFilter.serviceState = tempUInt64 > 0 ? 1 : 0;
					else
						service.stateFilter.serviceState = 1;

					if (pConfigImpl->get< UInt64 >(serviceNodeName + "StateFilter|AcceptingRequests", tempUInt64))
						service.stateFilter.acceptingRequests = tempUInt64 > 0 ? 1 : 0;
					else
						service.stateFilter.acceptingRequests = 1;

					service.stateFilter.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;

					service.stateFilter.flags &= ~RDM_SVC_STF_HAS_STATUS;

					OmmState::StreamState tempStreamState;
					if (pConfigImpl->get< OmmState::StreamState >(serviceNodeName + "StateFilter|Status|StreamState", tempStreamState))
					{
						service.stateFilter.status.streamState = tempStreamState;
						service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
					}
					else
						service.stateFilter.status.streamState = OmmState::OpenEnum;

					OmmState::DataState tempDataState;
					if (pConfigImpl->get< OmmState::DataState >(serviceNodeName + "StateFilter|Status|DataState", tempDataState))
					{
						service.stateFilter.status.dataState = tempDataState;
						service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
					}
					else
						service.stateFilter.status.dataState = OmmState::OkEnum;

					OmmState::StatusCode tempStatusCode;
					if (pConfigImpl->get< OmmState::StatusCode >(serviceNodeName + "StateFilter|Status|StatusCode", tempStatusCode))
					{
						service.stateFilter.status.code = tempStatusCode;
						service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
					}
					else
						service.stateFilter.status.code = OmmState::NoneEnum;

					if (pConfigImpl->get< EmaString >(serviceNodeName + "StateFilter|Status|StatusText", service.stateFilter.statusText))
					{
						service.stateFilter.status.text.data = (char*)service.stateFilter.statusText.c_str();
						service.stateFilter.status.text.length = service.stateFilter.statusText.length();
						service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
					}
					else
						rsslClearBuffer(&service.stateFilter.status.text);

					if (pConfigImpl->get< UInt64 >(serviceNodeName + "InfoFilter|ServiceId", tempUInt64))
					{
						if (tempUInt64 > maxUInt16)
						{
							EmaString errorMsg("service [");
							errorMsg.append(serviceNodeName).append("] specifies out of range ServiceId (value of ").append(tempUInt64).append("). Will drop this service.");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
							continue;
						}

						service.serviceId = (UInt16)tempUInt64;
						if (usedServiceIds.getPositionOf(service.serviceId) > -1)
						{
							EmaString errorMsg("service [");
							errorMsg.append(serviceNodeName).append("] specifies the same ServiceId (value of ").append(tempUInt64).append(") as already specified by another service. Will drop this service.");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
							continue;
						}

						usedServiceIds.push_back(service.serviceId);
					}
					else
					{
						while (emaAssignedServiceId <= maxUInt16 &&
							validConfiguredServiceIds.getPositionOf(emaAssignedServiceId) > -1)
							++emaAssignedServiceId;

						if (emaAssignedServiceId > maxUInt16)
						{
							EmaString errorMsg("EMA ran out of assignable service ids. Will drop rest of the services");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
							break;
						}

						EmaString errorMsg("service [");
						errorMsg.append(serviceNodeName).append("] contains no ServiceId. EMA will assign a value of ").append(emaAssignedServiceId);
						pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

						service.serviceId = (UInt16)emaAssignedServiceId;
						usedServiceIds.push_back(service.serviceId);
					}

					if (!pConfigImpl->get< EmaString >(serviceNodeName + "InfoFilter|Vendor", service.infoFilter.vendorName))
					{
						service.infoFilter.vendorName.clear();
					}
					else
					{
						service.infoFilter.flags |= RDM_SVC_IFF_HAS_VENDOR;
					}

					if (!pConfigImpl->get< EmaString >(serviceNodeName + "InfoFilter|ItemList", service.infoFilter.itemList))
					{
						service.infoFilter.itemList.clear();
					}
					else
					{
						service.infoFilter.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
					}

					if (pConfigImpl->get< UInt64 >(serviceNodeName + "InfoFilter|IsSource", tempUInt64))
						service.infoFilter.isSource = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.isSource = 0;
					
					service.infoFilter.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;

					if (pConfigImpl->get< UInt64 >(serviceNodeName + "InfoFilter|SupportsQoSRange", tempUInt64))
						service.infoFilter.supportsQosRange = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.supportsQosRange = 0;

					service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;

					if (pConfigImpl->get< UInt64 >(serviceNodeName + "InfoFilter|SupportsOutOfBandSnapshots", tempUInt64))
						service.infoFilter.supportsOutOfBandSnapshots = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.supportsOutOfBandSnapshots = 0;

					service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;

					if (pConfigImpl->get< UInt64 >(serviceNodeName + "InfoFilter|AcceptingConsumerStatus", tempUInt64))
						service.infoFilter.acceptingConsumerStatus = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.acceptingConsumerStatus = 0;

					service.infoFilter.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;

					EmaVector< EmaString > valueList;
					pConfigImpl->getAsciiAttributeValueList(serviceNodeName + "InfoFilter|DictionariesProvided", "DictionariesProvidedEntry", valueList);

					for (UInt32 idx = 0; idx < valueList.size(); ++idx)
					{
						EmaString dictionaryNodeName("DictionaryGroup|DictionaryList|Dictionary.");
						dictionaryNodeName.append(valueList[idx]).append("|");

						EmaString name;
						if (!pConfigImpl->get< EmaString >(dictionaryNodeName + "Name", name))
						{
							EmaString errorMsg("no configuration exists for dictionary [");
							errorMsg.append(dictionaryNodeName).append("]. Will use dictionary defaults");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
						}

						EmaString rdmFieldDictionaryItemName;
						if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "RdmFieldDictionaryItemName", rdmFieldDictionaryItemName))
						{
							rdmFieldDictionaryItemName.set("RWFFld");

							EmaString errorMsg("no configuration exists for RdmFieldDictionaryItemName in dictionary [");
							errorMsg.append(dictionaryNodeName).append("]. Will use default value of RWFFld");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
						}

						service.infoFilter.dictionariesProvided.push_back(rdmFieldDictionaryItemName);

						EmaString enumTypeItemName;
						if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "EnumTypeDefItemName", enumTypeItemName))
						{
							enumTypeItemName.set("RWFEnum");

							EmaString errorMsg("no configuration exists for EnumTypeDefItemName in dictionary [");
							errorMsg.append(dictionaryNodeName).append("]. Will use default value of RWFEnum");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
						}

						service.infoFilter.dictionariesProvided.push_back(enumTypeItemName);

						if (serviceDictionaryConfigList)
						{
							if ( !serviceDictionaryConfig )
								serviceDictionaryConfig = new ServiceDictionaryConfig();

							serviceDictionaryConfig->serviceId = service.serviceId;
							DictionaryConfig* dictionaryConfig = new DictionaryConfig();

							dictionaryConfig->dictionaryName = name;
							dictionaryConfig->rdmFieldDictionaryItemName = rdmFieldDictionaryItemName;
							dictionaryConfig->enumTypeDefItemName = enumTypeItemName;

							EmaString rdmFieldDictionaryFileName;
							if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "RdmFieldDictionaryFileName", rdmFieldDictionaryFileName))
							{
								rdmFieldDictionaryFileName.set("./RDMFieldDictionary");

								EmaString errorMsg("no configuration exists for RdmFieldDictionaryFileName in dictionary [");
								errorMsg.append(dictionaryNodeName).append("]. Will use default value of ./RDMFieldDictionary");
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
							}

							dictionaryConfig->rdmfieldDictionaryFileName = rdmFieldDictionaryFileName;

							EmaString enumTypeDefFileName;
							if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "EnumTypeDefFileName", enumTypeDefFileName))
							{
								enumTypeDefFileName.set("./enumtype.def");

								EmaString errorMsg("no configuration exists for EnumTypeDefFileName in dictionary [");
								errorMsg.append(dictionaryNodeName).append("]. Will use default value of ./enumtype.def");
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
							}

							dictionaryConfig->enumtypeDefFileName = enumTypeDefFileName;

							serviceDictionaryConfig->addDictionaryProvided(dictionaryConfig);
						}
					}

					service.infoFilter.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;

					pConfigImpl->getAsciiAttributeValueList(serviceNodeName + "InfoFilter|DictionariesUsed", "DictionariesUsedEntry", valueList);
			
					for (UInt32 idx = 0; idx < valueList.size(); ++idx)
					{
						EmaString dictionaryNodeName("DictionaryGroup|DictionaryList|Dictionary.");
						dictionaryNodeName.append(valueList[idx]).append("|");

						EmaString name;
						if (!pConfigImpl->get< EmaString >(dictionaryNodeName + "Name", name))
						{
							EmaString errorMsg("no configuration exists for consumer dictionary [");
							errorMsg.append(dictionaryNodeName).append("]. Will use dictionary defaults");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
						}

						EmaString rdmFieldDictionaryItemName;
						if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "RdmFieldDictionaryItemName", rdmFieldDictionaryItemName))
						{
							rdmFieldDictionaryItemName.set("RWFFld");

							EmaString errorMsg("no configuration exists for RdmFieldDictionaryItemName in dictionary [");
							errorMsg.append(dictionaryNodeName).append("]. Will use default value of RWFFld");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
						}

						service.infoFilter.dictionariesUsed.push_back(rdmFieldDictionaryItemName);

						EmaString enumTypeItemName;
						if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "EnumTypeDefItemName", enumTypeItemName))
						{
							enumTypeItemName.set("RWFEnum");

							EmaString errorMsg("no configuration exists for EnumTypeDefItemName in dictionary [");
							errorMsg.append(dictionaryNodeName).append("]. Will use default value of RWFEnum");
							pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
						}

						service.infoFilter.dictionariesUsed.push_back(enumTypeItemName);

						if (serviceDictionaryConfigList)
						{
							if (!serviceDictionaryConfig)
								serviceDictionaryConfig = new ServiceDictionaryConfig();

							serviceDictionaryConfig->serviceId = service.serviceId;
							DictionaryConfig* dictionaryConfig = new DictionaryConfig();

							dictionaryConfig->dictionaryName = name;
							dictionaryConfig->rdmFieldDictionaryItemName = rdmFieldDictionaryItemName;
							dictionaryConfig->enumTypeDefItemName = enumTypeItemName;

							EmaString rdmFieldDictionaryFileName;
							if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "RdmFieldDictionaryFileName", rdmFieldDictionaryFileName))
							{
								rdmFieldDictionaryFileName.set("./RDMFieldDictionary");

								EmaString errorMsg("no configuration exists for RdmFieldDictionaryFileName in dictionary [");
								errorMsg.append(dictionaryNodeName).append("]. Will use default value of ./RDMFieldDictionary");
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
							}

							dictionaryConfig->rdmfieldDictionaryFileName = rdmFieldDictionaryFileName;

							EmaString enumTypeDefFileName;
							if (!pConfigImpl->get<EmaString>(dictionaryNodeName + "EnumTypeDefFileName", enumTypeDefFileName))
							{
								enumTypeDefFileName.set("./enumtype.def");

								EmaString errorMsg("no configuration exists for EnumTypeDefFileName in dictionary [");
								errorMsg.append(dictionaryNodeName).append("]. Will use default value of ./enumtype.def");
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
							}

							dictionaryConfig->enumtypeDefFileName = enumTypeDefFileName;

							serviceDictionaryConfig->addDictionaryUsed(dictionaryConfig);
						}
					}

					if (serviceDictionaryConfigList)
					{
						serviceDictionaryConfigList->push_back(serviceDictionaryConfig);
						serviceDictionaryConfig = 0;
					}

					service.infoFilter.flags |= RDM_SVC_IFF_HAS_DICTS_USED;

					pConfigImpl->getAsciiAttributeValueList(serviceNodeName + "InfoFilter|Capabilities", "CapabilitiesEntry", valueList);

					for (UInt32 idx = 0; idx < valueList.size(); ++idx)
					{
						if (isdigit(valueList[idx].c_str()[0]))
						{
							UInt64 domainType;
							if (sscanf(valueList[idx].c_str(), "%llu", &domainType) == 1)
							{
								if (domainType > maxUInt16)
								{
									EmaString errorMsg("specified service [");
									errorMsg.append(serviceNodeName).append("] contains out of range capability = ").append(domainType).append(". Will drop this capability.");
									pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
									continue;
								}
								else
								{
									if (service.infoFilter.capabilities.getPositionOf((UInt16)domainType) == -1)
										service.infoFilter.capabilities.push_back((UInt16)domainType);
								}
							}
							else
							{
								EmaString errorMsg("failed to read or convert a capability from the specified service [");
								errorMsg.append(serviceNodeName).append("]. Will drop this capability. Its value is = ").append(valueList[idx]);
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
								continue;
							}
						}
						else
						{
							static struct
							{
								const char* configInput;
								UInt16 convertedValue;
							} converter[] =
							{
								{ "MMT_LOGIN", MMT_LOGIN },
								{ "MMT_DIRECTORY", MMT_DIRECTORY },
								{ "MMT_DICTIONARY", MMT_DICTIONARY },
								{ "MMT_MARKET_PRICE", MMT_MARKET_PRICE },
								{ "MMT_MARKET_BY_ORDER", MMT_MARKET_BY_ORDER },
								{ "MMT_MARKET_BY_PRICE", MMT_MARKET_BY_PRICE },
								{ "MMT_MARKET_MAKER", MMT_MARKET_MAKER },
								{ "MMT_SYMBOL_LIST", MMT_SYMBOL_LIST },
								{ "MMT_SERVICE_PROVIDER_STATUS", MMT_SERVICE_PROVIDER_STATUS },
								{ "MMT_HISTORY", MMT_HISTORY },
								{ "MMT_HEADLINE", MMT_HEADLINE },
								{ "MMT_REPLAYHEADLINE", MMT_REPLAYHEADLINE },
								{ "MMT_REPLAYSTORY", MMT_REPLAYSTORY },
								{ "MMT_TRANSACTION", MMT_TRANSACTION },
								{ "MMT_YIELD_CURVE", MMT_YIELD_CURVE },
								{ "MMT_CONTRIBUTION", MMT_CONTRIBUTION },
								{ "MMT_PROVIDER_ADMIN", MMT_PROVIDER_ADMIN },
								{ "MMT_ANALYTICS", MMT_ANALYTICS },
								{ "MMT_REFERENCE", MMT_REFERENCE },
								{ "MMT_NEWS_TEXT_ANALYTICS", MMT_NEWS_TEXT_ANALYTICS },
								{ "MMT_SYSTEM", MMT_SYSTEM },
							};

							bool found = false;
							for (int i = 0; i < sizeof converter / sizeof converter[0]; ++i)
							{
								if (!strcmp(converter[i].configInput, valueList[idx]))
								{
									found = true;
									if (service.infoFilter.capabilities.getPositionOf(converter[i].convertedValue) == -1)
										service.infoFilter.capabilities.push_back(converter[i].convertedValue);
									break;
								}
							}

							if (!found)
							{
								EmaString errorMsg("failed to read or convert a capability from the specified service [");
								errorMsg.append(serviceNodeName).append("]. Will drop this capability. Its value is = ").append(valueList[idx]);
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
								continue;
							}
						}
					}

					if (service.infoFilter.capabilities.empty())
					{
						EmaString errorMsg("specified service [");
						errorMsg.append(serviceNodeName).append("] contains no capabilities. Will drop this service.");
						pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
						continue;
					}

					for (UInt32 idx = 0; idx < service.infoFilter.capabilities.size() - 1; ++idx)
					{
						if (service.infoFilter.capabilities[idx] > service.infoFilter.capabilities[idx + 1])
						{
							UInt64 temp = service.infoFilter.capabilities[idx];
							service.infoFilter.capabilities[idx] = service.infoFilter.capabilities[idx + 1];
							service.infoFilter.capabilities[idx + 1] = temp;
							idx = 0;
						}
					}

					RsslQos rsslQos;
					EmaVector< XMLnode* > entryNodeList;
					pConfigImpl->getEntryNodeList(serviceNodeName + "InfoFilter|QoS", "QoSEntry", entryNodeList);

					if (entryNodeList.empty())
					{
						EmaString errorMsg("no configuration exists for service QoS [");
						errorMsg.append(serviceNodeName + "InfoFilter|QoS").append("]. Will use default QoS");
						pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

						rsslClearQos(&rsslQos);
						rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
						rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
						service.infoFilter.qos.push_back(rsslQos);
					}
					else
					{
						const UInt32 maxUInt32 = 0xFFFFFFFF;

						for (UInt32 idx = 0; idx < entryNodeList.size(); ++idx)
						{
							EmaString rateString;
							if (!entryNodeList[idx]->get< EmaString >("Rate", rateString))
							{
								EmaString errorMsg("no configuration exists for service QoS Rate [");
								errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate").append("]. Will use default Rate");
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

								rateString.set("Rate:TickByTick");
							}

							UInt32 rate;
							if (isdigit(rateString.c_str()[0]))
							{
								UInt64 temp;
								if (sscanf(rateString.c_str(), "%llu", &temp) == 1)
								{
									if (temp > maxUInt32)
									{
										EmaString errorMsg("specified service QoS::Rate [");
										errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate").append("] is greater than allowed maximum. Will use maximum Rate.");
										errorMsg.append(" Suspect Rate value is ").append(rateString);
										pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

										rate = maxUInt32;
									}
									else
										rate = (UInt32)temp;
								}
								else
								{
									EmaString errorMsg("failed to read or convert a QoS Rate from the specified service [");
									errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate").append("]. Will use default Rate.");
									errorMsg.append(" Suspect Rate value is ").append(rateString);
									pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);

									rate = 0;
								}
							}
							else
							{
								static struct
								{
									const char* configInput;
									UInt32 convertedValue;
								} converter[] =
								{
									{ "Rate::TickByTick", 0 },
									{ "Rate::JustInTimeConflated", 0xFFFFFF00 },
								};

								bool found = false;
								for (int i = 0; i < sizeof converter / sizeof converter[0]; ++i)
								{
									if (!strcmp(converter[i].configInput, rateString))
									{
										found = true;
										rate = converter[i].convertedValue;
										break;
									}
								}

								if (!found)
								{
									EmaString errorMsg("failed to read or convert a QoS Rate from the specified service [");
									errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate").append("]. Will use default Rate.");
									errorMsg.append(" Suspect Rate value is ").append(rateString);
									pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);

									rate = 0;
								}
							}

							EmaString timelinessString;
							if (!entryNodeList[idx]->get< EmaString >("Timeliness", timelinessString))
							{
								EmaString errorMsg("no configuration exists for service QoS Timeliness [");
								errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness").append("]. Will use default Timeliness");
								pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

								timelinessString.set("Timeliness:RealTime");
							}

							UInt32 timeliness;
							if (isdigit(timelinessString.c_str()[0]))
							{
								UInt64 temp;
								if (sscanf(timelinessString.c_str(), "%llu", &temp) == 1)
								{
									if (temp > maxUInt32)
									{
										EmaString errorMsg("specified service QoS::Timeliness [");
										errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness").append("] is greater than allowed maximum. Will use maximum Timeliness.");
										errorMsg.append(" Suspect Timeliness value is ").append(timelinessString);
										pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);

										timeliness = maxUInt32;
									}
									else
										timeliness = (UInt32)temp;
								}
								else
								{
									EmaString errorMsg("failed to read or convert a QoS Timeliness from the specified service [");
									errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness").append("]. Will use default Timeliness.");
									errorMsg.append(" Suspect Timeliness value is ").append(timelinessString);
									pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);

									timeliness = 0;
								}
							}
							else
							{
								static struct
								{
									const char* configInput;
									UInt32 convertedValue;
								} converter[] =
								{
									{ "Timeliness::RealTime", 0 },
									{ "Timeliness::InexactDelayed", 0xFFFFFFFF },
								};

								bool found = false;
								for (int i = 0; i < sizeof converter / sizeof converter[0]; ++i)
								{
									if (!strcmp(converter[i].configInput, timelinessString))
									{
										found = true;
										timeliness = converter[i].convertedValue;
										break;
									}
								}

								if (!found)
								{
									EmaString errorMsg("failed to read or convert a QoS Timeliness from the specified service [");
									errorMsg.append(serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness").append("]. Will use default Timeliness.");
									errorMsg.append(" Suspect Timeliness value is ").append(timelinessString);
									pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);

									timeliness = 0;
								}
							}

							rsslClearQos(&rsslQos);
							OmmQosDecoder::convertToRssl(&rsslQos, timeliness, rate);
							service.infoFilter.qos.push_back(rsslQos);
						}
					}

					service.infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;

					service.infoFilter.action = RSSL_FTEA_SET_ENTRY;
					service.stateFilter.action = RSSL_FTEA_SET_ENTRY;
					service.flags |= RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;

					if (pDirectoryCache)
					{
						addServiceIdAndNamePair(service.serviceId, new EmaString(service.infoFilter.serviceName), 0);
						pDirectoryCache->addService(service);
					}
				}
			}
		}
	}

	if (ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure())
	{
		if (pDirectoryCache)
		{
			ppc->retrieveDirectoryConfig(directoryName, *pDirectoryCache);
		}
	}
}

void DirectoryServiceStore::notifyOnServiceStateChange(ClientSession* clientSession, RsslRDMService& service)
{
	if (_pDirectoryServiceStoreClient)
	{
		_pDirectoryServiceStoreClient->onServiceStateChange(clientSession, service.serviceId, service.state);
	}
}

void DirectoryServiceStore::notifyOnServiceGroupChange(ClientSession* clientSession, RsslRDMService& service)
{
	if (_pDirectoryServiceStoreClient)
	{
		_pDirectoryServiceStoreClient->onServiceGroupChange(clientSession, service.serviceId, service.groupStateList, service.groupStateCount);
	}
}

void DirectoryServiceStore::notifyOnServiceDelete(ClientSession* clientSession, RsslRDMService& service)
{
	if (_pDirectoryServiceStoreClient)
	{
		_pDirectoryServiceStoreClient->onServiceDelete(clientSession, service.serviceId);
	}
}

bool DirectoryServiceStore::submitSourceDirectory(ClientSession* clientSession, RsslMsg* pMsg, RsslRDMDirectoryMsg& userSubmitSourceDirectory, RsslBuffer& sourceDirectoryBuffer, bool storeUserSubmitted)
{
	RsslDecodeIterator decIter;
	rsslClearDecodeIterator(&decIter);

	if (rsslSetDecodeIteratorRWFVersion(&decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION) != RSSL_RET_SUCCESS)
	{
		_ommCommonImpl.handleIue("Internal error. Failed to set decode iterator version in DirectoryServiceStore::submitSourceDirectory");
		return false;
	}

	if (rsslSetDecodeIteratorBuffer(&decIter, &pMsg->msgBase.encDataBody) != RSSL_RET_SUCCESS)
	{
		_ommCommonImpl.handleIue("Internal error. Failed to set decode iterator buffer in DirectoryServiceStore::submitSourceDirectory");
		return false;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	rsslClearRDMDirectoryMsg(&userSubmitSourceDirectory);

	RsslBuffer tempBuffer;

	tempBuffer = sourceDirectoryBuffer;

	RsslRet retCode;

	while (RSSL_RET_BUFFER_TOO_SMALL == (retCode = rsslDecodeRDMDirectoryMsg(&decIter, pMsg, &userSubmitSourceDirectory, &tempBuffer, &rsslErrorInfo)))
	{
		free(sourceDirectoryBuffer.data);
		sourceDirectoryBuffer.length += sourceDirectoryBuffer.length;
		sourceDirectoryBuffer.data = (char*)malloc(sourceDirectoryBuffer.length * sizeof(char));
		if (!sourceDirectoryBuffer.data)
		{
			_ommCommonImpl.handleMee("Failed to allocate memory in DirectoryServiceStore::submitSourceDirectory");
			return false;
		}

		tempBuffer = sourceDirectoryBuffer;
	}

	if (retCode < RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Attempt to submit invalid source directory domain message.");

			_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		EmaString temp("Internal error: failed to decode RsslRDMDirectoryMsg in DirectoryServiceStore::submitSourceDirectory");
		temp.append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);

		_ommCommonImpl.handleIue(temp);
		return false;
	}

	UInt32 serviceCount = 0;
	RsslRDMService* pServiceList = 0;

	switch (pMsg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:
		serviceCount = userSubmitSourceDirectory.refresh.serviceCount;
		pServiceList = userSubmitSourceDirectory.refresh.serviceList;
		break;
	case RSSL_MC_UPDATE:
		serviceCount = userSubmitSourceDirectory.update.serviceCount;
		pServiceList = userSubmitSourceDirectory.update.serviceList;
		break;
	default:
		break;
	}

	if (!storeUserSubmitted)
	{
		for (UInt32 idx = 0; idx < serviceCount; ++idx)
		{
			RsslRDMService& service = *pServiceList;
			switch (service.action)
			{
				case RSSL_MPEA_ADD_ENTRY:
				{
					if (service.flags & RDM_SVCF_HAS_STATE)
					{
						notifyOnServiceStateChange(clientSession, service);
					}

					notifyOnServiceGroupChange(clientSession, service);
				}
				break;
			case RSSL_MPEA_DELETE_ENTRY:
				{
					notifyOnServiceDelete(clientSession, service);
				}
				break;
			case RSSL_MPEA_UPDATE_ENTRY:
				{
					if (service.flags & RDM_SVCF_HAS_STATE)
					{
						notifyOnServiceStateChange(clientSession, service);
					}

					notifyOnServiceGroupChange(clientSession, service);
				}
				break;
			}

			++pServiceList;
		}

		return true;
	}
	else
	{
		for (UInt32 idx = 0; idx < serviceCount; ++idx)
		{
			switch ((*pServiceList).action)
			{
			case RSSL_MPEA_ADD_ENTRY:
			{
				Service* pService = _directoryCache.getService((*pServiceList).serviceId);
				if (!pService)
				{
					Service service;
					service.serviceId = (UInt16)(*pServiceList).serviceId;

					RsslUInt32 flags = 0;

					if (pServiceList->flags & RDM_SVCF_HAS_STATE)
					{
						switch (pServiceList->state.action)
						{
						case RSSL_FTEA_SET_ENTRY:

							notifyOnServiceStateChange(clientSession, *pServiceList);

							service.stateFilter.apply(pServiceList->state);
							flags |= RDM_SVCF_HAS_STATE;
							break;
						}
					}

					if (pServiceList->flags & RDM_SVCF_HAS_INFO)
					{
						switch (pServiceList->info.action)
						{
						case RSSL_FTEA_SET_ENTRY:

							service.infoFilter.apply(pServiceList->info);
							flags |= RDM_SVCF_HAS_INFO;
							break;
						}
					}

					if (pServiceList->flags & RDM_SVCF_HAS_LOAD)
					{
						switch (pServiceList->load.action)
						{
						case RSSL_FTEA_SET_ENTRY:

							service.loadFilter.apply(pServiceList->load);
							break;
						}
					}

					if (pServiceList->flags & RDM_SVCF_HAS_LINK)
					{
						EmaString linkName;
						Link link;

						switch (pServiceList->linkInfo.action)
						{
						case RSSL_FTEA_SET_ENTRY:
						{
							service.linkFilter.clear();

							for (RsslUInt32 idx = 0; idx < pServiceList->linkInfo.linkCount; idx++)
							{
								linkName.set(pServiceList->linkInfo.linkList[idx].name.data, pServiceList->linkInfo.linkList[idx].name.length);

								Link* pLink = service.linkFilter.getLink(linkName);

								if (pLink)
								{
									if (pServiceList->linkInfo.linkList[idx].action == RSSL_MPEA_UPDATE_ENTRY)
									{
										pLink->apply(pServiceList->linkInfo.linkList[idx]);
									}
								}
								else
								{
									if (pServiceList->linkInfo.linkList[idx].action == RSSL_MPEA_ADD_ENTRY)
									{
										link.apply(pServiceList->linkInfo.linkList[idx]);
										service.linkFilter.addLink(link);
									}
								}
							}
						}
						break;
						}
					}

					notifyOnServiceGroupChange(clientSession, *pServiceList);

					if (flags & (RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE))
					{
						service.flags = flags;
						_directoryCache.addService(service);
					}
				}
			}
			break;
			case RSSL_MPEA_DELETE_ENTRY:
			{
				_directoryCache.removeService((*pServiceList).serviceId);
				notifyOnServiceDelete(clientSession, *pServiceList);
			}
				break;
			case RSSL_MPEA_UPDATE_ENTRY:
			{
				Service* pService = _directoryCache.getService((*pServiceList).serviceId);
				if (pService)
				{
					if (pServiceList->flags & RDM_SVCF_HAS_STATE)
					{
						switch (pServiceList->state.action)
						{
						case RSSL_FTEA_SET_ENTRY:
						{
							notifyOnServiceStateChange(clientSession, *pServiceList);

							pService->stateFilter.clear();
							pService->stateFilter.apply(pServiceList->state);
						}
						break;

						case RSSL_FTEA_UPDATE_ENTRY:
						{
							notifyOnServiceStateChange(clientSession, *pServiceList);
							pService->stateFilter.apply(pServiceList->state);
						}
						break;
						case RSSL_FTEA_CLEAR_ENTRY:
							pService->stateFilter.clear();
							break;
						}
					}

					if (pServiceList->flags & RDM_SVCF_HAS_LOAD)
					{
						switch (pServiceList->load.action)
						{
						case RSSL_FTEA_SET_ENTRY:
						{
							pService->loadFilter.clear();
							pService->loadFilter.apply(pServiceList->load);
						}
						break;

						case RSSL_FTEA_UPDATE_ENTRY:
							pService->loadFilter.apply(pServiceList->load);
							break;

						case RSSL_FTEA_CLEAR_ENTRY:
							pService->loadFilter.clear();
							break;
						}
					}

					if (pServiceList->flags & RDM_SVCF_HAS_LINK)
					{
						EmaString linkName;
						Link link;

						switch (pServiceList->linkInfo.action)
						{
						case RSSL_FTEA_SET_ENTRY:
						{
							pService->linkFilter.clear();

							for (RsslUInt32 idx = 0; idx < pServiceList->linkInfo.linkCount; idx++)
							{
								linkName.set(pServiceList->linkInfo.linkList[idx].name.data, pServiceList->linkInfo.linkList[idx].name.length);

								Link* pLink = pService->linkFilter.getLink(linkName);

								if (pLink)
								{
									if (pServiceList->linkInfo.linkList[idx].action == RSSL_MPEA_UPDATE_ENTRY)
									{
										pLink->apply(pServiceList->linkInfo.linkList[idx]);
									}
								}
								else
								{
									if (pServiceList->linkInfo.linkList[idx].action == RSSL_MPEA_ADD_ENTRY)
									{
										link.apply(pServiceList->linkInfo.linkList[idx]);
										pService->linkFilter.addLink(link);
									}
								}
							}
						}
						break;
						case RSSL_FTEA_UPDATE_ENTRY:

							for (RsslUInt32 idx = 0; idx < pServiceList->linkInfo.linkCount; idx++)
							{
								linkName.set(pServiceList->linkInfo.linkList[idx].name.data, pServiceList->linkInfo.linkList[idx].name.length);

								Link* pLink = pService->linkFilter.getLink(linkName);

								if (pLink)
								{
									if (pServiceList->linkInfo.linkList[idx].action == RSSL_MPEA_UPDATE_ENTRY)
									{
										pLink->apply(pServiceList->linkInfo.linkList[idx]);
									}
								}
								else
								{
									if (pServiceList->linkInfo.linkList[idx].action == RSSL_MPEA_ADD_ENTRY)
									{
										link.apply(pServiceList->linkInfo.linkList[idx]);
										pService->linkFilter.addLink(link);
									}
								}
							}

							break;
						case RSSL_FTEA_CLEAR_ENTRY:
							pService->linkFilter.clear();
							break;
						}
					}

					notifyOnServiceGroupChange(clientSession, *pServiceList);
				}
			}
			break;
			default:
				break;
			}

			++pServiceList;
		}
	}

	return true;
}

bool DirectoryServiceStore::decodeSourceDirectory(RwfBuffer* pInBuffer, EmaString& errorText)
{
	RsslRet retCode = RSSL_RET_SUCCESS;
	RsslDecodeIterator dIter;
	rsslClearDecodeIterator( &dIter );

	retCode = rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		errorText.set( "Internal error. Failed to set decode iterator version in DirectoryServiceStore::decodeSourceDirectory(). Reason = " );
		errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
		return false;
	}

	retCode = rsslSetDecodeIteratorBuffer( &dIter, pInBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		errorText.set( "Internal error. Failed to set decode iterator buffer in DirectoryServiceStore::decodeSourceDirectory. Reason = " );
		errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
		return false;
	}

	RsslMap rsslMap;
	rsslClearMap( &rsslMap );

	if ( OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity )
		_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, "Begin decoding of SourceDirectory.");

	retCode = rsslDecodeMap( &dIter, &rsslMap );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		errorText.set( "Internal error. Failed to decode rsslMap in DirectoryServiceStore::decodeSourceDirectory. Reason = " );
		errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
		return false;
	}
	else if ( retCode == RSSL_RET_NO_DATA )
	{
		if (OmmLoggerClient::WarningEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::WarningEnum, "Passed in SourceDirectory map contains no entries (e.g. there is no service specified).");

		if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, "End decoding of SourceDirectory.");

		return true;
	}

	switch ( rsslMap.keyPrimitiveType )
	{
	case RSSL_DT_UINT:
		if ( !decodeSourceDirectoryKeyUInt( rsslMap, dIter, errorText ) )
			return false;
		break;
	default:
		errorText.set( "Attempt to specify SourceDirectory info with a Map using key DataType of " );
		errorText += DataType((DataType::DataTypeEnum)rsslMap.keyPrimitiveType).toString();
		errorText += EmaString( " while the expected key DataType is " );
		errorText += DataType( DataType::UIntEnum ).toString();
		return false;
	}

	if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
		_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, "End decoding of SourceDirectory.");

	return true;
}

bool DirectoryServiceStore::decodeSourceDirectoryKeyUInt(RsslMap& rsslMap, RsslDecodeIterator& dIter, EmaString& errorText)
{
	RsslRet retCode = RSSL_RET_SUCCESS;
	RsslUInt64 serviceId = 0;
	RsslMapEntry rsslMapEntry;
	rsslClearMapEntry(&rsslMapEntry);
	while ((retCode = rsslDecodeMapEntry(&dIter, &rsslMapEntry, &serviceId)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retCode != RSSL_RET_SUCCESS)
		{
			errorText.set("Internal error: Failed to Decode Map Entry. Reason = ");
			errorText.append(rsslRetCodeToString(retCode)).append(". ");
			return false;
		}

		if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Begin decoding of Service with id of ");
			temp.append(serviceId).append(". Action = ");
			switch (rsslMapEntry.action)
			{
			case RSSL_MPEA_UPDATE_ENTRY:
				temp.append("Update");
				break;
			case RSSL_MPEA_ADD_ENTRY:
				temp.append("Add");
				break;
			case RSSL_MPEA_DELETE_ENTRY:
				temp.append("Delete");
				break;
			}

			_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		if (rsslMapEntry.action == RSSL_MPEA_DELETE_ENTRY)
		{
			EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find(serviceId);

			if (pServiceNamePtr)
			{
				UInt64* pServiceId = _serviceNameToServiceId.find(*pServiceNamePtr);
				if (!pServiceId)
				{
					errorText.set("Internal error: mismatch between _serviceIdToServiceName and _serviceNameToServiceId tables.")
						.append(" ServiceName = ").append(**pServiceNamePtr).append(" serviceId = ").append(serviceId);
					return false;
				}

				EmaStringPtr pTemp = *pServiceNamePtr;
				_serviceNameToServiceId.erase(pTemp);
				_serviceIdToServiceName.erase(serviceId);
				_serviceNameList.removeValue(pTemp);
				delete pTemp;
			}

			if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("End decoding of Service with id of ");
				temp.append(serviceId);
				_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
			}

			continue;
		}
		else if (rsslMapEntry.action == RSSL_MPEA_ADD_ENTRY)
		{
			if (checkExistingServiceId(serviceId, errorText) == false)
			{
				return false;
			}
		}

		if (rsslMap.containerType != RSSL_DT_FILTER_LIST)
		{
			errorText.set("Attempt to specify Service with a container of ");
			errorText += DataType((DataType::DataTypeEnum)rsslMap.containerType).toString();
			errorText += EmaString(" rather than the expected ");
			errorText += DataType(DataType::FilterListEnum).toString();
			return false;
		}

		RsslFilterList rsslFilterList;
		RsslFilterEntry rsslFilterEntry;
		rsslClearFilterList(&rsslFilterList);
		rsslClearFilterEntry(&rsslFilterEntry);

		retCode = rsslDecodeFilterList(&dIter, &rsslFilterList);

		if (retCode < RSSL_RET_SUCCESS)
		{
			errorText.set("Internal error: Failed to Decode FilterList. Reason = ");
			errorText.append(rsslRetCodeToString(retCode)).append(". ");
			return false;
		}
		else if (retCode == RSSL_RET_NO_DATA)
		{
			if (OmmLoggerClient::WarningEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Service with id of ");
				temp.append(serviceId).append(" contains no FilterEntries. Skipping this service.");
				_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::WarningEnum, temp);
			}

			if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("End decoding of Service with id of ");
				temp.append(serviceId);
				_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
			}

			continue;
		}

		while ((retCode = rsslDecodeFilterEntry(&dIter, &rsslFilterEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retCode < RSSL_RET_SUCCESS)
			{
				errorText.set("Internal error: Failed to Decode Filter Entry. Reason = ");
				errorText.append(rsslRetCodeToString(retCode)).append(". ");
				return false;
			}

			if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Begin decoding of FilterEntry with id of ");
				temp.append(rsslFilterEntry.id);
				_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
			}

			if (rsslFilterEntry.id == RDM_DIRECTORY_SERVICE_INFO_ID)
			{
				if (rsslMapEntry.action == RSSL_MPEA_UPDATE_ENTRY)
				{
					errorText.set("Attempt to update Infofilter of service with id of ");
					errorText.append(serviceId).append(" while this is not allowed.");
					return false;
				}

				if (((rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE) && rsslFilterEntry.containerType != RSSL_DT_ELEMENT_LIST) ||
					rsslFilterList.containerType != RSSL_DT_ELEMENT_LIST)
				{
					RsslContainerType type = (rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE) ? rsslFilterEntry.containerType : rsslFilterList.containerType;
					errorText.set("Attempt to specify Service InfoFilter with a container of ");
					errorText += DataType((DataType::DataTypeEnum)type).toString();
					errorText += EmaString(" rather than the expected ");
					errorText += DataType(DataType::ElementListEnum).toString();
					return false;
				}

				RsslElementList	rsslElementList;
				RsslElementEntry rsslElementEntry;
				rsslClearElementList(&rsslElementList);
				rsslClearElementEntry(&rsslElementEntry);

				if ((retCode = rsslDecodeElementList(&dIter, &rsslElementList, NULL)) < RSSL_RET_SUCCESS)
				{
					errorText.set("Internal error: Failed to Decode Element List. Reason = ");
					errorText.append(rsslRetCodeToString(retCode)).append(". ");
					return false;
				}

				bool bServiceNameEntryFound = false;

				while ((retCode = rsslDecodeElementEntry(&dIter, &rsslElementEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (retCode < RSSL_RET_SUCCESS)
					{
						errorText.set("Internal error: Failed to Decode ElementEntry. Reason = ");
						errorText.append(rsslRetCodeToString(retCode)).append(". ");
						return false;
					}

					if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
					{
						EmaString temp("Decoding of ElementEntry with name of ");
						temp.append(EmaString(rsslElementEntry.name.data, rsslElementEntry.name.length));
						_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
					}

					if (!bServiceNameEntryFound && rsslBufferIsEqual(&rsslElementEntry.name, &RSSL_ENAME_NAME))
					{
						if (rsslElementEntry.dataType != RSSL_DT_ASCII_STRING)
						{
							errorText.set("Attempt to specify Service Name with a ");
							errorText += DataType((DataType::DataTypeEnum)rsslElementEntry.dataType).toString();
							errorText += EmaString(" rather than the expected ");
							errorText += DataType(DataType::AsciiEnum).toString();
							return false;
						}

						RsslBuffer serviceNameBuffer;
						rsslClearBuffer(&serviceNameBuffer);

						retCode = rsslDecodeBuffer(&dIter, &serviceNameBuffer);
						if (retCode < RSSL_RET_SUCCESS)
						{
							errorText.set("Internal error: Failed to Decode Buffer. Reason = ");
							errorText.append(rsslRetCodeToString(retCode)).append(". ");
							return false;
						}
						else if (retCode == RSSL_RET_BLANK_DATA)
						{
							errorText.set("Attempt to specify Service Name with a blank ascii string for service id of ");
							errorText.append(serviceId);
							return false;
						}

						bServiceNameEntryFound = true;
						EmaStringPtr pServiceName;

						try
						{
							pServiceName = new EmaString(serviceNameBuffer.data, serviceNameBuffer.length);
						}
						catch (std::bad_alloc)
						{
							errorText.set("Failed to allocate memory in DirectoryServiceStore::decodeSourceDirectoryKeyUInt()");
							return false;
						}

						if (addServiceIdAndNamePair(serviceId, pServiceName, &errorText) == false)
						{
							return false;
						}
					}
				}

				if (!bServiceNameEntryFound)
				{
					errorText.set("Attempt to specify service InfoFilter without required Service Name for service id of ");
					errorText.append(serviceId);
					return false;
				}
			}

			if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("End decoding of FilterEntry with id of ");
				temp.append(rsslFilterEntry.id);
				_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
			}
		}
	}

	return true;
}

bool DirectoryServiceStore::encodeService(const Service* service, RsslRDMService& rsslRDMService, RsslRDMDirectoryRefresh& directoryRefresh, int directoryServiceFilter)
{
	rsslClearRDMService(&rsslRDMService);

	rsslRDMService.action = RSSL_MPEA_ADD_ENTRY;
	rsslRDMService.serviceId = service->serviceId;

	if ((directoryServiceFilter &  RDM_DIRECTORY_SERVICE_INFO_FILTER) && (service->flags & RDM_SVCF_HAS_INFO))
	{
		directoryRefresh.filter |= RDM_DIRECTORY_SERVICE_INFO_FILTER;

		rsslRDMService.info.action = service->infoFilter.action;

		rsslRDMService.flags |= service->flags;

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_VENDOR)
		{
			rsslRDMService.info.vendor.data = (char*)service->infoFilter.vendorName.c_str();
			rsslRDMService.info.vendor.length = service->infoFilter.vendorName.length();
		}

		if (service->infoFilter.flags &  RDM_SVC_IFF_HAS_ITEM_LIST)
		{
			rsslRDMService.info.itemList.data = (char*)service->infoFilter.itemList.c_str();
			rsslRDMService.info.itemList.length = service->infoFilter.itemList.length();
		}

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_IS_SOURCE)
		{
			rsslRDMService.info.acceptingConsumerStatus = service->infoFilter.acceptingConsumerStatus;
		}

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_IS_SOURCE)
		{
			rsslRDMService.info.isSource = service->infoFilter.isSource;
		}

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS)
		{
			rsslRDMService.info.supportsOutOfBandSnapshots = service->infoFilter.supportsOutOfBandSnapshots;
		}

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
		{
			rsslRDMService.info.supportsQosRange = service->infoFilter.supportsQosRange;
		}

		rsslRDMService.info.capabilitiesCount = service->infoFilter.capabilities.size();

		try
		{
			rsslRDMService.info.capabilitiesList = new UInt64[rsslRDMService.info.capabilitiesCount];
		}
		catch (std::bad_alloc)
		{
			return false;
		}

		for (UInt32 jdx = 0; jdx < rsslRDMService.info.capabilitiesCount; ++jdx)
			rsslRDMService.info.capabilitiesList[jdx] = service->infoFilter.capabilities[jdx];

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
		{
			rsslRDMService.info.dictionariesProvidedCount = service->infoFilter.dictionariesProvided.size();

			if (rsslRDMService.info.dictionariesProvidedCount)
			{
				try
				{
					rsslRDMService.info.dictionariesProvidedList = new RsslBuffer[rsslRDMService.info.dictionariesProvidedCount];
				}
				catch (std::bad_alloc)
				{
					return false;
				}


				for (UInt32 jdx = 0; jdx < rsslRDMService.info.dictionariesProvidedCount; ++jdx)
				{
					rsslRDMService.info.dictionariesProvidedList[jdx].data = (char*)service->infoFilter.dictionariesProvided[jdx].c_str();
					rsslRDMService.info.dictionariesProvidedList[jdx].length = service->infoFilter.dictionariesProvided[jdx].length();
				}

				rsslRDMService.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
			}
		}

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_DICTS_USED)
		{
			rsslRDMService.info.dictionariesUsedCount = service->infoFilter.dictionariesUsed.size();

			if (rsslRDMService.info.dictionariesUsedCount)
			{
				try
				{
					rsslRDMService.info.dictionariesUsedList = new RsslBuffer[rsslRDMService.info.dictionariesUsedCount];
				}
				catch (std::bad_alloc)
				{
					return false;
				}

				for (UInt32 jdx = 0; jdx < rsslRDMService.info.dictionariesUsedCount; ++jdx)
				{
					rsslRDMService.info.dictionariesUsedList[jdx].data = (char*)service->infoFilter.dictionariesUsed[jdx].c_str();
					rsslRDMService.info.dictionariesUsedList[jdx].length = service->infoFilter.dictionariesUsed[jdx].length();
				}

				rsslRDMService.info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
			}
		}

		if (service->infoFilter.flags & RDM_SVC_IFF_HAS_QOS)
		{
			rsslRDMService.info.qosCount = service->infoFilter.qos.size();

			if (rsslRDMService.info.qosCount)
			{
				try
				{
					rsslRDMService.info.qosList = new RsslQos[rsslRDMService.info.qosCount];
				}
				catch (std::bad_alloc)
				{
					return false;
				}

				for (UInt32 jdx = 0; jdx < rsslRDMService.info.qosCount; ++jdx)
					rsslRDMService.info.qosList[jdx] = service->infoFilter.qos[jdx];

				rsslRDMService.info.flags |= RDM_SVC_IFF_HAS_QOS;
			}
		}

		rsslRDMService.info.serviceName.data = (char*)service->infoFilter.serviceName.c_str();
		rsslRDMService.info.serviceName.length = service->infoFilter.serviceName.length();

		rsslRDMService.info.flags = service->infoFilter.flags;
	}

	if ((directoryServiceFilter &  RDM_DIRECTORY_SERVICE_STATE_FILTER) && (service->flags & RDM_SVCF_HAS_STATE))
	{
		directoryRefresh.filter |= RDM_DIRECTORY_SERVICE_STATE_FILTER;

		rsslRDMService.flags |= service->flags;

		if (service->stateFilter.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS)
		{
			rsslRDMService.state.acceptingRequests = service->stateFilter.acceptingRequests;
		}

		rsslRDMService.state.action = service->stateFilter.action;
		rsslRDMService.state.serviceState = service->stateFilter.serviceState;

		if (service->stateFilter.flags & RDM_SVC_STF_HAS_STATUS)
		{
			rsslRDMService.state.status.streamState = service->stateFilter.status.streamState;
			rsslRDMService.state.status.dataState = service->stateFilter.status.dataState;
			rsslRDMService.state.status.code = service->stateFilter.status.code;
			rsslRDMService.state.status.text = service->stateFilter.status.text;
		}

		rsslRDMService.state.flags = service->stateFilter.flags;
	}

	if ((directoryServiceFilter &  RDM_DIRECTORY_SERVICE_LINK_FILTER) && (service->flags & RDM_SVCF_HAS_LINK))
	{
		if (service->linkFilter.getLinkList().size() > 0)
		{
			directoryRefresh.filter |= RDM_DIRECTORY_SERVICE_LINK_FILTER;

			rsslRDMService.flags |= service->flags;

			rsslRDMService.linkInfo.action = service->linkFilter.action;
			rsslRDMService.linkInfo.linkCount = service->linkFilter.getLinkList().size();

			try
			{
				rsslRDMService.linkInfo.linkList = new RsslRDMServiceLink[rsslRDMService.linkInfo.linkCount];
			}
			catch (std::bad_alloc)
			{
				return false;
			}

			Link* link = service->linkFilter.getLinkList().front();

			for (UInt32 jdx = 0; jdx < rsslRDMService.linkInfo.linkCount; ++jdx)
			{
				rsslRDMService.linkInfo.linkList[jdx].action = link->action;

				if (link->flags & RDM_SVC_LKF_HAS_TYPE)
				{
					rsslRDMService.linkInfo.linkList[jdx].type = link->type;
				}

				if (link->flags & RDM_SVC_LKF_HAS_CODE)
				{
					rsslRDMService.linkInfo.linkList[jdx].linkCode = link->linkCode;
				}

				if (link->flags & RDM_SVC_LKF_HAS_TEXT)
				{
					rsslRDMService.linkInfo.linkList[jdx].text.data = (char*)link->text.c_str();
					rsslRDMService.linkInfo.linkList[jdx].text.length = link->text.length();
				}

				rsslRDMService.linkInfo.linkList[jdx].linkState = link->linkState;
				rsslRDMService.linkInfo.linkList[jdx].name.data = (char*)link->name.c_str();
				rsslRDMService.linkInfo.linkList[jdx].name.length = link->name.length();


				rsslRDMService.linkInfo.linkList[jdx].flags = link->flags;

				link = link->next();
			}
		}
	}

	if ((directoryServiceFilter &  RDM_DIRECTORY_SERVICE_LOAD_FILTER) && (service->flags & RDM_SVCF_HAS_LOAD))
	{
		directoryRefresh.filter |= RDM_DIRECTORY_SERVICE_LOAD_FILTER;

		rsslRDMService.flags |= RDM_SVCF_HAS_LOAD;
		rsslRDMService.load.action = service->loadFilter.action;

		if (service->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT)
		{
			rsslRDMService.load.openLimit = service->loadFilter.openLimit;
		}

		if (service->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW)
		{
			rsslRDMService.load.openWindow = service->loadFilter.openWindow;
		}

		if (service->loadFilter.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR)
		{
			rsslRDMService.load.loadFactor = service->loadFilter.loadFactor;
		}

		rsslRDMService.load.flags = service->loadFilter.flags;

	}

	return true;
}

bool DirectoryServiceStore::encodeDirectoryRefreshMsg(const DirectoryCache& directoryCache, RsslRDMDirectoryRefresh& directoryRefresh, int directoryServiceFilter,
	bool specificServiceId, UInt16 serviceId)
{
	Service* pService = 0;

	if (specificServiceId)
	{
		Service* pService = directoryCache.getService(serviceId);

		if (pService == 0)
		{
			directoryRefresh.filter = 0;
			return true;
		}
	}

	if (specificServiceId)
	{
		directoryRefresh.serviceCount = 1;
	}
	else
	{
		directoryRefresh.serviceCount = directoryCache.getServiceList().size();
	}

	try
	{
		directoryRefresh.serviceList = new RsslRDMService[directoryRefresh.serviceCount];
	}
	catch (std::bad_alloc)
	{
		return false;
	}

	if (pService)
	{
		if (!encodeService(pService, directoryRefresh.serviceList[0], directoryRefresh, directoryServiceFilter))
		{
			return false;
		}
	}
	else
	{
		UInt32 idx = 0;
		pService = directoryCache.getServiceList().front();

		while (pService)
		{
			if (!encodeService(pService, directoryRefresh.serviceList[idx], directoryRefresh, directoryServiceFilter))
			{
				return false;
			}

			pService = pService->next();
			++idx;
		}
	}

	return true;
}

void DirectoryServiceStore::encodeService(const RsslRDMService& submittedRDMService, RsslRDMService& rsslRDMService,int requestFilter, int& responseFilter)
{
	rsslClearRDMService(&rsslRDMService);
	rsslRDMService.action = submittedRDMService.action;
	rsslRDMService.serviceId = submittedRDMService.serviceId;

	if ( (submittedRDMService.flags & RDM_SVCF_HAS_INFO) && (requestFilter & RDM_DIRECTORY_SERVICE_INFO_FILTER) )
	{
		rsslRDMService.info = submittedRDMService.info;
		rsslRDMService.flags |= RDM_SVCF_HAS_INFO;
		responseFilter |= RDM_DIRECTORY_SERVICE_INFO_FILTER;
	}

	if ( (submittedRDMService.flags & RDM_SVCF_HAS_STATE) && (requestFilter & RDM_DIRECTORY_SERVICE_STATE_FILTER) )
	{
		rsslRDMService.state = submittedRDMService.state;
		rsslRDMService.flags |= RDM_SVCF_HAS_STATE;
		responseFilter |= RDM_DIRECTORY_SERVICE_STATE_FILTER;
	}

	if ( (submittedRDMService.flags & RDM_SVCF_HAS_LOAD) && (requestFilter & RDM_DIRECTORY_SERVICE_LOAD_FILTER) )
	{
		rsslRDMService.load = submittedRDMService.load;
		rsslRDMService.flags |= RDM_SVCF_HAS_LOAD;
		responseFilter |= RDM_DIRECTORY_SERVICE_LOAD_FILTER;
	}

	if ( (submittedRDMService.flags & RDM_SVCF_HAS_DATA) && (requestFilter & RDM_DIRECTORY_SERVICE_DATA_FILTER) )
	{
		rsslRDMService.data = submittedRDMService.data;
		rsslRDMService.flags |= RDM_SVCF_HAS_DATA;
		responseFilter |= RDM_DIRECTORY_SERVICE_DATA_FILTER;
	}

	if ( (submittedRDMService.flags & RDM_SVCF_HAS_LINK) && (requestFilter & RDM_DIRECTORY_SERVICE_LINK_FILTER) )
	{
		rsslRDMService.linkInfo = submittedRDMService.linkInfo;
		rsslRDMService.flags |= RDM_SVCF_HAS_LINK;
		responseFilter |= RDM_DIRECTORY_SERVICE_LINK_FILTER;
	}
	
	if ( (submittedRDMService.flags & RDM_SVCF_HAS_SEQ_MCAST) && (requestFilter & RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER) )
	{
		rsslRDMService.seqMCastInfo = submittedRDMService.seqMCastInfo;
		rsslRDMService.flags |= RDM_SVCF_HAS_SEQ_MCAST;
		responseFilter |= RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER;
	}

	if (submittedRDMService.groupStateCount != 0)
	{
		rsslRDMService.groupStateCount = submittedRDMService.groupStateCount;
		rsslRDMService.groupStateList = submittedRDMService.groupStateList;

		responseFilter |= RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	}
}

bool DirectoryServiceStore::encodeDirectoryMsg(const RsslRDMDirectoryMsg& directoryMsg, RsslRDMDirectoryMsg& encodeDirectoryMsg, int requestFilter, bool specificServiceId, UInt16 serviceId)
{
	UInt32 serviceCount = 0;
	RsslRDMService* pServiceList = 0;

	switch (directoryMsg.rdmMsgBase.rdmMsgType)
	{
	case RDM_DR_MT_REFRESH:
	{
		serviceCount = directoryMsg.refresh.serviceCount;
		pServiceList = directoryMsg.refresh.serviceList;

		try
		{
			if (specificServiceId)
			{
				RsslRDMService* pRDMService = 0;

				for (UInt32 idx = 0; idx < serviceCount; ++idx)
				{
					if (pServiceList->serviceId == serviceId)
					{
						pRDMService = pServiceList;
						break;
					}

					++pServiceList;
				}

				if (pRDMService)
				{
					if (pRDMService->action == RSSL_MPEA_DELETE_ENTRY)
					{
						encodeDirectoryMsg.refresh.serviceCount = 1;
						encodeDirectoryMsg.refresh.serviceList = new RsslRDMService[encodeDirectoryMsg.refresh.serviceCount];

						encodeDirectoryMsg.refresh.serviceList->action = pRDMService->action;
						encodeDirectoryMsg.refresh.serviceList->serviceId = pRDMService->serviceId;
						encodeDirectoryMsg.refresh.serviceList->flags = 0;
					}
					else
					{
						encodeDirectoryMsg.refresh.serviceCount = 1;
						encodeDirectoryMsg.refresh.serviceList = new RsslRDMService[encodeDirectoryMsg.refresh.serviceCount];

						int responseFilter = 0;

						encodeService(*pRDMService, *encodeDirectoryMsg.refresh.serviceList, requestFilter, responseFilter);

						encodeDirectoryMsg.refresh.filter = responseFilter;
					}
				}
				else
				{
					encodeDirectoryMsg.refresh.serviceCount = 0;
					encodeDirectoryMsg.refresh.serviceList = 0;
				}
			}
			else
			{
				if (serviceCount > 0)
				{
					encodeDirectoryMsg.refresh.serviceCount = serviceCount;
					encodeDirectoryMsg.refresh.serviceList = new RsslRDMService[encodeDirectoryMsg.refresh.serviceCount];

					int responseFilter = 0;

					for (UInt32 idx = 0; idx < serviceCount; ++idx)
					{

						if (pServiceList[idx].action == RSSL_MPEA_DELETE_ENTRY)
						{
							encodeDirectoryMsg.refresh.serviceList[idx].action = pServiceList[idx].action;
							encodeDirectoryMsg.refresh.serviceList[idx].serviceId = pServiceList[idx].serviceId;
							encodeDirectoryMsg.refresh.serviceList[idx].flags = 0;
						}
						else
						{
							encodeService(pServiceList[idx], encodeDirectoryMsg.refresh.serviceList[idx], requestFilter, responseFilter);

							encodeDirectoryMsg.refresh.filter |= responseFilter;
						}
					}
				}
				else
				{
					encodeDirectoryMsg.refresh.serviceCount = 0;
					encodeDirectoryMsg.refresh.serviceList = 0;
				}
			}
		}
		catch (std::bad_alloc)
		{
			throwMeeException("Failed to allocate memory in DirectoryServiceStore::encodeDirectoryMsg()");
			return false;
		}
	}
		break;
	case RDM_DR_MT_UPDATE:
	{
		serviceCount = directoryMsg.update.serviceCount;
		pServiceList = directoryMsg.update.serviceList;

		try
		{
			if (specificServiceId)
			{
				RsslRDMService* pRDMService = 0;

				for (UInt32 idx = 0; idx < serviceCount; ++idx)
				{
					if (pServiceList->serviceId == serviceId)
					{
						pRDMService = pServiceList;
						break;
					}

					++pServiceList;
				}

				if (pRDMService)
				{
					if (pRDMService->action == RSSL_MPEA_DELETE_ENTRY)
					{
						encodeDirectoryMsg.update.serviceCount = 1;
						encodeDirectoryMsg.update.serviceList = new RsslRDMService[encodeDirectoryMsg.update.serviceCount];

						encodeDirectoryMsg.update.serviceList->action = pRDMService->action;
						encodeDirectoryMsg.update.serviceList->serviceId = pRDMService->serviceId;
						encodeDirectoryMsg.update.serviceList->flags = 0;
					}
					else
					{
						encodeDirectoryMsg.update.serviceCount = 1;
						encodeDirectoryMsg.update.serviceList = new RsslRDMService[encodeDirectoryMsg.update.serviceCount];

						int responseFilter = 0;

						encodeService(*pRDMService, *encodeDirectoryMsg.update.serviceList, requestFilter, responseFilter);

						encodeDirectoryMsg.update.flags |= RDM_DR_UPF_HAS_FILTER;
						encodeDirectoryMsg.update.filter = responseFilter;
					}
				}
				else
				{
					encodeDirectoryMsg.update.serviceCount = 0;
					encodeDirectoryMsg.update.serviceList = 0;
				}
			}
			else
			{
				if (serviceCount > 0)
				{
					encodeDirectoryMsg.update.serviceCount = serviceCount;
					encodeDirectoryMsg.update.serviceList = new RsslRDMService[encodeDirectoryMsg.update.serviceCount];

					int responseFilter = 0;

					for (UInt32 idx = 0; idx < serviceCount; ++idx)
					{
						if (pServiceList[idx].action == RSSL_MPEA_DELETE_ENTRY)
						{
							encodeDirectoryMsg.update.serviceList[idx].action = pServiceList[idx].action;
							encodeDirectoryMsg.update.serviceList[idx].serviceId = pServiceList[idx].serviceId;
							encodeDirectoryMsg.update.serviceList[idx].flags = 0;
						}
						else
						{
							encodeService(pServiceList[idx], encodeDirectoryMsg.update.serviceList[idx], requestFilter, responseFilter);

							encodeDirectoryMsg.update.flags |= RDM_DR_UPF_HAS_FILTER;
							encodeDirectoryMsg.update.filter |= responseFilter;
						}
					}
				}
				else
				{
					encodeDirectoryMsg.refresh.serviceCount = 0;
					encodeDirectoryMsg.refresh.serviceList = 0;
				}
			}
		}
		catch (std::bad_alloc)
		{
			throwMeeException("Failed to allocate memory in DirectoryServiceStore::encodeDirectoryMsg()");
			return false;
		}
	}
		break;
	}

	return true;
}

void DirectoryServiceStore::freeMemory(RsslRDMDirectoryRefresh& directoryRefresh, RsslBuffer* rsslMsgBuffer)
{
	for (UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp)
		if (directoryRefresh.serviceList[temp].info.qosList)
			delete[] directoryRefresh.serviceList[temp].info.qosList;

	for (UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp)
		if (directoryRefresh.serviceList[temp].info.dictionariesUsedList)
			delete[] directoryRefresh.serviceList[temp].info.dictionariesUsedList;

	for (UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp)
		if (directoryRefresh.serviceList[temp].info.dictionariesProvidedList)
			delete[] directoryRefresh.serviceList[temp].info.dictionariesProvidedList;

	for (UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp)
		if (directoryRefresh.serviceList[temp].info.capabilitiesList)
			delete[] directoryRefresh.serviceList[temp].info.capabilitiesList;

	for (UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp)
		if (directoryRefresh.serviceList[temp].linkInfo.linkList)
			delete[] directoryRefresh.serviceList[temp].linkInfo.linkList;

	if (directoryRefresh.serviceList)
		delete[] directoryRefresh.serviceList;

	if (rsslMsgBuffer && rsslMsgBuffer->data)
		free(rsslMsgBuffer->data);
}

DirectoryCache& DirectoryServiceStore::getDirectory()
{
	return _directoryCache;
}

RsslUInt64* DirectoryServiceStore::getServiceIdByName(const EmaString* pServiceName)
{
	return _serviceNameToServiceId.find(pServiceName);
}

const EmaString** DirectoryServiceStore::getServiceNameById(RsslUInt64 serviceId)
{
	return _serviceIdToServiceName.find(serviceId);
}

void DirectoryServiceStore::clearServiceNamePair()
{
	_serviceIdToServiceName.clear();
	_serviceNameToServiceId.clear();

	for (UInt32 idx = 0; idx < _serviceNameList.size(); ++idx)
		if (_serviceNameList[idx])
			delete _serviceNameList[idx];

	_serviceNameList.clear();
}

size_t DirectoryServiceStore::UInt64rHasher::operator()(const UInt64& value) const
{
	return value;
}

bool DirectoryServiceStore::UInt64Equal_To::operator()(const UInt64& x, const UInt64& y) const
{
	return x == y;
}

size_t DirectoryServiceStore::EmaStringPtrHasher::operator()(const EmaStringPtr& value) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)* s++;
	return result;
}

bool DirectoryServiceStore::EmaStringPtrEqual_To::operator()(const EmaStringPtr& x, const EmaStringPtr& y) const
{
	return *x == *y;
}

OmmIProviderDirectoryStore::OmmIProviderDirectoryStore(OmmIProviderImpl& ommIProviderImpl, OmmIProviderActiveConfig& ommIProviderActiveConfig) :
	DirectoryServiceStore(ommIProviderImpl.getProviderRole(), ommIProviderImpl, ommIProviderActiveConfig),
	_ommIProviderImpl(ommIProviderImpl),
	_ommIProviderActiveConfig(ommIProviderActiveConfig)
{

}

OmmIProviderDirectoryStore::~OmmIProviderDirectoryStore()
{

}

bool OmmIProviderDirectoryStore::checkExistingServiceId(RsslUInt64 serviceId, EmaString& errorText)
{
	if ( _ommIProviderActiveConfig.getDirectoryAdminControl() == OmmIProviderConfig::ApiControlEnum ) 
	{
		EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find(serviceId);

		if (pServiceNamePtr)
		{
			errorText.set("Attempt to add a service with name of ");
			errorText.append(**pServiceNamePtr).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
			return false;
		}

		return true;
	}
	else
	{
		return true;
	}
}

bool OmmIProviderDirectoryStore::addServiceIdAndNamePair(RsslUInt64 serviceId, const EmaString* pServiceName, EmaString* pErrorText)
{
	if (_ommIProviderActiveConfig.getDirectoryAdminControl() == OmmIProviderConfig::ApiControlEnum)
	{
		if (_serviceNameToServiceId.find(pServiceName))
		{
			if (pErrorText)
			{
				pErrorText->set("Attempt to add a service with name of ");
				pErrorText->append(*pServiceName).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
			}

			delete pServiceName;
			return false;
		}

		_serviceNameToServiceId.insert(pServiceName, serviceId);
		_serviceIdToServiceName.insert(serviceId, pServiceName);
		_serviceNameList.push_back(pServiceName);
	}
	else
	{
		if (_serviceNameToServiceId.find(pServiceName) == 0)
		{
			_serviceNameToServiceId.insert(pServiceName, serviceId);
			_serviceIdToServiceName.insert(serviceId, pServiceName);
			_serviceNameList.push_back(pServiceName);
		}
	}

	if ( pErrorText && OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
	{
		EmaString temp("Detected Service with name of ");
		temp.append(*pServiceName).append(" and Id of ").append(serviceId);
		_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
	}

	return true;
}

void OmmIProviderDirectoryStore::loadConfigDirectory(EmaConfigBaseImpl *pConfigImpl, bool loadDirectoryConfig )
{
	DirectoryCache* pDirectoryCache = 0;
	EmaList<ServiceDictionaryConfig*>* pServiceDictionaryConfigList = 0;

	if ( loadDirectoryConfig )
	{
		pDirectoryCache = &_directoryCache;
	}

	if (_ommIProviderActiveConfig.getDictionaryAdminControl() == OmmIProviderConfig::ApiControlEnum)
	{
		pServiceDictionaryConfigList = new EmaList<ServiceDictionaryConfig*>();
	}

	if (!pDirectoryCache && !pServiceDictionaryConfigList)
	{
		return;
	}
	
	DirectoryServiceStore::loadConfigDirectory(pDirectoryCache, pConfigImpl, pServiceDictionaryConfigList);

	if (pServiceDictionaryConfigList)
	{
		if (_bUsingDefaultService)
		{
			ServiceDictionaryConfig* serviceDictionaryConfig = new ServiceDictionaryConfig();
			serviceDictionaryConfig->serviceId = 1;

			DictionaryConfig* dictionaryProvided = new DictionaryConfig();
			dictionaryProvided->dictionaryType = Dictionary::FileDictionaryEnum;
			dictionaryProvided->rdmfieldDictionaryFileName = "./RDMFieldDictionary";
			dictionaryProvided->rdmFieldDictionaryItemName = "RWFFld";
			dictionaryProvided->enumtypeDefFileName = "./enumtype.def";
			dictionaryProvided->enumTypeDefItemName = "RWFEnum";
			serviceDictionaryConfig->addDictionaryProvided(dictionaryProvided);
			pServiceDictionaryConfigList->push_back(serviceDictionaryConfig);
		}

		_ommIProviderActiveConfig.setServiceDictionaryConfigList(*pServiceDictionaryConfigList);

		delete pServiceDictionaryConfigList;
	}
}

bool OmmIProviderDirectoryStore::IsAcceptingRequests(RsslUInt64 serviceId)
{
	_ommIProviderImpl.getUserMutex().lock();

	bool acceptingRequests = true;

	Service* pService = _directoryCache.getService(serviceId);

	if (pService)
	{
		if (pService->flags & RDM_DIRECTORY_SERVICE_STATE_FILTER)
		{
			if (pService->stateFilter.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS)
			{
				if (pService->stateFilter.acceptingRequests == 0)
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

	_ommIProviderImpl.getUserMutex().unlock();

	return acceptingRequests;
}

bool OmmIProviderDirectoryStore::IsValidQosRange(RsslUInt64 serviceId, RsslRequestMsg& rsslRequestMsg)
{
	_ommIProviderImpl.getUserMutex().lock();

	bool result = false;

	Service* pService = _directoryCache.getService(serviceId);

	if (pService)
	{
		if (pService->flags & RDM_DIRECTORY_SERVICE_INFO_FILTER)
		{
			if ((rsslRequestMsg.flags & RSSL_RQMF_HAS_QOS) &&
				(rsslRequestMsg.flags & RSSL_RQMF_HAS_WORST_QOS))
			{
				if (pService->infoFilter.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
				{
					if (pService->infoFilter.supportsQosRange != 0)
					{
						if (pService->infoFilter.flags & RDM_SVC_IFF_HAS_QOS)
						{
							EmaVector<RsslQos>& qosVector = pService->infoFilter.qos;

							for (UInt32 index = 0; qosVector.size(); index++)
							{
								if (rsslQosIsInRange(&rsslRequestMsg.qos, &rsslRequestMsg.worstQos, &qosVector[index]))
								{
									result = true;
									break;
								}
							}
						}
						else
						{
							RsslQos rsslQos;
							rsslClearQos(&rsslQos);
							rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
							rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;

							if (rsslQosIsInRange(&rsslRequestMsg.qos, &rsslRequestMsg.worstQos, &rsslQos))
							{
								result = true;
							}
						}
					}
				}
			}
			else if (rsslRequestMsg.flags & RSSL_RQMF_HAS_QOS)
			{
				if (pService->infoFilter.flags & RDM_SVC_IFF_HAS_QOS)
				{
					EmaVector<RsslQos>& qosVector = pService->infoFilter.qos;

					for (UInt32 index = 0; index < qosVector.size(); index++)
					{
						if (rsslQosIsEqual(&rsslRequestMsg.qos, &qosVector[index]))
						{
							result = true;
							break;
						}
					}
				}
				else
				{
					RsslQos rsslQos;
					rsslClearQos(&rsslQos);
					rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
					rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;

					if (rsslQosIsEqual(&rsslRequestMsg.qos, &rsslQos))
					{
						result = true;
					}
				}
			}
			else
			{
				RsslQos rsslQos;
				rsslClearQos(&rsslQos);
				rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
				rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;

				if (pService->infoFilter.flags & RDM_SVC_IFF_HAS_QOS)
				{
					EmaVector<RsslQos>& qosVector = pService->infoFilter.qos;

					for (UInt32 index = 0; index < qosVector.size(); index++)
					{
						if (rsslQosIsEqual(&rsslQos, &qosVector[index]))
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

	_ommIProviderImpl.getUserMutex().unlock();

	return result;
}

OmmNiProviderDirectoryStore::OmmNiProviderDirectoryStore(OmmNiProviderImpl& ommIProviderImpl, OmmNiProviderActiveConfig& ommIProviderActiveConfig) :
	DirectoryServiceStore(ommIProviderImpl.getProviderRole(), ommIProviderImpl, ommIProviderActiveConfig),
	_ommNiProviderImpl(ommIProviderImpl),
	_ommNiProviderActiveConfig(ommIProviderActiveConfig),
	_pDirectoryCacheApiControl(0)
{

}

OmmNiProviderDirectoryStore::~OmmNiProviderDirectoryStore()
{
	if (_pDirectoryCacheApiControl)
	{
		delete _pDirectoryCacheApiControl;
		_pDirectoryCacheApiControl = 0;
	}
}

bool OmmNiProviderDirectoryStore::checkExistingServiceId(RsslUInt64 serviceId, EmaString& errorText)
{
	EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find(serviceId);

	if (pServiceNamePtr)
	{
		errorText.set("Attempt to add a service with name of ");
		errorText.append(**pServiceNamePtr).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
		return false;
	}

	return true;
}

bool OmmNiProviderDirectoryStore::addServiceIdAndNamePair(RsslUInt64 serviceId, const EmaString* pServiceName, EmaString* pErrorText)
{
	if (_serviceNameToServiceId.find(pServiceName))
	{
		if (pErrorText)
		{
			pErrorText->set("Attempt to add a service with name of ");
			pErrorText->append(*pServiceName).append(" and id of ").append(serviceId).append(" while a service with the same id is already added.");
		}

		delete pServiceName;
		return false;
	}

	_serviceNameToServiceId.insert(pServiceName, serviceId);
	_serviceIdToServiceName.insert(serviceId, pServiceName);
	_serviceNameList.push_back(pServiceName);

	if ( pErrorText && OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
	{
		EmaString temp("Detected Service with name of ");
		temp.append(*pServiceName).append(" and Id of ").append(serviceId);
		_ommCommonImpl.getOmmLoggerClient().log(_baseConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
	}

	return true;
}

DirectoryCache& OmmNiProviderDirectoryStore::getApiControlDirectory()
{
	return *_pDirectoryCacheApiControl;
}

void OmmNiProviderDirectoryStore::loadConfigDirectory(EmaConfigBaseImpl *pConfigImpl)
{
	if (_ommNiProviderActiveConfig.getDirectoryAdminControl() == OmmNiProviderConfig::ApiControlEnum)
	{
		_pDirectoryCacheApiControl = new DirectoryCache();
	}

	if (!_pDirectoryCacheApiControl )
	{
		return;
	}

	DirectoryServiceStore::loadConfigDirectory(_pDirectoryCacheApiControl, pConfigImpl, 0);
}
