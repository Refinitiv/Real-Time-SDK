/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ExceptionTranslator.h"

#include "OmmInvalidConfigurationExceptionImpl.h"
#include "OmmInvalidUsageExceptionImpl.h"
#include "OmmSystemExceptionImpl.h"
#include "OmmOutOfRangeExceptionImpl.h"
#include "OmmInvalidHandleExceptionImpl.h"
#include "OmmMemoryExhaustionExceptionImpl.h"
#include "OmmInaccessibleLogFileExceptionImpl.h"
#include "OmmUnsupportedDomainTypeExceptionImpl.h"
#include "OmmJsonConverterExceptionImpl.h"

using namespace thomsonreuters::ema::access;

void throwIueException( const EmaString& text, Int32 errorCode )
{
	OmmInvalidUsageExceptionImpl::throwException( text, errorCode );
}

void throwIueException( const char* text, Int32 errorCode )
{
	OmmInvalidUsageExceptionImpl::throwException( text, errorCode );
}

void throwIceException( const EmaString& text )
{
	OmmInvalidConfigurationExceptionImpl::throwException( text );
}

void throwOorException( const EmaString& text )
{
	OmmOutOfRangeExceptionImpl::throwException( text );
}

void throwSeException( Int64 code, void* address, const char* text )
{
	OmmSystemExceptionImpl::throwException( text, code, address );
}

void throwSeException( Int64 code, void* address, const EmaString& text )
{
	OmmSystemExceptionImpl::throwException( text, code, address );
}

void throwIheException( UInt64 handle, const EmaString& text )
{
	OmmInvalidHandleExceptionImpl::throwException( text, handle );
}

void throwIheException( UInt64 handle, const char* text )
{
	OmmInvalidHandleExceptionImpl::throwException( text, handle );
}

void throwMeeException( const char* text )
{
	OmmMemoryExhaustionExceptionImpl::throwException( text );
}

void throwLfiException( const EmaString& fileName, const EmaString& text )
{
	OmmInaccessibleLogFileExceptionImpl::throwException( fileName, text );
}

void throwDtuException( thomsonreuters::ema::access::UInt16 domainType , const thomsonreuters::ema::access::EmaString& text )
{
	OmmUnsupportedDomainTypeExceptionImpl::throwException( text, domainType );
}

void throwJConverterException(const char* text, Int32 errorCode,
	RsslReactorChannel* reactorChannel, ClientSession* clientSession, OmmProvider* provider)
{
	OmmJsonConverterExceptionImpl::throwException(text, errorCode, reactorChannel, clientSession, provider);
}
