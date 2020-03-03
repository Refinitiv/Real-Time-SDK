/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ErrorClientHandler_h
#define __thomsonreuters_ema_access_ErrorClientHandler_h

#include "OmmConsumerErrorClient.h"
#include "OmmProviderErrorClient.h"
#include "OmmProvider.h"
#include "ClientSession.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ErrorClientHandler
{
public:

	ErrorClientHandler( OmmConsumerErrorClient& );
	
	ErrorClientHandler( OmmProviderErrorClient& );
	
	virtual ~ErrorClientHandler();

	bool hasErrorClientHandler();

	void onInaccessibleLogFile( const EmaString& , const EmaString& );

	void onInvalidHandle( UInt64 handle, const EmaString& );

	void onInvalidUsage( const EmaString& );

	void onInvalidUsage( const EmaString&, Int32 );

	void onMemoryExhaustion( const EmaString& );

	void onSystemError( Int64 , void* , const EmaString& );

	void onJsonConverter( const char*, Int32, RsslReactorChannel*, ClientSession*, OmmProvider* );

private:

	ErrorClientHandler();
	ErrorClientHandler( const ErrorClientHandler& );
	ErrorClientHandler& operator=( const ErrorClientHandler& );

	OmmConsumerErrorClient*		_pConsumerErrorClient;
	OmmProviderErrorClient*		_pProviderErrorClient;
};

}

}

}

#endif // __thomsonreuters_ema_access_ErrorClientHandler_h
