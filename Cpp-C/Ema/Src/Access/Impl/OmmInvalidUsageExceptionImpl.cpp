/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidUsageExceptionImpl.h"

using namespace thomsonreuters::ema::access;

OmmInvalidUsageExceptionImpl::OmmInvalidUsageExceptionImpl() :
 OmmInvalidUsageException()
{
}

OmmInvalidUsageExceptionImpl::~OmmInvalidUsageExceptionImpl()
{
}

OmmInvalidUsageExceptionImpl::OmmInvalidUsageExceptionImpl( const OmmInvalidUsageExceptionImpl& other ) :
 OmmInvalidUsageException( other )
{
}

OmmInvalidUsageExceptionImpl& OmmInvalidUsageExceptionImpl::operator=( const OmmInvalidUsageExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmInvalidUsageException::operator=( other );
	
	return *this;
}

void OmmInvalidUsageExceptionImpl::throwException( const EmaString& text, Int32 errorCode )
{
	OmmInvalidUsageExceptionImpl exception;

	exception.statusText( text );
	exception._errorCode = errorCode;

	throw exception;
}

void OmmInvalidUsageExceptionImpl::throwException( const char* text, Int32 errorCode)
{
	OmmInvalidUsageExceptionImpl exception;

	exception.statusText( text );
	exception._errorCode = errorCode;

	throw exception;
}
