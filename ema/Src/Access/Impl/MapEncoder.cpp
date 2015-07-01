/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "MapEncoder.h"
#include "ExceptionTranslator.h"
#include "OmmStateDecoder.h"
#include "OmmQosDecoder.h"
#include "OmmRealDecoder.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getMTypeAsString( OmmReal::MagnitudeType mType );

MapEncoder::MapEncoder() :
 _containerInitialized( false ),
 _rsslMap(),
 _rsslMapEntry()
{
}

MapEncoder::~MapEncoder()
{
}

void MapEncoder::clear()
{
	Encoder::releaseEncIterator();

	rsslClearMap( &_rsslMap );
	rsslClearMapEntry( &_rsslMapEntry );

	_containerInitialized = false;
}

void MapEncoder::initEncode( RsslDataTypes rsslKeyDataType, UInt8 rsslContainerDataType )
{
	_rsslMap.keyPrimitiveType = rsslKeyDataType;
	_rsslMap.containerType = rsslContainerDataType;

	RsslRet retCode = rsslEncodeMapInit( &(_pEncodeIter->_rsslEncIter), &_rsslMap, 0, 0 );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeMapComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeMapInit( &(_pEncodeIter->_rsslEncIter), &_rsslMap, 0, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize Map encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}

	_containerInitialized = true;
}

void MapEncoder::addEncodedEntry(void* keyValue, MapEntry::MapAction action, 
	UInt8 rsslDataType, const ComplexType& value, const EmaBuffer& permission, const char* methodName )
{
	if ( !static_cast<const ComplexType&>(value).getEncoder().isComplete() )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding Map. Pass not completed container." );
		throwIueException( temp );
	}

	_rsslMapEntry.encData = static_cast<const ComplexType&>(value).getEncoder().getRsslBuffer();

	_rsslMapEntry.action = action;

	encodePermissionData( permission );

	RsslRet retCode = rsslEncodeMapEntry( &_pEncodeIter->_rsslEncIter, &_rsslMapEntry, keyValue );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeMapEntry( &_pEncodeIter->_rsslEncIter, &_rsslMapEntry, keyValue );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding Map. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void MapEncoder::startEncodingEntry( void* keyValue, MapEntry::MapAction action, 
	UInt8 rsslDataType, const ComplexType& value, const EmaBuffer& permission, const char* methodName )
{
	_rsslMapEntry.encData.data = 0;
	_rsslMapEntry.encData.length = 0;

	_rsslMapEntry.action = action;

	encodePermissionData( permission );
	
	RsslRet retCode = rsslEncodeMapEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslMapEntry, keyValue, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeMapEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslMapEntry, keyValue, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to start encoding entry in Map::" );
		temp.append( methodName ).append( ". Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}

}

void MapEncoder::endEncodingEntry() const
{
	RsslRet retCode = rsslEncodeMapEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in Map. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void MapEncoder::keyFieldId( Int16 fieldId )
{
	if ( !_containerInitialized )
	{
		rsslMapApplyHasKeyFieldId ( &_rsslMap );
		_rsslMap.keyFieldId = fieldId;
	}
	else
	{
		EmaString temp( "Invalid attempt to call keyFieldId() when container is not empty." );
		throwIueException( temp );
	}
}

void MapEncoder::totalCountHint( UInt32 totalCountHint )
{
	if ( !_containerInitialized )
	{
		rsslMapApplyHasTotalCountHint( &_rsslMap );
		_rsslMap.totalCountHint = totalCountHint;
	}
	else
	{
		EmaString temp( "Invalid attempt to call totalCountHint() when container is not empty." );
		throwIueException( temp );
	}
}

void MapEncoder::summaryData( const ComplexType& data )
{
	if ( !_containerInitialized )
	{
		if ( static_cast<const ComplexType&>(data).getEncoder().isComplete() )
		{
			rsslMapApplyHasSummaryData( &_rsslMap );
			_rsslMap.encSummaryData = static_cast<const ComplexType&>(data).getEncoder().getRsslBuffer();
		}
		else
		{
			EmaString temp( "Invalid attempt to pass not completed container to summaryData()." );
			throwIueException( temp );
		}
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is not empty." );
		throwIueException( temp );
	}
}

void MapEncoder::addKeyInt( Int64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_INT, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &key, action, dataType, value, permissionData, "addKeyInt()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &key, action, dataType, value, permissionData, "addKeyInt()" );
	}
}

void MapEncoder::addKeyUInt( UInt64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_UINT, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &key, action, dataType, value, permissionData, "addKeyUInt()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &key, action, dataType, value, permissionData, "addKeyUInt()" );
	}
}

void MapEncoder::addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_REAL, dataType );
	}

	RsslReal real;
	real.hint = magnitudeType;
	real.value = mantissa;
	real.isBlank = false;

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &real, action, dataType, value, permissionData, "addKeyReal()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &real, action, dataType, value, permissionData, "addKeyReal()" );
	}
}

void MapEncoder::addKeyRealFromDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, OmmReal::MagnitudeType magnitudeType, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_REAL, dataType );
	}

	RsslReal real;
	if ( RSSL_RET_SUCCESS != rsslDoubleToReal( &real, &key, magnitudeType ) )
	{
		EmaString temp( "Attempt to addKeyRealFromDouble() with invalid magnitudeType='" );
		temp.append( getMTypeAsString( magnitudeType) ).append( "'. " );
		throwIueException( temp );
		return;
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &real, action, dataType, value, permissionData, "addKeyRealFromDouble()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &real, action, dataType, value, permissionData, "addKeyRealFromDouble()" );
	}
}

void MapEncoder::addKeyFloat( float key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_FLOAT, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &key, action, dataType, value, permissionData, "addKeyDouble()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &key, action, dataType, value, permissionData, "addKeyDouble()" );
	}
}

void MapEncoder::addKeyDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_DOUBLE, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &key, action, dataType, value, permissionData, "addKeyDouble()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &key, action, dataType, value, permissionData, "addKeyDouble()" );
	}
}

void MapEncoder::addKeyDate( UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	RsslDate date;
	date.year = year;
	date.month = month;
	date.day = day;

	if ( RSSL_FALSE == rsslDateIsValid( &date ) )
	{
		EmaString temp( "Attempt to specify invalid date. Passed in value is='" );
		temp.append( (UInt32)month ).append( " / " ).
			append( (UInt32)day ).append( " / " ).
			append( (UInt32)year ).append( "'." );
		throwOorException( temp );
		return;
	}

	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_DATE, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &date, action, dataType, value, permissionData, "addKeyDate()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &date, action, dataType, value, permissionData, "addKeyDate()" );
	}
}

void MapEncoder::addKeyTime( UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond, 
	MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	RsslTime time;
	time.hour = hour;
	time.minute = minute;
	time.second = second;
	time.millisecond = millisecond;
	time.microsecond = microsecond;
	time.nanosecond = nanosecond;

	if ( RSSL_FALSE == rsslTimeIsValid( &time ) )
	{
		EmaString temp( "Attempt to specify invalid time. Passed in value is='" );
		temp.append( (UInt32)hour ).append( ":" ).
			append( (UInt32)minute ).append( ":" ).
			append( (UInt32)second ).append( "." ).
			append( (UInt32)millisecond ).append( "." ).
			append( (UInt32)microsecond ).append( "." ).
			append( (UInt32)nanosecond ).append( "'." );
		throwOorException( temp );
		return;
	}

	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_TIME, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &time, action, dataType, value, permissionData, "addKeyTime()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &time, action, dataType, value, permissionData, "addKeyTime()" );
	}
}

void MapEncoder::addKeyDateTime( UInt16 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond,
	UInt16 microsecond, UInt16 nanosecond, MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	RsslDateTime dateTime;
	dateTime.date.year = year;
	dateTime.date.month = month;
	dateTime.date.day = day;
	dateTime.time.hour = hour;
	dateTime.time.minute = minute;
	dateTime.time.second = second;
	dateTime.time.millisecond = millisecond;
	dateTime.time.microsecond = microsecond;
	dateTime.time.nanosecond = nanosecond;

	if ( RSSL_FALSE == rsslDateTimeIsValid( &dateTime ) )
	{
		EmaString temp( "Attempt to specify invalid date time. Passed in value is='" );
		temp.append( (UInt32)month ).append( " / " ).
			append( (UInt32)day ).append( " / " ).
			append( (UInt32)year ).append( "  " ).
			append( (UInt32)hour ).append( ":" ).
			append( (UInt32)minute ).append( ":" ).
			append( (UInt32)second ).append( "." ).
			append( (UInt32)millisecond ).append( "." ).
			append( (UInt32)microsecond ).append( "." ).
			append( (UInt32)nanosecond ).append( "'." );
		throwOorException( temp );
		return;
	}

	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_DATETIME, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &dateTime, action, dataType, value, permissionData, "addKeyDateTime()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &dateTime, action, dataType, value, permissionData, "addKeyDateTime()" );
	}
}

void MapEncoder::addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_QOS, dataType );
	}

	RsslQos qos;
	OmmQosDecoder::convertToRssl( &qos, timeliness, rate );

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &qos, action, dataType, value, permissionData, "addKeyQos()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &qos, action, dataType, value, permissionData, "addKeyQos()" );
	}
}

void MapEncoder::addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
	MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_STATE, dataType );
	}

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &state, action, dataType, value, permissionData, "addKeyState()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &state, action, dataType, value, permissionData, "addKeyState()" );
	}
}

void MapEncoder::addKeyEnum( UInt16 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_ENUM, dataType );
	}

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &key, action, dataType, value, permissionData, "addKeyEnum()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &key, action, dataType, value, permissionData, "addKeyEnum()" );
	}
}

void MapEncoder::addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_BUFFER, dataType );
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &buffer, action, dataType, value, permissionData, "addKeyBuffer()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &buffer, action, dataType, value, permissionData, "addKeyBuffer()" );
	}
}

void MapEncoder::addKeyAscii( const EmaString& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_ASCII_STRING, dataType );
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_str();
	buffer.length = key.length();

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &buffer, action, dataType, value, permissionData, "addKeyAscii()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &buffer, action, dataType, value, permissionData, "addKeyAscii()" );
	}
}

void MapEncoder::addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_UTF8_STRING, dataType );
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &buffer, action, dataType, value, permissionData, "addKeyUtf8()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &buffer, action, dataType, value, permissionData, "addKeyUtf8()" );
	}
}

void MapEncoder::addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	UInt8 dataType = const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ).convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_RMTES_STRING, dataType );
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( static_cast<const ComplexType&>(value).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( &buffer, action, dataType, value, permissionData, "addKeyRmtes()" );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const ComplexType&>(value).getEncoder() ) );
		startEncodingEntry( &buffer, action, dataType, value, permissionData, "addKeyRmtes()" );
	}
}


void MapEncoder::complete()
{
	if ( !hasEncIterator() )
	{
		EmaString temp( "Cannot complete an empty Map" );
		throwIueException( temp );
		return;
	}

	RsslRet retCode = rsslEncodeMapComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete Map encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();

	_containerComplete = true;
}

void MapEncoder::encodePermissionData( const EmaBuffer& permission )
{
	if ( permission.length() > 0 )
	{
		rsslMapEntryApplyHasPermData ( &_rsslMapEntry );
		_rsslMapEntry.permData.length = permission.length();
		_rsslMapEntry.permData.data = const_cast<char *>(permission.c_buf());
	}
}
