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
#include "OmmNiProviderClient.h"	// from Include (unchanged)
#include "OmmNiProviderErrorClient.h"	// from Include (unchanged)
#include "Thread.h"
#include "OmmNiProviderActiveConfig.h"
#include "OmmClient.h"

namespace thomsonreuters {

namespace ema {

namespace access {

typedef const EmaString* EmaStringPtr;

class ItemCallbackClient;

class OmmNiProviderImpl : public OmmBaseImpl
{
public :

	OmmNiProviderImpl( const OmmNiProviderConfig& );
	OmmNiProviderImpl( const OmmNiProviderConfig& , OmmNiProviderErrorClient& );

	virtual ~OmmNiProviderImpl();
  void uninitialize();

	UInt64 registerClient( const ReqMsg& , OmmNiProviderClient& , void* closure = 0, UInt64 parentHandle = 0 ); 
	Int64 dispatch( Int64 timeOut = 0 ); 
	void addSocket( RsslSocket );
	void removeSocket( RsslSocket );
  void setRsslReactorChannelRole( RsslReactorChannelRole& );
  void submit( const RefreshMsg&, UInt64 handle = 0 );
  void submit( const UpdateMsg&, UInt64 handle = 0 );
  void submit( const StatusMsg&, UInt64 handle = 0 );

private :
  OmmClient< OmmNiProviderClient >* _theClient;
  OmmNiProviderActiveConfig _activeConfig;
  OmmNiProviderErrorClient* _ommNiProviderErrorClient;

  void decodeServiceNameAndId( const Channel* channel, RwfBuffer* buffer ); 


	class UInt64rHasher {
	public:
		size_t operator()( const UInt64 & ) const;
	};

	class UInt64Equal_To {
	public:
		bool operator()( const UInt64 & , const UInt64 & ) const;
	};

	class EmaStringPtrHasher {
	public:
		size_t operator()( const EmaStringPtr & ) const;
	};

	class EmaStringPtrEqual_To {
	public:
		bool operator()( const EmaStringPtr & , const EmaStringPtr & ) const;
	};

  typedef HashTable< UInt64 , Int32 , UInt64rHasher , UInt64Equal_To > HandleToStreamId;
  typedef HashTable< EmaStringPtr , UInt64 , EmaStringPtrHasher , EmaStringPtrEqual_To > ServiceNameToServiceId;

	HandleToStreamId					_handleToStreamId;
	ServiceNameToServiceId				_serviceNameToServiceId;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderImpl_h
