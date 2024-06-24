/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmSystemExceptionImpl.h"

using namespace refinitiv::ema::access;

OmmSystemExceptionImpl::OmmSystemExceptionImpl()
{
}

OmmSystemExceptionImpl::~OmmSystemExceptionImpl()
{
}

OmmSystemExceptionImpl::OmmSystemExceptionImpl( const OmmSystemExceptionImpl& other ) :
 OmmSystemException( other )
{
}

OmmSystemExceptionImpl& OmmSystemExceptionImpl::operator=( const OmmSystemExceptionImpl& other )
{
	if ( this == &other ) return *this;

	OmmSystemException::operator=( other );

	return *this;
}

void OmmSystemExceptionImpl::throwException( const char* text, Int64 code, void* address )
{
	OmmSystemExceptionImpl exception;

	exception._exceptionAddress = address;
	exception._exceptionCode = code;

	exception.statusText( text );

	throw exception;
}

void OmmSystemExceptionImpl::throwException( const EmaString& text, Int64 code, void* address )
{
	OmmSystemExceptionImpl exception;

	exception._exceptionAddress = address;
	exception._exceptionCode = code;

	exception.statusText( text );

	throw exception;
}
