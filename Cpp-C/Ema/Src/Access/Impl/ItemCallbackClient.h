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
#include "HashTable.h"
#include "EmaList.h"
#include "OmmState.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "OmmLoggerClient.h"
#include "OmmConsumerEvent.h"
#include "OmmProviderEvent.h"
#include "EmaVector.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmConsumerClient;
class OmmProviderClient;
class OmmConsumerImpl;
class OmmProviderImpl;

class OmmBaseImpl;
class OmmCommonImpl;
class OmmServerBaseImpl;
class ReqMsg;
class PostMsg;
class Channel;
class Directory;
class ReqMsgEncoder;
class OmmState;
class Item;
class TunnelStreamRequest;
class StreamId;
class ProviderItem;
class ItemWatchList;
class ClientSession;
class TimeOut;
class ClosedStatusInfo;

class ItemList
{
public :

	static ItemList* create( OmmCommonImpl& );

	static void destroy( ItemList*& );

	Int32 addItem( Item* );

	void removeItem( Item* );

private :

	static const EmaString		_clientName;

	EmaList< Item* >			_list;
	OmmCommonImpl&				_ommCommonImpl;

	ItemList( OmmCommonImpl& );
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
		NiProviderSingleItemEnum,
		IProviderSingleItemEnum,
		BatchItemEnum,
		LoginItemEnum,
		NiProviderLoginItemEnum,
		DirectoryItemEnum,
		DictionaryItemEnum,
		NiProviderDictionaryItemEnum,
		IProviderDictionaryItemEnum,
		TunnelItemEnum,
		SubItemEnum
	};

	const EmaString& getTypeAsString();

	static void destroy( Item*& );

	Int32 getStreamId() const;

	UInt8 getDomainType() const {
		return _domainType;
	}

	ClosedStatusInfo*	getClosedStatusInfo();

	virtual const Directory* getDirectory() = 0;

	virtual Int32 getNextStreamId(int numOfItem = 0) = 0;

	virtual bool open( const ReqMsg& ) = 0;
	virtual bool modify( const ReqMsg& ) = 0;
	virtual bool submit( const PostMsg& ) { return false; }
	virtual bool submit( const GenericMsg& ) = 0;
	virtual bool close() = 0;
	virtual void remove() = 0;

	virtual ItemType getType() const = 0;

	virtual void onAllMsg( const Msg& ) = 0;
	virtual void onRefreshMsg( const RefreshMsg& ) = 0;
	virtual void onUpdateMsg( const UpdateMsg& ) {}
	virtual void onStatusMsg( const StatusMsg& ) = 0;
	virtual void onAckMsg( const AckMsg& ) {}
	virtual void onGenericMsg( const GenericMsg& ) = 0;

protected :

	UInt8				_domainType;
	Int32				_streamId;
	ClosedStatusInfo*	_closedStatusInfo;

	Item();
	virtual ~Item();

private :

	Item( const Item& );
	Item& operator=( const Item& );
};

class ClosedStatusInfo
{
public :

	ClosedStatusInfo( Item* , const ReqMsgEncoder& , const EmaString& );

	ClosedStatusInfo( Item* , const TunnelStreamRequest& , const EmaString& );

	ClosedStatusInfo( ProviderItem*, const EmaString& );

	virtual ~ClosedStatusInfo();

	UInt16 getDomainType() const { return _domainType; }

	const EmaString& getStatusText() const { return _statusText; }

	const EmaString& getServiceName() const { return _serviceName; }

	RsslMsgKey* getRsslMsgKey() { return &_msgKey; }

	Item* getItem() const { return _pItem; }

	bool getPrivateStream() const { return _privateStream; }

	Int32 getStreamId() const { return _streamId; }

	const RsslState& getRsslState() const { return _rsslState; }

private :

	RsslMsgKey		_msgKey;
	EmaString		_statusText;
	EmaString		_serviceName;
	UInt16			_domainType;
	Int32			_streamId;
	Item*			_pItem;
	bool			_privateStream;
	RsslState       _rsslState;
};

class ConsumerItem : public Item
{
public :

	void onAllMsg( const Msg& );
	void onRefreshMsg( const RefreshMsg& );
	void onUpdateMsg( const UpdateMsg& );
	void onStatusMsg( const StatusMsg& );
	void onAckMsg( const AckMsg& );
	void onGenericMsg( const GenericMsg& );

	OmmBaseImpl& getImpl();

	Int32 getNextStreamId(int numOfItem = 0);

protected :

	ConsumerItem( OmmBaseImpl&, OmmConsumerClient&, void* , Item* );
	virtual ~ConsumerItem();

	OmmConsumerClient&		_client;
	OmmConsumerEvent		_event;
	OmmBaseImpl&			_ommBaseImpl;

private:

	ConsumerItem();
	ConsumerItem( const ConsumerItem& );
	ConsumerItem& operator=( const ConsumerItem& );
};

class ProviderItem : public Item
{
public :

	void onAllMsg( const Msg& );
	void onRefreshMsg( const RefreshMsg& );
	void onStatusMsg( const StatusMsg& );
	void onGenericMsg( const GenericMsg& );

	virtual bool submit( RsslRequestMsg* ) = 0;
	virtual bool submit( RsslCloseMsg*) = 0;

	const Directory* getDirectory();

	virtual void scheduleItemClosedStatus( const ReqMsgEncoder&, const EmaString& ) = 0;

	virtual bool processInitialResp( RsslRefreshMsg* rsslRefreshMsg, bool checkPrivateStream );

	bool modify( const ReqMsg& );

	const RsslMsgKey& getRsslMsgKey();

	bool isPrivateStream();

	void setProvider(OmmProvider* );

	const ClientSession* getClientSession();

	void setClientSession( const ClientSession* );

	TimeOut* getTimeOut();

	void cancelReqTimerEvent();

protected :

	ProviderItem(OmmCommonImpl&, OmmProviderClient&, ItemWatchList*, void*);

	virtual ~ProviderItem();

	virtual void scheduleItemClosedRecoverableStatus(const EmaString&) = 0;

	Directory*				_pDirectory;
	ItemWatchList*			_pItemWatchList;
	OmmProviderClient&		_client;
	OmmProviderEvent		_event;
	RsslMsgKey				_msgKey;
	bool					_isPrivateStream;
	TimeOut*				_pTimeOut;
	bool					_receivedInitResp;
	OmmCommonImpl&			_ommCommonImpl;
	bool					_timeOutExpired;
	bool					_specifiedServiceInReq;

	friend class			ItemWatchList;

private:

	const ClientSession*	_pClientSession;

	ProviderItem();
	ProviderItem(const ProviderItem&);
	ProviderItem& operator=(const ProviderItem&);
};

class SingleItem : public ConsumerItem
{
public :

	static SingleItem* create( OmmBaseImpl& , OmmConsumerClient&, void* , Item* );

	const Directory* getDirectory();

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();

	ItemType getType() const;
	void scheduleItemClosedStatus(const ReqMsgEncoder&, const EmaString&);

protected :

	SingleItem( OmmBaseImpl& , OmmConsumerClient& , void* , Item* );

	virtual ~SingleItem();

private :

	bool submit( RsslGenericMsg* );
	bool submit( RsslRequestMsg* );
	bool submit( RsslCloseMsg* );
	bool submit( RsslPostMsg* );

	static const EmaString		_clientName;

	const Directory*			_pDirectory;

	SingleItem();
	SingleItem( const SingleItem& );
	SingleItem& operator=( const SingleItem& );
};

class NiProviderSingleItem : public ProviderItem
{
public:

	static NiProviderSingleItem* create( OmmBaseImpl&, OmmProviderClient&, void* , Item* );

	bool open( const ReqMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();

	ItemType getType() const;

	OmmBaseImpl& getImpl();

	Int32 getNextStreamId(int numOfItem = 0);

protected:

	NiProviderSingleItem( OmmBaseImpl&, OmmProviderClient&, ItemWatchList*, void*, Item* );

	virtual ~NiProviderSingleItem();

	void scheduleItemClosedStatus( const ReqMsgEncoder&, const EmaString& );

	void scheduleItemClosedRecoverableStatus(const EmaString&);

	OmmBaseImpl&		_ommBaseImpl;

private:

	bool submit( RsslRequestMsg* );
	bool submit( RsslCloseMsg* );

	static const EmaString		_clientName;

	NiProviderSingleItem();
	NiProviderSingleItem( const NiProviderSingleItem& );
	NiProviderSingleItem& operator=( const NiProviderSingleItem& );
};

class IProviderSingleItem : public ProviderItem
{
public:

	static IProviderSingleItem* create(OmmServerBaseImpl&, OmmProviderClient&, void*, Item*);

	bool open(const ReqMsg&);
	bool submit(const GenericMsg&);
	bool close();
	void remove();

	ItemType getType() const;

	OmmServerBaseImpl& getImpl();

	Int32 getNextStreamId(int numOfItem = 0);

protected:

	IProviderSingleItem(OmmServerBaseImpl&, OmmProviderClient&, ItemWatchList*, void*, Item*);

	virtual ~IProviderSingleItem();

	void scheduleItemClosedStatus(const ReqMsgEncoder&, const EmaString&);

	void scheduleItemClosedRecoverableStatus(const EmaString&);

	OmmServerBaseImpl&			_ommServerBaseImpl;

private:

	bool submit(RsslGenericMsg*);
	bool submit(RsslRequestMsg*);
	bool submit(RsslCloseMsg*);

	static const EmaString		_clientName;

	IProviderSingleItem();
	IProviderSingleItem(const IProviderSingleItem&);
	IProviderSingleItem& operator=(const IProviderSingleItem&);
};

class BatchItem : public SingleItem
{
public :

	static BatchItem* create( OmmBaseImpl& , OmmConsumerClient& , void* );

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();

	bool addBatchItems( UInt32 batchSize );

	const EmaVector<SingleItem*>& getSingleItemList();

	SingleItem* getSingleItem( Int32 );

	void decreaseItemCount();

	ItemType getType() const;

	Item* getItem( Int32 );

private :

	static const EmaString		_clientName;

	EmaVector<SingleItem*>		_singleItemList;
	UInt32						_itemCount;

	BatchItem( OmmBaseImpl& , OmmConsumerClient& , void* );
	BatchItem();
	virtual ~BatchItem();
	BatchItem( const BatchItem& );
	BatchItem& operator=( const BatchItem& );
};

class TunnelItem : public ConsumerItem
{
public :

	static TunnelItem* create( OmmBaseImpl& , OmmConsumerClient& , void* );

	ItemType getType() const;

	const Directory* getDirectory();

	bool open( const TunnelStreamRequest& );
	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();

	void rsslTunnelStream( RsslTunnelStream* );

	UInt32 addSubItem( Item* , Int32 streamId = 0 );
	void removeSubItem( UInt32 );
	void returnSubItemStreamId( Int32 );

	Item* getSubItem( UInt32 );

	bool submitSubItemMsg( RsslMsg* );

protected :

	TunnelItem( OmmBaseImpl& , OmmConsumerClient&, void* );
	virtual ~TunnelItem();

	Int32 getSubItemStreamId();
	void scheduleItemClosedStatus( const TunnelStreamRequest& , const EmaString& );

private :

	bool submit( const TunnelStreamRequest& );

	static const EmaString		_clientName;
	Int32                       nextSubItemStreamId;		// todo ... use underscore
	EmaList< StreamId* >        returnedSubItemStreamIds;
	const Directory*			_pDirectory;
	RsslTunnelStream*			_pRsslTunnelStream;
	EmaVector< Item* >			_subItems;
    static const Int32          _startingSubItemStreamId = 5;
};

class SubItem : public ConsumerItem
{
public :

	static SubItem* create( OmmBaseImpl& , OmmConsumerClient&, void* , Item* );

	ItemType getType() const;

	const Directory* getDirectory();

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();

protected :

	SubItem( OmmBaseImpl& , OmmConsumerClient&, void* , Item* );

	virtual ~SubItem();

	void scheduleItemClosedStatus( const ReqMsgEncoder& , const EmaString& );

private :

	static const EmaString		_clientName;
};

typedef Item* ItemPtr;

class ItemCallbackClient
{
public :

	static void sendItemClosedStatus( void* );

	static ItemCallbackClient* create( OmmBaseImpl& );

	static ItemCallbackClient* create( OmmServerBaseImpl& );

	static void destroy( ItemCallbackClient*& );

	void initialize();

	UInt64 registerClient( const ReqMsg& , OmmConsumerClient&, void* , UInt64 );

	UInt64 registerClient( const ReqMsg&, OmmProviderClient&, void*, UInt64 );

	UInt64 registerClient( const TunnelStreamRequest& , OmmConsumerClient& , void* );

	void reissue( const ReqMsg& , UInt64 );

	void unregister( UInt64 );

	void submit( const PostMsg& , UInt64 );

	void submit( const GenericMsg& , UInt64 );

	RsslReactorCallbackRet processCallback( RsslReactor* , RsslReactorChannel* , RsslMsgEvent* );

	RsslReactorCallbackRet processCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* );

	RsslReactorCallbackRet processCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* );

	RsslReactorCallbackRet processCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* );

	RsslReactorCallbackRet processIProviderMsgCallback(RsslReactor* pReactor, RsslReactorChannel*, RsslMsgEvent* pMsgEvent, const RsslDataDictionary*);

	void addToList( Item* );
	void removeFromList( Item* );

	void addToMap( Item* );

	void removeFromMap( Item* );
	void addToItemMap( Item* );

	bool isStreamIdInUse( int );

	bool splitAndSendSingleRequest( const ReqMsg& , OmmConsumerClient& , void* );

	Int32 getNextStreamId(UInt32 numberOfBatchItems = 0);

	bool nextStreamIdWrapAround(UInt32);

private :

	static const EmaString			_clientName;

	RefreshMsg						_refreshMsg;

	UpdateMsg						_updateMsg;

	StatusMsg						_statusMsg;

	GenericMsg						_genericMsg;

	AckMsg							_ackMsg;

	OmmCommonImpl&					_ommCommonImpl;

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

	class Int32rHasher {
	public:
		Int32 operator()(const Int32 &) const;
	};

	class Int32Equal_To {
	public:
		bool operator()(const Int32 &, const Int32 &) const;
	};

	typedef HashTable< Int32, ItemPtr, Int32rHasher, Int32Equal_To > StreamIdMap;

	StreamIdMap						_streamIdMap;

	Int32							_nextStreamId;

	bool							_nextStreamIdWrapAround;

	Mutex							_streamIdAccessMutex;

	RsslReactorCallbackRet processAckMsg( RsslMsg*, RsslReactorChannel* pRsslReactorChannel, Item*, const RsslDataDictionary* );
	RsslReactorCallbackRet processGenericMsg( RsslMsg*, RsslReactorChannel* pRsslReactorChannel, Item*, const RsslDataDictionary* );
	RsslReactorCallbackRet processRefreshMsg( RsslMsg*, RsslReactorChannel* pRsslReactorChannel, Item*, const RsslDataDictionary* );
	RsslReactorCallbackRet processUpdateMsg( RsslMsg*, RsslReactorChannel* pRsslReactorChannel, Item*, const RsslDataDictionary* );
	RsslReactorCallbackRet processStatusMsg( RsslMsg*, RsslReactorChannel* pRsslReactorChannel, Item*, const RsslDataDictionary* );

	RsslReactorCallbackRet processProviderCallback(RsslReactor*, RsslReactorChannel*, RsslMsg*, ProviderItem*, const RsslDataDictionary*);

	RsslReactorCallbackRet processAckMsg( RsslTunnelStream* , RsslTunnelStreamMsgEvent*, Item* );
	RsslReactorCallbackRet processGenericMsg( RsslTunnelStream* , RsslTunnelStreamMsgEvent*, Item* );
	RsslReactorCallbackRet processRefreshMsg( RsslTunnelStream* , RsslTunnelStreamMsgEvent*, Item* );
	RsslReactorCallbackRet processUpdateMsg( RsslTunnelStream* , RsslTunnelStreamMsgEvent*, Item* );
	RsslReactorCallbackRet processStatusMsg( RsslTunnelStream* , RsslTunnelStreamMsgEvent*, Item* );

	ItemCallbackClient( OmmBaseImpl& );
	ItemCallbackClient( OmmServerBaseImpl& );
	virtual ~ItemCallbackClient();

	ItemCallbackClient();
	ItemCallbackClient( const ItemCallbackClient& );
	ItemCallbackClient& operator=( const ItemCallbackClient& );
};

class ItemWatchList
{
public:

	ItemWatchList();

	void addItem(ProviderItem*);

	void removeItem(ProviderItem*);

	static void itemRequestTimeout(void*);

	void processChannelEvent(RsslReactorChannelEvent*);

	void processCloseLogin(const ClientSession*);

	void processServiceDelete(const ClientSession*, UInt64);

	static RsslMsg* processRsslMsg(RsslMsg* msg, ProviderItem* providerItem, bool& addedMsgKey);

	virtual ~ItemWatchList();

private:

	void notifyClosedRecoverableStatusMessage();

	EmaVector<ProviderItem*>	_itemList;

	ItemWatchList(const ItemWatchList&);
	ItemWatchList& operator=(const ItemWatchList&);
};

}

}

}

#endif // __thomsonreuters_ema_access_ItemCallbackClient_h
