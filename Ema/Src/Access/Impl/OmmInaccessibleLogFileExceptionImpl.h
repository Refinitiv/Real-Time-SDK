/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmInaccessibleLogFileExceptionImpl_h
#define __thomsonreuters_ema_access_OmmInaccessibleLogFileExceptionImpl_h

#include "OmmInaccessibleLogFileException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmInaccessibleLogFileExceptionImpl : public OmmInaccessibleLogFileException
{
public :

	static void throwException( const EmaString&, const EmaString& );

	static void throwException( const EmaString&, const char* );

	OmmInaccessibleLogFileExceptionImpl( const EmaString& );

	virtual ~OmmInaccessibleLogFileExceptionImpl();

private :

	OmmInaccessibleLogFileExceptionImpl( const OmmInaccessibleLogFileExceptionImpl& );
	OmmInaccessibleLogFileExceptionImpl& operator=( const OmmInaccessibleLogFileExceptionImpl& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmInaccessibleLogFileExceptionImpl_h
