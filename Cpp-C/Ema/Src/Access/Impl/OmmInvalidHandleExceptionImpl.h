/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmInvalidHandleExceptionImpl_h
#define __refinitiv_ema_access_OmmInvalidHandleExceptionImpl_h

#include "OmmInvalidHandleException.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmInvalidHandleExceptionImpl : public OmmInvalidHandleException
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

#endif // __refinitiv_ema_access_OmmInvalidHandleExceptionImpl_h
