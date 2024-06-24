/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmInvalidConfigurationExceptionImpl_h
#define __refinitiv_ema_access_OmmInvalidConfigurationExceptionImpl_h

#include "OmmInvalidConfigurationException.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmInvalidConfigurationExceptionImpl : public OmmInvalidConfigurationException
{
public :

	static void throwException( const EmaString& text );

	OmmInvalidConfigurationExceptionImpl();

	virtual ~OmmInvalidConfigurationExceptionImpl();

private :

	OmmInvalidConfigurationExceptionImpl( const OmmInvalidConfigurationExceptionImpl& );
	OmmInvalidConfigurationExceptionImpl& operator=( const OmmInvalidConfigurationExceptionImpl& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmInvalidConfigurationExceptionImpl_h
