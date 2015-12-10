/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidUsageException.h"
#include "EmaStringInt.h"

using namespace thomsonreuters::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmInvalidUsageException::OmmInvalidUsageException() :
 OmmException()
{
}

OmmInvalidUsageException::~OmmInvalidUsageException()
{
}

OmmInvalidUsageException::OmmInvalidUsageException( const OmmInvalidUsageException& other ) :
 OmmException( other )
{
}

OmmInvalidUsageException& OmmInvalidUsageException::operator=( const OmmInvalidUsageException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	return *this;
}

const EmaString& OmmInvalidUsageException::toString() const
{
	return toStringInt();
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
