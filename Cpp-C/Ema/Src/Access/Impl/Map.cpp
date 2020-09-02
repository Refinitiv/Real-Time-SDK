/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Map.h"
#include "MapDecoder.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "OmmInvalidUsageException.h"

using namespace rtsdk::ema::access;

extern const EmaString& getMActionAsString( MapEntry::MapAction mAction );
extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Map::Map() :
 _toString(),
 _entry(),
 _summary(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

Map::~Map()
{
	if ( _pEncoder )
		g_pool._mapEncoderPool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool._mapDecoderPool.returnItem( _pDecoder );
}

Map& Map::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode Map::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum Map::getDataType() const
{
	return DataType::MapEnum;
}

bool Map::forth() const
{
	return !_pDecoder->getNextData();
}

void Map::reset() const
{
	_pDecoder->reset();
}

const EmaString& Map::toString() const
{
	return toString( 0 );
}

const EmaString& Map::toString( UInt64 indent ) const
{
	if ( !_pDecoder )
		return _toString.clear().append( "\nDecoding of just encoded object in the same application is not supported\n" );	
	
	MapDecoder tempDecoder;
	tempDecoder.clone( *_pDecoder );

	addIndent( _toString.clear(), indent ).append( "Map" );
			
	if ( tempDecoder.hasTotalCountHint() )
		_toString.append( " totalCountHint=\"" ).append( tempDecoder.getTotalCountHint() ).append( "\"" );

	if ( tempDecoder.hasKeyFieldId() )
		_toString.append( " keyFieldId=\"" ).append( tempDecoder.getKeyFieldId() ).append( "\"" );

	if ( tempDecoder.hasSummary() )
	{
		++indent;
		addIndent( _toString.append( "\n" ), indent )
			.append( "SummaryData dataType=\"" ).append( getDTypeAsString( tempDecoder.getSummaryData()->getDataType() ) ).append( "\"\n" );

		++indent;
		_toString += tempDecoder.getSummaryData()->toString( indent );
		--indent;

		addIndent( _toString, indent )
			.append( "SummaryDataEnd" );
		--indent;
	}

	++indent;
		
	while ( !tempDecoder.getNextData() )
	{
		addIndent( _toString.append( "\n" ), indent )
			.append( "MapEntry action=\"" ).append( getMActionAsString( tempDecoder.getAction() ) )
			.append( "\" key dataType=\"" ).append( getDTypeAsString( tempDecoder.getKeyData().getDataType() ) );
        if ( tempDecoder.getKeyData().getDataType() == DataType::BufferEnum )
		{
            _toString.append( "\" value=\n\n" ).append( tempDecoder.getKeyData().toString() );
			addIndent( _toString.append( "\n" ), indent );
		}
		else
			_toString.append( "\" value=\"" ).append( tempDecoder.getKeyData().toString() ).append( "\"" );

		if ( tempDecoder.hasEntryPermissionData() )
		{
			_toString.append( " permissionData=\"" );
			hexToString( _toString, tempDecoder.getEntryPermissionData() );
			_toString.append( "\"" );
		}

		_toString.append( " dataType=\"" ).append( getDTypeAsString( tempDecoder.getLoad().getDataType() ) ).append( "\"\n" );
		
		++indent;
		_toString += tempDecoder.getLoad().toString( indent );
		--indent;

		addIndent( _toString, indent )
			.append( "MapEntryEnd" );
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "MapEnd\n" );

	return _toString;
}

const EmaBuffer& Map::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& Map::getDecoder()
{
	if ( !_pDecoder )
	{
		_summary._pDecoder = _entry._pDecoder = _pDecoder = g_pool._mapDecoderPool.getItem();
		_entry._pLoad = &_pDecoder->getLoad();
		_entry._key._pData = &_pDecoder->getKeyData();
	}

	return *_pDecoder;
}

bool Map::hasDecoder() const
{
	return _pDecoder ? true : false;
}

const Encoder& Map::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	return *_pEncoder;
}

Int16 Map::getKeyFieldId() const
{
	return _pDecoder->getKeyFieldId();
}

UInt32 Map::getTotalCountHint() const
{
	return _pDecoder->getTotalCountHint();
}

bool Map::hasTotalCountHint() const
{
	return _pDecoder->hasTotalCountHint();
}

bool Map::hasKeyFieldId() const
{
	return _pDecoder->hasKeyFieldId();
}

const SummaryData& Map::getSummaryData() const
{
	return _summary;
}

const MapEntry& Map::getEntry() const
{
	if ( !_pDecoder || !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _entry;
}


Map& Map::keyFieldId( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->keyFieldId( fieldId );

	return *this;
}

Map& Map::totalCountHint( UInt32 totalCountHint )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->totalCountHint( totalCountHint );

	return *this;
}

Map& Map::summaryData( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->summaryData( data );

	return *this;
}

Map& Map::keyType(DataType::DataTypeEnum keyPrimitiveType)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->keyType(keyPrimitiveType);

	return *this;
}

Map& Map::addKeyInt( Int64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyInt( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyInt(Int64 key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyInt(key, action, permissionData);

	return *this;
}

Map& Map::addKeyUInt( UInt64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyUInt( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyUInt(UInt64 key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyUInt(key, action, permissionData);

	return *this;
}

Map& Map::addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, 
	MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyReal( mantissa, magnitudeType, action, value, permissionData );

	return *this;
}

Map& Map::addKeyReal(Int64 mantissa, OmmReal::MagnitudeType magnitudeType,
	MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyReal(mantissa, magnitudeType, action, permissionData);

	return *this;
}

Map& Map::addKeyRealFromDouble( double key, MapEntry::MapAction action,
	const ComplexType& value,
	OmmReal::MagnitudeType magnitudeType,
	const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyRealFromDouble( key, action, value, magnitudeType, permissionData );

	return *this;
}

Map& Map::addKeyRealFromDouble(double key, MapEntry::MapAction action,
	OmmReal::MagnitudeType magnitudeType,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyRealFromDouble(key, action, magnitudeType, permissionData);

	return *this;
}

Map& Map::addKeyFloat( float key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyFloat( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyFloat(float key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyFloat(key, action, permissionData);

	return *this;
}

Map& Map::addKeyDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyDouble( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyDouble(double key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyDouble(key, action, permissionData);

	return *this;
}

Map& Map::addKeyDate( UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyDate( year, month, day, action, value, permissionData );

	return *this;
}

Map& Map::addKeyDate(UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyDate(year, month, day, action, permissionData);

	return *this;
}

Map& Map::addKeyTime( UInt8 hour, UInt8 minute, UInt8 second,
	UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
	MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyTime( hour, minute, second, millisecond, microsecond, nanosecond, action, 
		value, permissionData );

	return *this;
}

Map& Map::addKeyTime(UInt8 hour, UInt8 minute, UInt8 second,
	UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
	MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyTime(hour, minute, second, millisecond, microsecond, nanosecond, action,
		permissionData);

	return *this;
}

Map& Map::addKeyDateTime( UInt16 year, UInt8 month, UInt8 day,
	UInt8 hour, UInt8 minute, UInt8 second,
	UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
	MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyDateTime( year, month, day, hour, minute, second, millisecond, microsecond, 
		nanosecond, action, value, permissionData );

	return *this;
}

Map& Map::addKeyDateTime(UInt16 year, UInt8 month, UInt8 day,
	UInt8 hour, UInt8 minute, UInt8 second,
	UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
	MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyDateTime(year, month, day, hour, minute, second, millisecond, microsecond,
		nanosecond, action, permissionData);

	return *this;
}

Map& Map::addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyQos( timeliness, rate, action, value, permissionData );

	return *this;
}

Map& Map::addKeyQos(UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyQos(timeliness, rate, action, permissionData);

	return *this;
}

Map& Map::addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, 
	const EmaString& statusText, MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyState( streamState, dataState, statusCode, statusText, action, value, permissionData );

	return *this;
}

Map& Map::addKeyState(OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode,
	const EmaString& statusText, MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyState(streamState, dataState, statusCode, statusText, action, permissionData);

	return *this;
}

Map& Map::addKeyEnum( UInt16 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyEnum( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyEnum(UInt16 key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyEnum(key, action, permissionData);

	return *this;
}

Map& Map::addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyBuffer( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyBuffer(const EmaBuffer& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyBuffer(key, action, permissionData);

	return *this;
}

Map& Map::addKeyAscii( const EmaString& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyAscii( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyAscii(const EmaString& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyAscii(key, action, permissionData);

	return *this;
}

Map& Map::addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyUtf8( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyUtf8(const EmaBuffer& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyUtf8(key, action, permissionData);

	return *this;
}

Map& Map::addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData	)
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyRmtes( key, action, value, permissionData );

	return *this;
}

Map& Map::addKeyRmtes(const EmaBuffer& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	if (!_pEncoder)
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->addKeyRmtes(key, action, permissionData);

	return *this;
}

const Map& Map::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._mapEncoderPool.getItem();

	_pEncoder->complete();

	return *this;
}

bool Map::hasEncoder() const
{
	return _pEncoder ? true : false;
}
