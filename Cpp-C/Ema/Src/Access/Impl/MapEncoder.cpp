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
 _rsslMap(),
 _rsslMapEntry(),
 _emaLoadType( DataType::NoDataEnum ),
 _emaKeyType( DataType::NoDataEnum ),
 _containerInitialized( false )
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

	_emaLoadType = DataType::NoDataEnum;

	_emaKeyType = DataType::NoDataEnum;

	_containerInitialized = false;
}

void MapEncoder::initEncode( RsslDataType rsslKeyDataType, UInt8 rsslContainerDataType, DataType::DataTypeEnum emaLoadType )
{
	if ( !_rsslMap.containerType )
	{
		_rsslMap.containerType = rsslContainerDataType;
	}
	else if ( _rsslMap.containerType != rsslContainerDataType )
	{
		EmaString temp( "Attempt to add an entry with a DataType different than summaryData's DataType. Passed in ComplexType has DataType of " );
		temp += DataType( emaLoadType ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	_rsslMap.keyPrimitiveType = rsslKeyDataType;

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

void MapEncoder::addDeleteActionEntry( void* keyValue, const EmaBuffer& permission, const char* methodName )
{
	rsslClearBuffer( &_rsslMapEntry.encData );

	_rsslMapEntry.action = RSSL_MPEA_DELETE_ENTRY;

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
		temp.append( methodName ).append( " while encoding MapEntry with Delete action. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void MapEncoder::addEncodedEntry( void* keyValue, MapEntry::MapAction action, 
	const ComplexType& value, const EmaBuffer& permission, const char* methodName )
{
	_rsslMapEntry.encData = value.getEncoder().getRsslBuffer();

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

void MapEncoder::addDecodedEntry( void* keyValue, MapEntry::MapAction action, 
	const ComplexType& value, const EmaBuffer& permission, const char* methodName )
{
	_rsslMapEntry.encData = const_cast<ComplexType&>( value ).getDecoder().getRsslBuffer();

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
		temp.append( methodName ).append( " while encoding Map in addDecodedEntry(). Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void MapEncoder::startEncodingEntry( void* keyValue, MapEntry::MapAction action, 
	const EmaBuffer& permission, const char* methodName )
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

		retCode = rsslEncodeMapEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
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
		const Encoder& enc = data.getEncoder();

		if ( data.hasEncoder() && enc.ownsIterator() )
		{
			if ( enc.isComplete() )
			{
				rsslMapApplyHasSummaryData( &_rsslMap );
				_rsslMap.encSummaryData = enc.getRsslBuffer();
			}
			else
			{
				EmaString temp( "Attempt to set summaryData() with a ComplexType while complete() was not called on this ComplexType." );
				throwIueException( temp );
				return;
			}
		}
		else if ( data.hasDecoder() )
		{
			rsslMapApplyHasSummaryData( &_rsslMap );
			_rsslMap.encSummaryData = const_cast<ComplexType&>(data).getDecoder().getRsslBuffer();
		}
		else
		{
			EmaString temp( "Attempt to pass an empty ComplexType to summaryData() while it is not supported." );
			throwIueException( temp );
			return;
		}

		_emaLoadType = data.getDataType();
		_rsslMap.containerType = enc.convertDataType( _emaLoadType );
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
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_INT, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::IntEnum;
	}
	else if ( _emaKeyType != DataType::IntEnum )
	{
		EmaString temp( "Attempt to addKeyInt() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &key, permissionData, "addKeyInt()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyInt()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &key, action, value, permissionData, "addKeyInt()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyInt()" );
	}
}

void MapEncoder::addKeyUInt( UInt64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_UINT, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::UIntEnum;
	}
	else if ( _emaKeyType != DataType::UIntEnum )
	{
		EmaString temp( "Attempt to addKeyUInt() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &key, permissionData, "addKeyUInt()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyUInt()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &key, action, value, permissionData, "addKeyUInt()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyUInt()" );
	}
}

void MapEncoder::addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_REAL, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::RealEnum;
	}
	else if ( _emaKeyType != DataType::RealEnum )
	{
		EmaString temp( "Attempt to addKeyReal() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslReal real;
	real.hint = magnitudeType;
	real.value = mantissa;
	real.isBlank = false;

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &real, permissionData, "addKeyReal()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &real, action, value, permissionData, "addKeyReal()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &real, action, value, permissionData, "addKeyReal()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &real, action, permissionData, "addKeyReal()" );
	}
}

void MapEncoder::addKeyRealFromDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, OmmReal::MagnitudeType magnitudeType, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_REAL, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::RealEnum;
	}
	else if ( _emaKeyType != DataType::RealEnum )
	{
		EmaString temp( "Attempt to addKeyRealFromDouble() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslReal real;
	if ( RSSL_RET_SUCCESS != rsslDoubleToReal( &real, &key, magnitudeType ) )
	{
		EmaString temp( "Attempt to addKeyRealFromDouble() with invalid magnitudeType='" );
		temp.append( getMTypeAsString( magnitudeType) ).append( "'. " );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &real, permissionData, "addKeyRealFromDouble()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &real, action, value, permissionData, "addKeyRealFromDouble()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &real, action, value, permissionData, "addKeyRealFromDouble()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &real, action, permissionData, "addKeyRealFromDouble()" );
	}
}

void MapEncoder::addKeyFloat( float key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_FLOAT, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::FloatEnum;
	}
	else if ( _emaKeyType != DataType::FloatEnum )
	{
		EmaString temp( "Attempt to addKeyFloat() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &key, permissionData, "addKeyFloat()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyFloat()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &key, action, value, permissionData, "addKeyFloat()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyFloat()" );
	}
}

void MapEncoder::addKeyDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_DOUBLE, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::DoubleEnum;
	}
	else if ( _emaKeyType != DataType::DoubleEnum )
	{
		EmaString temp( "Attempt to addKeyDouble() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &key, permissionData, "addKeyDouble()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyDouble()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &key, action, value, permissionData, "addKeyDouble()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyDouble()" );
	}
}

void MapEncoder::addKeyDate( UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

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

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_DATE, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::DateEnum;
	}
	else if ( _emaKeyType != DataType::DateEnum )
	{
		EmaString temp( "Attempt to addKeyDate() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &date, permissionData, "addKeyDate()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &date, action, value, permissionData, "addKeyDate()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &date, action, value, permissionData, "addKeyDate()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &date, action, permissionData, "addKeyDate()" );
	}
}

void MapEncoder::addKeyTime( UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond, 
	MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

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

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_TIME, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::TimeEnum;
	}
	else if ( _emaKeyType != DataType::TimeEnum )
	{
		EmaString temp( "Attempt to addKeyTime() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &time, permissionData, "addKeyTime()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &time, action, value, permissionData, "addKeyTime()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &time, action, value, permissionData, "addKeyTime()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &time, action, permissionData, "addKeyTime()" );
	}
}

void MapEncoder::addKeyDateTime( UInt16 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond,
	UInt16 microsecond, UInt16 nanosecond, MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

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

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_DATETIME, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::DateTimeEnum;
	}
	else if ( _emaKeyType != DataType::DateTimeEnum )
	{
		EmaString temp( "Attempt to addKeyDateTime() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &dateTime, permissionData, "addKeyDateTime()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &dateTime, action, value, permissionData, "addKeyDateTime()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &dateTime, action, value, permissionData, "addKeyDateTime()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &dateTime, action, permissionData, "addKeyDateTime()" );
	}
}

void MapEncoder::addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_QOS, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::QosEnum;
	}
	else if ( _emaKeyType != DataType::QosEnum )
	{
		EmaString temp( "Attempt to addKeyQos() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslQos qos;
	OmmQosDecoder::convertToRssl( &qos, timeliness, rate );

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &qos, permissionData, "addKeyQos()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &qos, action, value, permissionData, "addKeyQos()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &qos, action, value, permissionData, "addKeyQos()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &qos, action, permissionData, "addKeyQos()" );
	}
}

void MapEncoder::addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
	MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_STATE, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::StateEnum;
	}
	else if ( _emaKeyType != DataType::StateEnum )
	{
		EmaString temp( "Attempt to addKeyState() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &state, permissionData, "addKeyState()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &state, action, value, permissionData, "addKeyState()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &state, action, value, permissionData, "addKeyState()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &state, action, permissionData, "addKeyState()" );
	}
}

void MapEncoder::addKeyEnum( UInt16 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_ENUM, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::EnumEnum;
	}
	else if ( _emaKeyType != DataType::EnumEnum )
	{
		EmaString temp( "Attempt to addKeyEnum() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &key, permissionData, "addKeyEnum()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyEnum()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &key, action, value, permissionData, "addKeyEnum()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyEnum()" );
	}
}

void MapEncoder::addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_BUFFER, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::BufferEnum;
	}
	else if ( _emaKeyType != DataType::BufferEnum )
	{
		EmaString temp( "Attempt to addKeyBuffer() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &buffer, permissionData, "addKeyBuffer()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyBuffer()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &buffer, action, value, permissionData, "addKeyBuffer()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyBuffer()" );
	}
}

void MapEncoder::addKeyAscii( const EmaString& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_ASCII_STRING, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::AsciiEnum;
	}
	else if ( _emaKeyType != DataType::AsciiEnum )
	{
		EmaString temp( "Attempt to addKeyAscii() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_str();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &buffer, permissionData, "addKeyAscii()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyAscii()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &buffer, action, value, permissionData, "addKeyAscii()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyAscii()" );
	}
}

void MapEncoder::addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_UTF8_STRING, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::Utf8Enum;
	}
	else if ( _emaKeyType != DataType::Utf8Enum )
	{
		EmaString temp( "Attempt to addKeyUtf8() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &buffer, permissionData, "addKeyUtf8()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyUtf8()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &buffer, action, value, permissionData, "addKeyUtf8()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyUtf8()" );
	}
}

void MapEncoder::addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp );
		return;
	}

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );
	
	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode( RSSL_DT_RMTES_STRING, rsslDataType, value.getDataType() );

		_emaKeyType = DataType::RmtesEnum;
	}
	else if ( _emaKeyType != DataType::RmtesEnum )
	{
		EmaString temp( "Attempt to addKeyRmtes() while established key data type is " );
		temp += DataType( _emaKeyType ).toString();
		throwIueException( temp );
		return;
	}
	else if ( _rsslMap.containerType != rsslDataType )
	{
		EmaString temp( "Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of " );
		temp += DataType( value.getDataType() ).toString();
		temp += EmaString( " while the expected DataType is " );
		temp += DataType( _emaLoadType );
		throwIueException( temp );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addDeleteActionEntry( &buffer, permissionData, "addKeyRmtes()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyRmtes()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp );
			return;
		}
	}
	else if ( value.hasDecoder() )
	{
		addDecodedEntry( &buffer, action, value, permissionData, "addKeyRmtes()" );
	}
	else
	{
		if ( rsslDataType == RSSL_DT_MSG )
		{
			EmaString temp( "Attempt to pass in an empty message while it is not supported." );
			throwIueException( temp );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyRmtes()" );
	}
}

void MapEncoder::complete()
{
	if ( _containerComplete ) return;

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
