/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ExceptionTranlsator_h
#define __thomsonreuters_ema_access_ExceptionTranlsator_h

#include "Common.h"
#include "Access/Include/OmmProvider.h"
#include "ClientSession.h"

#include "rtr/rsslReactorChannel.h"

namespace thomsonreuters {
	namespace ema {
		namespace access {
			class EmaString;
		}
	}
}

void throwIueException( const thomsonreuters::ema::access::EmaString&, thomsonreuters::ema::access::Int32 );

void throwIueException( const char*, thomsonreuters::ema::access::Int32 );

void throwIceException( const thomsonreuters::ema::access::EmaString& );

void throwOorException( const thomsonreuters::ema::access::EmaString& );

void throwSeException( thomsonreuters::ema::access::Int64 , void* , const char* );

void throwSeException( thomsonreuters::ema::access::Int64 , void* , const thomsonreuters::ema::access::EmaString& );

void throwIheException( thomsonreuters::ema::access::UInt64 , const thomsonreuters::ema::access::EmaString& );

void throwIheException( thomsonreuters::ema::access::UInt64 , const char* );

void throwMeeException( const char* );

void throwLfiException( const thomsonreuters::ema::access::EmaString&, const thomsonreuters::ema::access::EmaString& );

void throwDtuException( thomsonreuters::ema::access::UInt16 , const thomsonreuters::ema::access::EmaString& );

void throwJConverterException(const char* text, thomsonreuters::ema::access::Int32 errorCode,
	RsslReactorChannel* reactorChannel, thomsonreuters::ema::access::ClientSession* clientSession, thomsonreuters::ema::access::OmmProvider* provider);

#endif // __thomsonreuters_ema_access_ExceptionTranlsator_h
