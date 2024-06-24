/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmMemoryExhaustionExceptionImpl_h
#define __refinitiv_ema_access_OmmMemoryExhaustionExceptionImpl_h

#include "OmmMemoryExhaustionException.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmMemoryExhaustionExceptionImpl : public OmmMemoryExhaustionException
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

#endif // __refinitiv_ema_access_OmmMemoryExhaustionExceptionImpl_h
