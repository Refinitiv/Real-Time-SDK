/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmProviderImpl_h
#define __thomsonreuters_ema_access_OmmProviderImpl_h

#include "OmmBaseImpl.h"
#include "OmmProviderClient.h"

namespace thomsonreuters {

namespace ema {

namespace access {

typedef const EmaString* EmaStringPtr;

class OmmProviderImpl
{
public :

	OmmProviderImpl();

	virtual ~OmmProviderImpl();

	virtual UInt64 registerClient( const ReqMsg&, OmmProviderClient&, void* closure = 0, UInt64 parentHandle = 0 ) = 0;

	virtual void unregister( UInt64 ) = 0;

	virtual Int64 dispatch( Int64 timeOut = 0 ) = 0;

	virtual void addSocket( RsslSocket ) = 0;

	virtual void removeSocket( RsslSocket ) = 0;

	virtual void setRsslReactorChannelRole( RsslReactorChannelRole& ) = 0;

	virtual void submit( const RefreshMsg&, UInt64 ) = 0;

	virtual void submit( const UpdateMsg&, UInt64 ) = 0;

	virtual void submit( const StatusMsg&, UInt64 ) = 0;

	virtual void submit( const GenericMsg&, UInt64 ) = 0;

	virtual const EmaString& getInstanceName() const = 0;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmProviderImpl_h
