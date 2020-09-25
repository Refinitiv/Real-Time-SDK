/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDoubleDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmDoubleDecoder::OmmDoubleDecoder() :
 _pRsslBuffer( 0 ),
 _rsslDouble( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmDoubleDecoder::~OmmDoubleDecoder()
{
}

Data::DataCode OmmDoubleDecoder::getCode() const
{
	return _dataCode;
}

bool OmmDoubleDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmDoubleDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmDoubleDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeDouble( dIter, &_rsslDouble ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		_rsslDouble = 0;
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
		throwIueException( text, retCode );
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

const RsslBuffer& OmmDoubleDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmDoubleDecoder::getErrorCode() const
{
	return _errorCode;
}
