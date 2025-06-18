/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020, 2025 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelCallbackClient.h"
#include "ConsumerRoutingChannel.h"
#include "ConsumerRoutingSession.h"
#include "DirectoryCallbackClient.h"
#include "LoginCallbackClient.h"
#include "OmmConsumerClient.h"
#include "OmmConsumerErrorClient.h"
#include "OmmConsumerEvent.h"
#include "OmmConsumerImpl.h"
#include "ReqMsg.h"
#include "ReqMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "StaticDecoder.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

#include <new>

using namespace refinitiv::ema::access;

#define DEFAULT_DIRECTORY_RESP_MSG_SIZE 8192

const EmaString DirectoryCallbackClient::_clientName( "DirectoryCallbackClient" );

DirectoryDictionaryInfo::DirectoryDictionaryInfo() :
	name(),
	count(0)
{

}

DirectoryDictionaryInfo::~DirectoryDictionaryInfo()
{
}

DirectoryDictionaryInfo& DirectoryDictionaryInfo::operator=(const DirectoryDictionaryInfo& other)
{
	name = other.name;
	count = other.count;

	return *this;
}

DictionaryList::DictionaryList() :
	_toString(),
	_toStringSet( false )
{
}

DictionaryList::~DictionaryList()
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i] != NULL)
		{
			delete _list[i];
			_list[i] = NULL;
		}
	}
}

DictionaryList::DictionaryList( const DictionaryList& other ) :
	_toString(),
	_toStringSet( false )
{
	UInt32 size = other._list.size();

	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		DirectoryDictionaryInfo* pTmpInfo = new DirectoryDictionaryInfo();
		*pTmpInfo = *other._list[idx];
		_list.push_back(pTmpInfo);
	}
}

DictionaryList& DictionaryList::operator=( const DictionaryList& other )
{
	if ( this != &other )
	{
		for (UInt32 i = 0; i < _list.size(); ++i)
		{
			if (_list[i] != NULL)
			{
				delete _list[i];
				_list[i] = NULL;
			}
		}
		_list.clear();

		_toStringSet = false;

		UInt32 size = other._list.size();

		for ( UInt32 idx = 0; idx < size; ++idx )
		{
			DirectoryDictionaryInfo* pTmpInfo = new DirectoryDictionaryInfo();
			*pTmpInfo = *other._list[idx];
			_list.push_back(pTmpInfo);
		}
			
	}

	return *this;
}



DictionaryList& DictionaryList::clear()
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i] != NULL)
		{
			delete _list[i];
			_list[i] = NULL;
		}
	}

	_list.clear();
	_toStringSet = false;
	return *this;
}

DictionaryList& DictionaryList::addDictionary( const EmaString& dictionaryName )
{
	DirectoryDictionaryInfo* pDictInfo = new DirectoryDictionaryInfo();
	pDictInfo->name = dictionaryName;
	pDictInfo->count = 1;
	_list.push_back(pDictInfo);
	_toStringSet = false;
	return *this;
}

DirectoryDictionaryInfo* DictionaryList::findDictionary(const EmaString& dictionaryName)
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i]->name == dictionaryName)
		{
			return _list[i];
		}
	}

	return NULL;
}

Int64 DictionaryList::findDictionaryIndex(const EmaString& dictionaryName)
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i]->name == dictionaryName)
		{
			return i;
		}
	}

	return -1;
}

DictionaryList& DictionaryList::removeDictionary(const EmaString& dictionaryName)
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i]->name == dictionaryName && _list[i]->count != 0)
		{
			_list[i]->count--;
			if(_list[i]->count == 0)
				_toStringSet = false;
			return *this;
		}
	}
	return *this;
}


DictionaryList& DictionaryList::deleteDictionaryFromList(const EmaString& dictionaryName)
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i]->name == dictionaryName && _list[i]->count != 0)
		{
			_list[i]->count--;
			if (_list[i]->count == 0)
			{
				DirectoryDictionaryInfo* pInfo = _list[i];
				_toStringSet = false;
				_list.removePosition(i);

				delete pInfo;
			}

			return *this;
		}
	}
	return *this;
}

bool DictionaryList::hasDictionary(const EmaString& dictionaryName)
{
	for (UInt32 i = 0; i < _list.size(); ++i)
	{
		if (_list[i]->name == dictionaryName && _list[i]->count != 0)
		{
			return true;
		}
	}

	return false;
}

const EmaVector<DirectoryDictionaryInfo*>& DictionaryList::getDictionaryList() const
{
	return _list;
}

const EmaString& DictionaryList::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		_toString.set( "'" );

		UInt32 size = _list.size();

		for ( UInt32 idx = 0; idx < size; ++idx )
			_toString.append( _list[idx]->name ).append( " " );

		_toString.append( "' " );
	}

	return _toString;
}

DirectoryDomainType::DirectoryDomainType()
{
	domain = 0;
	count = 0;
}

DirectoryDomainType::~DirectoryDomainType()
{
}

int DirectoryDomainType::compare(DirectoryDomainType*& first, DirectoryDomainType*& second)
{
	if (first->domain < second->domain)
		return -1;
	else if (first->domain == second->domain)
		return 0;
	else 
		return 1;
}

DirectoryQoS::DirectoryQoS()
{
	rsslClearQos(&qos);
	count = 0;
}


DirectoryQoS::~DirectoryQoS()
{
}

int DirectoryQoS::compare(DirectoryQoS*& first, DirectoryQoS*& second)
{
	if (rsslQosIsBetter(&first->qos, &second->qos))
		return -1;
	else if (rsslQosIsEqual(&first->qos, &second->qos))
		return 0;
	else
		return 1;
}


Directory& Directory::addDomain(UInt32 newDomain)
{
	DirectoryDomainType* domain = new DirectoryDomainType();
	domain->domain = newDomain;
	domain->count = 1;
	_supportedDomains.insert_sorted(domain, DirectoryDomainType::compare);
	return *this;
}

DirectoryDomainType* Directory::findDomain(UInt32 domain)
{
	DirectoryDomainType newDomainType;
	newDomainType.domain = domain;

	Int64 found = _supportedDomains.search(const_cast<DirectoryDomainType*>(& newDomainType), DirectoryDomainType::compare);
	if (found == -1)
	{
		return NULL;
	}
	else
	{
		return _supportedDomains[(UInt32)found];
	}
}

Int64 Directory::findDomainIndex(UInt32 domain)
{
	DirectoryDomainType newDomainType;
	newDomainType.domain = domain;

	return _supportedDomains.search(const_cast<DirectoryDomainType*>(&newDomainType), DirectoryDomainType::compare);	
}

Directory& Directory::addQos(RsslQos* newQos)
{
	DirectoryQoS* qos = new DirectoryQoS();
	qos->qos = *newQos;
	qos->count = 1;

	_supportedQos.insert_sorted(qos, DirectoryQoS::compare);
	return *this;

}

DirectoryQoS* Directory::findQos(RsslQos* qos)
{
	DirectoryQoS newQos;
	newQos.qos = *qos;

	Int64 found = _supportedQos.search(const_cast<DirectoryQoS*>(&newQos), DirectoryQoS::compare);

	if (found == -1)
	{
		return NULL;
	}
	else
	{
		return _supportedQos[(UInt32)found];
	}
}

Int64 Directory::findQosIndex(RsslQos* qos)
{
	DirectoryQoS newQos;
	newQos.qos = *qos;

	return _supportedQos.search(const_cast<DirectoryQoS*>(&newQos), DirectoryQoS::compare);
}

Directory* Directory::create( OmmBaseImpl& ommBaseImpl )
{
	try
	{
		return new Directory(ommBaseImpl);
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee("Failed to create Directory.");
	}

	return NULL;
}

void Directory::destroy( Directory*& pDirectory )
{
	if ( pDirectory )
	{
		delete pDirectory;
		pDirectory = 0;
	}
}

Directory::Directory(OmmBaseImpl& ommBaseImpl) :
	_ommBaseImpl(ommBaseImpl),
	_name(),
	_toString(),
	_id( 0 ),
	_toStringSet( false ),
	_pChannel( 0 ),
	_markDeleted(false),
	_dictionariesProvided(),
	_dictionariesUsed(),
	_qosList(NULL),
	_qosCount(0),
	_capabilitiesList(NULL),
	_capabilitiesCount(0),
	_supportedDomains(),
	_supportedQos(),
	_dataBuffer(),
	_serviceAllocated(false),
	_dictionariesProvidedList(NULL),
	_dictionariesUsedList(NULL),
	_dictionariesProvidedCount(0),
	_dictionariesUsedCount(0),
	_hasGeneratedServiceId(false),
	_generatedServiceId(0)
{
	rsslClearRDMService(&_service);
}

Directory::~Directory()
{
	for (UInt32 i = 0; i < _supportedDomains.size(); ++i)
	{
		if (_supportedDomains[i] != NULL)
		{
			delete _supportedDomains[i];
			_supportedDomains[i] = NULL;
		}
	}
	_supportedDomains.clear();
	for (UInt32 i = 0; i < _supportedQos.size(); ++i)
	{
		if (_supportedQos[i] != NULL)
		{
			delete _supportedQos[i];
			_supportedQos[i] = NULL;
		}
	}
	_supportedQos.clear();

	if (_qosList != NULL)
	{
		delete[] _qosList;
		_qosList = NULL;
	}

	if (_capabilitiesList != NULL)
	{
		delete[] _capabilitiesList;
		_capabilitiesList = NULL;
	}

	_dictionariesProvided.clear();
	if (_dictionariesProvidedList != NULL)
	{
		delete[] _dictionariesProvidedList;
		_dictionariesProvidedList = NULL;
	}

	_dictionariesUsed.clear();
	if (_dictionariesUsedList != NULL)
	{
		delete[] _dictionariesUsedList;
		_dictionariesUsedList = NULL;
	}
	_dataBuffer.clear();
}

Directory& Directory::clear()
{
	_name.clear();
	_id = 0;
	_toStringSet = false;
	_pChannel = 0;
	_markDeleted = false;
	rsslClearRDMService(&_service);
	for (UInt32 i = 0; i < _supportedDomains.size(); ++i)
	{
		if (_supportedDomains[i] != NULL)
		{
			delete _supportedDomains[i];
			_supportedDomains[i] = NULL;
		}
	}
	_supportedDomains.clear();
	for (UInt32 i = 0; i < _supportedQos.size(); ++i)
	{
		if (_supportedQos[i] != NULL)
		{
			delete _supportedQos[i];
			_supportedQos[i] = NULL;
		}
	}
	_supportedQos.clear();

	if (_qosList != NULL)
	{
		delete[] _qosList;
		_qosList = NULL;
	}

	if (_capabilitiesList != NULL)
	{
		delete[] _capabilitiesList;
		_capabilitiesList = NULL;
	}

	_dictionariesProvided.clear();
	if (_dictionariesProvidedList != NULL)
	{
		delete[] _dictionariesProvidedList;
		_dictionariesProvidedList = NULL;
	}

	_dictionariesUsed.clear();
	if (_dictionariesUsedList != NULL)
{
		delete[] _dictionariesUsedList;
		_dictionariesUsedList = NULL;
	}

	_dataBuffer.clear();
	return *this;
}

Directory& Directory::markDeleted()
{
	_toStringSet = false;
	_markDeleted = true;
	return *this;
}

Directory& Directory::markActive()
{
	_toStringSet = false;
	_markDeleted = false;
	return *this;
}

void Directory::setGeneratedServiceId(UInt64 serviceId)
{
	_hasGeneratedServiceId = true;
	_generatedServiceId = serviceId;
}

bool Directory::hasGeneratedServiceId() const
{
	return _hasGeneratedServiceId;
}

UInt64 Directory::getGeneratedServiceId() const
{
	return _generatedServiceId;
}

bool Directory::isDeleted() const
{
	return _markDeleted;
}

RsslRDMService* Directory::getService()
{
	if (_serviceAllocated == false)
	{
		// The list may be smaller if dictionaries were removed in an update.
		if (_dictionariesProvidedList != NULL)
		{
			delete[] _dictionariesProvidedList;
			_dictionariesProvidedList = NULL;
			_dictionariesProvidedCount = 0;
		}

		if (_dictionariesProvided.getDictionaryList().size() != 0)
		{
			_dictionariesProvidedList = new RsslBuffer[_dictionariesProvided.getDictionaryList().size()];
			_dictionariesProvidedCount = 0;

			for (UInt32 i = 0; i < _dictionariesProvided.getDictionaryList().size(); ++i)
			{
				DirectoryDictionaryInfo* pDictInfo = _dictionariesProvided.getDictionaryList()[i];

				if (pDictInfo->count != 0)
				{
					_dictionariesProvidedList[_dictionariesProvidedCount].data = (char*)pDictInfo->name.c_str();
					_dictionariesProvidedList[_dictionariesProvidedCount].length = pDictInfo->name.length();
					++_dictionariesProvidedCount;
				}
			}

			if (_dictionariesProvidedCount == 0)
			{
				delete[] _dictionariesProvidedList;
				_dictionariesProvidedList = NULL;
			}

			_service.info.dictionariesProvidedList = _dictionariesProvidedList;
			_service.info.dictionariesProvidedCount = _dictionariesProvidedCount;
		}

		if (_dictionariesUsedList != NULL)
		{
			delete[] _dictionariesUsedList;
			_dictionariesUsedList = NULL;
			_dictionariesUsedCount = 0;
		}

		if (_dictionariesUsed.getDictionaryList().size() != 0)
			{
			_dictionariesUsedList = new RsslBuffer[_dictionariesUsed.getDictionaryList().size()];
			_dictionariesUsedCount = 0;

			for (UInt32 i = 0; i < _dictionariesUsed.getDictionaryList().size(); ++i)
			{
				DirectoryDictionaryInfo* pDictInfo = _dictionariesUsed.getDictionaryList()[i];

				if (pDictInfo->count != 0)
				{
					_dictionariesUsedList[_dictionariesUsedCount].data = (char*)pDictInfo->name.c_str();
					_dictionariesUsedList[_dictionariesUsedCount].length = pDictInfo->name.length();
					++_dictionariesUsedCount;
				}
			}

			if (_dictionariesUsedCount == 0)
			{
				delete[] _dictionariesUsedList;
				_dictionariesUsedList = NULL;
			}

			_service.info.dictionariesUsedList = _dictionariesUsedList;
			_service.info.dictionariesUsedCount = _dictionariesUsedCount;
		}

		if (_qosList != NULL)
		{
			delete[] _qosList;
			_qosList = NULL;
			_qosCount = 0;
		}

		if (_supportedQos.size() != 0)
		{
			// The list may be smaller if qos were removed in an update.
			_qosList = new RsslQos[_supportedQos.size()];
			_qosCount = 0;

			for (UInt32 i = 0; i < _supportedQos.size(); ++i)
			{
				DirectoryQoS* pQosInfo = _supportedQos[i];

				if (pQosInfo->count != 0)
				{
					_qosList[_qosCount] = pQosInfo->qos;
					++_qosCount;
				}
			}

			if (_qosCount == 0)
			{
				delete[] _qosList;
				_qosList = NULL;
			}

			_service.info.qosList = _qosList;
			_service.info.qosCount = _qosCount;
		}


		if (_capabilitiesList != NULL)
		{
			delete[] _capabilitiesList;
			_capabilitiesList = NULL;
			_capabilitiesCount = 0		;
		}

		if (_supportedDomains.size() != 0)
		{
			// The list may be smaller if capabilities were removed in an update.
			_capabilitiesList = new RsslUInt[_supportedDomains.size()];
			_capabilitiesCount = 0;

			for (UInt32 i = 0; i < _supportedDomains.size(); ++i)
			{
				DirectoryDomainType* pDomain = _supportedDomains[i];

				if (pDomain->count != 0)
				{
					_capabilitiesList[_capabilitiesCount] = pDomain->domain;
					++_capabilitiesCount;
				}
			}

			if (_capabilitiesCount == 0)
			{
				delete[] _capabilitiesList;
				_capabilitiesList = NULL;
			}

			_service.info.capabilitiesList = _capabilitiesList;
			_service.info.capabilitiesCount = _capabilitiesCount;
		}


		if (_dataBuffer.length() != 0)
		{
			_service.data.data.data = (char*)_dataBuffer.c_buf();
			_service.data.data.length = _dataBuffer.length();

		}

		_serviceAllocated = true;
	}

	return &_service;
}

Directory& Directory::populateService(RsslRDMService* pService)
{
	if (_serviceAllocated == false)
		getService();

	*pService = _service;

	return *this;
}

bool Directory::serviceMatch(RsslRequestMsg* pReqMsg)
{
	DirectoryDomainType currentDomain;

	if (_service.state.serviceState == 0 || _service.state.acceptingRequests == 0)
		return false;

	currentDomain.domain = pReqMsg->msgBase.domainType;
	if (_supportedDomains.search(&currentDomain, DirectoryDomainType::compare) == -1)
		return false;

	if (pReqMsg->flags & RSSL_RQMF_HAS_QOS)
	{
		if ((pReqMsg->flags & RSSL_RQMF_HAS_WORST_QOS) == 0)
		{
			// Single Qos, check to see if the qos matches
			DirectoryQoS currentQos;
			currentQos.qos = pReqMsg->qos;

			// If the current qos is found, return true
			if (_supportedQos.search(&currentQos, DirectoryQoS::compare) != -1)
			{
				return true;
			}

			return false;
		}
		else
		{
			// Iterate through the supported qos, and the request is in range, return true.
			for (UInt32 i = 0; i < _supportedQos.size(); ++i)
			{
				// If the current qos is found, return true
				if (rsslQosIsInRange(&pReqMsg->qos, &pReqMsg->worstQos, &_supportedQos[i]->qos))
				{
					return true;
				}
			}
			return false;
		}
	}

	return true;
}

bool Directory::setService(RsslRDMService* pService)
{
	EmaString tempName(pService->info.serviceName.data, pService->info.serviceName.length);

	clear();
	_serviceAllocated = false;

	// Shallow copy the service, we will update the buffers after this.
	_service = *pService;

	if (pService->flags & RDM_SVCF_HAS_INFO)
	{
		_name.set(pService->info.serviceName.data, pService->info.serviceName.length);
		_service.info.serviceName.data = (char*)_name.c_str();
		_service.info.serviceName.length = _name.length();


		if (pService->info.flags & RDM_SVC_IFF_HAS_VENDOR)
		{
			_vendor.set(pService->info.vendor.data, pService->info.vendor.length);
			_service.info.vendor.data = (char*)_vendor.c_str();
			_service.info.vendor.length = _vendor.length();
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST)
		{
			_itemList.set(pService->info.itemList.data, pService->info.itemList.length);
			_service.info.itemList.data = (char*)_itemList.c_str();
			_service.info.itemList.length = _itemList.length();
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_IS_SOURCE)
			_service.info.isSource = pService->info.isSource;

		if (pService->info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
		{
			for (UInt32 idx = 0; idx < pService->info.dictionariesProvidedCount; ++idx)
			{
				_dictionariesProvided.addDictionary(EmaString(pService->info.dictionariesProvidedList[idx].data,
					pService->info.dictionariesProvidedList[idx].length));
			}

			// Set the dictionaries provided list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.dictionariesProvidedList = NULL;
			_service.info.dictionariesProvidedCount = 0;
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_DICTS_USED)
		{
			for (UInt32 idx = 0; idx < pService->info.dictionariesUsedCount; ++idx)
			{
				_dictionariesUsed.addDictionary(EmaString(pService->info.dictionariesUsedList[idx].data,
					pService->info.dictionariesUsedList[idx].length));
			}
			// Set the dictionaries used list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.dictionariesUsedList = NULL;
			_service.info.dictionariesUsedCount = 0;
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_QOS)
		{
			for (UInt32 idx = 0; idx < pService->info.qosCount; ++idx)
			{
				addQos(&pService->info.qosList[idx]);
				
			}

			// Set the qos list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.qosList = NULL;
			_service.info.qosCount = 0;

		}

		for (UInt32 idx = 0; idx < pService->info.capabilitiesCount; ++idx)
		{
			addDomain((UInt32)pService->info.capabilitiesList[idx]);

		}

		// Set the domain list here.  When this is re-encoded, it will be malloc'd
		_service.info.capabilitiesList = NULL;
		_service.info.capabilitiesCount = 0;
		}
		else
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			_ommBaseImpl.getOmmLoggerClient().log("DirectoryCallbackClient", OmmLoggerClient::ErrorEnum, "Received RsslRDMService with Add action but no Service Info");
		}
		return false;
	}

	if (pService->flags & RDM_SVCF_HAS_STATE)
	{
		if (pService->state.flags & RDM_SVC_STF_HAS_STATUS)
		{
			_stateText.set(pService->state.status.text.data, pService->state.status.text.length);
			_service.state.status.text.data = (char*)_stateText.c_str();
			_service.state.status.text.length = _stateText.length();
		}
	}

	if (pService->flags & RDM_SVCF_HAS_DATA)
	{
		if (pService->state.flags & RDM_SVC_STF_HAS_STATUS)
		{
			_stateText.set(pService->state.status.text.data, pService->state.status.text.length);
			_service.state.status.text.data = (char*)_stateText.c_str();
			_service.state.status.text.length = _stateText.length();
		}
	}

	if (pService->flags & RDM_SVCF_HAS_DATA)
	{
		if (pService->data.flags & RDM_SVC_DTF_HAS_DATA)
		{
			_dataBuffer.setFrom(pService->data.data.data, pService->data.data.length);
		}
	}

	// Set the Group State info nothing, as this is not used in the current consumer,
	// and is suppressed in Request Routing
	_service.groupStateCount = 0;
	_service.groupStateList = NULL;

	return true;

}

// Update the service information here.  
bool Directory::updateService(RsslRDMService* pService)
{
	_serviceAllocated = false;
	if (pService->flags & RDM_SVCF_HAS_INFO)
	{
		if (pService->info.flags & RDM_SVC_IFF_HAS_VENDOR)
	{
			_vendor.set(pService->info.vendor.data, pService->info.vendor.length);
			_service.info.vendor.data = (char*)_vendor.c_str();
			_service.info.vendor.length = _vendor.length();
	}

		if (pService->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST)
		{
			_itemList.set(pService->info.itemList.data, pService->info.itemList.length);
			_service.info.itemList.data = (char*)_itemList.c_str();
			_service.info.itemList.length = _itemList.length();
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_IS_SOURCE)
			_service.info.isSource = pService->info.isSource;

		if (pService->info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
		{
			_service.info.dictionariesProvidedCount = 0;

			// First, iterate through to see if there are any new ones.  If they were in the supported qos list but count = 0, increment them.
			for (UInt32 idx = 0; idx < pService->info.dictionariesProvidedCount; ++idx)
			{
				DirectoryDictionaryInfo* foundDictionary = _dictionariesProvided.findDictionary(EmaString(pService->info.dictionariesProvidedList[idx].data, pService->info.dictionariesProvidedList[idx].length));
				if (foundDictionary == NULL)
				{
					_dictionariesProvided.addDictionary(EmaString(pService->info.dictionariesProvidedList[idx].data,
						pService->info.dictionariesProvidedList[idx].length));
				}
				else if (foundDictionary->count == 0)
				{
					foundDictionary->count++;
			}
		}

			EmaVector<bool> foundList(_dictionariesProvided.getDictionaryList().size());

			// Initialize the find list.
			for (UInt32 idx = 0; idx < _dictionariesProvided.getDictionaryList().size(); ++idx)
			{	
				foundList.push_back(false);
			}

			// Now, iterate through, and mark the ones found
			for (UInt32 idx = 0; idx < pService->info.dictionariesProvidedCount; ++idx)
			{
				Int64 foundIndex = _dictionariesProvided.findDictionaryIndex(EmaString(pService->info.dictionariesProvidedList[idx].data, pService->info.dictionariesProvidedList[idx].length));
				
				if(foundIndex != -1)
					foundList[(UInt32)foundIndex] = true;
			}

			// Decrement the count of the not found ones.  These will be cleaned up whenever it gets aggregated.
			for (UInt32 idx = 0; idx < _dictionariesProvided.getDictionaryList().size(); ++idx)
			{
				if (foundList[idx] == false)
				{
					if (_dictionariesProvided.getDictionaryList()[idx]->count != 0)
					{
						--_dictionariesProvided.getDictionaryList()[idx]->count;
					}
				}
			}

			// Set the dictionaries provided list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.dictionariesProvidedList = NULL;
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_DICTS_USED)
		{
			// First, iterate through to see if there are any new ones.  If they were in the supported qos list but count = 0, increment them.
			for (UInt32 idx = 0; idx < pService->info.dictionariesUsedCount; ++idx)
			{
				DirectoryDictionaryInfo* foundDictionary = _dictionariesUsed.findDictionary(EmaString(pService->info.dictionariesUsedList[idx].data, pService->info.dictionariesUsedList[idx].length));
				if (foundDictionary == NULL)
				{
					_dictionariesUsed.addDictionary(EmaString(pService->info.dictionariesUsedList[idx].data,
						pService->info.dictionariesUsedList[idx].length));
				}
				else if (foundDictionary->count == 0)
				{
					foundDictionary->count++;
				}
			}

			EmaVector<bool> foundList(_dictionariesUsed.getDictionaryList().size());

			// Initialize the find list.
			for (UInt32 idx = 0; idx < _dictionariesUsed.getDictionaryList().size(); ++idx)
			{
				foundList.push_back(false);
			}

			// Now, iterate through, and mark the ones found
			for (UInt32 idx = 0; idx < pService->info.dictionariesUsedCount; ++idx)
			{
				Int64 foundIndex = _dictionariesUsed.findDictionaryIndex(EmaString(pService->info.dictionariesUsedList[idx].data, pService->info.dictionariesUsedList[idx].length));

				if(foundIndex != -1)
					foundList[(UInt32)foundIndex] = true;
			}

			// Decrement the count of the not found ones.  These will be cleaned up whenever it gets aggregated.
			for (UInt32 idx = 0; idx < _dictionariesUsed.getDictionaryList().size(); ++idx)
			{
				if (foundList[idx] == false)
				{
					if (_dictionariesUsed.getDictionaryList()[idx]->count != 0)
					{
						--_dictionariesUsed.getDictionaryList()[idx]->count;
					}
				}
			}

			// Set the dictionaries provided list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.dictionariesUsedList = NULL;
			_service.info.dictionariesUsedCount = 0;
		}

		if (pService->info.flags & RDM_SVC_IFF_HAS_QOS)
		{
			// First, iterate through to see if there are any new ones.  If they were in the supported qos list but count = 0, increment them.
			for (UInt32 idx = 0; idx < pService->info.qosCount; ++idx)
			{
				DirectoryQoS* foundQos = findQos(&pService->info.qosList[idx]);
				if (foundQos == NULL)
				{
					addQos(&pService->info.qosList[idx]);
				}
				else if (foundQos->count == 0)
				{
					foundQos->count++;
				}
			}

			EmaVector<bool> foundList(_supportedQos.size());

			// Initialize the find list.
			for (UInt32 idx = 0; idx < _supportedQos.size(); ++idx)
			{
				foundList.push_back(false);
			}

			// Now, iterate through, and mark the ones found
			for (UInt32 idx = 0; idx < pService->info.qosCount; ++idx)
			{
				Int64 foundIndex = findQosIndex(&pService->info.qosList[idx]);
				if(foundIndex != -1)
					foundList[(UInt32)foundIndex] = true;
			}

			// Decrement the count of the not found ones
			for (UInt32 idx = 0; idx < _supportedQos.size(); ++idx)
			{
				if (foundList[idx] == false)
				{
					if (_supportedQos[idx]->count != 0)
					{
						--_supportedQos[idx]->count;
					}
				}
			}

			// Set the qos list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.qosList = NULL;
			_service.info.qosCount = 0;
		}

		for (UInt32 idx = 0; idx < pService->info.capabilitiesCount; ++idx)
		{
			// First, iterate through to see if there are any new ones.  If they were in the supported qos list but count = 0, increment them.
			for (UInt32 idx = 0; idx < pService->info.capabilitiesCount; ++idx)
			{
				DirectoryDomainType* foundDomain = findDomain((UInt32)pService->info.capabilitiesList[idx]);
				if (foundDomain == NULL)
				{
					addDomain((UInt32)pService->info.capabilitiesList[idx]);
				}
				else if (foundDomain->count == 0)
				{
					foundDomain->count++;
				}
			}

			EmaVector<bool> foundList(_supportedDomains.size());

			// Initialize the find list.
			for (UInt32 idx = 0; idx < _supportedDomains.size(); ++idx)
			{
				foundList.push_back(false);
			}

			// Now, iterate through, and mark the ones found
			for (UInt32 idx = 0; idx < pService->info.capabilitiesCount; ++idx)
			{
				Int64 foundIndex = findDomainIndex((UInt32)pService->info.capabilitiesList[idx]);
				if(foundIndex != -1)
					foundList[(UInt32)foundIndex] = true;
			}

			// Decrement the count of the not found ones
			for (UInt32 idx = 0; idx < _supportedDomains.size(); ++idx)
			{
				if (foundList[idx] == false)
				{
					if (_supportedDomains[idx]->count != 0)
					{
						--_supportedDomains[idx]->count;
					}
				}
			}

			// Set the qos list to NULL here.  When this is re-encoded, it will be malloc'd
			_service.info.capabilitiesList = NULL;
			_service.info.capabilitiesCount = 0;
		}
	}

	if (pService->flags & RDM_SVCF_HAS_STATE)
	{
		_service.state = pService->state;

		if (pService->state.flags & RDM_SVC_STF_HAS_STATUS)
		{
			_stateText.set(pService->state.status.text.data, pService->state.status.text.length);
			_service.state.status.text.data = (char*)_stateText.c_str();
			_service.state.status.text.length = _stateText.length();
		}
	}

	if (pService->flags & RDM_SVCF_HAS_DATA)
	{
		if (pService->data.flags & RDM_SVC_DTF_HAS_DATA)
		{
			_dataBuffer.setFrom(pService->data.data.data, pService->data.data.length);
		}
	}

	return true;
}

const EmaString& Directory::getName() const
{
	return _name;
}

Directory& Directory::setName( const EmaString& name )
{
	_toStringSet = false;
	_name = name;
	return *this;
}

DictionaryList& Directory::getDictionariesUsed()
{
	return _dictionariesUsed;
}

DictionaryList& Directory::getDictionariesProvided()
{
	return _dictionariesProvided;
}


UInt64 Directory::getId() const
{
	return _id;
}

Directory& Directory::setId( UInt64 id )
{
	_id = id;
	_toStringSet = false;
	return *this;
}

Channel* Directory::getChannel() const
{
	return _pChannel;
}

UInt64	Directory::getServiceState()
{
	return _service.state.serviceState;
}

UInt64	Directory::getAcceptingRequests()
{
	return _service.state.acceptingRequests;
}

Directory& Directory::setChannel( Channel* pChannel )
{
	_toStringSet = false;
	_pChannel = pChannel;
	return *this;
}

DirectoryCallbackClient::DirectoryCallbackClient( OmmBaseImpl& ommBaseImpl ) :
	_directoryByIdHt( ommBaseImpl.getActiveConfig().serviceCountHint ),
	_directoryByNameHt( ommBaseImpl.getActiveConfig().serviceCountHint ),
	_ommBaseImpl( ommBaseImpl ),
	_refreshMsg(),
	_updateMsg(),
	_statusMsg(),
	_genericMsg(),
	fanoutDirectory(false)
{
	rsslClearRDMDirectoryRequest(&_directoryRequest);
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created DirectoryCallbackClient" );
	}

	if (ommBaseImpl.getConsumerRoutingSession() != NULL)
	{
		_requestList = ItemList::create(ommBaseImpl);
	}
	else
		_requestList = NULL;
}

DirectoryCallbackClient::~DirectoryCallbackClient()
{
	Directory* directory = _directoryList.front();

	while ( directory )
	{
		removeDirectory( directory );
		directory = _directoryList.front();
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed DirectoryCallbackClient" );
	}

	if (_requestList != NULL)
	{
		ItemList::destroy(_requestList);
		_requestList = NULL;
	}
}

DirectoryCallbackClient* DirectoryCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	try
	{
		return new DirectoryCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc& )
	{
		const char* temp = "Failed to create DirectoryCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

	return NULL;
}

void DirectoryCallbackClient::destroy( DirectoryCallbackClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void DirectoryCallbackClient::initialize()
{
	rsslClearRDMDirectoryRequest( &_directoryRequest );

	_directoryRequest.rdmMsgBase.streamId = 2;

	if ( !_ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg )
	{
		_directoryRequest.filter = ( RDM_DIRECTORY_SERVICE_INFO_FILTER
		                             | RDM_DIRECTORY_SERVICE_STATE_FILTER
		                             | RDM_DIRECTORY_SERVICE_GROUP_FILTER
		                             | RDM_DIRECTORY_SERVICE_LOAD_FILTER
		                             | RDM_DIRECTORY_SERVICE_DATA_FILTER
		                             | RDM_DIRECTORY_SERVICE_LINK_FILTER );

		_directoryRequest.flags = RDM_DR_RQF_STREAMING;
	}
	else
	{
		if ( _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER )
			_directoryRequest.filter = _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.filter;
		else
		{
			if ( _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity <= OmmLoggerClient::WarningEnum )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum,
				                                       "Configured source directory request message contains no filter. Will request all filters" );
			}

			_directoryRequest.filter = ( RDM_DIRECTORY_SERVICE_INFO_FILTER
			                             | RDM_DIRECTORY_SERVICE_STATE_FILTER
			                             | RDM_DIRECTORY_SERVICE_GROUP_FILTER
			                             | RDM_DIRECTORY_SERVICE_LOAD_FILTER
			                             | RDM_DIRECTORY_SERVICE_DATA_FILTER
			                             | RDM_DIRECTORY_SERVICE_LINK_FILTER );
		}

		if ( _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
		{
			_directoryRequest.serviceId = _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.serviceId;
			_directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
		}

		if ( !( _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->flags & RSSL_RQMF_STREAMING ) )
		{
			if ( _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity <= OmmLoggerClient::WarningEnum )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum,
				                                       "Configured source directory request message contains no streaming flag. Will request streaming" );
			}
		}

		_directoryRequest.flags = RDM_DR_RQF_STREAMING;
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "RDMDirectoryRequest message was populated with Filter(s)" );
		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_INFO_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_INFO_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_STATE_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_STATE_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_GROUP_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_LOAD_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_LOAD_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_DATA_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_DATA_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_LINK_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_LINK_FILTER" );

		if ( _directoryRequest.flags & RDM_DR_RQF_HAS_SERVICE_ID )
			temp.append( CR ).append( "requesting serviceId " ).append( _directoryRequest.serviceId );
		else
			temp.append( CR ).append( "requesting all services" );

		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

RsslRDMDirectoryRequest* DirectoryCallbackClient::getDirectoryRequest()
{
	return &_directoryRequest;
}

RsslReactorCallbackRet DirectoryCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDirectoryMsgEvent* pEvent )
{
	RsslRDMDirectoryMsg* pDirectoryMsg = pEvent->pRDMDirectoryMsg;

	Channel* pChannel = ((Channel*)pRsslReactorChannel->userSpecPtr);
	if (pChannel->getParentChannel() != NULL)
	{
		pChannel = pChannel->getParentChannel();
	}

	if ( !pDirectoryMsg )
	{
		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->baseMsgEvent.pErrorInfo;

			EmaString temp( "Received event without RDMDirectory message" );
			temp.append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pError->rsslError.rsslErrorId ? pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	// Only send this if it's a direct request.  Otherwise, we will fan out any updates as needed.
	if (pChannel->getConsumerRoutingChannel() == NULL && pEvent && pEvent->baseMsgEvent.pStreamInfo && pEvent->baseMsgEvent.pStreamInfo->pUserSpec)
	{
		SingleItem* pItem = (SingleItem*)pEvent->baseMsgEvent.pStreamInfo->pUserSpec;

		pItem->setEventChannel(pRsslReactorChannel);

		return processCallback(pRsslReactor, pRsslReactorChannel, pEvent, pItem);
	}
	else
	{
		switch ( pDirectoryMsg->rdmMsgBase.rdmMsgType )
		{
		case RDM_DR_MT_REFRESH:
		{
			RsslState* pState = &pDirectoryMsg->refresh.state;
	
			if ( pState->streamState != RSSL_STREAM_OPEN )
			{
	
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );
	
					EmaString temp( "RDMDirectory stream was closed with refresh message " );
					temp.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
	
				// The watchlist will have processed all of the services it knows about into DELETE actions, so this will perform the full aggregation and fanout if necessary.
				processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pChannel);
	
				// Close the channel here.
				if (pChannel->getConsumerRoutingChannel() == NULL)
				{
					_ommBaseImpl.unsetActiveRsslReactorChannel(pChannel);
					_ommBaseImpl.closeChannel(pRsslReactorChannel);
				}
				else
				{
					_ommBaseImpl.closeChannel(pRsslReactorChannel);
				}
	
				break;
			}
			else if ( pState->dataState == RSSL_DATA_SUSPECT )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );
	
					EmaString temp( "RDMDirectory stream state was changed to suspect with refresh message " );
					temp.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}
	
				if (pChannel->getConsumerRoutingChannel() == NULL)
					_ommBaseImpl.setState(OmmBaseImpl::DirectoryStreamOpenSuspectEnum);
				else
				{
					pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::DirectoryStreamOpenSuspectEnum;
					int loginSuspectCount = 0;
					ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;
					// Check to see if all of the channels are in a DirectoryStreamOpenSuspectEnum status or have been closed.  
					// If they are all in that status, set the ommbaseimpl state to DirectoryStreamOpenSuspectEnum so it can transition to failure after this.
					if (_ommBaseImpl.isInitialized() == false)
					{
						for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
						{
							if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::DirectoryStreamOpenSuspectEnum)
								loginSuspectCount++;
						}

						if (loginSuspectCount == pSession->activeChannelCount)
						{
							_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenSuspectEnum );
						}
					}
				}
	
				// The watchlist will have processed all of the services it knows about into DELETE actions, so this will perform the full aggregation and fanout if necessary.
				processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pChannel);
				break;
			}
	
			if (pChannel->getConsumerRoutingChannel() == NULL)
				_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenOkEnum );
			else
			{
				pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::DirectoryStreamOpenOkEnum;
				int directoryOkCount = 0;
				ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;
				// Check to see if all of the channels are in a DirectoryStreamOpenSuspectEnum status or have been closed.  
				// If they are all in that status, set the ommbaseimpl state to DirectoryStreamOpenSuspectEnum so it can transition to failure after this.
				if (_ommBaseImpl.isInitialized() == false)
				{
					for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
					{
						if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::DirectoryStreamOpenOkEnum)
							directoryOkCount++;
					}

					if (directoryOkCount == pSession->activeChannelCount)
					{
						_ommBaseImpl.setState(OmmBaseImpl::DirectoryStreamOpenOkEnum);
					}
				}
			}
	
			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );
	
				EmaString temp( "RDMDirectory stream was open with refresh message " );
				temp.append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
	
			processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pChannel);
			break;
		}
		case RDM_DR_MT_STATUS:
		{
			if ( pDirectoryMsg->status.flags & RDM_DR_STF_HAS_STATE )
			{
				RsslState* pState = &pDirectoryMsg->status.state;
	
				if ( pState->streamState != RSSL_STREAM_OPEN )
				{
					// Close the channel here.
					if (pChannel->getConsumerRoutingChannel() == NULL)
					{
						_ommBaseImpl.unsetActiveRsslReactorChannel(pChannel);
						_ommBaseImpl.closeChannel( pRsslReactorChannel );
					}
					else
					{
						_ommBaseImpl.closeChannel(pRsslReactorChannel);
					}
	
					if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString tempState( 0, 256 );
						stateToString( pState, tempState );
	
						EmaString temp( "RDMDirectory stream was closed with status message " );
						temp.append( tempState );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
					}
					break;
				}
				else if ( pState->dataState == RSSL_DATA_SUSPECT )
				{
					if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString tempState( 0, 256 );
						stateToString( pState, tempState );
	
						EmaString temp( "RDMDirectory stream state was changed to suspect with status message " );
						temp.append( tempState );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
					}
	
					if (pChannel->getConsumerRoutingChannel() == NULL)
						_ommBaseImpl.setState(OmmBaseImpl::DirectoryStreamOpenSuspectEnum);
					else
					{
						pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::DirectoryStreamOpenSuspectEnum;
						int directorySuspectCount = 0;
						ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;
						// Check to see if all of the channels are in a DirectoryStreamOpenSuspectEnum status or have been closed.  
						// If they are all in that status, set the ommbaseimpl state to DirectoryStreamOpenSuspectEnum so it can transition to failure after this.
						if (_ommBaseImpl.isInitialized() == false)
						{
							for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
							{
								if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::DirectoryStreamOpenSuspectEnum)
									directorySuspectCount++;
							}

							if (directorySuspectCount == pSession->activeChannelCount)
							{
								_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenSuspectEnum );
							}
						}
					}
					break;
				}
	
				if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );
	
					EmaString temp( "RDMDirectory stream was open with status message " );
					temp.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
				}
	
				if (pChannel->getConsumerRoutingChannel() == NULL)
					_ommBaseImpl.setState(OmmBaseImpl::DirectoryStreamOpenOkEnum);
				else
				{
					pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::DirectoryStreamOpenOkEnum;
					int directoryOkCount = 0;
					ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;
					// Check to see if all of the channels are in a DirectoryStreamOpenOkEnum status or have been closed.  
					// If they are all in that status, set the ommbaseimpl state to DirectoryStreamOpenOkEnum so it can transition to failure after this.
					if (_ommBaseImpl.isInitialized() == false)
					{
						for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
						{
							if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::DirectoryStreamOpenOkEnum)
								directoryOkCount++;
						}

						if (directoryOkCount == pSession->activeChannelCount)
						{
							_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenOkEnum );
						}
					}
				}
			}
			else
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, "Received RDMDirectory status message without the state" );
				}
			}
			break;
		}
		case RDM_DR_MT_UPDATE:
		{
			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Received RDMDirectory update message" );
			}
	
			processDirectoryPayload( pDirectoryMsg->update.serviceCount, pDirectoryMsg->update.serviceList, pChannel);
			break;
		}
		default:
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unknown RDMDirectory message type" );
				temp.append( CR )
				.append( "message type value " )
				.append( pDirectoryMsg->rdmMsgBase.rdmMsgType );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			break;
		}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void DirectoryCallbackClient::processDirectoryPayload( UInt32 count, RsslRDMService* pServiceList, void* userSpecPtr )
{
	Channel* pChannel = (Channel*)userSpecPtr;
	bool sendUpdate = false;
	if ( !pServiceList && count )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Received RDMDirectory message indicating a number of services but without a service list" );
		}
		return;
	}

	if ( !userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Internal error: no RsslReactorChannel->userSpecPtr" );
		}
		return;
	}

	for ( UInt32 jdx = 0; jdx < count; ++jdx )
	{
		switch ( pServiceList[jdx].action )
		{
		case RSSL_MPEA_ADD_ENTRY :
		{
			EmaString tempName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );
			DirectoryPtr* pDeletedDirectoryPtr = NULL;
			Channel* oldChannel = NULL;

			Directory* pDirectory = 0;

			if (pChannel->getConsumerRoutingChannel() != NULL)
			{
				pDeletedDirectoryPtr = pChannel->getConsumerRoutingChannel()->serviceByName.find(&tempName);
			}
			else
			{
				pDeletedDirectoryPtr = _directoryByNameHt.find(&tempName);
			}

			if ( !pDeletedDirectoryPtr )
				pDirectory = Directory::create( _ommBaseImpl );
			else
			{
				pDirectory = *pDeletedDirectoryPtr;
				oldChannel = (Channel*)pDirectory->getChannel();
				pDirectory->clear();
			}

			if (pDirectory->setService(&pServiceList[jdx]) == false)
			{
				// This has failed, remove it from the lists.
				if (pDeletedDirectoryPtr)
				{
					if (pChannel->getConsumerRoutingChannel() != NULL)
					{
						pChannel->getConsumerRoutingChannel()->serviceList.remove(*pDeletedDirectoryPtr);
						pChannel->getConsumerRoutingChannel()->serviceByName.erase(&(*pDeletedDirectoryPtr)->getName());
						pChannel->getConsumerRoutingChannel()->serviceById.erase((UInt16)(*pDeletedDirectoryPtr)->getId());
					}
					else
					{
						_directoryList.remove(*pDeletedDirectoryPtr);
						_directoryByNameHt.erase(&(*pDeletedDirectoryPtr)->getName());
						_directoryByIdHt.erase((*pDeletedDirectoryPtr)->getId());
					}
				}

				Directory::destroy( pDirectory );

				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Received RsslRDMService with Add action but no Service Info" );
				}
				break;
			}

			if (pDeletedDirectoryPtr)
			{
				if (oldChannel != ((Channel*)userSpecPtr))
				{
					static_cast<Channel*>(userSpecPtr)->setDictionary(oldChannel->getDictionary());
					(*pDeletedDirectoryPtr)->setChannel((Channel*)userSpecPtr);
				}
				else
				{
					(*pDeletedDirectoryPtr)->setChannel((Channel*)oldChannel);
				}

				if ( pDirectory->getId() != pServiceList[jdx].serviceId )
				{
					if (pChannel->getConsumerRoutingChannel() != NULL)
					{
						pChannel->getConsumerRoutingChannel()->serviceById.erase((UInt16)pDirectory->getId());
						pDirectory->setId(pServiceList[jdx].serviceId);
						pChannel->getConsumerRoutingChannel()->serviceById.insert((UInt16)pDirectory->getId(), pDirectory);

						if (_ommBaseImpl.isInitialized() == true && _ommBaseImpl.getConsumerRoutingSession()->aggregateDirectory(pDirectory, RSSL_MPEA_ADD_ENTRY) == true)
						{
							sendUpdate = true;
						}
					}
					else
					{
						_directoryByIdHt.erase( pDirectory->getId() );
						pDirectory->setId( pServiceList[jdx].serviceId );
						_directoryByIdHt.insert( pServiceList[jdx].serviceId, pDirectory );
					}
				}
			}
			else
			{
				pDirectory->setChannel((Channel*)userSpecPtr);
				pDirectory->setId(pServiceList[jdx].serviceId);

				if (pChannel->getConsumerRoutingChannel() != NULL)
				{
					pChannel->getConsumerRoutingChannel()->serviceList.push_back(pDirectory);
					pChannel->getConsumerRoutingChannel()->serviceByName.insert(&pDirectory->getName(), pDirectory);
					pChannel->getConsumerRoutingChannel()->serviceById.insert((UInt16)pDirectory->getId(), pDirectory);

					// This is a brand new service, so send an update if aggregating this makes it change the directory cache in a material way
					if (_ommBaseImpl.isInitialized() == true && _ommBaseImpl.getConsumerRoutingSession()->aggregateDirectory(pDirectory, RSSL_MPEA_ADD_ENTRY) == true)
					{
						sendUpdate = true;
					}

					// Set the dictionary here if the ommConsumer has already been initialized.
					// Otherwise, this will get set in OmmConsumerImpl.loadDirectory().
					if(_ommBaseImpl.isInitialized() == true)
					{
						pChannel->setDictionary(_ommBaseImpl.getDictionaryCallbackClient().getDefaultDictionary());
					}
				}
				else
				{
						addDirectory(pDirectory);
				}
			}

			// If the current directory is accepting requests and there are pending requests, send them now.
			if (_ommBaseImpl.isInitialized() == true && pChannel->getConsumerRoutingChannel() != NULL && pDirectory->getAcceptingRequests() == 1)
			{
				ConsumerRoutingService* pRoutingService = (ConsumerRoutingService*)*(pChannel->getConsumerRoutingChannel()->serviceByName.find(&pDirectory->getName()));

				EmaList<Item*>& pendingItems = pChannel->getConsumerRoutingChannel()->pRoutingSession->pendingRequestList.getList();

				int itemCount = pendingItems.size();

				for (int i = 0; i < itemCount; i++)
				{
					SingleItem* pSingleItem = (SingleItem*)pendingItems.pop_front();

					if (pSingleItem == NULL)
						break;

					pSingleItem->setItemList(NULL);

					// Check to see if the pending item matches the current directory
					if (pSingleItem->getServiceListName().length() != 0)
					{
						ServiceList** pServiceListPtr = _ommBaseImpl.getActiveConfig().serviceListByName.find(&(pSingleItem->getServiceListName()));

						EmaVector<EmaString>& concreteServiceList = (*pServiceListPtr)->concreteServiceList();

						bool foundCurrentService = false;

						// Iterate through the concrete service list, if there is a match, set the directory here and continue.
						for (UInt32 i = 0; i < concreteServiceList.size(); ++i)
						{
							if (concreteServiceList[i] == pDirectory->getName())
							{
								foundCurrentService = true;
								break;
							}
						}

						if (foundCurrentService == false)
						{
							// No match for any of the services, so push this item to the back of the list and continue to the next item.
							pendingItems.push_back((Item*)pSingleItem);
							continue;
						}
					}
					else
					{
						if (pSingleItem->getServiceName() != pDirectory->getName())
						{
							pendingItems.push_back((Item*)pSingleItem);
							continue;
						}
					}

					// Match the service here.  If it matches, re-submit the request to the connection associated with pDirectory
					if (pDirectory->serviceMatch(pSingleItem->getReqMsg()))
					{
						if (pSingleItem->sessionChannel != NULL)
						{
							pSingleItem->sendClose();
						}
						pSingleItem->setDirectory(pDirectory);
						pSingleItem->sessionChannel = pChannel->getConsumerRoutingChannel();
						pChannel->getConsumerRoutingChannel()->routedRequestList.addItem(pSingleItem);

						// This will call submit, which will re-set the service ID based on the newly set directory.
						pSingleItem->reSubmit(false);
					}
					else
					{
						pendingItems.push_back((Item*)pSingleItem);
					}
				}
			}

			break;
		}
		case RSSL_MPEA_UPDATE_ENTRY :
		{
			DirectoryPtr* pDirectoryPtr = NULL;

			if (pChannel->getConsumerRoutingChannel() != NULL)
			{
				pDirectoryPtr = pChannel->getConsumerRoutingChannel()->serviceById.find((UInt16)pServiceList[jdx].serviceId);
			}
			else
			{
				pDirectoryPtr = _directoryByIdHt.find(pServiceList[jdx].serviceId);
			}

			if ( !pDirectoryPtr )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received Update action for unknown RsslRDMService with service id " );
					temp.append( pServiceList[jdx].serviceId );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				break;
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Update action for RsslRDMService" );
				temp.append( CR )
				.append( "Service name " ).append( ( *pDirectoryPtr )->getName() ).append( CR )
				.append( "Service id " ).append( pServiceList[ jdx ].serviceId );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			(*pDirectoryPtr)->updateService(&pServiceList[jdx]);

			if ( ( *pDirectoryPtr )->getChannel() != ( ( Channel* ) userSpecPtr ) )
			{
				static_cast<Channel*>( userSpecPtr )->setDictionary( ( *pDirectoryPtr )->getChannel()->getDictionary() );
				( *pDirectoryPtr )->setChannel( ( Channel* ) userSpecPtr );
			}
			Directory* pDirectory = *pDirectoryPtr;

			if (pChannel->getConsumerRoutingChannel() != NULL)
			{
				if (_ommBaseImpl.getConsumerRoutingSession()->aggregateDirectory(pDirectory, RSSL_MPEA_UPDATE_ENTRY) == true)
				{
					sendUpdate = true;
				}

				ConsumerRoutingService* pRoutingService = (ConsumerRoutingService*)*(pChannel->getConsumerRoutingChannel()->serviceByName.find(&pDirectory->getName()));

				if(_ommBaseImpl.isInitialized() == true)
				{
					// If the current reactor channel is up and exists, and the service state is down(0), reroute any requests on this channel for this specific service
					if (pChannel->getConsumerRoutingChannel()->inPreferredHost == false && _ommBaseImpl.getConsumerRoutingSession()->enhancedItemRecovery == true && pDirectory->getServiceState() == 0 && pChannel->getConsumerRoutingChannel()->pReactorChannel->pRsslChannel != NULL && pChannel->getConsumerRoutingChannel()->pReactorChannel->pRsslChannel->state == RSSL_CH_STATE_ACTIVE )
					{
						EmaList<Item*>& channelItemList = pChannel->getConsumerRoutingChannel()->routedRequestList.getList();

						int itemCount = channelItemList.size();

						for (int i = 0; i < itemCount; i++)
						{
							SingleItem* pSingleItem = (SingleItem*)channelItemList.pop_front();

							if (pSingleItem == NULL)
								break;

							// If the item's directory is the same as this directory, re-route it, otherwise put the item back into the list
							if (((pSingleItem->getReqMsg()->flags & RSSL_RQMF_PRIVATE_STREAM) == 0) && pSingleItem->getDirectory() == pDirectory)
							{
								pSingleItem->sendClose();
								pSingleItem->setItemList(NULL);
								pSingleItem->reSubmit(true);
							}
							else
							{
								channelItemList.push_back((Item*)pSingleItem);
							}
						}

						// Go through the pending list, as well, because we may have gotten an OPEN/SUSPECT status, so the item was moved to the pending list
						EmaList<Item*>& pendingItemList = pChannel->getConsumerRoutingChannel()->pRoutingSession->pendingRequestList.getList();

						itemCount = pendingItemList.size();

						for (int i = 0; i < itemCount; i++)
						{
							SingleItem* pSingleItem = (SingleItem*)pendingItemList.pop_front();

							if (pSingleItem == NULL)
								break;

							// If the item's directory is the same as this directory, re-route it, otherwise put the item back into the list
							if (pSingleItem->sessionChannel == pChannel->getConsumerRoutingChannel() && ((pSingleItem->getReqMsg()->flags & RSSL_RQMF_PRIVATE_STREAM) == 0) && pSingleItem->getDirectory() == pDirectory)
							{
								pSingleItem->sendClose();
								pSingleItem->setItemList(NULL);
								pSingleItem->reSubmit(true);
							}
							else
							{
								pendingItemList.push_back((Item*)pSingleItem);
							}
						}
					}

					// If the current directory is active and accepting requests, and there are pending requests that can match this service, send them now.
					// All items should have gotten a OPEN/SUSPECT status from the underlying watchlist previous to this
					if (pDirectory->getServiceState() == 1 && pDirectory->getAcceptingRequests() == 1)
					{
						EmaList<Item*>& pendingItems = pChannel->getConsumerRoutingChannel()->pRoutingSession->pendingRequestList.getList();

						int itemCount = pendingItems.size();

						for (int i = 0; i < itemCount; i++)
						{
							SingleItem* pSingleItem = (SingleItem*)pendingItems.pop_front();

							if (pSingleItem == NULL)
								break;

							pSingleItem->setItemList(NULL);

							// Check to see if the pending item matches the current directory
							if (pSingleItem->getServiceListName().length() != 0)
							{
								ServiceList** pServiceListPtr = _ommBaseImpl.getActiveConfig().serviceListByName.find(&(pSingleItem->getServiceListName()));

								EmaVector<EmaString>& concreteServiceList = (*pServiceListPtr)->concreteServiceList();

								bool foundCurrentService = false;

								// Iterate through the concrete service list, if there is a match, set the directory here and continue.
								for (UInt32 i = 0; i < concreteServiceList.size(); ++i)
								{
									if (concreteServiceList[i] == pDirectory->getName())
									{
										foundCurrentService = true;
										break;
									}
								}

								if (foundCurrentService == false)
								{
									// No match for any of the services, so push this item to the back of the list and continue to the next item.
									pendingItems.push_back((Item*)pSingleItem);
									continue;
								}
							}
							else
							{
								if (pSingleItem->getServiceName() != pDirectory->getName())
								{
									pendingItems.push_back((Item*)pSingleItem);
									continue;
								}
							}

							// Match the service here.  If it matches, re-submit the request to the connection associated with pDirectory
							if (pDirectory->serviceMatch(pSingleItem->getReqMsg()))
							{
								if (pSingleItem->sessionChannel != NULL)
								{
									pSingleItem->sendClose();
								}
								pSingleItem->setDirectory(pDirectory);
								pSingleItem->sessionChannel = pChannel->getConsumerRoutingChannel();
								pChannel->getConsumerRoutingChannel()->routedRequestList.addItem(pSingleItem);

								// This will call submit, which will re-set the service ID based on the newly set directory.
								pSingleItem->reSubmit(false);
							}
							else
							{
								pendingItems.push_back((Item*)pSingleItem);
							}
						}
					}
				}
			}

			break;
		}
		case RSSL_MPEA_DELETE_ENTRY :
		{
			DirectoryPtr* pDirectoryPtr = NULL;

			if (pChannel->getConsumerRoutingChannel() != NULL)
			{
				pDirectoryPtr = pChannel->getConsumerRoutingChannel()->serviceById.find((UInt16)pServiceList[jdx].serviceId);
			}
			else
			{
				pDirectoryPtr = _directoryByIdHt.find(pServiceList[jdx].serviceId);
			}

			if ( !pDirectoryPtr )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received Delete action for unknown RsslRDMService with service id " );
					temp.append( pServiceList[jdx].serviceId );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				break;
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Delete action for RsslRDMService" );
				temp.append( CR )
				.append( "Service name " ).append( ( *pDirectoryPtr )->getName() ).append( CR )
				.append( "Service id " ).append( pServiceList[jdx].serviceId );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			( *pDirectoryPtr )->markDeleted();

			Directory* pDirectory = *pDirectoryPtr;

			if (_ommBaseImpl.isInitialized() == true && pChannel->getConsumerRoutingChannel() != NULL)
			{
				pDirectory->getService()->state.serviceState = 0;
				pDirectory->getService()->state.acceptingRequests = 0;

				if (_ommBaseImpl.getConsumerRoutingSession()->aggregateDirectory(pDirectory, RSSL_MPEA_DELETE_ENTRY) == true)
				{
					sendUpdate = true;
				}
			}
			// Note: We do not need to deal with re-routing the requests here, because the reactor will fan out a CLOSED_RECOVER status for all items associated with this service whenever a DELETE action is received.
			break;
		}
		default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unknown action for RsslRDMService. Action value " );
				temp.append( pServiceList[jdx].action );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			break;
		}
		}
	}
	
	// Only update if things have changed materially.  Also, this only applies to request routing connections.
	// Non RR connections were fanned out because they had an attached item.
	if(_ommBaseImpl.isInitialized() == true && pChannel->getConsumerRoutingChannel() != NULL && sendUpdate == true)
		fanoutAllDirectoryRequests((void*)&_ommBaseImpl);
}

size_t DirectoryCallbackClient::UInt64rHasher::operator()( const UInt64& value ) const
{
	return value;
}

bool DirectoryCallbackClient::UInt64Equal_To::operator()( const UInt64& x, const UInt64& y ) const
{
	return x == y ? true : false;
}

size_t DirectoryCallbackClient::EmaStringPtrHasher::operator()( const EmaStringPtr& value ) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while ( n-- )
		result = ( ( result % magic ) << 8 ) + ( size_t ) * s++;
	return result;
}

bool DirectoryCallbackClient::EmaStringPtrEqual_To::operator()( const EmaStringPtr& x, const EmaStringPtr& y ) const
{
	return *x == *y;
}

void DirectoryCallbackClient::addDirectory( Directory* pDirectory )
{
	_directoryList.push_back( pDirectory );

	_directoryByIdHt.insert( pDirectory->getId(), pDirectory );

	_directoryByNameHt.insert( &pDirectory->getName(), pDirectory);

	if ( _ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum ||
		(pDirectory->getAcceptingRequests() == 1 && pDirectory->getServiceState() == 1))
		_ommBaseImpl.getDictionaryCallbackClient().downloadDictionary( *pDirectory );
}

void DirectoryCallbackClient::removeDirectory( Directory* pDirectory )
{
	_directoryByNameHt.erase( &pDirectory->getName() );

	_directoryByIdHt.erase( pDirectory->getId() );

	_directoryList.remove( pDirectory );

	Directory::destroy( pDirectory );
}

const Directory* DirectoryCallbackClient::getDirectory( const EmaString& name ) const
{
	DirectoryPtr* pDirectoryPtr = _directoryByNameHt.find( &name );

	return pDirectoryPtr ? *pDirectoryPtr : 0;
}

const Directory* DirectoryCallbackClient::getDirectory( UInt32 id ) const
{
	UInt64 id64 = id;
	DirectoryPtr* pDirectoryPtr = _directoryByIdHt.find( id64 );

	return pDirectoryPtr ? *pDirectoryPtr : 0;
}


void DirectoryCallbackClient::addItem(Item* pItem)
{
	_requestList->addItem(pItem);
}

void DirectoryCallbackClient::removeItem(Item* pItem)
{
	_requestList->removeItem(pItem);
}

void DirectoryCallbackClient::fanoutAllDirectoryRequests(void* info)
{
	OmmBaseImpl* pBaseImpl = (OmmBaseImpl*)info;
	ConsumerRoutingSession* pRoutingSession = pBaseImpl->getConsumerRoutingSession();
	DirectoryCallbackClient& callbackClient = pBaseImpl->getDirectoryCallbackClient();

	DirectoryItem* pItem = NULL;

	pItem = (DirectoryItem*)callbackClient._requestList->getList().front();
	UInt32 count = 0;

	if (pItem == NULL)
		return;

	while (count < callbackClient._requestList->getList().size())
	{
		RsslRDMDirectoryUpdate directoryUpdate;
		RsslUpdateMsg updateMsg;
		RsslRet retCode;
		RsslBuffer rsslMsgBuffer;
		RsslEncIterator eIter;
		rsslClearUpdateMsg(&updateMsg);
		rsslClearEncodeIterator(&eIter);

		RsslRDMService* pService = NULL;
		count++;

		if (allocateAndSetEncodeIteratorBuffer(pBaseImpl, &rsslMsgBuffer, DEFAULT_DIRECTORY_RESP_MSG_SIZE, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
			&eIter, "DirectoryCallbackClient::fanoutAllDirectoryRequests") != RSSL_RET_SUCCESS)
		{
			return;
		}

		rsslClearRDMDirectoryUpdate(&directoryUpdate);

		directoryUpdate.filter = pItem->filter;
		directoryUpdate.rdmMsgBase.streamId = pItem->getStreamId();

		if (pItem->serviceName.length() != 0)
		{
			pService = (RsslRDMService*)malloc(sizeof(RsslRDMService));

			if (pService == NULL)
			{
				EmaString text("Internal error. Unable to allocate services");
				if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
					pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
						text.c_str());
				return;
			}

			rsslClearRDMService(pService);

			directoryUpdate.serviceCount = 1;

			if (pRoutingSession->addedServiceList.getPositionOf(pItem->serviceName) != -1)
			{
				ConsumerRoutingService** pRoutingServicePtr = pRoutingSession->serviceByName.find(&pItem->serviceName);

				if (pRoutingServicePtr == NULL)
				{
					EmaString text("Internal error. Unable to find added service in list in fanoutAllDirectoryRequests");
					if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
							text.c_str());
					free((void*)pService);
					return;
				}

				// Populate the service with the information
				(*pRoutingServicePtr)->populateService(pService);

				pService->action = RSSL_MPEA_ADD_ENTRY;
				pService->serviceId = (*pRoutingServicePtr)->getId();
				directoryUpdate.serviceList = pService;
			}
			else if (pRoutingSession->updatedServiceList.getPositionOf(pItem->serviceName) != -1)
			{
				ConsumerRoutingService** pRoutingServicePtr = pRoutingSession->serviceByName.find(&pItem->serviceName);

				if (pRoutingServicePtr == NULL)
				{
					EmaString text("Internal error. Unable to find updated service in list fanoutAllDirectoryRequests");
					if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
							text.c_str());
					free((void*)pService);
					return;
				}

				// Populate the service with the information
				(*pRoutingServicePtr)->populateService(pService);

				// Set the action to update
				pService->action = RSSL_MPEA_UPDATE_ENTRY;
				pService->serviceId = (*pRoutingServicePtr)->getId();
				directoryUpdate.serviceList = pService;
			}
			else if (pRoutingSession->deletedServiceList.getPositionOf(pItem->serviceName) != -1)
			{
				ConsumerRoutingService** pRoutingServicePtr = pRoutingSession->serviceByName.find(&pItem->serviceName);

				if (pRoutingServicePtr == NULL)
				{
					EmaString text("Internal error. Missing deleted service in list in fanoutAllDirectoryRequests");
					if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
							text.c_str());
					free((void*)pService);
					return;
				}

				// Set the action to delete
				pService->action = RSSL_MPEA_DELETE_ENTRY;
				pService->serviceId = (*pRoutingServicePtr)->getId();
				directoryUpdate.serviceList = pService;
			}
			else
			{
				free((void*)pService);
				pService = NULL;
				// This request does not match any of the changed values, so just continue with the loop.
				pItem = (DirectoryItem*)pItem->next();

				if (pItem == NULL)
					break;
				else
					continue;
			}
		}
		else
		{
			int serviceArraySize = pRoutingSession->addedServiceList.size() + pRoutingSession->updatedServiceList.size() + pRoutingSession->deletedServiceList.size();
			
			pService = (RsslRDMService*)malloc(sizeof(RsslRDMService)*serviceArraySize);

			if (pService == NULL)
			{
				EmaString text("Internal error. Unable to allocate services");
				if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
					pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
						text.c_str());
				return;
			}

			for(int i = 0; i < serviceArraySize; ++i)
				rsslClearRDMService(&pService[i]);

			directoryUpdate.serviceCount = 1;

			int serviceIter = 0;

			for(UInt32 i = 0; i < pRoutingSession->addedServiceList.size(); i++)
			{
				// Get the aggregated service id here.
				ConsumerRoutingService** pRoutingServicePtr = pRoutingSession->serviceByName.find(&pRoutingSession->addedServiceList[i]);

				if (pRoutingServicePtr == NULL)
				{
					EmaString text("Internal error. Missing new added service in fanoutAllDirectoryRequests");
					if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
							text.c_str());
					free((void*)pService);
					return;
				}

				// Populate the service with the information
				(*pRoutingServicePtr)->populateService(&pService[serviceIter]);
				pService[serviceIter].action = RSSL_MPEA_ADD_ENTRY;
				pService[serviceIter].serviceId = (*pRoutingServicePtr)->getId();
				serviceIter++;
			}

			for (UInt32 i = 0; i < pRoutingSession->updatedServiceList.size(); i++)
			{
				// Get the aggregated service id here.
				ConsumerRoutingService** pRoutingServicePtr = pRoutingSession->serviceByName.find(&pRoutingSession->updatedServiceList[i]);

				if (pRoutingServicePtr == NULL)
				{
					EmaString text("Internal error. Missing updated service in fanoutAllDirectoryRequests");
					if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
							text.c_str());
					free((void*)pService);
					return;
				}

				// Populate the service with the information
				(*pRoutingServicePtr)->populateService(&pService[serviceIter]);

				// Set the action to update
				pService[serviceIter].action = RSSL_MPEA_UPDATE_ENTRY;
				pService[serviceIter].serviceId = (*pRoutingServicePtr)->getId();
				serviceIter++;
			}

			for (UInt32 i = 0; i < pRoutingSession->deletedServiceList.size(); i++)
			{
				// Get the aggregated service id here.
				ConsumerRoutingService** pRoutingServicePtr = pRoutingSession->serviceByName.find(&pRoutingSession->deletedServiceList[i]);

				if (pRoutingServicePtr == NULL)
				{
					EmaString text("Internal error. Missing deleted service in fanoutAllDirectoryRequests");
					if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
						pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
							text.c_str());
					free((void*)pService);
					return;
				}

				// Set the action to delete
				pService[serviceIter].action = RSSL_MPEA_DELETE_ENTRY;
				pService[serviceIter].serviceId = (*pRoutingServicePtr)->getId();
				serviceIter++;
			}

			directoryUpdate.serviceCount = serviceIter;
			directoryUpdate.serviceList = pService;
		} 

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);
		retCode = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryUpdate, &rsslMsgBuffer.length, &rsslErrorInfo);

		while (retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			free(rsslMsgBuffer.data);

			rsslClearEncodeIterator(&eIter);
			if (allocateAndSetEncodeIteratorBuffer(pBaseImpl, &rsslMsgBuffer, DEFAULT_DIRECTORY_RESP_MSG_SIZE, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
				&eIter, "DirectoryCallbackClient::fanoutAllDirectoryRequests") != RSSL_RET_SUCCESS)
			{
				free((void*)pService);
				return;
			}

			clearRsslErrorInfo(&rsslErrorInfo);
			retCode = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryUpdate, &rsslMsgBuffer.length, &rsslErrorInfo);
		}

		RsslDecodeIterator dIter;
		rsslClearDecodeIterator(&dIter);
		/* set version info */
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		rsslSetDecodeIteratorBuffer(&dIter, &rsslMsgBuffer);

		
		if (rsslDecodeMsg(&dIter, (RsslMsg*)&updateMsg) != RSSL_RET_SUCCESS)
		{
			EmaString text("Internal error. Unable to decode generated message in  ");
			text.append("fanoutAllDirectoryRequests");
			if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
				pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
					text.c_str());
			free((void*)pService);
			return;
		}

		StaticDecoder::setRsslData(&callbackClient._updateMsg, &rsslMsgBuffer, RSSL_DT_MSG,
			(int)RSSL_RWF_MAJOR_VERSION, (int)RSSL_RWF_MINOR_VERSION, 0);

		pBaseImpl->msgDispatched();
		// Since this is generated, set the event channel to NULL.
		pItem->setEventChannel(NULL);
		pItem->onAllMsg(callbackClient._updateMsg);
		pItem->onUpdateMsg(callbackClient._updateMsg);

		free((void*)pService);
		pService = NULL;

		pItem = (DirectoryItem*)pItem->next();

		if (pItem == NULL)
			break;
	}

	// Clear out the service lists in pRoutingSession
	pRoutingSession->deletedServiceList.clear();
	pRoutingSession->addedServiceList.clear();
	pRoutingSession->updatedServiceList.clear();
}

// This takes in a DirectoryItem pointer, and will fan out a single refresh message containing all services that have an attached channel to that item. 
// This is for request routing only.
void DirectoryCallbackClient::fanoutSingleDirectoryRequest(void* info)
{
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRefreshMsg refreshMsg;
	RsslRet retCode;
	RsslBuffer rsslMsgBuffer = RSSL_INIT_BUFFER;
	RsslEncIterator eIter;

	rsslClearRefreshMsg(&refreshMsg);
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	rsslClearEncodeIterator(&eIter);

	RsslRDMService* pService;

	DirectoryItem* pItem = (DirectoryItem*)info;

	OmmBaseImpl* pBaseImpl = &pItem->getImpl();
	ConsumerRoutingSession* pRoutingSession = pBaseImpl->getConsumerRoutingSession();
	DirectoryCallbackClient& callbackClient = pBaseImpl->getDirectoryCallbackClient();

	int serviceArraySize = pRoutingSession->serviceList.size();
	pService = new RsslRDMService[serviceArraySize];

	for (int i = 0; i < serviceArraySize; ++i)
		rsslClearRDMService(&pService[i]);

	int serviceIter = 0;
	ConsumerRoutingService* pRoutingService = (ConsumerRoutingService*)pRoutingSession->serviceList.front();
	for(UInt32 i = 0; i < pRoutingSession->serviceList.size(); i++)
	{
		if (pRoutingService == NULL)
		{
			break;
		}

		if (pRoutingService->activeServiceCount == 0)
		{
			// This is a deleted service that isn't present on anything, so do not give it to the user.
			pRoutingService = (ConsumerRoutingService*)pRoutingService->next();
			continue;
		}
	
		if (pItem->serviceName.empty() || pItem->serviceName == pRoutingService->getName())
		{
			pRoutingService->populateService(&pService[serviceIter]);
			pService[serviceIter].action = RSSL_MPEA_ADD_ENTRY;
			pService[serviceIter].serviceId = pRoutingService->getId();
			pService[serviceIter].flags = pItem->filter & pService[serviceIter].flags;
			serviceIter++;
		}

		pRoutingService = (ConsumerRoutingService*)pRoutingService->next();
	} 

	directoryRefresh.serviceCount = serviceIter;
	directoryRefresh.serviceList = pService;
	directoryRefresh.filter = pItem->filter;
	directoryRefresh.rdmMsgBase.streamId = pItem->getStreamId();
	directoryRefresh.flags |= RDM_DR_RFF_SOLICITED;

	if (!pItem->serviceName.empty())
	{
		directoryRefresh.flags |= RDM_DR_RFF_HAS_SERVICE_ID;
		directoryRefresh.serviceId = (RsslUInt16)pItem->serviceId;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	retCode = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo);

	while (retCode == RSSL_RET_BUFFER_TOO_SMALL)
	{
		free(rsslMsgBuffer.data);

		rsslClearEncodeIterator(&eIter);
		if (allocateAndSetEncodeIteratorBuffer(pBaseImpl, &rsslMsgBuffer, DEFAULT_DIRECTORY_RESP_MSG_SIZE, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
			&eIter, "DirectoryCallbackClient::fanoutAllDirectoryRequests") != RSSL_RET_SUCCESS)
		{
			return;
		}

		clearRsslErrorInfo(&rsslErrorInfo);
		retCode = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo);
	}

	RsslDecodeIterator dIter;
	rsslClearDecodeIterator(&dIter);
	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	rsslSetDecodeIteratorBuffer(&dIter, &rsslMsgBuffer);


	if (rsslDecodeMsg(&dIter, (RsslMsg*)&refreshMsg) != RSSL_RET_SUCCESS)
	{
		EmaString text("Internal error. Unable to decode generated message in  ");
		text.append("fanoutAllDirectoryRequests");
		if (OmmLoggerClient::ErrorEnum >= pBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity)
			pBaseImpl->getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
				text.c_str());
		return;
	}

	StaticDecoder::setRsslData(&callbackClient._refreshMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		(int)RSSL_RWF_MAJOR_VERSION, (int)RSSL_RWF_MINOR_VERSION, 0);

	pBaseImpl->msgDispatched();
	pItem->onAllMsg(callbackClient._refreshMsg);
	pItem->onRefreshMsg(callbackClient._refreshMsg);

	delete[] pService;
	pService = NULL;

	pItem->sentRefresh = true;
}

// This is in response to an explititly sent directory request
RsslReactorCallbackRet DirectoryCallbackClient::processCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDirectoryMsgEvent* pEvent, SingleItem* pItem )
{
	RsslRet retCode;
	RsslBuffer rsslMsgBuffer;
	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );

	pItem->setEventChannel((void*)pRsslReactorChannel);

	if ( allocateAndSetEncodeIteratorBuffer(&_ommBaseImpl, &rsslMsgBuffer, DEFAULT_DIRECTORY_RESP_MSG_SIZE, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		&eIter, "DirectoryCallbackClient::processCallback" ) != RSSL_RET_SUCCESS )
	{
		return RSSL_RC_CRET_SUCCESS;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	retCode = rsslEncodeRDMDirectoryMsg( &eIter, pEvent->pRDMDirectoryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( rsslMsgBuffer.data );

		rsslClearEncodeIterator( &eIter );
		if ( allocateAndSetEncodeIteratorBuffer(&_ommBaseImpl, &rsslMsgBuffer, rsslMsgBuffer.length * 2, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
			&eIter, "DirectoryCallbackClient::processCallback" ) != RSSL_RET_SUCCESS )
		{
			return RSSL_RC_CRET_SUCCESS;
		}

		clearRsslErrorInfo( &rsslErrorInfo );
		retCode = rsslEncodeRDMDirectoryMsg( &eIter, pEvent->pRDMDirectoryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( rsslMsgBuffer.data );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in DirectoryCallbackClient::processCallback()" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}
		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType )
	{
	case RDM_DR_MT_REFRESH :
	{
		StaticDecoder::setRsslData( &_refreshMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _refreshMsg );
		pItem->onRefreshMsg( _refreshMsg );

		if ( _refreshMsg.getState().getStreamState() == OmmState::NonStreamingEnum )
		{
			if ( _refreshMsg.getComplete() )
				pItem->remove();
		}
		else if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
		{
			pItem->remove();
		}
	}
	break;
	case RDM_DR_MT_UPDATE :
	{
		StaticDecoder::setRsslData( &_updateMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _updateMsg );
		pItem->onUpdateMsg( _updateMsg );
	}
	break;
	case RDM_DR_MT_STATUS :
	{
		StaticDecoder::setRsslData( &_statusMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _statusMsg );
		pItem->onStatusMsg( _statusMsg );

		if ( _statusMsg.hasState() && ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum ) )
			pItem->remove();
	}
	break;
	case RDM_DR_MT_CONSUMER_STATUS :
	{
		StaticDecoder::setRsslData( &_genericMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _genericMsg );
		pItem->onGenericMsg( _genericMsg );
	}
	break;
	default :
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			                                       "Internal error. Received unexpected type of RsslRDMDirectoryMsg in DirectoryCallbackClient::processCallback()" );
		break;
	}
	}

	free( rsslMsgBuffer.data );

	return RSSL_RC_CRET_SUCCESS;
}

int DirectoryCallbackClient::allocateAndSetEncodeIteratorBuffer(OmmBaseImpl* pOmmBaseImpl, RsslBuffer* rsslBuffer, UInt32 allocateBufferSize, UInt8 majorVersion, UInt8 minorVersion,
	RsslEncodeIterator* rsslEncodeIterator, const char* methodName )
{
	rsslBuffer->length = allocateBufferSize;

	rsslBuffer->data = (char*)malloc( sizeof(char) * rsslBuffer->length );

	if ( !rsslBuffer->data )
	{
		EmaString text( "Failed to allocate memory in " );
		text.append( methodName );
		pOmmBaseImpl->handleMee( text.c_str() );
		return RSSL_RET_FAILURE;
	}

	int retCode = rsslSetEncodeIteratorRWFVersion( rsslEncodeIterator, majorVersion, minorVersion );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString text( "Internal error. Failed to set encode iterator RWF version in " );
		text.append( methodName );
		if ( OmmLoggerClient::ErrorEnum >= pOmmBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity )
			pOmmBaseImpl->getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				text.c_str() );

		free( rsslBuffer->data );
		return retCode;
	}

	retCode = rsslSetEncodeIteratorBuffer( rsslEncodeIterator, rsslBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString text( "Internal error. Failed to set encode iterator buffer in " );
		text.append( methodName );
		if ( OmmLoggerClient::ErrorEnum >= pOmmBaseImpl->getActiveConfig().loggerConfig.minLoggerSeverity )
			pOmmBaseImpl->getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				text.c_str() );

		free( rsslBuffer->data );
		return retCode;
	}

	return RSSL_RET_SUCCESS;
}

const EmaString DirectoryItem::_clientName( "DirectoryCallbackClient" );

DirectoryItem::DirectoryItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, const Channel* channel ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_channel( channel ),
	_pDirectory( 0 ), 
	serviceName (),
	serviceId(-1)
{
	filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER | RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER;
	sentRefresh = false;
}

DirectoryItem::~DirectoryItem()
{
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		_ommBaseImpl.getItemCallbackClient().removeFromList( this );
	else
		_ommBaseImpl.getDirectoryCallbackClient().removeItem(this);

	_ommBaseImpl.getItemCallbackClient().removeFromMap(this);
}

DirectoryItem* DirectoryItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, const Channel* channel )
{
	try
	{
		return new DirectoryItem( ommBaseImpl, ommConsClient, closure, channel );
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create DirectoryItem" );
	}

	return NULL;
}

const Directory* DirectoryItem::getDirectory()
{
	return 0;
}

bool DirectoryItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	const Directory* pDirectory = 0;

	if ( reqMsgEncoder.hasServiceName() )
	{
		serviceName = reqMsgEncoder.getServiceName();
		if(_ommBaseImpl.getConsumerRoutingSession() == NULL)
			pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getServiceName() );
		else
		{
			ConsumerRoutingService** directoryPtr = _ommBaseImpl.getConsumerRoutingSession()->serviceByName.find(&reqMsgEncoder.getServiceName());
			if (directoryPtr != NULL)
			{
				pDirectory = *directoryPtr;
			}
		}

		// Let the request go through if the directory isn't found and either request routing is on, or request routing is off and single open is off
		if ( !pDirectory && (_ommBaseImpl.getConsumerRoutingSession() != NULL || 
				(_ommBaseImpl.getConsumerRoutingSession() == NULL && !_ommBaseImpl.getLoginCallbackClient().getLoginRefresh()->singleOpen)))
		{
			EmaString temp( "Service name of '" );
			temp.append( reqMsgEncoder.getServiceName() ).append( "' is not found." );

			scheduleItemClosedStatus(reqMsgEncoder, temp);
			return true;
		}

		if(pDirectory != NULL)
			serviceId = pDirectory->getId();
	}
	else
	{
		if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
		{
			if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
				pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId );
			else
			{
				ConsumerRoutingService** directoryPtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId);
				if (directoryPtr != NULL)
				{
					pDirectory = *directoryPtr;
				}
			}

			// Let the request go through if the directory isn't found and either request routing is on, or request routing is off and single open is off
			if (!pDirectory && (_ommBaseImpl.getConsumerRoutingSession() != NULL ||
				(_ommBaseImpl.getConsumerRoutingSession() == NULL && !_ommBaseImpl.getLoginCallbackClient().getLoginRefresh()->singleOpen)))
			{
				EmaString temp( "Service id of '" );
				temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId ).
				append( "' is not found." );

				scheduleItemClosedStatus(reqMsgEncoder, temp);
				return true;
			}
			
			if (pDirectory != NULL)
				serviceName = pDirectory->getName();

			serviceId = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId;
		}
	}

	if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.filter != 0)
		filter = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.filter;

	_pDirectory = pDirectory;

	// Set the stream ID here.
	if (!_streamId)
	{
		if (!reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId)
			_streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId();
		else
			_streamId = reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId;
	}

	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		return submit( reqMsgEncoder.getRsslRequestMsg() );
	else
		new TimeOut(_ommBaseImpl, 1000, DirectoryCallbackClient::fanoutSingleDirectoryRequest, (void*)this, true);

	return true;
}

bool DirectoryItem::modify( const ReqMsg& reqMsg )
{
	// The only thing that can realistically change here is the filter, so apply the new one to the current item.
	if (static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg()->msgBase.msgKey.filter != 0)
		filter = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg()->msgBase.msgKey.filter;

	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		return submit( static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).getRsslRequestMsg() );
	else
		new TimeOut(_ommBaseImpl, 1000, DirectoryCallbackClient::fanoutSingleDirectoryRequest, (void*)this, true);

	return true;
}

bool DirectoryItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on directory stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool DirectoryItem::submit( const GenericMsg& genMsg )
{
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
	else
	{
		EmaString temp("Invalid attempt to submit GenericMsg on directory stream for Request Routing. ");
		temp.append("Instance name='").append(_ommBaseImpl.getInstanceName()).append("'.");

		_ommBaseImpl.handleIue(temp, OmmInvalidUsageException::InvalidOperationEnum);
		return false;
	}
}

bool DirectoryItem::close()
{
	bool retCode( true );

	// Do not send the close message if this is a request routing session.
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		RsslCloseMsg rsslCloseMsg;
		rsslClearCloseMsg( &rsslCloseMsg );

		rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		rsslCloseMsg.msgBase.domainType = _domainType;

		retCode = submit( &rsslCloseMsg );
	}

	remove();
	return retCode;
}

void DirectoryItem::remove()
{
	delete this;
}

bool DirectoryItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	RsslReactorChannel* pReactorChannel = _ommBaseImpl.getRsslReactorChannel();

	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslGenericMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _ommBaseImpl.getRsslReactor(),
											pReactorChannel,
	                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslGenericMsg* )" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit GenericMsg on directory stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
		.append( ". Error text: " )
		.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}


bool DirectoryItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	if ( !( pRsslRequestMsg->flags & RSSL_RQMF_HAS_QOS ) )
	{
		pRsslRequestMsg->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		pRsslRequestMsg->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		pRsslRequestMsg->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		pRsslRequestMsg->worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		pRsslRequestMsg->worstQos.rateInfo = 65535;
		pRsslRequestMsg->flags |= ( RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS );
	}

	pRsslRequestMsg->flags |= _ommBaseImpl.getActiveConfig().msgKeyInUpdates ? RSSL_RQMF_MSG_KEY_IN_UPDATES : 0;
	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslRequestMsg;

	RsslBuffer serviceNameBuffer = RSSL_INIT_BUFFER;
	if ( _pDirectory )
	{
		serviceNameBuffer.data = ( char* )_pDirectory->getName().c_str();
		serviceNameBuffer.length = _pDirectory->getName().length();
		submitMsgOpts.pServiceName = &serviceNameBuffer;
		pRsslRequestMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;
	}
	else
	{
		if ( serviceName.length() != 0 )
		{	
			serviceNameBuffer.data = ( char* )serviceName.c_str();
			serviceNameBuffer.length = serviceName.length();
			submitMsgOpts.pServiceName = &serviceNameBuffer;
			pRsslRequestMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;
		}
		else
			submitMsgOpts.pServiceName = 0;
	}

	submitMsgOpts.majorVersion = _ommBaseImpl.getRsslReactorChannel()->majorVersion;
	submitMsgOpts.minorVersion = _ommBaseImpl.getRsslReactorChannel()->minorVersion;

	submitMsgOpts.requestMsgOptions.pUserSpec = ( void* )this;
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	if ( !_domainType )
		_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
	else
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret = rsslReactorSubmitMsg( _ommBaseImpl.getRsslReactor(), _ommBaseImpl.getRsslReactorChannel(), &submitMsgOpts, &rsslErrorInfo);
	if ( ret != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslRequestMsg* )" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to open or modify directory request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
		.append( ". Error text: " )
		.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

bool DirectoryItem::submit( RsslCloseMsg* pRsslCloseMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	RsslReactorChannel* pReactorChannel = _ommBaseImpl.getRsslReactorChannel();

	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslCloseMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;
	if ( !_streamId )
	{
		if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
			submitMsgOpts.pRsslMsg->msgBase.streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId();
		_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _ommBaseImpl.getRsslReactor(), pReactorChannel,
	                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( pRsslCloseMsg* )" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to close directory stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
		.append( ". Error text: " )
		.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

void DirectoryItem::scheduleItemClosedStatus(const ReqMsgEncoder& reqMsgEncoder, const EmaString& text)
{
	if (_closedInfo) return;

	_closedInfo = new ItemStatusInfo(this, reqMsgEncoder, text);

	new TimeOut(_ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true);
}