/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmOutOfRangeException.h"
#include "EmaStringInt.h"

using namespace rtsdk::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmOutOfRangeException::OmmOutOfRangeException() :
 OmmException()
{
}

OmmOutOfRangeException::~OmmOutOfRangeException()
{
}

OmmOutOfRangeException::OmmOutOfRangeException( const OmmOutOfRangeException& other ) :
 OmmException( other )
{
}
OmmOutOfRangeException& OmmOutOfRangeException::operator=( const OmmOutOfRangeException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	return *this;
}

const EmaString& OmmOutOfRangeException::toString() const
{
	return toStringInt();
}

OmmException::ExceptionType OmmOutOfRangeException::getExceptionType() const
{
	return OmmException::OmmOutOfRangeExceptionEnum;
}

const EmaString& OmmOutOfRangeException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}
