/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInvalidConfigurationException.h"
#include "EmaStringInt.h"
#include "Utilities.h"

#include <stdio.h>

using namespace thomsonreuters::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmInvalidConfigurationException::OmmInvalidConfigurationException() :
 OmmException()
{
}

OmmInvalidConfigurationException::~OmmInvalidConfigurationException()
{
}

OmmInvalidConfigurationException::OmmInvalidConfigurationException( const OmmInvalidConfigurationException& other ) :
 OmmException( other )
{
}

OmmInvalidConfigurationException& OmmInvalidConfigurationException::operator=( const OmmInvalidConfigurationException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	return *this;
}

const EmaString& OmmInvalidConfigurationException::toString() const
{
	int length = snprintf( _space + EMASTRING_SIZE, MAX_SIZE_PLUS_PADDING - EMASTRING_SIZE, "Exception Type='%s', Text='%s'",
		getExceptionTypeAsString().c_str(),
		_errorText + EMASTRING_SIZE );

	reinterpret_cast<EmaStringInt*>(_space)->setInt( _space + EMASTRING_SIZE, length, true );

	return *reinterpret_cast<const EmaString*>( _space );
}

OmmException::ExceptionType OmmInvalidConfigurationException::getExceptionType() const
{
	return OmmException::OmmInvalidConfigurationExceptionEnum;
}

const EmaString& OmmInvalidConfigurationException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}
