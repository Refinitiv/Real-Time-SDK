/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmIntDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmIntDecoder::OmmIntDecoder() :
 _pRsslBuffer( 0 ),
 _rsslInt( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmIntDecoder::~OmmIntDecoder()
{
}

Data::DataCode OmmIntDecoder::getCode() const
{
	return _dataCode;
}

bool OmmIntDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmIntDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmIntDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeInt( dIter, &_rsslInt ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		_rsslInt = 0;
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

const EmaString& OmmIntDecoder::toString()
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
	RsslRet retCode = rsslPrimitiveToString( &_rsslInt, RSSL_DT_INT, &tempBuffer );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString text( "Failed to convert OmmInt to string. Reason: " );
		text += rsslRetCodeToString( retCode );
		throwIueException( text, retCode );
	}
	else
	{
		_toString.set( temp );
	}

	return _toString;
}

Int64 OmmIntDecoder::getInt() const
{
	return _rsslInt;
}

const EmaBuffer& OmmIntDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmIntDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmIntDecoder::getErrorCode() const
{
	return _errorCode;
}
