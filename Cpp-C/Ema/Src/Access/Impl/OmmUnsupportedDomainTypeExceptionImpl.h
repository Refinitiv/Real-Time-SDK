/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmUnsupportedDomainTypeExceptionImpl_h
#define __thomsonreuters_ema_access_OmmUnsupportedDomainTypeExceptionImpl_h

#include "OmmUnsupportedDomainTypeException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmUnsupportedDomainTypeExceptionImpl : public OmmUnsupportedDomainTypeException
{
public :

	static void throwException( const EmaString& text, UInt16 domainType );

	OmmUnsupportedDomainTypeExceptionImpl();

	virtual ~OmmUnsupportedDomainTypeExceptionImpl();

private :

	OmmUnsupportedDomainTypeExceptionImpl( const OmmUnsupportedDomainTypeExceptionImpl& );
	OmmUnsupportedDomainTypeExceptionImpl& operator=( const OmmUnsupportedDomainTypeExceptionImpl& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmUnsupportedDomainTypeExceptionImpl_h
