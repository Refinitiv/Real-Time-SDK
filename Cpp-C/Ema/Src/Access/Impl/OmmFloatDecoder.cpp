/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmFloatDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmFloatDecoder::OmmFloatDecoder() :
 _pRsslBuffer( 0 ),
 _rsslFloat( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmFloatDecoder::~OmmFloatDecoder()
{
}

Data::DataCode OmmFloatDecoder::getCode() const
{
	return _dataCode;
}

bool OmmFloatDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmFloatDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmFloatDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeFloat( dIter, &_rsslFloat ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		_rsslFloat = 0;
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

const EmaString& OmmFloatDecoder::toString()
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
	RsslRet retCode = rsslPrimitiveToString( &_rsslFloat, RSSL_DT_FLOAT, &tempBuffer );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString text( "Failed to convert OmmFloat to string. Reason: " );
		text += rsslRetCodeToString( retCode );
		throwIueException( text );
	}
	else
	{
		_toString.set( temp );
	}

	return _toString;
}

float OmmFloatDecoder::getFloat() const
{
	return _rsslFloat;
}

const EmaBuffer& OmmFloatDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmFloatDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmFloatDecoder::getErrorCode() const
{
	return _errorCode;
}
