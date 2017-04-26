/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmNiProviderImpl_h
#define __thomsonreuters_ema_access_OmmNiProviderImpl_h

#include "OmmBaseImpl.h"
#include "OmmNiProviderActiveConfig.h"
#include "OmmProviderImpl.h"
#include "DirectoryServiceStore.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmProviderErrorClient;
class OmmProvider;

class OmmNiProviderImpl : public OmmProviderImpl, public OmmBaseImpl
{
public :

	OmmNiProviderImpl( OmmProvider*, const OmmNiProviderConfig& );

	OmmNiProviderImpl(OmmProvider*, const OmmNiProviderConfig&, OmmProviderClient&, void* );

	OmmNiProviderImpl( OmmProvider*, const OmmNiProviderConfig&, OmmProviderErrorClient& );

	OmmNiProviderImpl(OmmProvider*, const OmmNiProviderConfig&, OmmProviderClient&, OmmProviderErrorClient&, void*);

	virtual ~OmmNiProviderImpl();

	UInt64 registerClient( const ReqMsg& , OmmProviderClient& , void* closure = 0, UInt64 parentHandle = 0 );

	void unregister( UInt64 );

	Int64 dispatch( Int64 timeOut = 0 ); 

	void addSocket( RsslSocket );

	void removeSocket( RsslSocket );

	void setRsslReactorChannelRole( RsslReactorChannelRole& );

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

private :

	void reLoadConfigSourceDirectory();

	void reLoadUserSubmitSourceDirectory();

	void removeItems();

	void readCustomConfig( EmaConfigImpl* );

	bool realocateBuffer( RsslBuffer* , RsslBuffer* , RsslEncodeIterator* , EmaString& );

	bool isApiDispatching() const;

	Int32 getNextProviderStreamId();

	void returnProviderStreamId(Int32);

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

		StreamInfo( Int32 streamId = 0, UInt16 serviceId = 0, UInt8 domainType = 0) : _streamId( streamId ), _serviceId( serviceId ), _domainType(domainType) {}

		StreamInfo( const StreamInfo& other ) : _streamId( other._streamId ), _serviceId( other._serviceId ), _domainType(other._domainType) {}

		StreamInfo& operator=( const StreamInfo& other )
		{
			if ( this == &other ) return *this;

			_streamId = other._streamId;
			_serviceId = other._serviceId;
			_domainType = other._domainType;

			return *this;
		}

		virtual ~StreamInfo() {}

		Int32		_streamId;
		UInt16		_serviceId;
		UInt8		_domainType;
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
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderImpl_h
