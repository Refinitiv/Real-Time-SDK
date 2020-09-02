/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidHandleExceptionImpl.h"

using namespace rtsdk::ema::access;

OmmInvalidHandleExceptionImpl::OmmInvalidHandleExceptionImpl( UInt64 handle ) :
 OmmInvalidHandleException( handle )
{
}

OmmInvalidHandleExceptionImpl::~OmmInvalidHandleExceptionImpl()
{
}

OmmInvalidHandleExceptionImpl::OmmInvalidHandleExceptionImpl( const OmmInvalidHandleExceptionImpl& other ) :
 OmmInvalidHandleException( other )
{
}

OmmInvalidHandleExceptionImpl& OmmInvalidHandleExceptionImpl::operator=( const OmmInvalidHandleExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmInvalidHandleException::operator=( other );
	
	return *this;
}

void OmmInvalidHandleExceptionImpl::throwException( const EmaString& text, UInt64 handle )
{
	OmmInvalidHandleExceptionImpl exception( handle );

	exception.statusText( text );

	throw exception;
}

void OmmInvalidHandleExceptionImpl::throwException( const char* text, UInt64 handle )
{
	OmmInvalidHandleExceptionImpl exception( handle );

	exception.statusText( text );

	throw exception;
}
