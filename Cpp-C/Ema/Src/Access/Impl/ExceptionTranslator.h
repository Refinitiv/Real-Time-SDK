/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ExceptionTranlsator_h
#define __refinitiv_ema_access_ExceptionTranlsator_h

#include "Common.h"
#include "Access/Include/OmmProvider.h"
#include "ClientSession.h"

#include "rtr/rsslReactorChannel.h"

namespace refinitiv {
	namespace ema {
		namespace access {
			class EmaString;
		}
	}
}

void throwIueException( const refinitiv::ema::access::EmaString&, refinitiv::ema::access::Int32 );

void throwIueException( const char*, refinitiv::ema::access::Int32 );

void throwIceException( const refinitiv::ema::access::EmaString& );

void throwOorException( const refinitiv::ema::access::EmaString& );

void throwSeException( refinitiv::ema::access::Int64 , void* , const char* );

void throwSeException( refinitiv::ema::access::Int64 , void* , const refinitiv::ema::access::EmaString& );

void throwIheException( refinitiv::ema::access::UInt64 , const refinitiv::ema::access::EmaString& );

void throwIheException( refinitiv::ema::access::UInt64 , const char* );

void throwMeeException( const char* );

void throwLfiException( const refinitiv::ema::access::EmaString&, const refinitiv::ema::access::EmaString& );

void throwDtuException( refinitiv::ema::access::UInt16 , const refinitiv::ema::access::EmaString& );

void throwJConverterException(const char* text, refinitiv::ema::access::Int32 errorCode,
	RsslReactorChannel* reactorChannel, refinitiv::ema::access::ClientSession* clientSession, refinitiv::ema::access::OmmProvider* provider);

#endif // __refinitiv_ema_access_ExceptionTranlsator_h
