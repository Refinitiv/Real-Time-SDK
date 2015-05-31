/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "RmtesBufferImpl.h"
#include "ExceptionTranslator.h"
#include "RmtesBuffer.h"
#include <stdlib.h>

using namespace thomsonreuters::ema::access;

RmtesBufferImpl::RmtesBufferImpl() :
 _rsslBuffer(),
 _rsslCacheBuffer(),
 _rsslUTF8Buffer(),
 _rsslUTF16Buffer(),
 _toString(),
 _utf8Buffer(),
 _utf16Buffer(),
 _rsslUTF8BufferSet( false ),
 _rsslUTF16BufferSet( false ),
 _applyToCache( false )
{
}

RmtesBufferImpl::RmtesBufferImpl( UInt32 length ) :
 _rsslBuffer(),
 _rsslCacheBuffer(),
 _rsslUTF8Buffer(),
 _rsslUTF16Buffer(),
 _toString(),
 _utf8Buffer(),
 _utf16Buffer(),
 _rsslUTF8BufferSet( false ),
 _rsslUTF16BufferSet( false ),
 _applyToCache( false )
{
	if ( length )
	{
		_rsslCacheBuffer.data = (char*)malloc( length + 20 );
		if ( !_rsslCacheBuffer.data )
		{
			const char* temp = "Failed to allocate memory in RmtesBufferImpl( UInt32 ).";
			throwMeeException( temp );
			return;
		}

		_rsslCacheBuffer.allocatedLength = length + 20;
		_rsslCacheBuffer.length = 0;
	}
}

RmtesBufferImpl::RmtesBufferImpl( const char* buf, UInt32 length ) :
 _rsslBuffer(),
 _rsslCacheBuffer(),
 _rsslUTF8Buffer(),
 _rsslUTF16Buffer(),
 _toString(),
 _utf8Buffer(),
 _utf16Buffer(),
 _rsslUTF8BufferSet( false ),
 _rsslUTF16BufferSet( false ),
 _applyToCache( false )
{
	_rsslBuffer.data = (char*) buf;
	_rsslBuffer.length = length;
	
	if ( _rsslBuffer.length )
	{
		if ( !rsslHasPartialRMTESUpdate( &_rsslBuffer ) )
		{
			EmaString temp( "Failed to construct RmtesBufferImpl( const char*, UInt32 ) due to invalid data." );
			throwIueException( temp );
			return;
		}

		_rsslCacheBuffer.data = (char*)malloc( _rsslBuffer.length + 20 );
		if ( !_rsslCacheBuffer.data )
		{
			const char* temp = "Failed to allocate memory in RmtesBufferImpl( const char* , UInt32 ).";
			throwMeeException( temp );
			return;
		}

		_rsslCacheBuffer.allocatedLength = _rsslBuffer.length + 20;
		_rsslCacheBuffer.length = 0;

		RsslRet retCode = rsslRMTESApplyToCache( &_rsslBuffer, &_rsslCacheBuffer );

		while ( RSSL_RET_BUFFER_TOO_SMALL == retCode )
		{
			free( _rsslCacheBuffer.data );

			_rsslCacheBuffer.data = (char*)malloc( _rsslCacheBuffer.allocatedLength + _rsslCacheBuffer.allocatedLength );
			if ( !_rsslCacheBuffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl( const char* , UInt32 ).";
				throwMeeException( temp );
				return;
			}

			_rsslCacheBuffer.allocatedLength += _rsslCacheBuffer.allocatedLength;
			_rsslCacheBuffer.length = 0;

			retCode = rsslRMTESApplyToCache( &_rsslBuffer, &_rsslCacheBuffer );
		}

		if ( retCode < RSSL_RET_SUCCESS )
		{
			free( _rsslCacheBuffer.data );

			EmaString temp( "Failed to construct RmtesBufferImpl( const char* , UInt32 ). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
			return;
		}

		_applyToCache = true;
	}
}

RmtesBufferImpl::RmtesBufferImpl( const RmtesBufferImpl& other ) :
 _rsslBuffer(),
 _rsslCacheBuffer(),
 _rsslUTF8Buffer(),
 _rsslUTF16Buffer(),
 _toString(),
 _utf8Buffer(),
 _utf16Buffer(),
 _rsslUTF8BufferSet( false ),
 _rsslUTF16BufferSet( false ),
 _applyToCache( false )
{
	_rsslBuffer = other._rsslBuffer;
	
	if ( other._applyToCache )
	{
		_rsslCacheBuffer.allocatedLength = other._rsslCacheBuffer.allocatedLength;
		_rsslCacheBuffer.length = other._rsslCacheBuffer.length;

		if ( _rsslCacheBuffer.allocatedLength )
		{
			_rsslCacheBuffer.data = (char*)malloc( _rsslCacheBuffer.allocatedLength );
			if ( !_rsslCacheBuffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl( const RmtesBufferImpl& ).";
				throwMeeException( temp );
				return;
			}

			memcpy( (void*)_rsslCacheBuffer.data, (void*)other._rsslCacheBuffer.data, _rsslCacheBuffer.length );

			_applyToCache = true;
		}
	}
	else if ( _rsslBuffer.length )
	{
		if ( rsslHasPartialRMTESUpdate( &_rsslBuffer ) )
		{
			EmaString temp( "Failed to construct RmtesBufferImpl( const RmtesBuffer& ) due to invalid data." );
			throwIueException( temp );
			return;
		}

		_rsslCacheBuffer.data = (char*)malloc( _rsslBuffer.length + 20 );
		if ( !_rsslCacheBuffer.data )
		{
			const char* temp = "Failed to allocate memory in RmtesBufferImpl( const RmtesBuffer& ).";
			throwMeeException( temp );
			return;
		}

		_rsslCacheBuffer.allocatedLength = _rsslBuffer.length + 20;
		_rsslCacheBuffer.length = 0;

		RsslRet retCode = rsslRMTESApplyToCache( &_rsslBuffer, &_rsslCacheBuffer );

		while ( RSSL_RET_BUFFER_TOO_SMALL == retCode )
		{
			free( _rsslCacheBuffer.data );

			_rsslCacheBuffer.data = (char*)malloc( _rsslCacheBuffer.allocatedLength + _rsslCacheBuffer.allocatedLength );
			if ( !_rsslCacheBuffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl( const RmtesBufferImpl& ).";
				throwMeeException( temp );
				return;
			}

			_rsslCacheBuffer.allocatedLength += _rsslCacheBuffer.allocatedLength;
			_rsslCacheBuffer.length = 0;

			retCode = rsslRMTESApplyToCache( &_rsslBuffer, &_rsslCacheBuffer );
		}

		if ( retCode < RSSL_RET_SUCCESS )
		{
			free( _rsslCacheBuffer.data );

			EmaString temp( "Failed to construct RmtesBufferImpl( const RmtesBuffer& ). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
			return;
		}

		_applyToCache = true;
	}
}

RmtesBufferImpl::~RmtesBufferImpl()
{
	if ( _rsslCacheBuffer.data )
		free( _rsslCacheBuffer.data );

	if ( _rsslUTF8Buffer.data )
		free( _rsslUTF8Buffer.data );

	if ( _rsslUTF16Buffer.data )
		free( _rsslUTF16Buffer.data );
}

void RmtesBufferImpl::clear()
{
	_rsslCacheBuffer.length = 0;
	_rsslUTF8Buffer.length = 0;
	_rsslUTF16Buffer.length = 0;

	rsslClearBuffer( &_rsslBuffer );

	_toString.clear();
	_applyToCache = false;
	_rsslUTF8BufferSet = false;
	_rsslUTF16BufferSet = false;
}

const EmaBuffer& RmtesBufferImpl::getAsUTF8()
{
	if ( !_rsslUTF8BufferSet )
	{
		if ( !_applyToCache )
			apply( _rsslBuffer.data, _rsslBuffer.length );

		if ( _rsslCacheBuffer.length == 0 )
		{
			_utf8Buffer.clear();
			return _utf8Buffer.toBuffer();
		}

		if ( !_rsslUTF8Buffer.data )
		{
			_rsslUTF8Buffer.length = _rsslCacheBuffer.allocatedLength;
			_rsslUTF8Buffer.data =  (char*)malloc( _rsslUTF8Buffer.length );
			if ( !_rsslUTF8Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::getAsUTF8()";
				throwMeeException( temp );
				return _utf8Buffer.toBuffer();
			}	
		}
		else if ( _rsslCacheBuffer.length > _rsslUTF8Buffer.length )
		{
			free( _rsslUTF8Buffer.data );
			
			_rsslUTF8Buffer.length = ( _rsslUTF8Buffer.length == 0 ) ? _rsslCacheBuffer.allocatedLength : _rsslCacheBuffer.length;
			_rsslUTF8Buffer.data =  (char*)malloc( _rsslUTF8Buffer.length );
			if ( !_rsslUTF8Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::getAsUTF8()";
				throwMeeException( temp );
				return _utf8Buffer.toBuffer();
			}
		}

		RsslRet retCode = rsslRMTESToUTF8( &_rsslCacheBuffer, &_rsslUTF8Buffer );

		while ( RSSL_RET_BUFFER_TOO_SMALL == retCode )
		{
			free( _rsslUTF8Buffer.data );

			_rsslUTF8Buffer.length += _rsslUTF8Buffer.length;
			_rsslUTF8Buffer.data =  (char*)malloc( _rsslUTF8Buffer.length );
			if ( !_rsslUTF8Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::getAsUTF8()";
				throwMeeException( temp );
				return _utf8Buffer.toBuffer();
			}

			retCode = rsslRMTESToUTF8( &_rsslCacheBuffer, &_rsslUTF8Buffer );
		}

		if ( retCode < RSSL_RET_SUCCESS )
		{
			EmaString temp( "Failed to convert to UTF8 in RmtesBufferImpl::getAsUTF8(). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
			return _utf8Buffer.toBuffer();
		}
		
		_rsslUTF8BufferSet = true;
	}
		
	_utf8Buffer.setFromInt( _rsslUTF8Buffer.data, _rsslUTF8Buffer.length );

	return _utf8Buffer.toBuffer();
}

const EmaBufferU16& RmtesBufferImpl::getAsUTF16()
{
	if ( !_rsslUTF16BufferSet )
	{
		if ( !_applyToCache )
			apply( _rsslBuffer.data, _rsslBuffer.length );
		
		if ( _rsslCacheBuffer.length == 0 )
		{
			_utf16Buffer.clear();
			return _utf16Buffer.toBuffer();
		}

		if ( !_rsslUTF16Buffer.data )
		{
			_rsslUTF16Buffer.length = _rsslCacheBuffer.allocatedLength;
			_rsslUTF16Buffer.data =  (RsslUInt16*)calloc( _rsslUTF16Buffer.length,  sizeof( RsslUInt16 ) );
			if ( !_rsslUTF16Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::getAsUTF16()";
				throwMeeException( temp );
				return _utf16Buffer.toBuffer();
			}	
		}
		else if ( _rsslCacheBuffer.length > _rsslUTF16Buffer.length )
		{
			free( _rsslUTF16Buffer.data );
			
			_rsslUTF16Buffer.length = ( _rsslUTF16Buffer.length == 0 ) ? _rsslCacheBuffer.allocatedLength : _rsslCacheBuffer.length;
			_rsslUTF16Buffer.data =  (RsslUInt16*)calloc( _rsslUTF16Buffer.length,  sizeof( RsslUInt16 ) );
			if ( !_rsslUTF16Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::getAsUTF16()";
				throwMeeException( temp );
				return _utf16Buffer.toBuffer();
			}	
		}
	
		RsslRet retCode = rsslRMTESToUCS2( &_rsslCacheBuffer, &_rsslUTF16Buffer );

		while ( RSSL_RET_BUFFER_TOO_SMALL == retCode )
		{
			free( _rsslUTF16Buffer.data );

			_rsslUTF16Buffer.length += _rsslUTF16Buffer.length;

			_rsslUTF16Buffer.data =  (RsslUInt16*)calloc( _rsslUTF16Buffer.length,  sizeof( RsslUInt16 ) );
			if ( !_rsslUTF16Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::getAsUTF16()";
				throwMeeException( temp );
				return _utf16Buffer.toBuffer();
			}	
		}

		if ( retCode < RSSL_RET_SUCCESS )
		{
			EmaString temp( "Failed to convert to UTF8 in RmtesBufferImpl::getAsUTF16(). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
			return _utf16Buffer.toBuffer();
		}
		
		_rsslUTF16BufferSet = true;
	}
		
	_utf16Buffer.setFromInt( _rsslUTF16Buffer.data, _rsslUTF16Buffer.length );

	return _utf16Buffer.toBuffer();
}

const EmaString& RmtesBufferImpl::toString()
{
	if ( !_rsslUTF8BufferSet )
	{
		if ( !_applyToCache )
			apply( _rsslBuffer.data, _rsslBuffer.length );

		if ( _rsslCacheBuffer.length == 0 )
		{
			_toString.clear();
			return _toString.toString();
		}

		if ( !_rsslUTF8Buffer.data )
		{
			_rsslUTF8Buffer.length = _rsslCacheBuffer.allocatedLength;
			_rsslUTF8Buffer.data =  (char*)malloc( _rsslUTF8Buffer.length );
			if ( !_rsslUTF8Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::toString()";
				throwMeeException( temp );
				return _toString.toString();
			}	
		}
		else if ( _rsslCacheBuffer.length > _rsslUTF8Buffer.length )
		{
			free( _rsslUTF8Buffer.data );
			
			_rsslUTF8Buffer.length = ( _rsslUTF8Buffer.length == 0 ) ? _rsslCacheBuffer.allocatedLength : _rsslCacheBuffer.length;
			_rsslUTF8Buffer.data =  (char*)malloc( _rsslUTF8Buffer.length );
			if ( !_rsslUTF8Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::toString()";
				throwMeeException( temp );
				return _toString.toString();
			}	
		}
	
		RsslRet retCode = rsslRMTESToUTF8( &_rsslCacheBuffer, &_rsslUTF8Buffer );

		while ( RSSL_RET_BUFFER_TOO_SMALL == retCode )
		{
			free( _rsslUTF8Buffer.data );

			_rsslUTF8Buffer.length += _rsslUTF8Buffer.length;
			_rsslUTF8Buffer.data =  (char*)malloc( _rsslUTF8Buffer.length );
			if ( !_rsslUTF8Buffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::toString()";
				throwMeeException( temp );
				return _toString.toString();
			}

			retCode = rsslRMTESToUTF8( &_rsslCacheBuffer, &_rsslUTF8Buffer );
		}

		if ( retCode < RSSL_RET_SUCCESS )
		{
			EmaString temp( "Failed to convert to UTF8 in RmtesBufferImpl::toString(). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
			return _toString.toString();
		}
		
		_rsslUTF8BufferSet = true;
	}
		
	_toString.setInt( _rsslUTF8Buffer.data, _rsslUTF8Buffer.length, false );

	return _toString.toString();
}

void RmtesBufferImpl::apply( const RmtesBufferImpl& source )
{
	_rsslUTF8BufferSet = false;
	_rsslUTF16BufferSet = false;
  
	_rsslBuffer = source._rsslBuffer;
	
	if ( source._applyToCache )
	{
		if ( _rsslCacheBuffer.allocatedLength <  source._rsslCacheBuffer.length ) 
		{
			if ( _rsslCacheBuffer.data ) 
				free( _rsslCacheBuffer.data );

			_rsslCacheBuffer.data = (char*)malloc( source._rsslCacheBuffer.length );
			if ( !_rsslCacheBuffer.data )
			{
				const char* temp = "Failed to allocate memory in RmtesBufferImpl::apply( const RmtesBufferImpl& )";
				throwMeeException( temp );
				return;
			}	

			_rsslCacheBuffer.allocatedLength = source._rsslCacheBuffer.length;
		}

		_rsslCacheBuffer.length = source._rsslCacheBuffer.length;
		memcpy( (void*)_rsslCacheBuffer.data, (void*)source._rsslCacheBuffer.data, _rsslCacheBuffer.length );

		_applyToCache = true;
	}
	else if ( _rsslBuffer.length )
	{
		if ( rsslHasPartialRMTESUpdate( &_rsslBuffer ) )
		{
			UInt32 actualLen = _rsslBuffer.length + _rsslCacheBuffer.length;
			if ( actualLen > _rsslCacheBuffer.allocatedLength )
			{
				_rsslCacheBuffer.allocatedLength += ( _rsslCacheBuffer.allocatedLength == 0 ?  _rsslBuffer.length + 20 : _rsslBuffer.length );
				char* newBuffer = static_cast< char *>( malloc( _rsslCacheBuffer.allocatedLength ) );
				if ( !newBuffer )
				{
					throwMeeException( "Failed to allocate memory in RmtesBufferImpl::apply( const RmtesBufferImpl& )" );
					return;
				}	
				memcpy( (void*)newBuffer, (void*)_rsslCacheBuffer.data, _rsslCacheBuffer.length );
				free( _rsslCacheBuffer.data );
				_rsslCacheBuffer.data = newBuffer;
			}
		}
		else
		{
			if ( _rsslBuffer.length > _rsslCacheBuffer.allocatedLength )
			{
				_rsslCacheBuffer.allocatedLength += ( _rsslCacheBuffer.allocatedLength == 0 ?  _rsslBuffer.length + 20 : _rsslBuffer.length );

				if ( _rsslCacheBuffer.data )
					free(_rsslCacheBuffer.data); 

				_rsslCacheBuffer.data = (char*)malloc( _rsslCacheBuffer.allocatedLength );
				if ( !_rsslCacheBuffer.data )
				{
					throwMeeException( "Failed to allocate memory in RmtesBufferImpl::apply( const RmtesBufferImpl& )" );
					return;
				}	
			}

			_rsslCacheBuffer.length = 0;
		}

		RsslRet retCode = rsslRMTESApplyToCache( &_rsslBuffer, &_rsslCacheBuffer );
		if ( retCode < RSSL_RET_SUCCESS )
		{
			EmaString temp( "Failed to apply in RmtesBufferImpl::apply( const RmtesBufferImpl& ). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}

		_applyToCache = true;
	}
}

void RmtesBufferImpl::apply( const char* buf, UInt32 length )
{
	_rsslUTF8BufferSet = false;
	_rsslUTF16BufferSet = false;
  
	_rsslBuffer.data = (char*)buf;
	_rsslBuffer.length = length;
	
	if ( _rsslBuffer.length )
	{
		if ( rsslHasPartialRMTESUpdate( &_rsslBuffer ) )
		{
			UInt32 actualLen = _rsslBuffer.length + _rsslCacheBuffer.length;
			if ( actualLen > _rsslCacheBuffer.allocatedLength )
			{
				_rsslCacheBuffer.allocatedLength += ( _rsslCacheBuffer.allocatedLength == 0 ?  _rsslBuffer.length + 20 : _rsslBuffer.length );
				char* newBuffer = static_cast< char *>( malloc( _rsslCacheBuffer.allocatedLength ) );
				if ( !newBuffer )
				{
					throwMeeException( "Failed to allocate memory in RmtesBufferImpl::apply( const char*, UInt32 )" );
					return;
				}	
				memcpy( (void*)newBuffer, (void*)_rsslCacheBuffer.data, _rsslCacheBuffer.length );
				free( _rsslCacheBuffer.data );
				_rsslCacheBuffer.data = newBuffer;
			}
		}
		else
		{
			if ( _rsslBuffer.length > _rsslCacheBuffer.allocatedLength )
			{
				_rsslCacheBuffer.allocatedLength += ( _rsslCacheBuffer.allocatedLength == 0 ?  _rsslBuffer.length + 20 : _rsslBuffer.length );

				if ( _rsslCacheBuffer.data )
					free(_rsslCacheBuffer.data); 

				_rsslCacheBuffer.data = (char*)malloc( _rsslCacheBuffer.allocatedLength );
				if ( !_rsslCacheBuffer.data )
				{
					throwMeeException( "Failed to allocate memory in RmtesBufferImpl::apply( const char* , UInt32 )" );
					return;
				}	
			}

			_rsslCacheBuffer.length = 0;
		}

		RsslRet retCode = rsslRMTESApplyToCache( &_rsslBuffer, &_rsslCacheBuffer );
		if ( retCode < RSSL_RET_SUCCESS )
		{
			EmaString temp( "Failed to apply in RmtesBufferImpl::apply( const char* , UInt32 ). Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}

		_applyToCache = true;
	}

}

void RmtesBufferImpl::apply( const EmaBuffer& buf )
{
	apply( buf.c_buf(), buf.length() );
}
