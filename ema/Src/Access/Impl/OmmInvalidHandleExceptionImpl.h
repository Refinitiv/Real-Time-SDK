/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmInvalidHandleExceptionImpl_h
#define __thomsonreuters_ema_access_OmmInvalidHandleExceptionImpl_h

#include "OmmInvalidHandleException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmInvalidHandleExceptionImpl : public OmmInvalidHandleException
{
public :

	static void throwException( const EmaString& , UInt64 handle );

	static void throwException( const char* , UInt64 handle );

	OmmInvalidHandleExceptionImpl( UInt64 handle );

	virtual ~OmmInvalidHandleExceptionImpl();

private :

	OmmInvalidHandleExceptionImpl();
	OmmInvalidHandleExceptionImpl( const OmmInvalidHandleExceptionImpl& );
	OmmInvalidHandleExceptionImpl& operator=( const OmmInvalidHandleExceptionImpl& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmInvalidHandleExceptionImpl_h
