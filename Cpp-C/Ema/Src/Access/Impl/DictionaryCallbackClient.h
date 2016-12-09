/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_DictionaryCallbackClient_h
#define __thomsonreuters_ema_access_DictionaryCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaString.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "ItemCallbackClient.h"

namespace thomsonreuters {

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

	static void destroy( LocalDictionary*& );

	const RsslDataDictionary* getRsslDictionary() const;

	bool isLoaded() const;

	bool load( const EmaString& fldName, const EmaString& enumName );

	DictionaryType getType() const;

private :

	LocalDictionary( OmmCommonImpl&, BaseConfig& );
	virtual ~LocalDictionary();

	static const EmaString		_clientName;
	OmmCommonImpl&				_ommCommonImpl;
	BaseConfig&					_baseConfig;
	RsslDataDictionary			_rsslDictionary;
	bool						_isLoaded;

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

	RsslReactorCallbackRet processRefreshMsg( RsslBuffer*, UInt32 , UInt8 , UInt8 , DictionaryItem* );

	RsslReactorCallbackRet processRefreshMsg( RsslBuffer*, UInt8 , UInt8 , DictionaryItem* );

	RsslReactorCallbackRet processStatusMsg( RsslBuffer*, UInt8 , UInt8 , DictionaryItem* );

	DictionaryItem* getDictionaryItem( const ReqMsg&, OmmConsumerClient&, void* );

	bool downloadDictionary( const Directory& );

	bool isDictionaryReady() const;

	Dictionary* getDefaultDictionary() const;

	static void sendInternalMsg( void* );

private :

	bool downloadDictionaryFromService( const Directory& );

	void loadDictionaryFromFile();

	static const EmaString			_clientName;

	EmaList< ChannelDictionary* >	_channelDictionaryList;

	LocalDictionary*				_localDictionary;

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

	void setName( const EmaString& );
	void setFilter( unsigned char );
	void setCurrentFid( Int32 fid );

	static RsslRet encodeDataDictionaryResp( RsslBuffer&, const EmaString&, unsigned char, RsslInt32 streamId, bool, RsslDataDictionary*, Int32& );

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

	DictionaryItem( OmmBaseImpl&, OmmConsumerClient& , void* );

	DictionaryItem();
	virtual ~DictionaryItem();
	DictionaryItem( const DictionaryItem& );
	DictionaryItem& operator=( const DictionaryItem& );
};

}

}

}

#endif // __thomsonreuters_ema_access_DictionaryCallbackClient_h
