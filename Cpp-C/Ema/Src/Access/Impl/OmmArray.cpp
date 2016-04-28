/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmArray.h"
#include "OmmBuffer.h"
#include "ExceptionTranslator.h"
#include "OmmArrayDecoder.h"
#include "OmmArrayEncoder.h"
#include "Utilities.h"
#include "GlobalPool.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

OmmArray::OmmArray() :
 _toString(),
 _entry(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

OmmArray::~OmmArray()
{
	if ( _pEncoder )
		g_pool._arrayEncoderPool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool._arrayDecoderPool.returnItem( _pDecoder );
}

OmmArray& OmmArray::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

DataType::DataTypeEnum OmmArray::getDataType() const
{
	return DataType::ArrayEnum;
}

Data::DataCode OmmArray::getCode() const
{
	return _pDecoder->getCode();
}

const OmmArrayEntry& OmmArray::getEntry() const
{
	if ( !_pDecoder || !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp );
	}

	return _entry;
}

bool OmmArray::forth() const
{
	return !_pDecoder->getNextData();
}

void OmmArray::reset() const
{
	_pDecoder->reset();
}

const EmaString& OmmArray::toString() const
{
	return toString( 0 );
}

const EmaString& OmmArray::toString( UInt64 indent ) const
{
	if ( _pDecoder->getCode() == Data::BlankEnum )
	{
		addIndent( _toString.clear(), indent )
			.append( "OmmArray" );

		++indent;
		addIndent( _toString.append( "\n" ), indent ).append( "blank array" );
		--indent;
	}
	else
	{
		OmmArrayDecoder tempDecoder;
		tempDecoder.clone( *_pDecoder );

		addIndent( _toString.clear(), indent )
			.append( "OmmArray with entries of dataType=\"" )
			.append( getDTypeAsString( dataType[ tempDecoder.getRsslDataType() ] ) ).append( "\"" );

		if ( tempDecoder.hasFixedWidth() )
			_toString.append( " fixed width=\"" ).append( (UInt32)tempDecoder.getFixedWidth() ).append( "\"" );

		++indent;

		while ( !tempDecoder.getNextData() )
		{
			addIndent( _toString.append( "\n" ), indent ).append( "value=" );
			if ( tempDecoder.getLoad().getDataType() == DataType::BufferEnum )
			{
				_toString.append( "\n" ).append( tempDecoder.getLoad().toString() );
			}
			else if ( tempDecoder.getLoad().getDataType() == DataType::ErrorEnum )
			{
				_toString.append( "\n" ).append( tempDecoder.getLoad().toString( indent ) );
			}
			else
			{
				_toString.append( "\"" ).append( tempDecoder.getLoad().toString() ).append( "\"" );
			}
		}

		--indent;
	}

	addIndent( _toString.append( "\n" ), indent ).append( "OmmArrayEnd\n" );

	return _toString;
}

const EmaBuffer& OmmArray::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

bool OmmArray::hasFixedWidth() const
{
	return _pDecoder->hasFixedWidth();
}

UInt16 OmmArray::getFixedWidth() const
{
	return _pDecoder->getFixedWidth();
}

Decoder& OmmArray::getDecoder()
{
	if ( !_pDecoder )
	{
		_pDecoder = g_pool._arrayDecoderPool.getItem();
		_entry._pDecoder = _pDecoder;
		_entry._pLoad = &_pDecoder->getLoad();
	}

	return *_pDecoder;
}

bool OmmArray::hasDecoder() const
{
	return _pDecoder ? true : false;
}

const Encoder& OmmArray::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	return *_pEncoder;
}

OmmArray& OmmArray::fixedWidth( UInt16 width )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->fixedWidth( width );

	return *this;
}

OmmArray& OmmArray::addInt( Int64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addInt( value );

	return *this;
}

OmmArray& OmmArray::addUInt( UInt64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addUInt( value );

	return *this;
}

OmmArray& OmmArray::addReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addReal( mantissa, magnitudeType );

	return *this;
}

OmmArray& OmmArray::addRealFromDouble( double value, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addRealFromDouble( value, magnitudeType );

	return *this;
}

OmmArray& OmmArray::addFloat( float value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addFloat( value );

	return *this;
}

OmmArray& OmmArray::addDouble( double value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addDouble( value );

	return *this;
}

OmmArray& OmmArray::addDate( UInt16 year, UInt8 month, UInt8 day )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addDate( year, month, day );

	return *this;
}

OmmArray& OmmArray::addTime( UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond,
					  UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addTime( hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

OmmArray& OmmArray::addDateTime( UInt16 year, UInt8 month, UInt8 day,
					   UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond,
					   UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addDateTime( year, month, day, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

OmmArray& OmmArray::addQos( UInt32 timeliness, UInt32 rate )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addQos( timeliness, rate );

	return *this;
}

OmmArray& OmmArray::addState( OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addState( streamState, dataState, statusCode, statusText );

	return *this;
}

OmmArray& OmmArray::addEnum( UInt16 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addEnum( value );

	return *this;
}

OmmArray& OmmArray::addBuffer( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addBuffer( value );

	return *this;
}

OmmArray& OmmArray::addAscii( const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addAscii( value );

	return *this;
}

OmmArray& OmmArray::addUtf8( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addUtf8( value );

	return *this;
}

OmmArray& OmmArray::addRmtes( const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addRmtes( value );

	return *this;
}

OmmArray& OmmArray::addCodeInt()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeInt();

	return *this;
}

OmmArray& OmmArray::addCodeUInt()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeUInt();

	return *this;
}

OmmArray& OmmArray::addCodeReal()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeReal();

	return *this;
}

OmmArray& OmmArray::addCodeFloat()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeFloat();

	return *this;
}

OmmArray& OmmArray::addCodeDouble()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeDouble();

	return *this;
}

OmmArray& OmmArray::addCodeDate()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeDate();

	return *this;
}

OmmArray& OmmArray::addCodeTime()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeTime();

	return *this;
}

OmmArray& OmmArray::addCodeDateTime()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeDateTime();

	return *this;
}

OmmArray& OmmArray::addCodeQos()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeQos();

	return *this;
}

OmmArray& OmmArray::addCodeState()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeState();

	return *this;
}

OmmArray& OmmArray::addCodeEnum()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeEnum();

	return *this;
}

OmmArray& OmmArray::addCodeBuffer()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeBuffer();

	return *this;
}

OmmArray& OmmArray::addCodeAscii()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeAscii();

	return *this;
}

OmmArray& OmmArray::addCodeUtf8()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeUtf8();

	return *this;
}

OmmArray& OmmArray::addCodeRmtes()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->addCodeRmtes();

	return *this;
}

const OmmArray& OmmArray::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._arrayEncoderPool.getItem();

	_pEncoder->complete();

	return *this;
}

bool OmmArray::hasEncoder() const
{
	return _pEncoder ? true : false;
}
