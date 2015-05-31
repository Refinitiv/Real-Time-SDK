/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "EmaString.h"
#include "ExceptionTranslator.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace thomsonreuters::ema::access;

EmaString::EmaString() :
 _pString( 0 ),
 _length( 0 ),
 _capacity( 0 )
{
}

EmaString::EmaString( const char* str, UInt32 length ) :
 _pString( 0 ),
 _length( str ? length : 0 ),
 _capacity( length + 1 )
{
	if ( !length )
	{
		_length = str ? static_cast<UInt32>( strlen( str ) ) : 0;
		_capacity = _length + 1;
	}

	if ( _length )
	{
		_pString = (char*)malloc( _capacity );

		if ( !_pString )
		{
			const char* temp = "Failed to allocate memory in EmaString( const char* , UInt32 ).";
			throwMeeException( temp );
			return;
		}

		if ( str )
		{
			memcpy( _pString, str, _length );
			*(_pString + _length) = 0x00;
		}
		else
		{
			*_pString = 0x00;
		}
	}
	else
	{
		_capacity = 0;
	}
}

EmaString::EmaString( const EmaString& other ) :
 _pString( 0 ),
 _length( other._length ),
 _capacity( other._length + 1 )
{
	if ( _length )
	{
		_pString = (char*)malloc( _capacity );

		if ( !_pString )
		{
			const char* temp = "Failed to allocate memory in EmaString( const EmaString& ).";
			throwMeeException( temp );
			return;
		}

		if ( other._pString )
		{
			memcpy( _pString, other._pString, _length );
			*(_pString + _length) = 0x00;
		}
		else
		{
			*_pString = 0x00;
		}
	}
	else
	{
		_capacity = 0;
	}
}

EmaString::~EmaString()
{
	if ( _pString )
		free( _pString );
}

EmaString& EmaString::clear()
{
	_length = 0;
	if ( _pString )
		*_pString = 0x00;

	return *this;
}

bool EmaString::empty() const
{
	return !_length ? true : false;
}

const char* EmaString::c_str() const
{
	return _pString ? _pString : "";
}

UInt32 EmaString::length() const
{
	return _length;
}

EmaString& EmaString::operator=( const char* str )
{
	return set( str );
}

EmaString& EmaString::operator=( const EmaString& other )
{
	if ( this == &other ) return *this;

	if ( other._length )
	{
		if ( _capacity <= other._length )
		{
			_capacity = other._length + 1;
		
			if ( _pString )
			{
				free( _pString );
				_pString = 0;
			}
		
			_pString = (char*)malloc( _capacity );
			if ( !_pString )
			{
				const char* temp = "Failed to allocate memory in EmaString::operator=( const EmaString& ).";
				throwMeeException( temp );
				return *this;
			}
		}

		_length = other._length;

		if ( other._pString )
			memcpy( _pString, other._pString, _length );

		*(_pString + _length) = 0x00;
	}
	else
	{
		clear();
	}

	return *this;
}

EmaString& EmaString::set( const char* str, UInt32 length )
{
	if ( !length )
		_length = str ? static_cast<UInt32>( strlen( str ) ) : 0;
	else
		_length = str ? length : 0;

	if ( _length )
	{
		if ( _capacity <= (str ? _length : length) )
		{
			_capacity = (str ? _length : length) + 1;
		
			if ( _pString )
			{
				free( _pString );
				_pString = 0;
			}
		
			_pString = (char*)malloc( _capacity );
			if ( !_pString )
			{
				const char* temp = "Failed to allocate memory in EmaString::set( const char* , UInt32 ).";
				throwMeeException( temp );
				return *this;
			}
		}

		if ( str )
		{
			memcpy( _pString, str, _length );
			*(_pString + _length) = 0x00;
		}
		else
		{
			*_pString = 0x00;
		}
	}
	else
	{
		clear();
	}

	return *this;
}

EmaString& EmaString::append( Int64 i )
{
	if ( _capacity <= _length + 21 )
	{
		_capacity = _length + 22;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( Int64 ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		_length += sprintf( pNewString + _length, "%lld", i );

		_pString = pNewString;
	}
	else
	{
		_length += sprintf( _pString + _length, "%lld", i );
	}
		
	return *this;
}

EmaString& EmaString::append( UInt64 i )
{
	if ( _capacity <= _length + 21 )
	{
		_capacity = _length + 22;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( UInt64 ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

        _pString = pNewString;
	}
	
	_length += sprintf( _pString + _length, "%llu", i );

	return *this;
}

EmaString& EmaString::append( Int32 i )
{
	if ( _capacity <= _length + 12 )
	{
		_capacity = _length + 13;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( Int32 ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		_length += sprintf( pNewString + _length, "%i", i );

		_pString = pNewString;
	}
	else
	{
		_length += sprintf( _pString + _length, "%i", i );
	}
		
	return *this;
}

EmaString& EmaString::append( UInt32 i )
{
	if ( _capacity <= _length + 12 )
	{
		_capacity = _length + 13;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( UInt32 ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		_length += sprintf( pNewString + _length, "%u", i );

		_pString = pNewString;
	}
	else
	{
		_length += sprintf( _pString + _length, "%u", i );
	}
		
	return *this;
}

EmaString& EmaString::append( float f )
{
	if ( _capacity <= _length + 32 )
	{
		_capacity = _length + 33;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( float ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		_length += sprintf( pNewString + _length, "%g", f );

		_pString = pNewString;
	}
	else
	{
		_length += sprintf( _pString + _length, "%g", f );
	}
		
	return *this;
}

EmaString& EmaString::append( double d )
{
	if ( _capacity <= _length + 32 )
	{
		_capacity = _length + 33;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( double ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		_length += sprintf( pNewString + _length, "%lg", d );

		_pString = pNewString;
	}
	else
	{
		_length += sprintf( _pString + _length, "%lg", d );
	}
		
	return *this;
}

EmaString& EmaString::append( const char* str )
{
	UInt32 strLength = str ? static_cast<UInt32>( strlen( str ) ) : 0;

	if ( !strLength ) return *this;

	if ( _capacity <= _length + strLength )
	{
		_capacity = _length + strLength + 1;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( const char* ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		memcpy( pNewString + _length, str, strLength );

		_length += strLength;

		*(pNewString + _length) = 0x00;

		_pString = pNewString;
	}
	else if ( strLength )
	{
		memcpy( _pString + _length, str, strLength );

		_length += strLength;

		*(_pString + _length) = 0x00;
	}
		
	return *this;
}

EmaString& EmaString::append( const EmaString& other )
{
	if ( !other._length ) return *this;

	if ( _capacity <= _length + other._length )
	{
		_capacity = _length + other._length + 1;

		char* pNewString = (char*)malloc( _capacity );
		if ( !pNewString )
		{
			const char* temp = "Failed to allocate memory in EmaString::append( const EmaString& ).";
			throwMeeException( temp );
			return *this;
		}

		if ( _pString )
		{
			memcpy( pNewString, _pString, _length );
			free( _pString );
		}

		memcpy( pNewString + _length, other._pString, other._length );

		_length += other._length;

		*(pNewString + _length) = 0x00;

		_pString = pNewString;
	}
	else if ( other._length )
	{
		memcpy( _pString + _length, other._pString, other._length );
		
		_length += other._length;

		*(_pString + _length) = 0x00;
	}
		
	return *this;
}

EmaString& EmaString::operator+=( Int64 i )
{
	return append( i );
}

EmaString& EmaString::operator+=( UInt64 i )
{
	return append( i );
}

EmaString& EmaString::operator+=( Int32 i )
{
	return append( i );
}

EmaString& EmaString::operator+=( UInt32 i )
{
	return append( i );
}

EmaString& EmaString::operator+=( float f )
{
	return append( f );
}

EmaString& EmaString::operator+=( double d )
{
	return append( d );
}

EmaString& EmaString::operator+=( const char* str )
{
	return append( str );
}

EmaString& EmaString::operator+=( const EmaString& str )
{
	return append( str );
}

EmaString EmaString::operator+( const EmaString& other )
{
	return EmaString( *this ).append( other );
}

EmaString EmaString::operator+( const char* str )
{
	return EmaString( *this ).append( str );
}

EmaString::operator const char *() const
{
	return c_str();
}

EmaString EmaString::substr( UInt32 index, UInt32 length ) const
{
	if ( index > _length ||
		index + length > _length )
	{
		EmaString text( "Attempt to access out of range position in EmaString::substr( UInt32 , UInt32 ) const. Passed in index is " );
		text.append( index ).append( " and passed in length is " ).append( length ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
		return EmaString();
	}

	EmaString retVal;

	retVal.set( _pString + index, length );

	return retVal;
}

char EmaString::operator[]( UInt32 pos ) const
{
	if ( pos > _length )
	{
		EmaString text( "Attempt to access out of range position in EmaString::operator[]() const. Passed in index is " );
		text.append( pos ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
	}
	
	return _pString[pos];
}

char& EmaString::operator[]( UInt32 pos )
{
	if ( pos > _length )
	{
		EmaString text( "Attempt to access out of range position in EmaString::operator[](). Passed in index is " );
		text.append( pos ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
	}
	
	return _pString[pos];
}

bool EmaString::operator==( const EmaString& other ) const
{
	if ( this == &other ) return true;

	if ( _length != other._length ) return false;

	if ( !_pString && other._pString ) return false;

	if ( _pString && !other._pString ) return false;

	if ( !_pString && !other._pString ) return true;

	return ( 0 == memcmp( _pString, other._pString, _length ) ? true : false );
}

bool EmaString::operator!=( const EmaString& other ) const
{
	if ( this == &other ) return false;

	if ( _length != other._length ) return true;

	if ( !_pString && other._pString ) return true;

	if ( _pString && !other._pString ) return true;

	if ( !_pString && !other._pString ) return true;

	return ( 0 == memcmp( _pString, other._pString, _length ) ? false : true );
}

bool EmaString::operator==( const char* other ) const
{
	if ( !other )
		return false;

	if ( _length != strlen( other ) )
		return false;

	return !memcmp( _pString, other, _length );
}

bool EmaString::operator!=( const char* other ) const
{
	if ( !other )
		return true;

	if ( _length != strlen( other ) )
		return true;

	return memcmp( _pString, other, _length ) != 0;
}

bool EmaString::operator>( const EmaString& rhs ) const
{
    return compare( rhs.c_str() ) > 0;
}

bool EmaString::operator<( const EmaString& rhs ) const
{
    return compare( rhs.c_str() ) < 0;
}

bool EmaString::operator>=( const EmaString& rhs ) const
{
    return compare( rhs.c_str() ) >= 0;
}

bool EmaString::operator<=( const EmaString& rhs ) const
{
    return compare( rhs.c_str() ) <= 0;
}
	
bool EmaString::operator>( const char* rhs ) const
{
    return compare( rhs ) > 0;
}

bool EmaString::operator<( const char* rhs ) const
{
    return compare( rhs ) < 0;
}
	
bool EmaString::operator>=( const char* rhs ) const
{
    return compare( rhs ) >= 0;
}

bool EmaString::operator<=( const char* rhs ) const
{
    return compare( rhs ) <= 0;
}

EmaString& EmaString::trimWhitespace()
{
	if ( !_length ) return *this;

	char* p = _pString;
	while ( *p && isspace(*p) )
		++p;

	if ( !*p )
	{
		_pString[0] = 0;
		_length = 0;
		return *this;
	}

	if ( p != _pString )
	{ 
		_length -= static_cast<UInt32>( p - _pString );
		memmove( _pString, p, _length );
		_pString[_length] = 0;
	}

	p = _pString + _length - 1;
	while ( isspace( *p ) )
		--p;

	if ( p != _pString + _length - 1 )
	{
		_length = static_cast<UInt32>( p - _pString + 1 );
		_pString[_length] = 0;
	}

	return *this;
}

Int32 EmaString::find( const EmaString & source, Int32 index ) const
{
	if ( index < 0 || static_cast<UInt32>(index) >= _length ||! source._length )
		return -1;
		   
	char* p = _pString + index;
	Int32 retVal( 0 );
	while ( *p )
	{
		if ( _length - ( p - _pString ) < source._length )
			return -1;

		char* q = source._pString;
		retVal = static_cast<UInt32>( p - _pString );
		while ( *p == *q )
		{
			if ( ! *++q )
				return retVal;

			if ( ! *++p )
				return -1;
		}

		if ( q == source._pString )
			++p;
	}

	return -1;
}

Int32 EmaString::find( const char* source, Int32 index ) const
{
	if ( index < 0 || static_cast<UInt32>(index) >= _length )
		return -1;

	UInt32 sourceLength( static_cast<UInt32>(strlen(source)) );
	if ( ! sourceLength )
		return -1;
  
	char *p = _pString + index;
	Int32 retVal(0);
	while ( *p )
	{
		if ( _length - static_cast<UInt32>( p - _pString ) < sourceLength )
			return -1;

		const char *q = source;
		retVal = static_cast<Int32>( p - _pString );
		while ( *p == *q )
		{
			if ( ! *++q )
				return retVal;

			if ( ! *++p )
				return -1;
		}

		if ( q == source )
			++p;
	}

	return -1;
}

Int32 EmaString::findLast( const EmaString& str ) const 
{
	if ( ! str._length || ! _length )
		return -1;

	if ( str._length > _length )
		return -1;
  
	const char *p, *q;
	Int32 retVal( _length - str._length );
	while ( true )
	{
		p = _pString + retVal;
		q = str._pString;
		while ( *q && *p == *q )
		{
			++p;
			++q;
		}
    
		if ( ! *q )
			return retVal;

		if ( --retVal < 0 )
			return -1;
	}
}

Int32 EmaString::findLast( const char* str ) const
{
	if ( !str || str[0] == 0 || !_length )
		return -1;

	Int32 strLen( static_cast<Int32>( strlen( str ) ) );

	const char *p, *q;
	Int32 retVal( _length - strLen );
	while ( true )
	{
		p = _pString + retVal;
		q = str;
		while ( *q && *p == *q )
		{
			++p;
			++q;
		}
    
		if ( ! *q )
			return retVal;

		if ( --retVal < 0 )
			return -1;
	}
}

bool EmaString::caseInsensitiveCompare( const EmaString& rhs ) const
{
	if ( ( this == &rhs ) ||
		 ( !_pString && !rhs._pString ) )
		return true;

	if ( _length != rhs._length )
		return false;
	
	for ( UInt32 i = 0; i < _length; ++i )
        if ( tolower( _pString[i] ) != tolower( rhs._pString[i] ) )
            return false;

	return true;
}

bool EmaString::caseInsensitiveCompare( const char * rhs ) const
{
	if ( ! rhs )
		return ! _pString;

	if ( strlen( rhs ) != _length )
		return false;

	for ( UInt32 i = 0; i < _length; ++i )
		if ( tolower(_pString[i] ) != tolower( rhs[i] ) )
            return false;

	return true;
}
    
int EmaString::compare( const char * rhs ) const
{
    for ( unsigned int i = 0; i < _length  && i < strlen( rhs ); ++i )
        if ( _pString[ i ] != rhs[ i ] )
            return _pString[ i ] - rhs[ i ];

    return static_cast< int >( _length - strlen( rhs ) );
}
