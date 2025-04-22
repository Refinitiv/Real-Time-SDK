/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_DictionaryCallbackClient_h
#define __refinitiv_ema_access_DictionaryCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaString.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "ItemCallbackClient.h"
#include "DataDictionaryImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmCommonImpl;
class OmmConsumerClient;
class Channel;
class Directory;
class DictionaryItem;
class BaseConfig;

class Dictionary
{
public :

	enum DictionaryType
	{
		FileDictionaryEnum = 0,
		ChannelDictionaryEnum
	};

	virtual const RsslDataDictionary* getRsslDictionary() const = 0;

	virtual bool isLoaded() const = 0;

	virtual DictionaryType getType() const = 0;

	Int32 getEnumStreamId() const;
	Int32 getFldStreamId() const;

protected :

	Int32	_enumStreamId;
	Int32	_fldStreamId;

	Dictionary();
	virtual ~Dictionary();

private :

	Dictionary( const Dictionary& );
	Dictionary& operator=( const Dictionary& );
};

class LocalDictionary : public Dictionary
{
public :

	static LocalDictionary* create( OmmCommonImpl&, BaseConfig& );

	static LocalDictionary* create(OmmCommonImpl&, BaseConfig&, RsslDataDictionary*);

	static void destroy( LocalDictionary*& );

	const RsslDataDictionary* getRsslDictionary() const;

	bool isLoaded() const;

	bool load( const EmaString& fldName, const EmaString& enumName, RsslRet& retCode );

	DictionaryType getType() const;

private :

	LocalDictionary( OmmCommonImpl&, BaseConfig& );
	LocalDictionary( OmmCommonImpl&, BaseConfig&, RsslDataDictionary*);
	virtual ~LocalDictionary();

	static const EmaString		_clientName;
	OmmCommonImpl&				_ommCommonImpl;
	BaseConfig&					_baseConfig;
	RsslDataDictionary			_rsslDictionary;
	bool						_isLoaded;
	bool						_deleteRsslDictionary;

	LocalDictionary( const LocalDictionary& );
	LocalDictionary& operator=( const LocalDictionary& );
};

class ChannelDictionary : public Dictionary, public ListLinks< ChannelDictionary >
{
public :

	static ChannelDictionary* create( OmmBaseImpl& );
	static void destroy( ChannelDictionary*& );

	Channel* getChannel() const;
	ChannelDictionary& setChannel( Channel* );

	const RsslDataDictionary* getRsslDictionary() const;

	bool isLoaded() const;

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDictionaryMsgEvent* );

	void notifyStatusToListener( const RsslRDMDictionaryStatus& );

	DictionaryType getType() const;

	void acquireLock();

	void releaseLock();

	void addListener( DictionaryItem* item );

	void removeListener( DictionaryItem* item );

	const EmaVector<DictionaryItem*>* getListenerList() const;

private :

	ChannelDictionary( OmmBaseImpl& );
	virtual ~ChannelDictionary();

	static const EmaString		_clientName;
	OmmBaseImpl&				_ommBaseImpl;
	Channel*					_pChannel;
	RsslDataDictionary			_rsslDictionary;
	bool						_isFldLoaded;
	bool						_isEnumLoaded;
	Mutex						_channelDictLock;
	EmaVector<DictionaryItem*>* 				_pListenerList;

	ChannelDictionary( const ChannelDictionary& );
	ChannelDictionary& operator=( const ChannelDictionary& );
};

class DictionaryCallbackClient
{
public :

	static DictionaryCallbackClient* create( OmmBaseImpl& );

	static void destroy( DictionaryCallbackClient*& );

	void initialize();

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDictionaryMsgEvent* );

	RsslReactorCallbackRet processRefreshMsg( RsslBuffer*, UInt8 , UInt8 , DictionaryItem*, bool );

	RsslReactorCallbackRet processStatusMsg( RsslBuffer*, UInt8 , UInt8 , DictionaryItem* );

	DictionaryItem* getDictionaryItem( const ReqMsg&, OmmConsumerClient&, void* );

	bool downloadDictionary( Directory& );

	bool isDictionaryReady() const;

	Dictionary* getDefaultDictionary() const;

	void encodeAndNotifyStatusMsg(DictionaryItem*, RsslStatusMsg*, RsslBuffer*, UInt32, UInt8, UInt8, RsslEncodeIterator*, const char *);

	static void sendInternalMsg( void* );

	static const EmaString	_rwfFldName;

	static const EmaString	_rwfEnumName;

	void loadDictionaryFromFile();

	bool sentRequest;
private :

	int allocateAndSetEncodeIteratorBuffer(RsslBuffer*, UInt32, UInt8, UInt8, RsslEncodeIterator*, const char *);

	bool downloadDictionaryFromService( const Directory& );

	static const EmaString			_clientName;

	EmaList< ChannelDictionary* >	_channelDictionaryList;

	LocalDictionary*				_localDictionary;

	ChannelDictionary*				_channelDictionary;

	OmmBaseImpl&					_ommBaseImpl;

	RefreshMsg						_refreshMsg;

	StatusMsg						_statusMsg;

	DictionaryCallbackClient( OmmBaseImpl& );

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDictionaryMsgEvent*, DictionaryItem* );

	virtual ~DictionaryCallbackClient();

	DictionaryCallbackClient();
	DictionaryCallbackClient( const DictionaryCallbackClient& );
	DictionaryCallbackClient& operator=( const DictionaryCallbackClient& );
};

class DictionaryItem : public SingleItem
{
public :

	static DictionaryItem* create( OmmBaseImpl&, OmmConsumerClient&, void* );

	bool open( const ReqMsg& );
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();
	void remove();

	const EmaString& getName();
	unsigned char getFilter();
	Int32 getCurrentFid();
	Int32 getStreamId();
	bool isRemoved();
	UInt64 getDictionaryType();

	void setName( const EmaString& );
	void setFilter( unsigned char );
	void setCurrentFid( Int32 fid );
	void setDictionaryType( UInt64 );

	static RsslRet encodeDataDictionaryResp( DictionaryItem&, RsslBuffer&, const EmaString&, unsigned char, RsslInt32 streamId, bool, RsslDataDictionary*, Int32& );

	static void ScheduleRemove( void* );

	ItemType getType() const
	{
		return Item::DictionaryItemEnum;
	}

private :

	static const EmaString		_clientName;
	EmaString					_name;
	UInt8						_filter;
	Int32						_currentFid;
	bool						_isRemoved;
	UInt64						_dictionaryType;

	DictionaryItem( OmmBaseImpl&, OmmConsumerClient& , void* );

	DictionaryItem();
	virtual ~DictionaryItem();
	DictionaryItem( const DictionaryItem& );
	DictionaryItem& operator=( const DictionaryItem& );
};

class NiProviderDictionaryItem : public NiProviderSingleItem
{
public:

	static NiProviderDictionaryItem* create(OmmBaseImpl&, OmmProviderClient&, void*);

	bool modify(const ReqMsg& reqMsg);

	ItemType getType() const
	{
		return Item::NiProviderDictionaryItemEnum;
	}

private:

	static const EmaString		_clientName;

	NiProviderDictionaryItem(OmmBaseImpl&, OmmProviderClient&, ItemWatchList*, void*);

	NiProviderDictionaryItem();
	virtual ~NiProviderDictionaryItem();
	NiProviderDictionaryItem(const NiProviderDictionaryItem&);
	NiProviderDictionaryItem& operator=(const NiProviderDictionaryItem&);
};

class IProviderDictionaryItem : public IProviderSingleItem
{
public:

	static IProviderDictionaryItem* create(OmmServerBaseImpl&, OmmProviderClient&, void*);

	bool modify(const ReqMsg& reqMsg);

	ItemType getType() const
	{
		return Item::IProviderDictionaryItemEnum;
	}

private:

	static const EmaString		_clientName;

	IProviderDictionaryItem(OmmServerBaseImpl&, OmmProviderClient&, ItemWatchList*, void*);

	IProviderDictionaryItem();
	virtual ~IProviderDictionaryItem();
	IProviderDictionaryItem(const IProviderDictionaryItem&);
	IProviderDictionaryItem& operator=(const IProviderDictionaryItem&);
};

}

}

}

#endif // __refinitiv_ema_access_DictionaryCallbackClient_h
