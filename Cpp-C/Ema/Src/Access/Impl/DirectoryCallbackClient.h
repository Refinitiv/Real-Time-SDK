/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_DirectoryCallbackClient_h
#define __thomsonreuters_ema_access_DirectoryCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaVector.h"
#include "EmaList.h"
#include "HashTable.h"
#include "OmmLoggerClient.h"
#include "ItemCallbackClient.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmBaseImpl;
class Channel;
class Item;

class DictionaryList
{
public :

	DictionaryList();
	DictionaryList( const DictionaryList& );
	DictionaryList& operator=( const DictionaryList& );
	virtual ~DictionaryList();

	DictionaryList& clear();

	DictionaryList& addDictionary( const EmaString& );

	const EmaVector<EmaString>& getDictionaryList() const;

	const EmaString& toString() const;

private :

	EmaVector< EmaString >	_list;
	mutable EmaString		_toString;
	mutable bool			_toStringSet;
};

class Info
{
public :

	Info();
	Info& operator=( const Info& );
	Info( const Info& );
	virtual ~Info();

	Info& clear();

	const EmaString& getName() const;
	Info& setName( const EmaString& );
	Info& setName( const char*, UInt32 );

	const EmaString& getVendor() const;
	Info& setVendor( const EmaString& );
	Info& setVendor( const char*, UInt32 );

	const EmaString& getItemList() const;
	Info& setItemList( const EmaString& );
	Info& setItemList( const char*, UInt32 );

	UInt64 getIsSource() const;
	Info& setIsSource( UInt64 );

	const DictionaryList& getDictionariesProvided() const;
	Info& addDictionaryProvided( const EmaString& );
	Info& addDictionaryProvided( const char*, UInt32 );

	const DictionaryList& getDictionariesUsed() const;
	Info& addDictionaryUsed( const EmaString& );
	Info& addDictionaryUsed( const char*, UInt32 );

private :

	EmaString			_name;
	EmaString			_vendor;
	EmaString			_itemList;
	DictionaryList		_dictionariesProvided;
	DictionaryList		_dictionariesUsed;
	mutable EmaString	_toString;
	UInt64				_isSource;
	mutable bool		_toStringSet;
};

class State
{
public :

	State();
	virtual ~State();

	State& clear();

	State& operator=( const State& );

	UInt64 getServiceState() const;
	State& setServiceState( UInt64 );

	UInt64 getAcceptingRequests() const;
	State& setAcceptingRequests( UInt64 );

	State& markDeleted();

	const EmaString& toString() const;

private :

	UInt64				_serviceState;
	UInt64				_acceptingRequests;
	bool				_deleted;
	mutable EmaString	_toString;
	mutable bool		_toStringSet;

	State( const State& );
};

typedef const EmaString* EmaStringPtr;

class Directory : public ListLinks< Directory >
{
public :

	static Directory* create( OmmCommonImpl& );

	static void destroy( Directory*& );

	Directory& clear();

	const Info& getInfo() const;
	Directory& setInfo( const Info& );

	const State& getState() const;
	Directory& setState( const State& );

	const EmaString& getName() const;
	EmaStringPtr getNamePtr() const;
	Directory& setName( const EmaString& );
	Directory& setName( const char*, UInt32 );

	UInt64 getId() const;
	Directory& setId( UInt64 );

	Channel* getChannel() const;
	Directory& setChannel( Channel* );

	Directory& markDeleted();

private :

	Info				_info;
	State				_state;
	EmaString			_name;
	mutable EmaString	_toString;
	UInt64				_id;
	Channel*			_pChannel;
	bool				_hasInfo;
	bool				_hasState;
	mutable bool		_toStringSet;

	Directory();
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

private :

	int allocateAndSetEncodeIteratorBuffer(RsslBuffer*, UInt32, UInt8, UInt8, RsslEncodeIterator*, const char *);

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

#endif // __thomsonreuters_ema_access_DirectoryCallbackClient_h
