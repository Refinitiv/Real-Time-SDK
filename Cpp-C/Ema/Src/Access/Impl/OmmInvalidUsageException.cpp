/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidUsageException.h"
#include "EmaStringInt.h"

#include <stdio.h>
#include <string.h>

using namespace thomsonreuters::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmInvalidUsageException::OmmInvalidUsageException() :
 OmmException()
{
	_errorCode = NoErrorEnum;
}

OmmInvalidUsageException::~OmmInvalidUsageException()
{
}

OmmInvalidUsageException::OmmInvalidUsageException( const OmmInvalidUsageException& other ) :
 OmmException( other )
{
	_errorCode = other._errorCode;
}

OmmInvalidUsageException& OmmInvalidUsageException::operator=( const OmmInvalidUsageException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );
	_errorCode = other._errorCode;

	return *this;
}

const EmaString& OmmInvalidUsageException::toString() const
{
	int length = snprintf(_space + EMASTRING_SIZE, MAX_SIZE_PLUS_PADDING - EMASTRING_SIZE, "Exception Type='%s', Text='%s', ErrorCode='%d'",
			getExceptionTypeAsString().c_str(),
			_errorText + EMASTRING_SIZE,
			_errorCode); // Overrides this function to print error code as well.

	reinterpret_cast<EmaStringInt*>(_space)->setInt(_space + EMASTRING_SIZE, length, true);

	return *reinterpret_cast<const EmaString*>(_space);
}

Int32 OmmInvalidUsageException::getErrorCode() const
{
	return _errorCode;
}

OmmException::ExceptionType OmmInvalidUsageException::getExceptionType() const
{
	return OmmException::OmmInvalidUsageExceptionEnum;
}

const EmaString& OmmInvalidUsageException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}
