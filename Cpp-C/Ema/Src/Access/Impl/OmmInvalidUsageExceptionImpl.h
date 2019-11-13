/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmInvalidUsageExceptionImpl_h
#define __thomsonreuters_ema_access_OmmInvalidUsageExceptionImpl_h

#include "OmmInvalidUsageException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmInvalidUsageExceptionImpl : public OmmInvalidUsageException
{
public :

	static void throwException( const EmaString&, Int32 );

	static void throwException( const char*, Int32 );

	OmmInvalidUsageExceptionImpl();

	virtual ~OmmInvalidUsageExceptionImpl();

private :

	OmmInvalidUsageExceptionImpl( const OmmInvalidUsageExceptionImpl& );
	OmmInvalidUsageExceptionImpl& operator=( const OmmInvalidUsageExceptionImpl& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmInvalidUsageExceptionImpl_h
