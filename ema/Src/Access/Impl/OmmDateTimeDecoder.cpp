/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDateTimeDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmDateTimeDecoder::OmmDateTimeDecoder() :
 _rsslBuffer(),
 _rsslDateTime(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
	rsslClearDateTime( &_rsslDateTime );
}

OmmDateTimeDecoder::~OmmDateTimeDecoder()
{
}

Data::DataCode OmmDateTimeDecoder::getCode() const
{
	return _dataCode;
}

void OmmDateTimeDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmDateTimeDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmDateTimeDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeDateTime( dIter, &_rsslDateTime );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_dataCode = Data::BlankEnum;
		break;
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
	default :
		{
			_dataCode = Data::BlankEnum;
			EmaString temp( "Failed to decode OmmDateTime. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmDateTimeDecoder::toString()
{
	if ( !_toStringSet )
	{
		if ( _dataCode == Data::BlankEnum )
		{
			_toStringSet = true;
			_toString.set( "(blank data)" );
			return _toString;
		}

		char dateTimeString[512];
		RsslBuffer tempRsslBuffer;
		tempRsslBuffer.data = dateTimeString;
		tempRsslBuffer.length = 512;
		RsslRet retCode = rsslDateTimeToString( &tempRsslBuffer, RSSL_DT_DATETIME, &_rsslDateTime );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString temp( "Failed to convert OmmDateTime to string. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		else
		{
			_toStringSet = true;
			_toString.set( dateTimeString );
		}
	}

	return _toString;
}

UInt16 OmmDateTimeDecoder::getYear() const
{
	return _rsslDateTime.date.year;
}

UInt8 OmmDateTimeDecoder::getMonth() const
{
	return _rsslDateTime.date.month;
}

UInt8 OmmDateTimeDecoder::getDay() const
{
	return _rsslDateTime.date.day;
}

UInt8 OmmDateTimeDecoder::getHour() const
{
	return _rsslDateTime.time.hour;
}

UInt8 OmmDateTimeDecoder::getMinute() const
{
	return _rsslDateTime.time.minute;
}

UInt8 OmmDateTimeDecoder::getSecond() const
{
	return _rsslDateTime.time.second;
}

UInt16 OmmDateTimeDecoder::getMillisecond() const
{
	return _rsslDateTime.time.millisecond;
}

UInt16 OmmDateTimeDecoder::getMicrosecond() const
{
	return _rsslDateTime.time.microsecond;
}

UInt16 OmmDateTimeDecoder::getNanosecond() const
{
	return _rsslDateTime.time.nanosecond;
}

const EmaBuffer& OmmDateTimeDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
