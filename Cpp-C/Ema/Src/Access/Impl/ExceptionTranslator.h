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

namespace rtsdk {
	namespace ema {
		namespace access {
			class EmaString;
		}
	}
}

void throwIueException( const rtsdk::ema::access::EmaString&, rtsdk::ema::access::Int32 );

void throwIueException( const char*, rtsdk::ema::access::Int32 );

void throwIceException( const rtsdk::ema::access::EmaString& );

void throwOorException( const rtsdk::ema::access::EmaString& );

void throwSeException( rtsdk::ema::access::Int64 , void* , const char* );

void throwSeException( rtsdk::ema::access::Int64 , void* , const rtsdk::ema::access::EmaString& );

void throwIheException( rtsdk::ema::access::UInt64 , const rtsdk::ema::access::EmaString& );

void throwIheException( rtsdk::ema::access::UInt64 , const char* );

void throwMeeException( const char* );

void throwLfiException( const rtsdk::ema::access::EmaString&, const rtsdk::ema::access::EmaString& );

void throwDtuException( rtsdk::ema::access::UInt16 , const rtsdk::ema::access::EmaString& );

void throwJConverterException(const char* text, rtsdk::ema::access::Int32 errorCode,
	RsslReactorChannel* reactorChannel, rtsdk::ema::access::ClientSession* clientSession, rtsdk::ema::access::OmmProvider* provider);

#endif // __thomsonreuters_ema_access_ExceptionTranlsator_h
