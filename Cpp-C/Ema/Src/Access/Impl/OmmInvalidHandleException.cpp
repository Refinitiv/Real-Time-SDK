/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidHandleException.h"
#include "EmaStringInt.h"
#include "Utilities.h"

#include <stdio.h>

using namespace thomsonreuters::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmInvalidHandleException::OmmInvalidHandleException() :
 OmmException(),
 _handle( 0 )
{
}

OmmInvalidHandleException::OmmInvalidHandleException( UInt64 handle ) :
 OmmException(),
 _handle( handle )
{
}

OmmInvalidHandleException::~OmmInvalidHandleException()
{
}

OmmInvalidHandleException::OmmInvalidHandleException( const OmmInvalidHandleException& other ) :
 OmmException( other ),
 _handle( other._handle )
{
}

OmmInvalidHandleException& OmmInvalidHandleException::operator=( const OmmInvalidHandleException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	_handle = other._handle;

	return *this;
}

const EmaString& OmmInvalidHandleException::toString() const
{
	int length = snprintf( _space + EMASTRING_SIZE, MAX_SIZE_PLUS_PADDING - EMASTRING_SIZE, "Exception Type='%s', Text='%s', Handle='%llu'",
		getExceptionTypeAsString().c_str(),
		_errorText + EMASTRING_SIZE,
		_handle );

	reinterpret_cast<EmaStringInt*>(_space)->setInt( _space + EMASTRING_SIZE, length, true );

	return *reinterpret_cast<const EmaString*>( _space );
}

OmmException::ExceptionType OmmInvalidHandleException::getExceptionType() const
{
	return OmmException::OmmInvalidHandleExceptionEnum;
}

const EmaString& OmmInvalidHandleException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}

UInt64 OmmInvalidHandleException::getHandle() const
{
	return _handle;
}
