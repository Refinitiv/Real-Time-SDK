/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ElementList.h"
#include "OmmBuffer.h"
#include "EmaString.h"
#include "ExceptionTranslator.h"
#include "ElementListDecoder.h"
#include "ElementListEncoder.h"
#include "Utilities.h"
#include "GlobalPool.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

ElementList::ElementList() :
 _toString(),
 _entry(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

ElementList::~ElementList()
{
	if ( _pEncoder )
		g_pool._elementListEncoderPool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool._elementListDecoderPool.returnItem( _pDecoder );
}

ElementList& ElementList::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

DataType::DataTypeEnum ElementList::getDataType() const
{
	return DataType::ElementListEnum;
}

Data::DataCode ElementList::getCode() const
{
	return Data::NoCodeEnum;
}

bool ElementList::hasInfo() const
{
	return _pDecoder->hasInfo();
}

Int16 ElementList::getInfoElementListNum() const
{
	return _pDecoder->getInfoElementListNum();
}

bool ElementList::forth() const
{
	return _pDecoder->getNextData();
}

bool ElementList::forth( const EmaString& name ) const
{
	return _pDecoder->getNextData( name );
}

bool ElementList::forth( const Data& data ) const
{
	return _pDecoder->getNextData( data );
}

void ElementList::reset() const
{
	_pDecoder->reset();
}

const EmaString& ElementList::toString() const
{
	return toString( 0 );
}

const EmaString& ElementList::toString( UInt64 indent ) const
{
	ElementListDecoder tempDecoder;
	tempDecoder.clone( *_pDecoder );

	addIndent( _toString.clear(), indent ).append( "ElementList" );
			
	if ( tempDecoder.hasInfo() )
		_toString.append( " ElementListNum=\"" ).append( tempDecoder.getInfoElementListNum() );

	++indent;
		
	while ( !tempDecoder.getNextData() )
	{
		addIndent( _toString.append( "\n" ), indent )
			.append( "ElementEntry name=\"" ).append( tempDecoder.getName() )
			.append( "\" dataType=\"" ).append( getDTypeAsString( tempDecoder.getLoad().getDataType() ) );

		if ( tempDecoder.getLoad().getDataType() <= DataType::ArrayEnum )
		{
			++indent; 
			_toString.append( "\"\n" ).append( tempDecoder.getLoad().toString( indent ) );
			--indent;
			addIndent( _toString, indent ).append( "ElementEntryEnd" );
		}
		else if ( tempDecoder.getLoad().getDataType() == DataType::BufferEnum )
		{
			_toString.append( "\" value=\n\n" ).append( tempDecoder.getLoad().getAsHex() ).append( "\n" );
			addIndent( _toString, indent ).append( "ElementEntryEnd" );
		}
		else
			_toString.append( "\" value=\"" ).append( tempDecoder.getLoad().toString() ).append( "\"" );
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "ElementListEnd\n" );

	return _toString;
}

const EmaBuffer& ElementList::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& ElementList::getDecoder()
{
	if ( !_pDecoder )
	{
		_pDecoder = g_pool._elementListDecoderPool.getItem();
		_entry._pDecoder = _pDecoder;
		_entry._pLoad = &_pDecoder->getLoad();
	}

	return *_pDecoder;
}

const ElementEntry& ElementList::getEntry() const
{
	if ( !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp );
	}

	return _entry;
}

const Encoder& ElementList::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	return *_pEncoder;
}

ElementList& ElementList::info( Int16 elmentListNum )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->info( elmentListNum );

	return *this;
}

ElementList& ElementList::addInt( const EmaString& name, Int64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addInt( name, value );

	return *this;
}

ElementList& ElementList::addUInt( const EmaString& name, UInt64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addUInt( name, value );

	return *this;
}

ElementList& ElementList::addReal( const EmaString& name, Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addReal( name, mantissa, magnitudeType );

	return *this;
}

ElementList& ElementList::addRealFromDouble( const EmaString& name, double value,
								OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addRealFromDouble( name, value, magnitudeType );

	return *this;
}

ElementList& ElementList::addFloat( const EmaString& name, float value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addFloat( name, value );

	return *this;
}

ElementList& ElementList::addDouble( const EmaString& name, double value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addDouble( name, value );

	return *this;
}

ElementList& ElementList::addDate( const EmaString& name, UInt16 year, UInt8 month, UInt8 day )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addDate( name, year, month, day );

	return *this;
}

ElementList& ElementList::addTime( const EmaString& name, UInt8 hour, UInt8 minute, UInt8 second,
								  UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addTime( name, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

ElementList& ElementList::addDateTime( const EmaString& name,
						UInt16 year, UInt8 month, UInt8 day,
						UInt8 hour, UInt8 minute, UInt8 second,
						UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addDateTime( name, year, month, day, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

ElementList& ElementList::addQos( const EmaString& name,
					UInt32 timeliness,
					UInt32 rate )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addQos( name, timeliness, rate );

	return *this;
}

ElementList& ElementList::addState( const EmaString& name,
					OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addState( name, streamState, dataState, statusCode, statusText );

	return *this;
}

ElementList& ElementList::addEnum( const EmaString& name, UInt16 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addEnum( name, value );

	return *this;
}

ElementList& ElementList::addBuffer( const EmaString& name, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addBuffer( name, value );

	return *this;
}

ElementList& ElementList::addAscii( const EmaString& name, const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addAscii( name, value );

	return *this;
}

ElementList& ElementList::addUtf8( const EmaString& name, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addUtf8( name, value );

	return *this;
}

ElementList& ElementList::addRmtes( const EmaString& name, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addRmtes( name, value );

	return *this;
}

ElementList& ElementList::addArray( const EmaString& name, const OmmArray& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addArray( name, value );

	return *this;
}

ElementList& ElementList::addElementList( const EmaString& name, const ElementList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addElementList( name, value );

	return *this;
}

ElementList& ElementList::addFieldList( const EmaString& name, const FieldList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addFieldList( name, value );

	return *this;
}

ElementList& ElementList::addReqMsg( const EmaString& name, const ReqMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addReqMsg( name, value );

	return *this;
}

ElementList& ElementList::addRefreshMsg( const EmaString& name, const RefreshMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addRefreshMsg( name, value );

	return *this;
}

ElementList& ElementList::addUpdateMsg( const EmaString& name, const UpdateMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addUpdateMsg( name, value );

	return *this;
}

ElementList& ElementList::addStatusMsg( const EmaString& name, const StatusMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addStatusMsg( name, value );

	return *this;
}

ElementList& ElementList::addPostMsg( const EmaString& name, const PostMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addPostMsg( name, value );

	return *this;
}

ElementList& ElementList::addAckMsg( const EmaString& name, const AckMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addAckMsg( name, value );

	return *this;
}

ElementList& ElementList::addGenericMsg( const EmaString& name, const GenericMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addGenericMsg( name, value );

	return *this;
}

ElementList& ElementList::addMap( const EmaString& name, const Map& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addMap( name, value );

	return *this;
}

ElementList& ElementList::addVector( const EmaString& name, const Vector& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addVector( name, value );

	return *this;
}

ElementList& ElementList::addSeries( const EmaString& name, const Series& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addSeries( name, value );

	return *this;
}

ElementList& ElementList::addFilterList( const EmaString& name, const FilterList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addFilterList( name, value );

	return *this;
}

ElementList& ElementList::addOpaque( const EmaString& name, const OmmOpaque& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addOpaque( name, value );

	return *this;
}

ElementList& ElementList::addXml( const EmaString& name, const OmmXml& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addXml( name, value );

	return *this;
}

ElementList& ElementList::addAnsiPage( const EmaString& name, const OmmAnsiPage& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addAnsiPage( name, value );

	return *this;
}

ElementList& ElementList::addCodeInt( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeInt( name );

	return *this;
}

ElementList& ElementList::addCodeUInt( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeUInt( name );

	return *this;
}

ElementList& ElementList::addCodeReal( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeReal( name );

	return *this;
}

ElementList& ElementList::addCodeFloat( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeFloat( name );

	return *this;
}

ElementList& ElementList::addCodeDouble( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeDouble( name );

	return *this;
}

ElementList& ElementList::addCodeDate( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeDate( name );

	return *this;
}

ElementList& ElementList::addCodeTime( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeTime( name );

	return *this;
}

ElementList& ElementList::addCodeDateTime( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeDateTime( name );

	return *this;
}

ElementList& ElementList::addCodeQos( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeQos( name );

	return *this;
}

ElementList& ElementList::addCodeState( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeState( name );

	return *this;
}

ElementList& ElementList::addCodeEnum( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeEnum( name );

	return *this;
}

ElementList& ElementList::addCodeBuffer( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeBuffer( name );

	return *this;
}

ElementList& ElementList::addCodeAscii( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeAscii( name );

	return *this;
}

ElementList& ElementList::addCodeUtf8( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeUtf8( name );

	return *this;
}

ElementList& ElementList::addCodeRmtes( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->addCodeRmtes( name );

	return *this;
}

const ElementList& ElementList::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._elementListEncoderPool.getItem();

	_pEncoder->complete();

	return *this;
}
