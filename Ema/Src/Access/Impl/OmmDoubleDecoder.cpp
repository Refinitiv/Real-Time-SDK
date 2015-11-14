/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDoubleDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmDoubleDecoder::OmmDoubleDecoder() :
 _pRsslBuffer( 0 ),
 _rsslDouble( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum )
{
}

OmmDoubleDecoder::~OmmDoubleDecoder()
{
}

Data::DataCode OmmDoubleDecoder::getCode() const
{
	return _dataCode;
}

void OmmDoubleDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmDoubleDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmDoubleDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	if ( rsslDecodeDouble( dIter, &_rsslDouble ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
	{
		_dataCode = Data::BlankEnum;
		_rsslDouble = 0;
	}
}

const EmaString& OmmDoubleDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	char temp[256];
	RsslBuffer tempBuffer;
	tempBuffer.length = 256;
	tempBuffer.data = temp;
	RsslRet retCode = rsslPrimitiveToString( &_rsslDouble, RSSL_DT_DOUBLE, &tempBuffer );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString text( "Failed to convert OmmDouble to string. Reason: " );
		text += rsslRetCodeToString( retCode );
		throwIueException( text );
	}
	else
	{
		_toString.set( temp );
	}

	return _toString;
}

double OmmDoubleDecoder::getDouble() const
{
	return _rsslDouble;
}

const EmaBuffer& OmmDoubleDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}
