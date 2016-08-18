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

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmProviderErrorClient;

typedef const EmaString* EmaStringPtr;

class OmmNiProviderImpl : public OmmProviderImpl, public OmmBaseImpl
{
public :

	OmmNiProviderImpl( const OmmNiProviderConfig& );

	OmmNiProviderImpl( const OmmNiProviderConfig&, OmmProviderErrorClient& );

	virtual ~OmmNiProviderImpl();

	UInt64 registerClient( const ReqMsg& , OmmProviderClient& , void* closure = 0, UInt64 parentHandle = 0 );

	void unregister( UInt64 );

	Int64 dispatch( Int64 timeOut = 0 ); 

	void addSocket( RsslSocket );

	void removeSocket( RsslSocket );

	void setRsslReactorChannelRole( RsslReactorChannelRole& );

	void submit( const RefreshMsg&, UInt64 );

	void submit( const UpdateMsg&, UInt64 );

	void submit( const StatusMsg&, UInt64 );

	void submit( const GenericMsg&, UInt64 );

	void loadDirectory();

	void reLoadDirectory();

	void storeUserSubmitSourceDirectory( RsslMsg* );

	void loadDictionary();

	void createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& );

	void createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& );

	void processChannelEvent( RsslReactorChannelEvent* );

	const EmaString& getInstanceName() const;

	bool getServiceId( const EmaString& , UInt64& );

	bool getServiceName( UInt64 , EmaString& );

private :

	void reLoadConfigSourceDirectory();

	void reLoadUserSubmitSourceDirectory();

	void removeItems();

	void readCustomConfig( EmaConfigImpl* );

	void populateDefaultService( ServiceConfig& ) const;

	void freeMemory( RsslRDMDirectoryRefresh& , RsslBuffer& );

	bool decodeSourceDirectory( RwfBuffer* , RsslBuffer* , EmaString& );

	bool decodeSourceDirectoryKeyUInt( RsslMap& , RsslDecodeIterator& , EmaString& );

	bool realocateBuffer( RsslBuffer* , RsslBuffer* , RsslEncodeIterator* , EmaString& );

	bool isApiDispatching() const;

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

	class StreamInfo
	{
	public :

		StreamInfo( Int32 streamId = 0, UInt16 serviceId = 0 ) : _streamId( streamId ), _serviceId( serviceId ) {}

		StreamInfo( const StreamInfo& other ) : _streamId( other._streamId ), _serviceId( other._serviceId ) {}

		StreamInfo& operator=( const StreamInfo& other )
		{
			if ( this == &other ) return *this;

			_streamId = other._streamId;
			_serviceId = other._serviceId;

			return *this;
		}

		virtual ~StreamInfo() {}

		Int32		_streamId;
		UInt16		_serviceId;
	};

	typedef const StreamInfo* StreamInfoPtr;

	typedef HashTable< UInt64 , StreamInfoPtr , UInt64rHasher , UInt64Equal_To > HandleToStreamInfo;
	typedef HashTable< EmaStringPtr , UInt64 , EmaStringPtrHasher , EmaStringPtrEqual_To > ServiceNameToServiceId;
	typedef HashTable< UInt64 , EmaStringPtr , UInt64rHasher , UInt64Equal_To > ServiceIdToServiceName;
	typedef EmaVector< EmaStringPtr > ServiceNameList;
	typedef EmaVector< StreamInfoPtr > StreamInfoList;

	OmmNiProviderActiveConfig		_activeConfig;
	HandleToStreamInfo				_handleToStreamInfo;
	ServiceNameToServiceId			_serviceNameToServiceId;
	ServiceIdToServiceName			_serviceIdToServiceName;
	ServiceNameList					_serviceNameList;
	StreamInfoList					_streamInfoList;
	bool							_bIsStreamIdZeroRefreshSubmitted;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderImpl_h
