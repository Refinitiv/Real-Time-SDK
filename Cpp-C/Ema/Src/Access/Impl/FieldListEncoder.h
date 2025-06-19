/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2018-2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_FieldListEncoder_h
#define __refinitiv_ema_access_FieldListEncoder_h

#include "Encoder.h"
#include "OmmReal.h"
#include "EmaPool.h"
#include "OmmState.h"
#include "rtr/rsslFieldList.h"

namespace refinitiv {

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
class RefreshMsg;
class StatusMsg;
class UpdateMsg;
class OmmAnsiPage;
class OmmOpaque;
class OmmXml;
class OmmJson;

class FieldListEncoder : public Encoder
{
public :

	FieldListEncoder();

	virtual ~FieldListEncoder();

	void clear();

	void release();

	void info( Int16 dictionaryId, Int16 fieldListNum );

	void addReqMsg( Int16 fieldId, const ReqMsg& value );

	void addRefreshMsg( Int16 fieldId, const RefreshMsg& value );

	void addUpdateMsg( Int16 fieldId, const UpdateMsg& value );

	void addStatusMsg( Int16 fieldId, const StatusMsg& value );

	void addPostMsg( Int16 fieldId, const PostMsg& value );

	void addAckMsg( Int16 fieldId, const AckMsg& value );

	void addGenericMsg( Int16 fieldId, const GenericMsg& value );

	void addFieldList( Int16 fieldId, const FieldList& value );

	void addElementList( Int16 fieldId, const ElementList& value );

	void addMap( Int16 fieldId, const Map& value );

	void addVector( Int16 fieldId, const Vector& value );

	void addSeries( Int16 fieldId, const Series& value );

	void addFilterList( Int16 fieldId, const FilterList& value );

	void addOpaque( Int16 fieldId, const OmmOpaque& value );

	void addXml( Int16 fieldId, const OmmXml& value );

	void addJson( Int16 fieldId, const OmmJson& value );

	void addAnsiPage( Int16 fieldId, const OmmAnsiPage& value );

	void addInt( Int16 fieldId, Int64 value );

	void addUInt( Int16 fieldId, UInt64 value );

	void addReal( Int16 fieldId, Int64 mantissa, OmmReal::MagnitudeType magnitudeType );

	void addRealFromDouble( Int16 fieldId, double value,
							OmmReal::MagnitudeType magnitudeType );

	void addFloat( Int16 fieldId, float value );

	void addDouble( Int16 fieldId, double value );

	void addDate( Int16 fieldId, UInt16 year, UInt8 month, UInt8 day );

	void addTime( Int16 fieldId, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond );

	void addDateTime( Int16 fieldId, UInt16 year, UInt8 month, UInt8 day,
					UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond );

	void addQos( Int16 fieldId, UInt32 timeliness, UInt32 rate );

	void addState( Int16 fieldId, OmmState::StreamState streamState,
						OmmState::DataState dataState,
						UInt8 statusCode, const EmaString& statusText );

	void addEnum( Int16 fieldId, UInt16 value );

	void addBuffer( Int16 fieldId, const EmaBuffer& value );

	void addAscii( Int16 fieldId, const EmaString& value );

	void addUtf8( Int16 fieldId, const EmaBuffer& value );

	void addRmtes( Int16 fieldId, const EmaBuffer& value );

	void addArray( Int16 fieldId, const OmmArray& value );

	void addCodeInt( Int16 fieldId );

	void addCodeUInt( Int16 fieldId );

	void addCodeReal( Int16 fieldId );

	void addCodeFloat( Int16 fieldId );

	void addCodeDouble( Int16 fieldId );

	void addCodeDate( Int16 fieldId );

	void addCodeTime( Int16 fieldId );

	void addCodeDateTime( Int16 fieldId );

	void addCodeQos( Int16 fieldId );

	void addCodeState( Int16 fieldId );

	void addCodeEnum( Int16 fieldId );

	void addCodeBuffer( Int16 fieldId );

	void addCodeAscii( Int16 fieldId );

	void addCodeUtf8( Int16 fieldId );

	void addCodeRmtes( Int16 fieldId );

	void complete();

private :

	void initEncode();

	void addPrimitiveEntry( Int16 , RsslDataType , const char* , void* );

	void addEncodedEntry( Int16 , RsslDataType , const char* , const RsslBuffer& );

	void startEncodingEntry( Int16 , RsslDataType , const char* );

	void endEncodingEntry() const;

	RsslFieldList			_rsslFieldList;

	RsslFieldEntry			_rsslFieldEntry;

	DataType::DataTypeEnum	_emaLoadType;

	bool					_containerInitialized;
};


class FieldListEncoderPool : public EncoderPool< FieldListEncoder >
{
public :

	FieldListEncoderPool( unsigned int size = 5 ) : EncoderPool< FieldListEncoder >( size ) {};

	virtual ~FieldListEncoderPool() {}

private :

	FieldListEncoderPool( const FieldListEncoderPool& );
	FieldListEncoderPool& operator=( const FieldListEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_FieldListEncoder_h
