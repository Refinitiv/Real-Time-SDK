/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ItemCallbackClient_h
#define __thomsonreuters_ema_access_ItemCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaString.h"
#include "EmaList.h"
#include "OmmState.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "OmmLoggerClient.h"
#include "OmmConsumerEvent.h"
#include "HashTable.h"
#include "EmaVector.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmConsumerImpl;
class OmmConsumerClient;
class ReqMsg;
class PostMsg;
class Channel;
class Directory;
class ReqMsgEncoder;
class OmmState;
class Item;

class ItemList
{
public :

	static ItemList* create( OmmConsumerImpl& );

	static void destroy( ItemList*& );

	Int32 addItem( Item* );

	void removeItem( Item* );

private :

	static const EmaString		_clientName;

	EmaList< Item >				_list;
	OmmConsumerImpl&			_ommConsImpl;

	ItemList( OmmConsumerImpl& );
	ItemList();
	virtual ~ItemList();
	ItemList( const ItemList& );
	ItemList& operator=( const ItemList& );
};

class Item : public ListLinks< Item >
{
public :

	enum ItemType
	{
		SingleItemEnum,
		BatchItemEnum,
		LoginItemEnum,
		DirectoryItemEnum,
		DictionaryItemEnum
	};

	static void destroy( Item*& );

	OmmConsumerClient& getClient() const;
	void* getClosure() const;

	virtual bool open( const ReqMsg& ) = 0;
	virtual bool modify( const ReqMsg& ) = 0;
	virtual bool submit( const PostMsg& ) = 0;
	virtual bool submit( const GenericMsg& ) = 0;
	virtual bool close() = 0;
	virtual void remove() = 0;
	virtual bool submit( RsslGenericMsg* ) = 0;
	virtual ~Item();

	ItemType getType() const;
	UInt32 addItem( Item* );
	void removeItem( Item* );
	bool operator<(const Item & rhs) { return true; }

	OmmConsumerImpl&			getOmmConsumerImpl();

protected :

	UInt8						_domainType;
	Int32						_streamId;
	void*						_closure;
	OmmConsumerClient&			_ommConsClient;
	OmmConsumerImpl&			_ommConsImpl;
	ItemType					_itemType;

	Item( OmmConsumerImpl& , OmmConsumerClient& , void* );

private :

	Item();
	Item( const Item& );
	Item& operator=( const Item& );
};

class ClosedStatusInfo
{
public :

	ClosedStatusInfo( Item* , const ReqMsgEncoder& , const EmaString& );
	virtual ~ClosedStatusInfo();

	UInt8 getDomainType() const { return _domainType; }

	const EmaString& getStatusText() const { return _statusText; }

	const EmaString& getServiceName() const { return _serviceName; }

	RsslMsgKey* getRsslMsgKey() { return &_msgKey; }

	Item* getItem() const { return _pItem; }

private :

	RsslMsgKey		_msgKey;
	EmaString		_statusText;
	EmaString		_serviceName;
	UInt8			_domainType;
	Item*			_pItem;
};

class SingleItem : public Item
{
public :

	static SingleItem* create( OmmConsumerImpl& , OmmConsumerClient& , void* , SingleItem* );

	const Directory* getDirectory() const;

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();

	bool submit( RsslGenericMsg* );

	void state( const OmmState& );

protected :

	SingleItem( OmmConsumerImpl& , OmmConsumerClient& , void* , SingleItem *);
	virtual ~SingleItem();

	void scheduleItemClosedStatus( const ReqMsgEncoder& , const EmaString& );

private :

	bool submit( RsslRequestMsg* );
	bool submit( RsslCloseMsg* );
	bool submit( RsslPostMsg* );

	static const EmaString				_clientName;

	SingleItem*							_pBatchItem;

	const Directory*					_pDirectory;
	ClosedStatusInfo*					_closedStatusInfo;

	SingleItem();
	SingleItem( const SingleItem& );
	SingleItem& operator=( const SingleItem& );
};

class BatchItem : public SingleItem
{
public :

	static BatchItem* create( OmmConsumerImpl& , OmmConsumerClient& , void* );

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();

	void addBatchItems( const EmaVector<EmaString>& );

	const EmaVector<SingleItem*> & getSingleItemList();

	SingleItem * getSingleItem( Int32 );

	void decreaseItemCount();

protected :

private :

	static const EmaString		_clientName;

	EmaVector<SingleItem*>		_singleItemList;

	UInt32						_itemCount;

	BatchItem( OmmConsumerImpl& , OmmConsumerClient& , void* );
	BatchItem();
	virtual ~BatchItem();
	BatchItem( const BatchItem& );
	BatchItem& operator=( const BatchItem& );
};

typedef Item* ItemPtr;

class ItemCallbackClient
{
public :

	static void sendItemClosedStatus( void* );

	static ItemCallbackClient* create( OmmConsumerImpl& );

	static void destroy( ItemCallbackClient*& );

	void initialize();

	UInt64 registerClient( const ReqMsg& , OmmConsumerClient& , void* );

	void reissue( const ReqMsg& , UInt64 );

	void unregister( UInt64 );

	void submit( const PostMsg& , UInt64 );

	void submit( const GenericMsg& , UInt64 );

	RsslReactorCallbackRet processCallback(  RsslReactor* , RsslReactorChannel* , RsslMsgEvent* );

	void addToList( SingleItem* );
	void removeFromList( SingleItem* );

	void addToMap( Item* );
	void removeFromMap( Item* );

private :

	static const EmaString			_clientName;

	RefreshMsg						_refreshMsg;

	UpdateMsg						_updateMsg;

	StatusMsg						_statusMsg;

	GenericMsg						_genericMsg;

	AckMsg							_ackMsg;

	OmmConsumerEvent				_event;

	OmmConsumerImpl&				_ommConsImpl;

	ItemList*						_itemList;

	class UInt64rHasher {
	public:
		size_t operator()( const UInt64 & ) const;
	};

	class UInt64Equal_To {
	public:
		bool operator()( const UInt64 & , const UInt64 & ) const;
	};

	typedef HashTable< UInt64 , ItemPtr , UInt64rHasher , UInt64Equal_To > ItemMap;

	ItemMap							_itemMap;

	RsslReactorCallbackRet processAckMsg( RsslMsg* , RsslReactorChannel* , RsslMsgEvent* );
	RsslReactorCallbackRet processGenericMsg( RsslMsg* , RsslReactorChannel* , RsslMsgEvent* );
	RsslReactorCallbackRet processRefreshMsg( RsslMsg* , RsslReactorChannel* , RsslMsgEvent* );
	RsslReactorCallbackRet processUpdateMsg( RsslMsg* , RsslReactorChannel* , RsslMsgEvent* );
	RsslReactorCallbackRet processStatusMsg( RsslMsg* , RsslReactorChannel* , RsslMsgEvent* );

	ItemCallbackClient( OmmConsumerImpl& );
	virtual ~ItemCallbackClient();

	ItemCallbackClient();
	ItemCallbackClient( const ItemCallbackClient& );
	ItemCallbackClient& operator=( const ItemCallbackClient& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ItemCallbackClient_h
