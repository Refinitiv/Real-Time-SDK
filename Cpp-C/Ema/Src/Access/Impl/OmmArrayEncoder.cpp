/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmArrayEncoder.h"
#include "ExceptionTranslator.h"
#include "EmaBuffer.h"
#include "OmmStateDecoder.h"
#include "OmmQosDecoder.h"
#include "OmmRealDecoder.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );
extern const EmaString& getMTypeAsString( OmmReal::MagnitudeType mType );

OmmArrayEncoder::OmmArrayEncoder() :
 _rsslArray()
{
}

OmmArrayEncoder::~OmmArrayEncoder()
{
}

void OmmArrayEncoder::clear()
{
	Encoder::clearEncIterator();

	rsslClearArray( &_rsslArray );
}

void OmmArrayEncoder::release()
{
	Encoder::releaseEncIterator();

	rsslClearArray(&_rsslArray);
}

void OmmArrayEncoder::fixedWidth( UInt16 width )
{
	if ( _rsslArray.primitiveType )
	{
		EmaString temp( "Attempt to set fixed width of OmmArray after add***() was already called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_rsslArray.itemLength = width;
}

void OmmArrayEncoder::initEncode()
{
	RsslRet retCode = rsslEncodeArrayInit( &(_pEncodeIter->_rsslEncIter), &_rsslArray );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeArrayComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeArrayInit( &(_pEncodeIter->_rsslEncIter), &_rsslArray );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize OmmArray encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void OmmArrayEncoder::addPrimitiveEntry( const char* methodName, void* value )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslRet retCode = rsslEncodeArrayEntry( &_pEncodeIter->_rsslEncIter, 0, value );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeArrayEntry( &_pEncodeIter->_rsslEncIter, 0, value );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding OmmArray. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void OmmArrayEncoder::addInt( Int64 value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_INT;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_INT )
	{
		EmaString temp( "Attempt to addInt() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	switch ( _rsslArray.itemLength )
	{
	case 0 :
	case 8 :
		break;
	case 1 :
		if ( value > 127 || value < -127 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
		break;
	case 2 :
		if ( value > 32767 || value < -32767 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
		break;
	case 4 :
		if ( value > 2147483647  || value < -2147483647  )
		{
			EmaString temp( "Out of range value for the specified fixed width in addInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
		break;
	default :
		{
			EmaString temp( "Unsupported fixedWidth encoding in addInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
		break;
	}
		
	addPrimitiveEntry( "addInt()", &value );
}

void OmmArrayEncoder::addUInt( UInt64 value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_UINT;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_UINT )
	{
		EmaString temp( "Attempt to addUInt() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	} 

	switch ( _rsslArray.itemLength )
	{
	case 0 :
	case 8 :
		break;
	case 1 :
		if ( value > 255 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addUInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
		break;
	case 2 :
		if ( value > 65535 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addUInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
		break;
	case 4 :
		if ( value > 4294967295 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addUInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
		break;
	default :
		{
			EmaString temp( "Unsupported fixedWidth encoding in addUInt(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		}
		return;
	}

	addPrimitiveEntry( "addUInt()",  &value );
}

void OmmArrayEncoder::addReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addReal(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_REAL;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_REAL )
	{
		EmaString temp( "Attempt to addReal() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslReal real;
	real.isBlank = RSSL_FALSE;
	real.value = mantissa;
	real.hint = magnitudeType;

	addPrimitiveEntry( "addReal()",  &real );
}

void OmmArrayEncoder::addRealFromDouble( double value, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addRealFromDouble(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_REAL;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_REAL )
	{
		EmaString temp( "Attempt to addRealFromDouble() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslReal real;
	if ( RSSL_RET_SUCCESS != rsslDoubleToReal( &real, &value, magnitudeType ) )
	{
		EmaString temp( "Attempt to addRealFromDouble() with invalid magnitudeType='" );
		temp.append( getMTypeAsString( magnitudeType) ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	addPrimitiveEntry( "addRealFromDouble()",  &real );
}

void OmmArrayEncoder::addFloat( float value )
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength && _rsslArray.itemLength != 4 )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addFloat(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_FLOAT;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_FLOAT )
	{
		EmaString temp( "Attempt to addFloat() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addFloat()", &value );
}

void OmmArrayEncoder::addDouble( double value )
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength && _rsslArray.itemLength != 8 )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addDouble(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_DOUBLE;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_DOUBLE )
	{
		EmaString temp( "Attempt to addDouble() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addDouble()", &value );
}

void OmmArrayEncoder::addDate( UInt16 year, UInt8 month, UInt8 day )
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

	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength && _rsslArray.itemLength != 4 )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addDate(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_DATE;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_DATE )
	{
		EmaString temp( "Attempt to addDate() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addDate()", &date );
}

void OmmArrayEncoder::addTime( UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
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

	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_TIME;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_TIME )
	{
		EmaString temp( "Attempt to addTime() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if ( !_rsslArray.itemLength ||
		_rsslArray.itemLength == 5 ||
		(_rsslArray.itemLength == 3 && millisecond == 0 ) )
	{
		addPrimitiveEntry( "addTime()", &time );
	}
	else
	{
		EmaString temp( "Unsupported fixedWidth encoding in addTime(). Fixed width='" );
		temp.append( _rsslArray.itemLength ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void OmmArrayEncoder::addDateTime( UInt16 year, UInt8 month, UInt8 day,
					UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
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

	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_DATETIME;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_DATETIME )
	{
		EmaString temp( "Attempt to addDateTime() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if ( !_rsslArray.itemLength ||
		_rsslArray.itemLength == 9 ||
		(_rsslArray.itemLength == 7 && millisecond == 0 ) )
	{
		addPrimitiveEntry( "addDateTime()", &dateTime );
	}
	else
	{
		EmaString temp( "Unsupported fixedWidth encoding in addDateTime(). Fixed width='" );
		temp.append( _rsslArray.itemLength ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void OmmArrayEncoder::addQos( UInt32 timeliness, UInt32 rate )
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addQos(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_QOS;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_QOS )
	{
		EmaString temp( "Attempt to addQos() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslQos qos;
	OmmQosDecoder::convertToRssl( &qos, timeliness, rate );

	addPrimitiveEntry( "addQos()", &qos );
}

void OmmArrayEncoder::addState( OmmState::StreamState streamState,
				OmmState::DataState dataState,
				UInt8 statusCode,
				const EmaString& statusText )
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addState(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_STATE;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_STATE )
	{
		EmaString temp( "Attempt to addState() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	addPrimitiveEntry( "addState()", &state );
}

void OmmArrayEncoder::addEnum( UInt16 value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_ENUM;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_ENUM )
	{
		EmaString temp( "Attempt to addEnum() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	switch ( _rsslArray.itemLength )
	{
	case 0 :
		break;
	case 1 :
		if ( value > 255 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addEnum(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		}
		return;
	case 2 :
		if ( value > 65535 )
		{
			EmaString temp( "Out of range value for the specified fixed width in addEnum(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "' value='" ).append( value ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		}
		return;
	default :
		{
			EmaString temp( "Unsupported fixedWidth encoding in addEnum(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		}
		return;
	}

	addPrimitiveEntry( "addEnum()", &value );
}

void OmmArrayEncoder::addBuffer( const EmaBuffer& value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_BUFFER;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_BUFFER )
	{
		EmaString temp( "Attempt to addBuffer() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if ( _rsslArray.itemLength && _rsslArray.itemLength < value.length() )
	{
		EmaString temp( "Passed in value is longer than fixed width in addBuffer(). Fixed width='" );
		temp.append( _rsslArray.itemLength ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( "addBuffer()", &buffer );
}

void OmmArrayEncoder::addAscii( const EmaString& value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_ASCII_STRING;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_ASCII_STRING )
	{
		EmaString temp( "Attempt to addAscii() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if ( _rsslArray.itemLength && _rsslArray.itemLength < value.length() )
	{
		EmaString temp( "Passed in value is longer than fixed width in addAscii(). Fixed width='" );
		temp.append( _rsslArray.itemLength ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_str();
	buffer.length = value.length();

	addPrimitiveEntry( "addAscii()", &buffer );
}

void OmmArrayEncoder::addUtf8( const EmaBuffer& value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_UTF8_STRING;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_UTF8_STRING )
	{
		EmaString temp( "Attempt to addUtf8() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if ( _rsslArray.itemLength && _rsslArray.itemLength < value.length() )
	{
		EmaString temp( "Passed in value is longer than fixed width in addUtf8(). Fixed width='" );
		temp.append( _rsslArray.itemLength ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( "addUtf8()", &buffer );
}

void OmmArrayEncoder::addRmtes( const EmaBuffer& value )
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_RMTES_STRING;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_RMTES_STRING )
	{
		EmaString temp( "Attempt to addRmtes() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	if ( _rsslArray.itemLength && _rsslArray.itemLength < value.length() )
	{
		EmaString temp( "Passed in value is longer than fixed width in addRmtes(). Fixed width='" );
		temp.append( _rsslArray.itemLength ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( "addRmtes()", &buffer );
}

void OmmArrayEncoder::addCodeInt()
{
	if ( !_rsslArray.primitiveType )
	{
		switch ( _rsslArray.itemLength )
		{
		case 0 :
		case 1 :
		case 2 :
		case 4 :
		case 8 :
			break;
		default :
			{
				EmaString temp( "Unsupported fixedWidth encoding in addCodeInt(). Fixed width='" );
				temp.append( _rsslArray.itemLength ).append( "'. " );
				throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			}
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_INT;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_INT )
	{
		EmaString temp( "Attempt to addCodeInt() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}
		
	addPrimitiveEntry( "addCodeInt()", 0 );
}

void OmmArrayEncoder::addCodeUInt()
{
	if ( !_rsslArray.primitiveType )
	{
		switch ( _rsslArray.itemLength )
		{
		case 0 :
		case 1 :
		case 2 :
		case 4 :
		case 8 :
			break;
		default :
			{
				EmaString temp( "Unsupported fixedWidth encoding in addCodeUInt(). Fixed width='" );
				temp.append( _rsslArray.itemLength ).append( "'. " );
				throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			}
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_UINT;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_UINT )
	{
		EmaString temp( "Attempt to addCodeUInt() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	} 

	addPrimitiveEntry( "addCodeUInt()", 0 );
}

void OmmArrayEncoder::addCodeReal()
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addCodeReal(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_REAL;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_REAL )
	{
		EmaString temp( "Attempt to addCodeReal() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeReal()", 0 );
}

void OmmArrayEncoder::addCodeFloat()
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength && _rsslArray.itemLength != 4 )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addCodeFloat(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_FLOAT;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_FLOAT )
	{
		EmaString temp( "Attempt to addCodeFloat() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeFloat()", 0 );
}

void OmmArrayEncoder::addCodeDouble()
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength && _rsslArray.itemLength != 8 )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addCodeDouble(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_DOUBLE;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_DOUBLE )
	{
		EmaString temp( "Attempt to addCodeDouble() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeDouble()", 0 );
}

void OmmArrayEncoder::addCodeDate()
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength && _rsslArray.itemLength != 4 )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addCodeDate(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_DATE;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_DATE )
	{
		EmaString temp( "Attempt to addCodeDate() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeDate()", 0 );
}

void OmmArrayEncoder::addCodeTime()
{
	if ( !_rsslArray.primitiveType )
	{
		switch ( _rsslArray.itemLength )
		{
		case 0 :
		case 3 :
		case 5 :
			break;
		default :
			{
				EmaString temp( "Unsupported fixedWidth encoding in addCodeTime(). Fixed width='" );
				temp.append( _rsslArray.itemLength ).append( "'. " );
				throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			}
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_TIME;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_TIME )
	{
		EmaString temp( "Attempt to addCodeTime() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeTime()", 0 );
}

void OmmArrayEncoder::addCodeDateTime()
{
	if ( !_rsslArray.primitiveType )
	{
		switch ( _rsslArray.itemLength )
		{
		case 0 :
		case 7 :
		case 9 :
			break;
		default :
			{
				EmaString temp( "Unsupported fixedWidth encoding in addCodeDateTime(). Fixed width='" );
				temp.append( _rsslArray.itemLength ).append( "'. " );
				throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			}
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_DATETIME;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_DATETIME )
	{
		EmaString temp( "Attempt to addCodeDateTime() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeDateTime()", 0 );
}

void OmmArrayEncoder::addCodeQos()
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addCodeQos(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_QOS;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_QOS )
	{
		EmaString temp( "Attempt to addCodeQos() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeQos()", 0 );
}

void OmmArrayEncoder::addCodeState()
{
	if ( !_rsslArray.primitiveType )
	{
		if ( _rsslArray.itemLength )
		{
			EmaString temp( "Unsupported fixedWidth encoding in addCodeState(). Fixed width='" );
			temp.append( _rsslArray.itemLength ).append( "'. " );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_STATE;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_STATE )
	{
		EmaString temp( "Attempt to addCodeState() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeState()", 0 );
}

void OmmArrayEncoder::addCodeEnum()
{
	if ( !_rsslArray.primitiveType )
	{
		switch ( _rsslArray.itemLength )
		{
		case 0 :
		case 1 :
		case 2 :
			break;
		default :
			{
				EmaString temp( "Unsupported fixedWidth encoding in addCodeEnum(). Fixed width='" );
				temp.append( _rsslArray.itemLength ).append( "'. " );
				throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			}
			return;
		}

		_rsslArray.primitiveType = RSSL_DT_ENUM;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_ENUM )
	{
		EmaString temp( "Attempt to addCodeEnum() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeEnum()", 0 );
}

void OmmArrayEncoder::addCodeBuffer()
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_BUFFER;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_BUFFER )
	{
		EmaString temp( "Attempt to addCodeBuffer() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeBuffer()", 0 );
}

void OmmArrayEncoder::addCodeAscii()
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_ASCII_STRING;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_ASCII_STRING )
	{
		EmaString temp( "Attempt to addCodeAscii() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeAscii()", 0 );
}

void OmmArrayEncoder::addCodeUtf8()
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_UTF8_STRING;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_UTF8_STRING )
	{
		EmaString temp( "Attempt to addCodeUtf8() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeUtf8()", 0 );
}

void OmmArrayEncoder::addCodeRmtes()
{
	if ( !_rsslArray.primitiveType )
	{
		_rsslArray.primitiveType = RSSL_DT_RMTES_STRING;

		acquireEncIterator();

		initEncode();
	}
	else if ( _rsslArray.primitiveType != RSSL_DT_RMTES_STRING )
	{
		EmaString temp( "Attempt to addCodeRmtes() while OmmArray contains='" );
		temp.append(getDTypeAsString((DataType::DataTypeEnum)_rsslArray.primitiveType)).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	addPrimitiveEntry( "addCodeRmtes()", 0 );
}

void OmmArrayEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( !_rsslArray.primitiveType )
	{
		EmaString temp( "Attempt to complete() while no OmmArray::add***() were called yet." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	RsslRet retCode = rsslEncodeArrayComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete OmmArray encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();

	_containerComplete = true;
}

void OmmArrayEncoder::endEncodingEntry() const
{
}
