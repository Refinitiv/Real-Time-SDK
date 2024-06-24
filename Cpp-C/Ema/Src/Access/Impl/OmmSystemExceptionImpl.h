/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmSystemExceptionImpl_h
#define __refinitiv_ema_access_OmmSystemExceptionImpl_h

#include "OmmSystemException.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmSystemExceptionImpl : public OmmSystemException
{
public :

	static void throwException( const char* , Int64 , void* );

	static void throwException( const EmaString& , Int64 , void* );

	OmmSystemExceptionImpl();

	virtual ~OmmSystemExceptionImpl();

private :

	OmmSystemExceptionImpl( const OmmSystemExceptionImpl& );
	OmmSystemExceptionImpl& operator=( const OmmSystemExceptionImpl& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmSystemExceptionImpl_h
