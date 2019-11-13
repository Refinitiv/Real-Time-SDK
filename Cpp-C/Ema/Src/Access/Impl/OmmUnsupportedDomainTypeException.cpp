/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUnsupportedDomainTypeException.h"
#include "EmaStringInt.h"
#include "Utilities.h"

#include <stdio.h>

using namespace thomsonreuters::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmUnsupportedDomainTypeException::OmmUnsupportedDomainTypeException() :
 OmmException(),
 _domainType( 0 )
{
}

OmmUnsupportedDomainTypeException::~OmmUnsupportedDomainTypeException()
{
}

OmmUnsupportedDomainTypeException::OmmUnsupportedDomainTypeException( const OmmUnsupportedDomainTypeException& other ) :
 OmmException( other ),
 _domainType( other._domainType )
{
}

OmmUnsupportedDomainTypeException& OmmUnsupportedDomainTypeException::operator=( const OmmUnsupportedDomainTypeException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	_domainType = other._domainType;

	return *this;
}

UInt16 OmmUnsupportedDomainTypeException::getDomainType() const
{
	return _domainType;
}

const EmaString& OmmUnsupportedDomainTypeException::toString() const
{
	int length = snprintf( _space + EMASTRING_SIZE, MAX_SIZE_PLUS_PADDING - EMASTRING_SIZE, "Exception Type='%s', Text='%s', DomainType='%u'",
		getExceptionTypeAsString().c_str(),
		_errorText + EMASTRING_SIZE,
		_domainType );

	reinterpret_cast<EmaStringInt*>(_space)->setInt( _space + EMASTRING_SIZE, length, true );

	return *reinterpret_cast<const EmaString*>( _space );
}

OmmException::ExceptionType OmmUnsupportedDomainTypeException::getExceptionType() const
{
	return OmmException::OmmUnsupportedDomainTypeExceptionEnum;
}

const EmaString& OmmUnsupportedDomainTypeException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}
