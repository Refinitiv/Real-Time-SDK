/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_MapEncoder_h
#define __rtsdk_ema_access_MapEncoder_h

#include "Encoder.h"
#include "ComplexType.h"
#include "OmmReal.h"
#include "EmaPool.h"
#include "OmmState.h"
#include "MapEntry.h"
#include "rtr/rsslMap.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmArray;
class ElementList;
class FieldList;
class FilterList;
class Map;
class Series;
class Vector;
class AckMsg;
class GenericMsg;
class PostMsg;
class ReqMsg;
class RespMsg;
class OmmAnsiPage;
class OmmOpaque;
class OmmXml;

class MapEncoder : public Encoder
{
public :

	MapEncoder();

	virtual ~MapEncoder();

	void clear();

	void keyFieldId( Int16 fieldId );

	void totalCountHint( UInt32 totalCountHint );

	void summaryData( const ComplexType& data );

	void keyType( DataType::DataTypeEnum keyPrimitiveType );

	void addKeyInt( Int64 key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyInt(Int64 key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyUInt( UInt64 key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyUInt(UInt64 key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyReal(Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyRealFromDouble( double key, MapEntry::MapAction action,
		const ComplexType& value,
		OmmReal::MagnitudeType magnitudeType,
		const EmaBuffer& permissionData );

	void addKeyRealFromDouble(double key, MapEntry::MapAction action,
		OmmReal::MagnitudeType magnitudeType,
		const EmaBuffer& permissionData);

	void addKeyFloat( float key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyFloat(float key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyDouble( double key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyDouble(double key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyDate( UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyDate(UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyTime( UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyTime(UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyDateTime( UInt16 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
		MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyDateTime(UInt16 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
		MapEntry::MapAction action, const EmaBuffer& permissionData);

	void addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyQos(UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
		MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyState(OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
		MapEntry::MapAction action, const EmaBuffer& permissionData);

	void addKeyEnum( UInt16 key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyEnum(UInt16 key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyBuffer(const EmaBuffer& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyAscii( const EmaString& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyAscii(const EmaString& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyUtf8(const EmaBuffer& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData );

	void addKeyRmtes(const EmaBuffer& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData);

	void complete();

private :

	void initEncode( RsslDataType rsslKeyDataType, UInt8 rsslContainerDataType, DataType::DataTypeEnum emaLoadType );

	void addEntryWithNoPayload( void* keyValue, MapEntry::MapAction action, const EmaBuffer& permission, const char* methodName );

	void addEncodedEntry( void* keyValue, MapEntry::MapAction action, 
							const ComplexType& value, const EmaBuffer& permission, const char* methodName );

	void addDecodedEntry( void* keyValue, MapEntry::MapAction action, 
							const ComplexType& value, const EmaBuffer& permission, const char* methodName );

	void startEncodingEntry( void* keyValue, MapEntry::MapAction action, 
							const EmaBuffer& permission, const char* methodName );

	void endEncodingEntry() const;

	void encodePermissionData( const EmaBuffer& permission );

	void validateEntryKeyAndPayLoad(RsslDataType rsslKeyDataType, UInt8 rsslLoadDataType, DataType::DataTypeEnum emaLoadType,
		const char* methodName);

	RsslMap					_rsslMap;

	RsslMapEntry			_rsslMapEntry;

	DataType::DataTypeEnum	_emaLoadType;

	DataType::DataTypeEnum	_emaKeyType;

	bool					_keyTypeSet;

	bool					_containerInitialized;
};


class MapEncoderPool : public EncoderPool< MapEncoder >
{
public :

	MapEncoderPool( unsigned int size = 5 ) : EncoderPool< MapEncoder >( size ) {};

	virtual ~MapEncoderPool() {}

private :

	MapEncoderPool( const MapEncoderPool& );
	MapEncoderPool& operator=( const MapEncoderPool& );
};

}

}

}

#endif // __rtsdk_ema_access_MapEncoder_h
