/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "EmaBufferU16.h"
#include "EmaString.h"
#include "ExceptionTranslator.h"

#include <stdlib.h>
#include <string.h>
#include <cstdio>

#include <new>

using namespace thomsonreuters::ema::access;

EmaBufferU16::EmaBufferU16() :
 _pBuffer( 0 ),
 _length( 0 ),
 _capacity( 0 )
{
}

EmaBufferU16::EmaBufferU16( const UInt16* buf, UInt32 length ) :
 _pBuffer( 0 ),
 _length( length ),
 _capacity( length )
{
	if ( _capacity )
	{
		_pBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBufferU16( const UInt16* , UInt32 ).";
			throwMeeException( temp );
			return;
		}

		memcpy( (void*)_pBuffer, (void*)buf, length * sizeof( UInt16 ) );
	}
}

EmaBufferU16::EmaBufferU16( const EmaBufferU16& other ) :
 _pBuffer( 0 ),
 _length( other._length ),
 _capacity( other._length )
{
	if ( _capacity )
	{
		_pBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBufferU16( const EmaBufferU16& ).";
			throwMeeException( temp );
			return;
		}

		memcpy( (void*)_pBuffer, (void*)other._pBuffer, _length * sizeof( UInt16 ));
	}
}

EmaBufferU16::~EmaBufferU16()
{
	if ( _pBuffer )
		free( _pBuffer );
}

EmaBufferU16& EmaBufferU16::clear()
{
	_length = 0;

	return *this;
}

const UInt16* EmaBufferU16::u16_buf() const
{
	return _pBuffer;
}

UInt32 EmaBufferU16::length() const
{
	return _length;
}

EmaBufferU16& EmaBufferU16::operator=( const EmaBufferU16& other )
{
	if ( this == &other ) return *this;

	if ( _capacity < other._length )
	{
		_capacity = other._length;

		if ( _pBuffer )
			free( _pBuffer );

		_pBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBufferU16::operator=( const EmaBufferU16& ).";
			throwMeeException( temp );
			return *this;
		}		
	}

	_length = other._length;
	
	if ( _length )
		memcpy( (void*)_pBuffer, (void*)other._pBuffer, _length * sizeof( UInt16 ) );

	return *this;
}

bool EmaBufferU16::operator==( const EmaBufferU16& other ) const
{
	if ( this == &other ) return true;

	if ( _length != other._length ) return false;

	return ( 0 == memcmp( (void*)_pBuffer, (void*)other._pBuffer, _length * sizeof( UInt16 ) ) ? true : false );
}

EmaBufferU16& EmaBufferU16::setFrom( const UInt16* buf, UInt32 length )
{
	if ( _capacity < length )
	{
		_capacity = length;

		if ( _pBuffer )
			free( _pBuffer );

		_pBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBufferU16::setFrom( const UInt16*, UInt32 ).";
			throwMeeException( temp );
			return *this;
		}		
	}

	_length = length;

	if ( length )
		memcpy( (void*)_pBuffer, (void*)buf, length * sizeof( UInt16 ) );

	return *this;
}

EmaBufferU16& EmaBufferU16::append( const EmaBufferU16& other )
{
	if ( other.length() )
	{
		if ( _length + other.length() > _capacity )
		{
			_capacity += other.length();
			UInt16* newBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
			if ( !newBuffer )
			{
				throwMeeException( "Failed to allocate memory in EmaBufferU16::append( const EmaBufferU16& )" );
				return *this;
			}	

			memcpy( newBuffer, _pBuffer, _length * sizeof( UInt16 ) );
			free( _pBuffer );
			_pBuffer = newBuffer;
		}

		memcpy( _pBuffer + _length * sizeof( UInt16 ), other.u16_buf(), other.length() * sizeof( UInt16 ) );
		_length += other.length();
	}

	return *this;
}

EmaBufferU16& EmaBufferU16::append( UInt16 u16 )
{
	if ( _length + 1 > _capacity )
	{
		++_capacity;

		UInt16* newBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
		if ( !newBuffer )
		{
			throwMeeException( "Failed to allocate memory in EmaBufferU16::append( UInt16 )" );
			return *this;
		}

		memcpy( newBuffer, _pBuffer, _length );
		free( _pBuffer );
		_pBuffer = newBuffer;
	}

	_pBuffer[ _length++ ] = u16;

	return *this;
}

EmaBufferU16& EmaBufferU16::append( const UInt16* str, UInt32 length )
{
	if ( length )
	{
		if ( _length + length > _capacity )
		{
			_capacity += length;
			UInt16* newBuffer = (UInt16*)malloc( _capacity * sizeof( UInt16 ) );
			if ( !newBuffer )
			{
				throwMeeException( "Failed to allocate memory in EmaBufferU16::append( const UInt16*, UInt32 )" );
				return *this;
			}

			memcpy( newBuffer, _pBuffer, _length * sizeof( UInt16 ) );
			free( _pBuffer );
			_pBuffer = newBuffer;
		}

		memcpy( _pBuffer + _length, str, length * sizeof( UInt16 ) );
		_length += length;
	}

	return *this;
}

EmaBufferU16& EmaBufferU16::operator+=( const EmaBufferU16& rhs )
{
	append( rhs );
	return *this;
}

EmaBufferU16& EmaBufferU16::operator +=( UInt16 u16 )
{
	append( u16 );
	return *this;
}

EmaBufferU16 thomsonreuters::ema::access::operator +( EmaBufferU16 lhs, const EmaBufferU16& rhs )
{
	lhs += rhs;
	return lhs;
}

UInt16& EmaBufferU16::operator[]( UInt32 index )
{
	if ( index >= _length )
	{
		EmaString text( "Attempt to access out of range position in EmaBufferU16::operator[](). Passed in index is " );
		text.append( index ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
	}

	return *(_pBuffer + index * sizeof( UInt16));
}

UInt16 EmaBufferU16::operator[]( UInt32 index ) const
{
	if ( index >= _length )
	{
		EmaString text( "Attempt to access out of range position in EmaBufferU16::operator[]() const. Passed in index is " );
		text.append( index ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
	}

	return *(_pBuffer + index * sizeof( UInt16));
}
