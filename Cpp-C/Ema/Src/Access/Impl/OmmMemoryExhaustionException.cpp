/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmMemoryExhaustionException.h"
#include "EmaStringInt.h"

using namespace refinitiv::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

OmmMemoryExhaustionException::OmmMemoryExhaustionException() :
 OmmException()
{
}

OmmMemoryExhaustionException::~OmmMemoryExhaustionException()
{
}

OmmMemoryExhaustionException::OmmMemoryExhaustionException( const OmmMemoryExhaustionException& other ) :
 OmmException( other )
{
}

OmmMemoryExhaustionException& OmmMemoryExhaustionException::operator=( const OmmMemoryExhaustionException& other )
{
	if ( this == &other ) return *this;

	OmmException::operator=( other );

	return *this;
}

const EmaString& OmmMemoryExhaustionException::toString() const
{
	return toStringInt();
}

OmmException::ExceptionType OmmMemoryExhaustionException::getExceptionType() const
{
	return OmmException::OmmMemoryExhaustionExceptionEnum;
}

const EmaString& OmmMemoryExhaustionException::getText() const
{
	reinterpret_cast<EmaStringInt*>(_errorText)->setInt( _errorText + EMASTRING_SIZE, _errorTextLength, true );
	return *reinterpret_cast<const EmaString*>( _errorText );
}
