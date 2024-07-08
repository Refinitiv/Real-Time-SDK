/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "OmmException.h"
#include "EmaStringInt.h"
#include "Utilities.h"

#include <stdio.h>
#include <string.h>

#include <new>

using namespace refinitiv::ema::access;

#define EMASTRING_SIZE sizeof( EmaStringInt )

const EmaString OmmInvalidUsageExceptionString( "OmmInvalidUsageException" );
const EmaString OmmInvalidConfigurationExceptionString( "OmmInvalidConfigurationException" );
const EmaString OmmSystemExceptionString( "OmmSystemException" );
const EmaString OmmOutOfRangeExceptionString( "OmmOutOfRangeException" );
const EmaString OmmInvalidHandleExceptionString( "OmmInvalidHandleException" );
const EmaString OmmMemoryExhaustionExceptionString( "OmmMemoryExhaustionException" );
const EmaString OmmInaccessibleLogFileExceptionString( "OmmInaccessibleLogFileException" );
const EmaString OmmUnsupportedDomainTypeExceptionString( "OmmUnsupportedDomainTypeException" );
const EmaString OmmJsonConverterExceptionString( "OmmJsonConverterException" );
EmaString TempETString;

OmmException::OmmException() :
 _errorTextLength( 0 )
{
	*(_space + EMASTRING_SIZE) = 0x00;
	*(_errorText + EMASTRING_SIZE) = 0x00;

	new (_space) EmaStringInt();
	new (_errorText) EmaStringInt();
}

OmmException::~OmmException()
{
	reinterpret_cast<EmaStringInt*>(_errorText)->~EmaStringInt();
	reinterpret_cast<EmaStringInt*>(_space)->~EmaStringInt();
}

OmmException::OmmException( const OmmException& other ) :
 _errorTextLength( other._errorTextLength )
{
	*(_space + EMASTRING_SIZE) = 0x00;
	memcpy( _errorText + EMASTRING_SIZE, other._errorText + EMASTRING_SIZE, _errorTextLength + 1 );

	new (_space) EmaStringInt();
	new (_errorText) EmaStringInt();
}

const EmaString& OmmException::getExceptionTypeAsString() const
{
	switch ( getExceptionType() )
	{
	case OmmInvalidUsageExceptionEnum :
		return OmmInvalidUsageExceptionString;
	case OmmInvalidConfigurationExceptionEnum :
		return OmmInvalidConfigurationExceptionString;
	case OmmSystemExceptionEnum :
		return OmmSystemExceptionString;
	case OmmOutOfRangeExceptionEnum :
		return OmmOutOfRangeExceptionString;
	case OmmInvalidHandleExceptionEnum :
		return OmmInvalidHandleExceptionString;
	case OmmMemoryExhaustionExceptionEnum :
		return OmmMemoryExhaustionExceptionString;
	case OmmInaccessibleLogFileExceptionEnum :
		return OmmInaccessibleLogFileExceptionString;
	case OmmUnsupportedDomainTypeExceptionEnum :
		return OmmUnsupportedDomainTypeExceptionString;
	case OmmJsonConverterExceptionEnum:
		return OmmJsonConverterExceptionString;
	default :
		return TempETString.set( "Unknonwn ExceptionType value " ).append( (Int64)getExceptionType() );
	}
}

OmmException& OmmException::operator=( const OmmException& other )
{
	if ( this == & other ) return *this;

	_errorTextLength = other._errorTextLength;

	*(_space + EMASTRING_SIZE) = 0x00;
	memcpy( _errorText + EMASTRING_SIZE, other._errorText + EMASTRING_SIZE, _errorTextLength + 1 );

	return *this;
}

OmmException& OmmException::statusText( const EmaString& statusText )
{
	const char* text = statusText.c_str();
	_errorTextLength = 0;
	while ( *(text + _errorTextLength) )
	{
		*(_errorText + EMASTRING_SIZE + _errorTextLength) = *(text + _errorTextLength);
		if ( ++_errorTextLength  >= MAX_SIZE - EMASTRING_SIZE - 1 )
			break;
	}

	*(_errorText + EMASTRING_SIZE + _errorTextLength) = 0x00;
	
	return *this;
}

OmmException& OmmException::statusText( const char* text )
{
	_errorTextLength = 0;
	while ( *(text + _errorTextLength) )
	{
		*(_errorText + EMASTRING_SIZE + _errorTextLength) = *(text + _errorTextLength);
		if ( ++_errorTextLength  >= MAX_SIZE - EMASTRING_SIZE - 1 )
			break;
	}

	*(_errorText + EMASTRING_SIZE + _errorTextLength) = 0x00;
	
	return *this;
}

OmmException::operator const char* () const
{
	return toString().c_str();
}

const EmaString& OmmException::toStringInt() const
{
	int length = snprintf( _space + EMASTRING_SIZE, MAX_SIZE_PLUS_PADDING - EMASTRING_SIZE, "Exception Type='%s', Text='%s'",
		getExceptionTypeAsString().c_str(),
		_errorText + EMASTRING_SIZE );

	reinterpret_cast<EmaStringInt*>(_space)->setInt( _space + EMASTRING_SIZE, length, true );

	return *reinterpret_cast<const EmaString*>( _space );
}
