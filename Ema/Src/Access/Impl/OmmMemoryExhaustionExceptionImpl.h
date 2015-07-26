/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmMemoryExhaustionExceptionImpl_h
#define __thomsonreuters_ema_access_OmmMemoryExhaustionExceptionImpl_h

#include "OmmMemoryExhaustionException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmMemoryExhaustionExceptionImpl : public OmmMemoryExhaustionException
{
public :

	static void throwException( const char* );

	OmmMemoryExhaustionExceptionImpl();

	virtual ~OmmMemoryExhaustionExceptionImpl();

private :

	OmmMemoryExhaustionExceptionImpl( const OmmMemoryExhaustionExceptionImpl& );
	OmmMemoryExhaustionExceptionImpl& operator=( const OmmMemoryExhaustionExceptionImpl& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmMemoryExhaustionExceptionImpl_h
