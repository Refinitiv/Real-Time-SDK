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
 _pRsslBuffer( 0 ),
 _rsslDate(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum )
{
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

void OmmDateDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	if ( rsslDecodeDate( dIter, &_rsslDate ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
	{
		_dataCode = Data::BlankEnum;
		rsslClearDate( &_rsslDate );
	}
}

const EmaString& OmmDateDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
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
		_toString.set( dateString );
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
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}
