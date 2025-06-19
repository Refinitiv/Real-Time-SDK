/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_DirectoryCallbackClient_h
#define __refinitiv_ema_access_DirectoryCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaVector.h"
#include "EmaList.h"
#include "HashTable.h"
#include "OmmLoggerClient.h"
#include "ItemCallbackClient.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmBaseImpl;
class Channel;
class Item;

class DirectoryDictionaryInfo
{
public:
	DirectoryDictionaryInfo();
	virtual ~DirectoryDictionaryInfo();
	DirectoryDictionaryInfo& operator=(const DirectoryDictionaryInfo&);
	EmaString name;
	UInt32 count;
};

class DictionaryList
{
public :

	DictionaryList();
	DictionaryList( const DictionaryList& );
	DictionaryList& operator=( const DictionaryList& );
	virtual ~DictionaryList();

	DictionaryList& clear();

	DictionaryList& addDictionary( const EmaString& );
	DictionaryList& removeDictionary(const EmaString&);
	DictionaryList& deleteDictionaryFromList(const EmaString&);
	DirectoryDictionaryInfo* findDictionary(const EmaString& dictionaryName);
	Int64 findDictionaryIndex(const EmaString& dictionaryName);

	bool hasDictionary(const EmaString&);

	const EmaVector<DirectoryDictionaryInfo*>& getDictionaryList() const;

	const EmaString& toString() const;

private :

	EmaVector< DirectoryDictionaryInfo* >	_list;
	mutable EmaString		_toString;
	mutable bool			_toStringSet;
};

class DirectoryDomainType
{
public :
	DirectoryDomainType();
	virtual ~DirectoryDomainType();
	UInt32 domain;		// Set to RsslDomainTypes.
	UInt32		count;

	static int compare(DirectoryDomainType*&, DirectoryDomainType*&);
};

class DirectoryQoS
{
public :
	DirectoryQoS();
	virtual ~DirectoryQoS();
	RsslQos qos;
	UInt32		count;

	static int compare(DirectoryQoS*&, DirectoryQoS*&);
};

typedef const EmaString* EmaStringPtr;

class Directory : public ListLinks< Directory >
{
public :

	static Directory* create(OmmBaseImpl&);

	static void destroy( Directory*& );

	Directory& clear();

	bool setService(RsslRDMService*);
	bool updateService(RsslRDMService*);

	Directory& populateService(RsslRDMService*);

	RsslRDMService* getService();

	const EmaString& getName() const;
	Directory& setName(const EmaString& name);

	DictionaryList& getDictionariesUsed();
	DictionaryList& getDictionariesProvided();

	UInt64 getId() const;
	Directory& setId( UInt64 );

	Channel* getChannel() const;
	Directory& setChannel( Channel* );

	Directory& addDomain(UInt32);
	DirectoryDomainType* findDomain(UInt32);
	Int64 findDomainIndex(UInt32);


	Directory& addQos(RsslQos*);
	DirectoryQoS* findQos(RsslQos*);
	Int64 findQosIndex(RsslQos*);

	UInt64	getServiceState();
	UInt64	getAcceptingRequests();

	Directory& markDeleted();
	Directory& markActive();

	void setGeneratedServiceId(UInt64);
	bool hasGeneratedServiceId() const;
	UInt64 getGeneratedServiceId() const;
	

	bool isDeleted() const;

	// Determines if the current service matches what's in the Request message
	// This will see if the request's domain is supported by the directory, and if the qos matches, or(if a worst qos is specified) the provided qos are in the range
	bool serviceMatch(RsslRequestMsg*);

protected :

	friend class ConsumerRoutingService;

	OmmBaseImpl&		_ommBaseImpl;

	RsslRDMService		_service;

	EmaString			_name;
	mutable EmaString	_toString;
	UInt64				_id;
	Channel*			_pChannel;
	mutable bool		_toStringSet;
	mutable bool		_serviceAllocated;

	EmaString			_vendor;
	EmaString			_itemList;

	EmaBuffer			_dataBuffer;
	

	EmaString			_stateText;


	DictionaryList		_dictionariesProvided;
	DictionaryList		_dictionariesUsed;
	EmaVector<DirectoryDomainType*> _supportedDomains;
	EmaVector< DirectoryQoS*>		_supportedQos;

	RsslQos*			_qosList;
	UInt32				_qosCount;

	RsslUInt*				_capabilitiesList;
	UInt32				_capabilitiesCount;

	RsslBuffer*			_dictionariesProvidedList;
	UInt32				_dictionariesProvidedCount;

	RsslBuffer*			_dictionariesUsedList;
	UInt32				_dictionariesUsedCount;

	bool				_markDeleted;

	bool				_hasGeneratedServiceId;
	UInt64			_generatedServiceId;


	Directory(OmmBaseImpl&);
	virtual ~Directory();

	Directory( const Directory& );
	Directory& operator=( const Directory& );
};

typedef Directory* DirectoryPtr;

class DirectoryCallbackClient
{
public :

	static DirectoryCallbackClient* create( OmmBaseImpl& );

	static void destroy( DirectoryCallbackClient*& );

	void initialize();

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDirectoryMsgEvent* );

	RsslRDMDirectoryRequest* getDirectoryRequest();

	const Directory* getDirectory( const EmaString& ) const;

	const Directory* getDirectory( UInt32 ) const;

	void addItem(Item*);
	void removeItem(Item*);

	// Takes in: ommBaseImpl pointer and will fan out all of the requests.
	static void fanoutAllDirectoryRequests(void* info);

	// Takes in: DirectoryItem pointer, and will fanout to only that request.
	static void fanoutSingleDirectoryRequest(void* info);


	static int allocateAndSetEncodeIteratorBuffer(OmmBaseImpl*, RsslBuffer*, UInt32, UInt8, UInt8, RsslEncodeIterator*, const char*);

private :


	class EmaStringPtrHasher
	{
	public:
		size_t operator()( const EmaStringPtr& ) const;
	};

	class EmaStringPtrEqual_To
	{
	public:
		bool operator()( const EmaStringPtr&, const EmaStringPtr& ) const;
	};

	typedef HashTable< EmaStringPtr , DirectoryPtr , EmaStringPtrHasher , EmaStringPtrEqual_To > DirectoryByName;

	class UInt64rHasher
	{
	public:
		size_t operator()( const UInt64& ) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()( const UInt64&, const UInt64& ) const;
	};

	typedef HashTable< UInt64 , DirectoryPtr , UInt64rHasher , UInt64Equal_To > DirectoryById;

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDirectoryMsgEvent*, SingleItem* );

	void processDirectoryPayload( UInt32 , RsslRDMService*, void* );

	void addDirectory( Directory* );

	void removeDirectory( Directory* );

	static const EmaString			_clientName;

	RsslRDMDirectoryRequest			_directoryRequest;

	DirectoryById					_directoryByIdHt;

	DirectoryByName					_directoryByNameHt;

	EmaList< Directory* >			_directoryList;

	OmmBaseImpl&					_ommBaseImpl;

	RefreshMsg						_refreshMsg;

	UpdateMsg						_updateMsg;

	StatusMsg						_statusMsg;

	GenericMsg						_genericMsg;

	ItemList*						_requestList;

	bool fanoutDirectory;			

	DirectoryCallbackClient( OmmBaseImpl& );

	virtual ~DirectoryCallbackClient();

	DirectoryCallbackClient();
	DirectoryCallbackClient( const DirectoryCallbackClient& );
	DirectoryCallbackClient& operator=( const DirectoryCallbackClient& );
};

class DirectoryItem : public ConsumerItem
{
public :

	static DirectoryItem* create( OmmBaseImpl&, OmmConsumerClient& , void* , const Channel* );

	const Directory* getDirectory();

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();
	bool submit( RsslGenericMsg* );

	ItemType getType() const
	{
		return Item::DirectoryItemEnum;
	}

	EmaString				serviceName;
	Int64					serviceId;
	RsslUInt32				filter;
	bool					sentRefresh;

private :

	static const EmaString		_clientName;

	const Channel*				_channel;
	const Directory*			_pDirectory;

	void scheduleItemClosedStatus(const ReqMsgEncoder&, const EmaString& );

	bool submit( RsslRequestMsg* );
	bool submit( RsslCloseMsg* );

	DirectoryItem( OmmBaseImpl&, OmmConsumerClient& , void* , const Channel* );
	DirectoryItem();
	virtual ~DirectoryItem();
	DirectoryItem( const BatchItem& );
	DirectoryItem& operator=( const DirectoryItem& );
};

}

}

}

#endif // __refinitiv_ema_access_DirectoryCallbackClient_h
