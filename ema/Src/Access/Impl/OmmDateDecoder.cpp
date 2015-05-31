/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDateDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmDateDecoder::OmmDateDecoder() :
 _rsslBuffer(),
 _rsslDate(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
	rsslClearDate( &_rsslDate );
}

OmmDateDecoder::~OmmDateDecoder()
{
}

Data::DataCode OmmDateDecoder::getCode() const
{
	return _dataCode;
}

void OmmDateDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmDateDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmDateDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeDate( dIter, &_rsslDate );

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
			EmaString temp( "Failed to decode OmmDate. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

const EmaString& OmmDateDecoder::toString()
{
	if ( !_toStringSet )
	{
		if ( _dataCode == Data::BlankEnum )
		{
			_toStringSet = true;
			_toString.set( "(blank data)" );
			return _toString;
		}

		char dateString[256];
		RsslBuffer tempRsslBuffer;
		tempRsslBuffer.data = dateString;
		tempRsslBuffer.length = 256;
		RsslDateTime rsslDateTime;
		rsslDateTime.date = _rsslDate;
		RsslRet retCode = rsslDateTimeToString( &tempRsslBuffer, RSSL_DT_DATE, &rsslDateTime );

		if ( RSSL_RET_SUCCESS != retCode )
		{
			EmaString temp( "Failed to convert OmmDate to string. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		else
		{
			_toStringSet = true;
			_toString.set( dateString );
		}
	}

	return _toString;
}

UInt16 OmmDateDecoder::getYear() const
{
	return _rsslDate.year;
}

UInt8 OmmDateDecoder::getMonth() const
{
	return _rsslDate.month;
}

UInt8 OmmDateDecoder::getDay() const
{
	return _rsslDate.day;
}

const EmaBuffer& OmmDateDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}
