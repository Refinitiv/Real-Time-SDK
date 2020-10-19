/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmNiProviderImpl_h
#define __refinitiv_ema_access_OmmNiProviderImpl_h

#include "OmmBaseImpl.h"
#include "OmmNiProviderActiveConfig.h"
#include "OmmProviderImpl.h"
#include "DirectoryServiceStore.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmProviderErrorClient;
class OmmProvider;

class OmmNiProviderImpl : public OmmProviderImpl, public OmmBaseImpl, public DirectoryServiceStoreClient
{
public :

	OmmNiProviderImpl( OmmProvider*, const OmmNiProviderConfig& );

	OmmNiProviderImpl(OmmProvider*, const OmmNiProviderConfig&, OmmProviderClient&, void* );

	OmmNiProviderImpl( OmmProvider*, const OmmNiProviderConfig&, OmmProviderErrorClient& );

	OmmNiProviderImpl(OmmProvider*, const OmmNiProviderConfig&, OmmProviderClient&, OmmProviderErrorClient&, void*);

	OmmNiProviderImpl(const OmmNiProviderConfig&, OmmProviderClient&); //for internal UnitTest only

	virtual ~OmmNiProviderImpl();

	UInt64 registerClient( const ReqMsg& , OmmProviderClient& , void* closure = 0, UInt64 parentHandle = 0 );

	void unregister( UInt64 );

	Int64 dispatch( Int64 timeOut = 0 ); 

	void addSocket( RsslSocket );

	void removeSocket( RsslSocket );

	void setRsslReactorChannelRole( RsslReactorChannelRole&, RsslReactorOAuthCredential* );

	void reissue(const ReqMsg&, UInt64);

	void submit( const RefreshMsg&, UInt64 );

	void submit( const UpdateMsg&, UInt64 );

	void submit( const StatusMsg&, UInt64 );

	void submit( const GenericMsg&, UInt64 );

	void submit( const AckMsg&, UInt64 );

	void loadDirectory();

	void reLoadDirectory();

	bool storeUserSubmitSourceDirectory( RsslMsg* );

	void loadDictionary();

	void createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& );

	void createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& );

	void processChannelEvent( RsslReactorChannelEvent* );

	const EmaString& getInstanceName() const;

	OmmProviderConfig::ProviderRole getProviderRole() const;

	OmmProvider* getProvider() const;

	bool getServiceId( const EmaString& , UInt64& );

	bool getServiceName( UInt64 , EmaString& );

	ImplementationType getImplType();

	ItemWatchList& getItemWatchList();

	void onServiceDelete(ClientSession* clientSession, RsslUInt serviceId);

	UInt32 getRequestTimeout();

	Int32 getNextProviderStreamId();

	void returnProviderStreamId(Int32);

	void setActiveRsslReactorChannel( Channel* activeChannel );

	void unsetActiveRsslReactorChannel( Channel* cancelChannel);

	DirectoryServiceStore& getDirectoryServiceStore() const;

    void getConnectedClientChannelInfo(EmaVector<ChannelInformation>&);
    void getChannelInformation(ChannelInformation&);

	void getConnectedClientChannelStats(UInt64, ChannelStatistics&);

	void modifyIOCtl(Int32 code, Int32 value, UInt64 handle);

	void closeChannel(UInt64 clientHandle);

private :

	void reLoadConfigSourceDirectory();

	void reLoadUserSubmitSourceDirectory();

	void removeItems();

	void readCustomConfig( EmaConfigImpl* );

	bool realocateBuffer( RsslBuffer* , RsslBuffer* , RsslEncodeIterator* , EmaString& );

	bool isApiDispatching() const;

	UInt64 generateHandle(UInt64);

	OmmNiProviderImpl();
	OmmNiProviderImpl( const OmmNiProviderImpl& );
	OmmNiProviderImpl& operator=( const OmmNiProviderImpl& );

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

	class StreamInfo
	{
	public :

		enum StreamType
		{
			ConsumingEnum = 1,
			ProvidingEnum = 2
		};

		StreamInfo(StreamType streamType, Int32 streamId = 0, UInt16 serviceId = 0, UInt8 domainType = 0) :
			_streamType(streamType), _streamId(streamId), _serviceId(serviceId), _domainType(domainType), _actualHandle(0) {}

		StreamInfo(const StreamInfo& other) : _streamType(other._streamType), _streamId(other._streamId), _serviceId(other._serviceId),
			_domainType(other._domainType), _actualHandle(other._actualHandle) {}

		StreamInfo& operator=( const StreamInfo& other )
		{
			if ( this == &other ) return *this;

			_streamId = other._streamId;
			_serviceId = other._serviceId;
			_domainType = other._domainType;
			_actualHandle = other._actualHandle;
			_streamType = other._streamType;

			return *this;
		}

		virtual ~StreamInfo() {}

		Int32		_streamId;
		UInt16		_serviceId;
		UInt8		_domainType;
		UInt64		_actualHandle;
		StreamType	_streamType;
	};

	typedef const StreamInfo* StreamInfoPtr;

	typedef HashTable< UInt64 , StreamInfoPtr , UInt64rHasher , UInt64Equal_To > HandleToStreamInfo;
	typedef EmaVector< StreamInfoPtr > StreamInfoList;

	OmmNiProviderActiveConfig						_activeConfig;
	HandleToStreamInfo								_handleToStreamInfo;
	StreamInfoList									_streamInfoList;
	OmmNiProviderDirectoryStore						_ommNiProviderDirectoryStore;
	bool											_bIsStreamIdZeroRefreshSubmitted;
	RsslRDMDirectoryMsg								_rsslDirectoryMsg;
	RsslBuffer										_rsslDirectoryMsgBuffer;

	Int32				    						_nextProviderStreamId;
	EmaList<StreamId*>			 					_reusedProviderStreamIds;
	Channel*										_activeChannel;
	ItemWatchList									_itemWatchList;
};

}

}

}

#endif // __refinitiv_ema_access_OmmNiProviderImpl_h
