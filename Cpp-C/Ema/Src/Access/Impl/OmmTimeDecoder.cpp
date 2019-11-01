/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmTimeDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmTimeDecoder::OmmTimeDecoder() :
 _pRsslBuffer( 0 ),
 _rsslTime(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum ),
 _format(DateTimeStringFormat::STR_DATETIME_RSSL)
{
}

OmmTimeDecoder::~OmmTimeDecoder()
{
}

Data::DataCode OmmTimeDecoder::getCode() const
{
	return _dataCode;
}

bool OmmTimeDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmTimeDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmTimeDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeTime( dIter, &_rsslTime ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		rsslClearTime( &_rsslTime );
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
DateTimeStringFormat::DateTimeStringFormatTypes OmmTimeDecoder::setDateTimeStringFormatType(DateTimeStringFormat::DateTimeStringFormatTypes format)
{
	_format = format;
	return _format;
}

DateTimeStringFormat::DateTimeStringFormatTypes OmmTimeDecoder::getDateTimeStringFormatType()
{
	return _format;
}
const EmaString& OmmTimeDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	char timeString[512];
	RsslBuffer tempRsslBuffer;
	tempRsslBuffer.data = timeString;
	tempRsslBuffer.length = 512;
	RsslDateTime rsslDateTime;
	rsslDateTime.time = _rsslTime;
	RsslRet retCode = rsslDateTimeToStringFormat( &tempRsslBuffer, RSSL_DT_TIME, &rsslDateTime, _format );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "Failed to convert OmmTime to string. Reason: " );
		temp += rsslRetCodeToString( retCode );
		throwIueException( temp, retCode );
	}
	else
	{
		_toString.set( timeString );
	}

	return _toString;
}

UInt8 OmmTimeDecoder::getHour() const
{
	return _rsslTime.hour;
}

UInt8 OmmTimeDecoder::getMinute() const
{
	return _rsslTime.minute;
}

UInt8 OmmTimeDecoder::getSecond() const
{
	return _rsslTime.second;
}

UInt16 OmmTimeDecoder::getMillisecond() const
{
	return _rsslTime.millisecond;
}

UInt16 OmmTimeDecoder::getMicrosecond() const
{
	return _rsslTime.microsecond;
}

UInt16 OmmTimeDecoder::getNanosecond() const
{
	return _rsslTime.nanosecond;
}

const EmaBuffer& OmmTimeDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmTimeDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmTimeDecoder::getErrorCode() const
{
	return _errorCode;
}
