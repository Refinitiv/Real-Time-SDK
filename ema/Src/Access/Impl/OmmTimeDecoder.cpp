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
 _rsslBuffer(),
 _rsslTime(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
	rsslClearTime( &_rsslTime );
}

OmmTimeDecoder::~OmmTimeDecoder()
{
}

Data::DataCode OmmTimeDecoder::getCode() const
{
	return _dataCode;
}

void OmmTimeDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmTimeDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmTimeDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeTime( dIter, &_rsslTime );

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
			EmaString temp( "Failed to decode OmmTime. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmTimeDecoder::toString()
{
	if ( !_toStringSet )
	{
		if ( _dataCode == Data::BlankEnum )
		{
			_toStringSet = true;
			_toString.set( "(blank data)" );
			return _toString;
		}

		char timeString[512];
		RsslBuffer tempRsslBuffer;
		tempRsslBuffer.data = timeString;
		tempRsslBuffer.length = 512;
		RsslDateTime rsslDateTime;
		rsslDateTime.time = _rsslTime;
		RsslRet retCode = rsslDateTimeToString( &tempRsslBuffer, RSSL_DT_TIME, &rsslDateTime );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString temp( "Failed to convert OmmTime to string. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		else
		{
			_toStringSet = true;
			_toString.set( timeString );
		}
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
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
