/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "MapEncoder.h"
#include "ExceptionTranslator.h"
#include "OmmStateDecoder.h"
#include "OmmQosDecoder.h"
#include "OmmRealDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getMTypeAsString( OmmReal::MagnitudeType mType );

MapEncoder::MapEncoder() :
 _rsslMap(),
 _rsslMapEntry(),
 _emaLoadType( DataType::NoDataEnum ),
 _emaKeyType( DataType::BufferEnum ),
 _containerInitialized( false ),
 _keyTypeSet( false )
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

	_emaKeyType = DataType::BufferEnum;

	_containerInitialized = false;

	_keyTypeSet = false;
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
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	if ( _keyTypeSet && (_emaKeyType != rsslKeyDataType ) )
	{
		EmaString temp("Attempt to add an entry key type of ");
		temp += DataType((DataType::DataTypeEnum)rsslKeyDataType).toString();
		temp += EmaString(" while Map entry key is set to ");
		temp += DataType(_emaKeyType).toString();
		temp += EmaString(" with the keyType() method");
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
		throwIueException( temp, retCode );
	}

	_containerInitialized = true;
}

void MapEncoder::addEntryWithNoPayload( void* keyValue, MapEntry::MapAction action, const EmaBuffer& permission, const char* methodName )
{
	rsslClearMapEntry(&_rsslMapEntry);

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
		temp.append( methodName ).append( " while encoding MapEntry with Delete action. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void MapEncoder::addEncodedEntry( void* keyValue, MapEntry::MapAction action, 
	const ComplexType& value, const EmaBuffer& permission, const char* methodName )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_MAP ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to add MapEntry while complete() was not called for passed in container:  ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	rsslClearMapEntry(&_rsslMapEntry);

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
		throwIueException( temp, retCode );
	}
}

void MapEncoder::addDecodedEntry( void* keyValue, MapEntry::MapAction action, 
	const ComplexType& value, const EmaBuffer& permission, const char* methodName )
{
	rsslClearMapEntry(&_rsslMapEntry);

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
		throwIueException( temp, retCode );
	}
}

void MapEncoder::startEncodingEntry( void* keyValue, MapEntry::MapAction action, 
	const EmaBuffer& permission, const char* methodName )
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_MAP ||
		_levelInfo->_encodingState != RSSL_EIS_ENTRIES)
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to add MapEntry while complete() was not called for passed in container:  ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

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
		throwIueException( temp, retCode );
	}
}

void MapEncoder::endEncodingEntry() const
{
	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_MAP ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRY_INIT &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete Map while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeMapEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	/* Reallocate does not need here. The data is placed in already allocated memory */

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in Map. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void MapEncoder::validateEntryKeyAndPayLoad(RsslDataType rsslKeyDataType, UInt8 rsslLoadDataType, DataType::DataTypeEnum emaLoadType,
	const char* methodName)
{
	if (_containerComplete)
	{
		EmaString temp("Attempt to add an entry after complete() was called.");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if (!hasEncIterator())
	{
		acquireEncIterator();

		initEncode(rsslKeyDataType, rsslLoadDataType, emaLoadType);

		_emaKeyType = (DataType::DataTypeEnum)rsslKeyDataType;
		_emaLoadType = emaLoadType;
	}
	else if (_emaKeyType != rsslKeyDataType)
	{
		EmaString temp("Attempt to ");
		temp += methodName;
		temp += " while established key data type is ";
		temp += DataType(_emaKeyType).toString();
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}
	else if (_rsslMap.containerType != rsslLoadDataType)
	{
		EmaString temp("Attempt to add an entry with a different DataType. Passed in ComplexType has DataType of ");
		temp += DataType(emaLoadType).toString();
		temp += EmaString(" while the expected DataType is ");
		temp += DataType(_emaLoadType);
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
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
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
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
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
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
				throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		_emaLoadType = data.getDataType();
		_rsslMap.containerType = enc.convertDataType( _emaLoadType );
	}
	else
	{
		EmaString temp( "Invalid attempt to call summaryData() when container is not empty." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void MapEncoder::keyType( DataType::DataTypeEnum keyPrimitiveType )
{
	if (_containerComplete)
	{
		EmaString temp("Attempt to call keyType() after complete() was called.");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if (keyPrimitiveType > DataType::RmtesEnum || keyPrimitiveType == DataType::ArrayEnum)
	{
		EmaString temp("The specified key type '");
		temp.append(DataType(keyPrimitiveType)).append("' is not a primitive type");
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	_emaKeyType = keyPrimitiveType;
	_keyTypeSet = true;
}

void MapEncoder::addKeyInt( Int64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_INT, rsslDataType, value.getDataType(), "addKeyInt()");

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &key, action, permissionData, "addKeyInt()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyInt()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyInt()" );
	}
}

void MapEncoder::addKeyInt(Int64 key, MapEntry::MapAction action,
	 const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_INT, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyInt()");

	addEntryWithNoPayload(&key, action, permissionData, "addKeyInt()");
}

void MapEncoder::addKeyUInt( UInt64 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_UINT, rsslDataType, value.getDataType(), "addKeyUInt()");

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &key, action, permissionData, "addKeyUInt()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyUInt()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyUInt()" );
	}
}
void MapEncoder::addKeyUInt(UInt64 key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_UINT, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyUInt()");

	addEntryWithNoPayload(&key, action, permissionData, "addKeyUInt()");
}

void MapEncoder::addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_REAL, rsslDataType, value.getDataType(), "addKeyReal()");
	
	RsslReal real;
	real.hint = magnitudeType;
	real.value = mantissa;
	real.isBlank = false;

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &real, action, permissionData, "addKeyReal()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &real, action, value, permissionData, "addKeyReal()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &real, action, permissionData, "addKeyReal()" );
	}
}

void MapEncoder::addKeyReal(Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_REAL, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyReal()");

	RsslReal real;
	real.hint = magnitudeType;
	real.value = mantissa;
	real.isBlank = false;

	addEntryWithNoPayload(&real, action, permissionData, "addKeyReal()");
}

void MapEncoder::addKeyRealFromDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, OmmReal::MagnitudeType magnitudeType, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_REAL, rsslDataType, value.getDataType(), "addKeyRealFromDouble()");

	RsslReal real;
	if ( RSSL_RET_SUCCESS != rsslDoubleToReal( &real, &key, magnitudeType ) )
	{
		EmaString temp( "Attempt to addKeyRealFromDouble() with invalid magnitudeType='" );
		temp.append( getMTypeAsString( magnitudeType) ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &real, action, permissionData, "addKeyRealFromDouble()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &real, action, value, permissionData, "addKeyRealFromDouble()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &real, action, permissionData, "addKeyRealFromDouble()" );
	}
}

void MapEncoder::addKeyRealFromDouble(double key, MapEntry::MapAction action,
	OmmReal::MagnitudeType magnitudeType, const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_REAL, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyRealFromDouble()");

	RsslReal real;
	if (RSSL_RET_SUCCESS != rsslDoubleToReal(&real, &key, magnitudeType))
	{
		EmaString temp("Attempt to addKeyRealFromDouble() with invalid magnitudeType='");
		temp.append(getMTypeAsString(magnitudeType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	addEntryWithNoPayload(&real, action, permissionData, "addKeyRealFromDouble()");
}

void MapEncoder::addKeyFloat( float key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_FLOAT, rsslDataType, value.getDataType(), "addKeyFloat()");
	
	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &key, action, permissionData, "addKeyFloat()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyFloat()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyFloat()" );
	}
}

void MapEncoder::addKeyFloat(float key, MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_FLOAT, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyFloat()");

	addEntryWithNoPayload(&key, action, permissionData, "addKeyFloat()");
}

void MapEncoder::addKeyDouble( double key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_DOUBLE, rsslDataType, value.getDataType(), "addKeyDouble()");
	
	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload(&key, action, permissionData, "addKeyDouble()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyDouble()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyDouble()" );
	}
}

void MapEncoder::addKeyDouble(double key, MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_DOUBLE, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyDouble()");

	addEntryWithNoPayload(&key, action, permissionData, "addKeyDouble()");
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

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_DATE, rsslDataType, value.getDataType(), "addKeyDate()");
	
	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &date, action, permissionData, "addKeyDate()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &date, action, value, permissionData, "addKeyDate()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &date, action, permissionData, "addKeyDate()" );
	}
}

void MapEncoder::addKeyDate(UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	RsslDate date;
	date.year = year;
	date.month = month;
	date.day = day;

	if (RSSL_FALSE == rsslDateIsValid(&date))
	{
		EmaString temp("Attempt to specify invalid date. Passed in value is='");
		temp.append((UInt32)month).append(" / ").
			append((UInt32)day).append(" / ").
			append((UInt32)year).append("'.");
		throwOorException(temp);
		return;
	}

	validateEntryKeyAndPayLoad(RSSL_DT_DATE, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyDate()");

	addEntryWithNoPayload(&date, action, permissionData, "addKeyDate()");
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

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_TIME, rsslDataType, value.getDataType(), "addKeyTime()");

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &time, action, permissionData, "addKeyTime()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &time, action, value, permissionData, "addKeyTime()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &time, action, permissionData, "addKeyTime()" );
	}
}

void MapEncoder::addKeyTime(UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
	MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	RsslTime time;
	time.hour = hour;
	time.minute = minute;
	time.second = second;
	time.millisecond = millisecond;
	time.microsecond = microsecond;
	time.nanosecond = nanosecond;

	if (RSSL_FALSE == rsslTimeIsValid(&time))
	{
		EmaString temp("Attempt to specify invalid time. Passed in value is='");
		temp.append((UInt32)hour).append(":").
			append((UInt32)minute).append(":").
			append((UInt32)second).append(".").
			append((UInt32)millisecond).append(".").
			append((UInt32)microsecond).append(".").
			append((UInt32)nanosecond).append("'.");
		throwOorException(temp);
		return;
	}

	validateEntryKeyAndPayLoad(RSSL_DT_TIME, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyTime()");

	addEntryWithNoPayload(&time, action, permissionData, "addKeyTime()");
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

	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_DATETIME, rsslDataType, value.getDataType(), "addKeyDateTime()");

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &dateTime, action, permissionData, "addKeyDateTime()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &dateTime, action, value, permissionData, "addKeyDateTime()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &dateTime, action, permissionData, "addKeyDateTime()" );
	}
}

void MapEncoder::addKeyDateTime(UInt16 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond,
	UInt16 microsecond, UInt16 nanosecond, MapEntry::MapAction action, const EmaBuffer& permissionData)
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

	if (RSSL_FALSE == rsslDateTimeIsValid(&dateTime))
	{
		EmaString temp("Attempt to specify invalid date time. Passed in value is='");
		temp.append((UInt32)month).append(" / ").
			append((UInt32)day).append(" / ").
			append((UInt32)year).append("  ").
			append((UInt32)hour).append(":").
			append((UInt32)minute).append(":").
			append((UInt32)second).append(".").
			append((UInt32)millisecond).append(".").
			append((UInt32)microsecond).append(".").
			append((UInt32)nanosecond).append("'.");
		throwOorException(temp);
		return;
	}

	validateEntryKeyAndPayLoad(RSSL_DT_DATETIME, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyDateTime()");

	addEntryWithNoPayload(&dateTime, action, permissionData, "addKeyDateTime()");
}

void MapEncoder::addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_QOS, rsslDataType, value.getDataType(), "addKeyQos()");

	RsslQos qos;
	OmmQosDecoder::convertToRssl( &qos, timeliness, rate );

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &qos, action, permissionData, "addKeyQos()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &qos, action, value, permissionData, "addKeyQos()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &qos, action, permissionData, "addKeyQos()" );
	}
}

void MapEncoder::addKeyQos(UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_QOS, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyQos()");

	RsslQos qos;
	OmmQosDecoder::convertToRssl(&qos, timeliness, rate);

	addEntryWithNoPayload(&qos, action, permissionData, "addKeyQos()");
}

void MapEncoder::addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
	MapEntry::MapAction action, const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_STATE, rsslDataType, value.getDataType(), "addKeyState()");

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &state, action, permissionData, "addKeyState()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &state, action, value, permissionData, "addKeyState()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &state, action, permissionData, "addKeyState()" );
	}
}

void MapEncoder::addKeyState(OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
	MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_STATE, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyState()");

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	addEntryWithNoPayload(&state, action, permissionData, "addKeyState()");
}

void MapEncoder::addKeyEnum( UInt16 key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_ENUM, rsslDataType, value.getDataType(), "addKeyEnum()");

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &key, action, permissionData, "addKeyEnum()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &key, action, value, permissionData, "addKeyEnum()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &key, action, permissionData, "addKeyEnum()" );
	}
}

void MapEncoder::addKeyEnum(UInt16 key, MapEntry::MapAction action, const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_ENUM, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyEnum()");

	addEntryWithNoPayload(&key, action, permissionData, "addKeyEnum()");
}

void MapEncoder::addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_BUFFER, rsslDataType, value.getDataType(), "addKeyBuffer()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &buffer, action, permissionData, "addKeyBuffer()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyBuffer()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyBuffer()" );
	}
}

void MapEncoder::addKeyBuffer(const EmaBuffer& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_BUFFER, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyBuffer()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	addEntryWithNoPayload(&buffer, action, permissionData, "addKeyBuffer()");
}

void MapEncoder::addKeyAscii( const EmaString& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_ASCII_STRING, rsslDataType, value.getDataType(), "addKeyAscii()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_str();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &buffer, action, permissionData, "addKeyAscii()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyAscii()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyAscii()" );
	}
}

void MapEncoder::addKeyAscii(const EmaString& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_ASCII_STRING, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyAscii()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_str();
	buffer.length = key.length();

	addEntryWithNoPayload(&buffer, action, permissionData, "addKeyAscii()");
}

void MapEncoder::addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_UTF8_STRING, rsslDataType, value.getDataType(), "addKeyUtf8()");
	
	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &buffer, action, permissionData, "addKeyUtf8()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyUtf8()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyUtf8()" );
	}
}

void MapEncoder::addKeyUtf8(const EmaBuffer& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_UTF8_STRING, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyUtf8()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	addEntryWithNoPayload(&buffer, action, permissionData, "addKeyUtf8()");
}

void MapEncoder::addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
	const ComplexType& value, const EmaBuffer& permissionData )
{
	const Encoder& enc = value.getEncoder();

	UInt8 rsslDataType = enc.convertDataType( value.getDataType() );

	validateEntryKeyAndPayLoad(RSSL_DT_RMTES_STRING, rsslDataType, value.getDataType(), "addKeyRmtes()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	if ( action == MapEntry::DeleteEnum )
	{
		addEntryWithNoPayload( &buffer, action, permissionData, "addKeyRmtes()" );
	}
	else if ( value.hasEncoder() && enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( &buffer, action, value, permissionData, "addKeyRmtes()" );
		else
		{
			EmaString temp( "Attempt to add a ComplexType while complete() was not called on this ComplexType." );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( &buffer, action, permissionData, "addKeyRmtes()" );
	}
}

void MapEncoder::addKeyRmtes(const EmaBuffer& key, MapEntry::MapAction action,
	const EmaBuffer& permissionData)
{
	validateEntryKeyAndPayLoad(RSSL_DT_RMTES_STRING, RSSL_DT_NO_DATA, DataType::NoDataEnum, "addKeyRmtes()");

	RsslBuffer buffer;
	buffer.data = (char*)key.c_buf();
	buffer.length = key.length();

	addEntryWithNoPayload(&buffer, action, permissionData, "addKeyRmtes()");
}

void MapEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( !hasEncIterator() )
	{
		acquireEncIterator();

		initEncode(_emaKeyType, convertDataType(_emaLoadType), _emaLoadType);
	}

	RsslEncodingLevel *_levelInfo = &(_pEncodeIter->_rsslEncIter._levelInfo[_pEncodeIter->_rsslEncIter._encodingLevel]);

	if (_levelInfo->_containerType != RSSL_DT_MAP ||
		(_levelInfo->_encodingState != RSSL_EIS_ENTRIES &&
		 _levelInfo->_encodingState != RSSL_EIS_SET_DEFINITIONS &&
		 _levelInfo->_encodingState != RSSL_EIS_WAIT_COMPLETE))
	{
		/*If an internal container is not completed. Internal container empty.*/
		EmaString temp("Attemp to complete Map while complete() was not called for passed in container: ");
		temp.append(DataType(_emaLoadType));
		throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslRet retCode = rsslEncodeMapComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete Map encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
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
