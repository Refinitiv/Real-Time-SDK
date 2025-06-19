/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "EmaStringInt.h"
#include "ExceptionTranslator.h"
#include <stdlib.h>
#include <string.h>

using namespace refinitiv::ema::access;

EmaStringInt::EmaStringInt() :
 _pTempString( 0 ),
 _nullTerminated( false )
{
}

EmaStringInt::~EmaStringInt()
{
	if ( _pTempString )
		free( (void*)_pTempString );
	_pString = 0;
}

void EmaStringInt::setInt( const char* str, UInt32 length, bool nullTerm )
{
	_pString = (char*)str;
	_length = length;
	_nullTerminated = nullTerm;
}

void EmaStringInt::clear()
{
	_pString = 0;
	_length = 0;
	_nullTerminated = false;
}

const EmaString& EmaStringInt::toString() const
{
	return static_cast<const EmaString&>(*this);
}

const char* EmaStringInt::c_str() const
{
	if ( _nullTerminated )
		return _pString;

	if ( _capacity < _length + 1 )
	{
		_capacity = _length + 1;

		free( _pTempString );

		_pTempString = (char*)malloc( _capacity );

		if ( !_pTempString )
		{
			const char* temp = "Failed to allocate memory in EmaString::c_str().";
			throwMeeException( temp );
			return _pString;
		}
	}

	if ( _pString )
	{
		memcpy( _pTempString, _pString, _length );
		*(_pTempString + _length) = 0x00;
	}
	else
	{
		*_pTempString = 0x00;
	}

	_pString = _pTempString;
	_nullTerminated = true;

	return _pString;
}
