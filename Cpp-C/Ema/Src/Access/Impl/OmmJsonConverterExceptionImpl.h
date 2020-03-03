/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmJsonConverterExceptionImpl_h
#define __thomsonreuters_ema_access_OmmJsonConverterExceptionImpl_h

#include "Access/Include/OmmJsonConverterException.h"
#include "Access/Include/ConsumerSessionInfo.h"
#include "Access/Include/ProviderSessionInfo.h"
#include "ClientSession.h"

#include "rtr/rsslReactorChannel.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmJsonConverterExceptionImpl : public OmmJsonConverterException
{
public:
	static void throwException(const char* text, Int32 errorCode, RsslReactorChannel*, ClientSession*, OmmProvider*);

	OmmJsonConverterExceptionImpl();

	virtual ~OmmJsonConverterExceptionImpl();

	const SessionInfo& getSessionInfo() const;

private:

	OmmJsonConverterExceptionImpl(const OmmJsonConverterExceptionImpl&);
	OmmJsonConverterExceptionImpl& operator=(const OmmJsonConverterExceptionImpl&);

	SessionInfo* pSessionInfo;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmJsonConverterExceptionImpl_h
