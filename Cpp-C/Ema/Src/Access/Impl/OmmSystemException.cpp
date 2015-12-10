/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmSystemException.h"
#include "EmaStringInt.h"

#include <stdio.h>

using namespace thomsonreuters::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmSystemException::OmmSystemException() :
 OmmException(),
 _exceptionCode( 0 ),
 _exceptionAddress( 0 )
{
}

OmmSystemException::~OmmSystemException()
{
}

OmmSystemException::OmmSystemException( const OmmSystemException& other ) :
 OmmException( other ),
 _exceptionCode( other._exceptionCode ),
 _exceptionAddress( other._exceptionAddress )
{
}

OmmSystemException& OmmSystemException::operator=( const OmmSystemException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	_exceptionCode = other._exceptionCode;
	_exceptionAddress = other._exceptionAddress;

	return *this;
}

Int64 OmmSystemException::getSystemExceptionCode() const
{
	return _exceptionCode;
}

void* OmmSystemException::getSystemExceptionAddress() const
{
	return _exceptionAddress;
}

const EmaString& OmmSystemException::toString() const
{
	int length = sprintf( _space + EMASTRING_SIZE, "Exception Type='%s', Text='%s', exceptionCode='%llu', exceptionAddress='0x%p'",
		getExceptionTypeAsString().c_str(),
		_errorText + EMASTRING_SIZE,
		_exceptionCode,
		_exceptionAddress );

	reinterpret_cast<EmaStringInt*>(_space)->setInt( _space + EMASTRING_SIZE, length, true );

	return *reinterpret_cast<const EmaString*>( _space );
}

OmmException::ExceptionType OmmSystemException::getExceptionType() const
{
	return OmmException::OmmSystemExceptionEnum;
}

const EmaString& OmmSystemException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}

