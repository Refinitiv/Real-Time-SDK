/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDateTimeDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmDateTimeDecoder::OmmDateTimeDecoder() :
 _pRsslBuffer( 0 ),
 _rsslDateTime(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum ),
 _format(DateTimeStringFormat::STR_DATETIME_RSSL)
{
}

OmmDateTimeDecoder::~OmmDateTimeDecoder()
{
}

Data::DataCode OmmDateTimeDecoder::getCode() const
{
	return _dataCode;
}

bool OmmDateTimeDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmDateTimeDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmDateTimeDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeDateTime( dIter, &_rsslDateTime ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		rsslClearDateTime( &_rsslDateTime );
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_INCOMPLETE_DATA :
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	default :
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}
}

DateTimeStringFormat::DateTimeStringFormatTypes OmmDateTimeDecoder::setDateTimeStringFormatType(DateTimeStringFormat::DateTimeStringFormatTypes format)
{
	_format = format;
	return _format;
}

DateTimeStringFormat::DateTimeStringFormatTypes OmmDateTimeDecoder::getDateTimeStringFormatType()
{
	return _format;
}

const EmaString& OmmDateTimeDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	char dateTimeString[512];
	RsslBuffer tempRsslBuffer;
	tempRsslBuffer.data = dateTimeString;
	tempRsslBuffer.length = 512;
	RsslRet retCode = rsslDateTimeToStringFormat( &tempRsslBuffer, RSSL_DT_DATETIME, &_rsslDateTime, _format );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "Failed to convert OmmDateTime to string. Reason: " );
		temp += rsslRetCodeToString( retCode );
		throwIueException( temp, retCode );
	}
	else
	{
		_toString.set( dateTimeString );
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
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmDateTimeDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmDateTimeDecoder::getErrorCode() const
{
	return _errorCode;
}
