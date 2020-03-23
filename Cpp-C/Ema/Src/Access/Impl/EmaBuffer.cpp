/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "EmaBuffer.h"
#include "EmaString.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"
#include "rtr/rsslTransport.h"
#include <new>

using namespace thomsonreuters::ema::access;

class thomsonreuters::ema::access::CastingOperatorContext
{
public :

	CastingOperatorContext() :
	 _pBuffer( 0 ),
	 _allocatedLength( 0 ),
	 _currentLength( 0 ),
	 _bDirty( true )
	{
	}

	virtual ~CastingOperatorContext()
	{
		if ( _pBuffer )
		{
			free( _pBuffer );
			_pBuffer = 0;
			_allocatedLength = 0;
			_currentLength = 0;
		}
	}

	UInt32 getHexStringLength() const
	{
		return _currentLength;
	}

	const char* getHexString( char* pBuffer, UInt32 length )
	{
		if ( _bDirty )
		{
			const UInt32 valuesPerLine = 16;

			RsslBuffer buffer;
			buffer.length = length;
			buffer.data = pBuffer;

			UInt32 hexSize = rsslCalculateHexDumpOutputSize( &buffer, valuesPerLine );

			RsslError error;
            RsslRet retVal;
			while ( true )
			{
				if ( hexSize > _allocatedLength )
				{
					free ( _pBuffer );
					_allocatedLength = hexSize;
					_pBuffer = (char*)malloc( hexSize );

					if ( !_pBuffer )
					{
						const char* temp = "Failed to allocate memory in EmaBuffer::operator const char*().";
						throwMeeException( temp );
						return 0;
					}
				}

				RsslBuffer output;
				output.length = hexSize;
				output.data = _pBuffer;

				retVal = rsslBufferToHexDump( &buffer, &output, valuesPerLine, &error );
				if ( retVal == RSSL_RET_SUCCESS )
					break;
				else if ( retVal == RSSL_RET_BUFFER_TOO_SMALL )
					hexSize *= 2;
				else
				{
					EMA_ASSERT( retVal == RSSL_RET_SUCCESS, "rsslBufferToHexDump return not SUCCESS or BUFFER_TOO_SMALL" );
					break;
				}

				_currentLength = output.length;
			}

			_bDirty = false;
		}

		return _pBuffer;
	}

	const char* getRawHexString(char* pBuffer, UInt32 length)
	{
		if (_bDirty)
		{
			const UInt32 valuesPerLine = 16;

			RsslBuffer buffer;
			buffer.length = length;
			buffer.data = pBuffer;

			UInt32 hexSize = rsslCalculateHexDumpOutputSize(&buffer, valuesPerLine);

			RsslError error;
			RsslRet retVal;
			while (true)
			{
				if (hexSize > _allocatedLength)
				{
					free(_pBuffer);
					_allocatedLength = hexSize;
					_pBuffer = (char*)malloc(hexSize);

					if (!_pBuffer)
					{
						const char* temp = "Failed to allocate memory in EmaBuffer::operator const char*().";
						throwMeeException(temp);
						return 0;
					}
				}

				RsslBuffer output;
				output.length = hexSize;
				output.data = _pBuffer;

				retVal = rsslBufferToRawHexDump(&buffer, &output, valuesPerLine, &error);
				if (retVal == RSSL_RET_SUCCESS)
					break;
				else if (retVal == RSSL_RET_BUFFER_TOO_SMALL)
					hexSize *= 2;
				else
				{
					EMA_ASSERT(retVal == RSSL_RET_SUCCESS, "rsslBufferToRawHexDump return not SUCCESS or BUFFER_TOO_SMALL");
					break;
				}

				_currentLength = output.length;
			}

			_bDirty = false;
		}

		return _pBuffer;
	}

	void markDirty()
	{
		_bDirty = true;
	}

private :

	char*		_pBuffer;
	UInt32		_allocatedLength;
	UInt32		_currentLength;
	bool		_bDirty;
};


EmaBuffer::EmaBuffer() :
 _pBuffer( 0 ),
 _length( 0 ),
 _capacity( 0 ),
 _pCastingOperatorContext( 0 )
{
}

EmaBuffer::EmaBuffer( const char* buf, UInt32 length ) :
 _pBuffer( 0 ),
 _length( length ),
 _capacity( length ),
 _pCastingOperatorContext( 0 )
{
	if ( _capacity )
	{
		_pBuffer = (char*)malloc( _capacity );

		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBuffer( const char* , UInt32 ).";
			throwMeeException( temp );
			return;
		}

		memcpy( (void*)_pBuffer, (void*)buf, length );
	}
}

EmaBuffer::EmaBuffer( const EmaBuffer& other ) :
 _pBuffer( 0 ),
 _length( other._length ),
 _capacity( other._length ),
 _pCastingOperatorContext( 0 )
{
	if ( _capacity )
	{
		_pBuffer = (char*)malloc( _capacity );

		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBuffer( const EmaBuffer& ).";
			throwMeeException( temp );
			return;
		}

		memcpy( (void*)_pBuffer, (void*)other._pBuffer, _length );
	}
}

EmaBuffer::~EmaBuffer()
{
	if ( _pBuffer )
		free( _pBuffer );

	if ( _pCastingOperatorContext )
		delete _pCastingOperatorContext;
}

EmaBuffer& EmaBuffer::clear()
{
	_length = 0;

	markDirty();

	return *this;
}

const char* EmaBuffer::c_buf() const
{
	return _pBuffer;
}

UInt32 EmaBuffer::length() const
{
	return _length;
}

EmaBuffer& EmaBuffer::operator=( const EmaBuffer& other )
{
	if ( this == &other ) return *this;

	if ( _capacity < other._length )
	{
		_capacity = other._length;

		if ( _pBuffer )
			free( _pBuffer );

		_pBuffer = (char*)malloc( _capacity );
		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBuffer::operator=( const EmaBuffer& ).";
			throwMeeException( temp );
			return *this;
		}		
	}

	_length = other._length;
	
	if ( _length )
		memcpy( (void*)_pBuffer, (void*)other._pBuffer, _length );

	markDirty();

	return *this;
}

bool EmaBuffer::operator==( const EmaBuffer& other ) const
{
	if ( this == &other ) return true;

	if ( _length != other._length ) return false;

	return ( 0 == memcmp( (void*)_pBuffer, (void*)other._pBuffer, _length ) ? true : false );
}

EmaBuffer& EmaBuffer::setFrom( const char* buf, UInt32 length )
{
	if ( _capacity < length )
	{
		_capacity = length;

		if ( _pBuffer )
			free( _pBuffer );

		_pBuffer = (char*)malloc( _capacity );
		if ( !_pBuffer )
		{
			const char* temp = "Failed to allocate memory in EmaBuffer::setFrom( const char* buf, UInt32 length ).";
			throwMeeException( temp );
			return *this;
		}		
	}

	_length = length;

	if ( length )
		memcpy( (void*)_pBuffer, (void*)buf, length );

	markDirty();

	return *this;
}

EmaBuffer::operator const char* () const
{
	if ( !_pCastingOperatorContext )
	{
		try {
			_pCastingOperatorContext = new CastingOperatorContext;
		}
		catch ( std::bad_alloc& )
		{
			const char* temp = "Failed to allocate memory in EmaBuffer::operator const char* () const.";
			throwMeeException( temp );
			return 0;
		}
	}

	return _pCastingOperatorContext->getHexString( _pBuffer, _length );
}

const char* EmaBuffer::asRawHexString() const
{
	if (!_pCastingOperatorContext)
	{
		try {
			_pCastingOperatorContext = new CastingOperatorContext;
		}
		catch (std::bad_alloc)
		{
			const char* temp = "Failed to allocate memory in EmaBuffer::operator const char* () const.";
			throwMeeException(temp);
			return 0;
		}
	}

	return _pCastingOperatorContext->getRawHexString(_pBuffer, _length);
}

EmaBuffer& EmaBuffer::append( const EmaBuffer & other )
{
	if ( other.length() )
	{
		if ( _length + other.length() > _capacity )
		{
			_capacity += other.length();
			char* newBuffer = static_cast< char *>( malloc( _capacity ) );

			if ( !newBuffer )
			{
				throwMeeException( "Failed to allocate memory in EmaBuffer::append( const EmaBuffer & )" );
				return *this;
			}

			memcpy( newBuffer, _pBuffer, _length );
			free( _pBuffer );
			_pBuffer = newBuffer;
		}

		memcpy( _pBuffer + _length, other.c_buf(), other.length() );
		_length += other.length();
	}

	markDirty();

	return *this;
}

EmaBuffer& EmaBuffer::append( char c )
{
	if ( _length + 1 > _capacity )
	{
		++_capacity;
		char* newBuffer = static_cast< char *>( malloc( _capacity ) );
		
		if ( !newBuffer )
		{
			throwMeeException( "Failed to allocate memory in EmaBuffer::append( char )" );
			return *this;
		}

		memcpy( newBuffer, _pBuffer, _length );
		free( _pBuffer );
		_pBuffer = newBuffer;
	}

	_pBuffer[ _length++ ] = c;

	markDirty();

	return *this;
}

EmaBuffer& EmaBuffer::append( const char * str, UInt32 length )
{
	if ( length )
	{
		if ( _length + length > _capacity )
		{
			_capacity += length;
			char* newBuffer = static_cast< char *>( malloc( _capacity ) );
			if ( !newBuffer )
			{
				throwMeeException( "Failed to allocate memory in EmaBuffer::append( const char *, UInt32 )" );
				return *this;
			}
			memcpy( newBuffer, _pBuffer, _length );
			free( _pBuffer );
			_pBuffer = newBuffer;
		}

		memcpy( _pBuffer + _length, str, length );
		_length += length;
	}

	markDirty();

	return *this;
}

EmaBuffer& EmaBuffer::operator +=( const EmaBuffer & rhs )
{
	append( rhs );
	return *this;
}

EmaBuffer& EmaBuffer::operator +=( char c )
{
	append( c );
	return *this;
}

EmaBuffer operator+( EmaBuffer lhs, const EmaBuffer& rhs )
{
	lhs += rhs;
	return lhs;
}

void EmaBuffer::markDirty() const
{
	if ( _pCastingOperatorContext )
		_pCastingOperatorContext->markDirty();
}

char& EmaBuffer::operator[]( UInt32 index )
{
	if ( index >= _length )
	{
		EmaString text( "Attempt to access out of range position in EmaBuffer::operator[](). Passed in index is " );
		text.append( index ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
	}

	markDirty();

	return *(_pBuffer + index);
}

char EmaBuffer::operator[]( UInt32 index ) const
{
	if ( index >= _length )
	{
		EmaString text( "Attempt to access out of range position in EmaBuffer::operator[]() const. Passed in index is " );
		text.append( index ).append( " while length is " ).append( _length ).append( "." );
		throwOorException( text );
	}

	return *(_pBuffer + index);
}
