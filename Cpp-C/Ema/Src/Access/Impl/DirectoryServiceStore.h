/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.
*|                See the project's LICENSE.md for details.
*|           Copyright (C) 2019-2023 LSEG. All rights reserved.              --
*|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_DirectoryServiceStore_h
#define __refinitiv_ema_access_DirectoryServiceStore_h

#include "ActiveConfig.h"
#include "EmaList.h"
#include "OmmProviderConfig.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmNiProviderImpl;
class OmmIProviderImpl;
class OmmNiProviderActiveConfig;
class OmmIProviderActiveConfig;
class ClientSession;

typedef const EmaString* EmaStringPtr;

class InfoFilter
{
public:

	InfoFilter();
	InfoFilter(const InfoFilter&);
	InfoFilter& operator=(const InfoFilter&);

	virtual ~InfoFilter();

	void clear();

	void apply(RsslRDMServiceInfo&);

	UInt32					flags;
	RsslFilterEntryActions  action;
	EmaString				serviceName;
	EmaString				vendorName;
	UInt64					isSource;
	EmaVector< UInt64 >		capabilities;
	EmaVector< EmaString >	dictionariesProvided;
	EmaVector< EmaString >	dictionariesUsed;
	EmaVector< RsslQos >	qos;
	UInt64					supportsQosRange;
	EmaString				itemList;
	UInt64					supportsOutOfBandSnapshots;
	UInt64					acceptingConsumerStatus;
};

class StateFilter
{
public:

	StateFilter();
	StateFilter(const StateFilter&);
	StateFilter& operator=(const StateFilter&);

	virtual ~StateFilter();

	void clear();

	void apply(RsslRDMServiceState&);

	UInt32					flags;
	RsslFilterEntryActions  action;
	RsslUInt				serviceState;
	RsslUInt				acceptingRequests;
	RsslState				status;
	EmaString				statusText;
};

class GroupFilter : EmaList<GroupFilter>
{
public:

	GroupFilter();
	GroupFilter(const GroupFilter&);
	GroupFilter& operator=(const GroupFilter&);

	virtual ~GroupFilter();

	void clear();

	UInt32					flags;
	RsslFilterEntryActions  action;
	EmaBuffer				groupId;
	EmaBuffer				mergedToGroupId;
	RsslState				status;
	EmaString				statusText;
};

class LoadFilter
{
public:

	LoadFilter();
	LoadFilter(const LoadFilter&);
	LoadFilter& operator=(const LoadFilter&);

	virtual ~LoadFilter();

	void clear();

	void apply(RsslRDMServiceLoad&);

	UInt32					flags;
	RsslFilterEntryActions  action;
	RsslUInt				openLimit;
	RsslUInt				openWindow;
	RsslUInt				loadFactor;
};

class Link : public ListLinks<Link>
{
public:

	Link();
	Link(const Link&);
	Link& operator=(const Link&);

	virtual ~Link();

	void clear();

	void apply(RsslRDMServiceLink&);

	UInt32				flags;
	RsslMapEntryActions	action;
	RsslUInt			type;
	RsslUInt			linkState;
	RsslUInt			linkCode;
	EmaString			text;

	EmaString			name;
};

class LinkFilter
{
public:

	LinkFilter();
	virtual ~LinkFilter();

	void clear();

	RsslFilterEntryActions  action;

	void addLink(const Link&);

	void removeLink(const EmaString&);

	Link* getLink(const EmaString&);

	const EmaList< Link* >& getLinkList() const;

private:
	
	EmaList< Link* > _linkList;

};


class Service : public ListLinks<Service>
{
public:

	Service();
	Service(const Service&);
	Service& operator=(const Service&);

	virtual ~Service();

	void clear();

	UInt32				flags;
	UInt16				serviceId;
	InfoFilter			infoFilter;
	StateFilter			stateFilter;
	LoadFilter			loadFilter;
	LinkFilter			linkFilter;
};

class DirectoryCache
{
public:

	DirectoryCache();
	virtual ~DirectoryCache();

	void clear();

	Service* addService(const Service&);

	void removeService(UInt64);

	Service* getService(UInt64) const;

	Service* getService(EmaString serviceName) const;

	const EmaList< Service* >& getServiceList() const;

	EmaString		directoryName;

private:

	class UInt64rHasher
	{
	public:
		size_t operator()(const UInt64&) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()(const UInt64&, const UInt64&) const;
	};

	typedef HashTable< UInt64, Service*, UInt64rHasher, UInt64Equal_To > ServiceHash;

	ServiceHash	_serviceHash;

	EmaList< Service* >		_serviceList;
};

class DirectoryServiceStoreClient
{
public:
	virtual void onServiceDelete(ClientSession* clientSession, RsslUInt serviceId) = 0;

	virtual void onServiceStateChange(ClientSession* clientSession, RsslUInt serviceId, const RsslRDMServiceState&)  {}

	virtual void onServiceGroupChange(ClientSession* clientSession, RsslUInt serviceId, RsslRDMServiceGroupState*&, RsslUInt32 groupStateCount) {}

	virtual ~DirectoryServiceStoreClient() {}
};

class DirectoryServiceStore
{
public:

	class UInt64rHasher
	{
	public:
		size_t operator()(const UInt64&) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()(const UInt64&, const UInt64&) const;
	};

	class EmaStringPtrHasher
	{
	public:
		size_t operator()(const EmaStringPtr&) const;
	};

	class EmaStringPtrEqual_To
	{
	public:
		bool operator()(const EmaStringPtr&, const EmaStringPtr&) const;
	};

	typedef HashTable< EmaStringPtr, UInt64, EmaStringPtrHasher, EmaStringPtrEqual_To > ServiceNameToServiceId;
	typedef HashTable< UInt64, EmaStringPtr, UInt64rHasher, UInt64Equal_To > ServiceIdToServiceName;
	typedef EmaVector< EmaStringPtr > ServiceNameList;

	DirectoryServiceStore(OmmProviderConfig::ProviderRole, OmmCommonImpl&, BaseConfig&);

	virtual ~DirectoryServiceStore();

	bool decodeSourceDirectory(RwfBuffer*, EmaString&, Int32&);

	bool decodeSourceDirectoryKeyUInt(RsslMap&, RsslDecodeIterator&, EmaString&, Int32&);

	static bool encodeDirectoryRefreshMsg(const DirectoryCache& directoryCache, RsslRDMDirectoryRefresh&, int directoryServiceFilter = (RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER),
		bool specificServiceId = false, UInt16 serviceId = 0);

	static bool encodeDirectoryMsg(const RsslRDMDirectoryMsg& directoryMsg, RsslRDMDirectoryMsg& encodeDirectoryMsg, int directoryServiceFilter, bool specificServiceId = false, UInt16 serviceId = 0);

	static bool encodeService(const Service* service, RsslRDMService& rsslRDMService, RsslRDMDirectoryRefresh& directoryRefresh, int directoryServiceFilter);

	static void encodeService(const RsslRDMService&, RsslRDMService&, int requestFilter, int& responseFilter);

	RsslUInt64* getServiceIdByName(const EmaString* pServiceName);

	const EmaString** getServiceNameById(RsslUInt64 serviceId);

	void removeServiceNamePair(UInt64 serviceId);

	void clearServiceNamePair();

	bool submitSourceDirectory(ClientSession* clientSession, RsslMsg*, RsslRDMDirectoryMsg&, RsslBuffer&, bool);

	virtual DirectoryCache& getDirectory();

	virtual bool checkExistingServiceId(RsslUInt64 serviceId, EmaString& errorText) = 0;

	virtual bool addServiceIdAndNamePair(RsslUInt64 serviceId, const EmaString* serviceName, EmaString* pErrorText);

	static void freeMemory(RsslRDMDirectoryRefresh&, RsslBuffer* );

	void setClient(DirectoryServiceStoreClient*);

	const DirectoryCache& getDirectoryCache();

protected:

	void populateDefaultService(Service&);

	void loadConfigDirectory(DirectoryCache* directoryCache, EmaConfigBaseImpl*, EmaList<ServiceDictionaryConfig*>*);

	void notifyOnServiceStateChange(ClientSession* clientSession, RsslRDMService&);

	void notifyOnServiceGroupChange(ClientSession* clientSession, RsslRDMService&);

	void notifyOnServiceDelete(ClientSession* clientSession, RsslRDMService&);

	DirectoryCache					_directoryCache;
	OmmProviderConfig::ProviderRole _providerRole;
	ServiceNameToServiceId			_serviceNameToServiceId;
	ServiceIdToServiceName			_serviceIdToServiceName;
	ServiceNameList					_serviceNameList;
	OmmCommonImpl&					_ommCommonImpl;
	BaseConfig&						_baseConfig;
	DirectoryServiceStoreClient*    _pDirectoryServiceStoreClient;
	bool							_bUsingDefaultService;
};

class OmmIProviderDirectoryStore : public DirectoryServiceStore
{
public:

	OmmIProviderDirectoryStore(OmmIProviderImpl&, OmmIProviderActiveConfig&);

	void loadConfigDirectory(EmaConfigBaseImpl*, bool);

	bool IsAcceptingRequests(RsslUInt64 serviceId);

	bool IsValidQosRange(RsslUInt64 serviceId, RsslRequestMsg& rsslRequestMsg);

	virtual ~OmmIProviderDirectoryStore();

	OmmIProviderActiveConfig& getOmmIProviderActiveConfig() const;

private:

	bool checkExistingServiceId(RsslUInt64 serviceId, EmaString& errorText);

	OmmIProviderImpl&			_ommIProviderImpl;
	OmmIProviderActiveConfig&	_ommIProviderActiveConfig;
};

class OmmNiProviderDirectoryStore : public DirectoryServiceStore
{
public:

	OmmNiProviderDirectoryStore(OmmNiProviderImpl&, OmmNiProviderActiveConfig&);

	DirectoryCache& getApiControlDirectory();

	void loadConfigDirectory(EmaConfigBaseImpl*);

	virtual ~OmmNiProviderDirectoryStore();

private:

	bool checkExistingServiceId(RsslUInt64 serviceId, EmaString& errorText);

	DirectoryCache*				_pDirectoryCacheApiControl;
	OmmNiProviderImpl&			_ommNiProviderImpl;
	OmmNiProviderActiveConfig&	_ommNiProviderActiveConfig;
};

}

}

}

#endif // __refinitiv_ema_access_DirectoryServiceStore_h
