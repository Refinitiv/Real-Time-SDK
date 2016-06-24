/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ErrorClientHandler_h
#define __thomsonreuters_ema_access_ErrorClientHandler_h

#include "OmmConsumerErrorClient.h"
#include "OmmProviderErrorClient.h"

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

	void onMemoryExhaustion( const EmaString& );

	void onSystemError( Int64 , void* , const EmaString& );

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
