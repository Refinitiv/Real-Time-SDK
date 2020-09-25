/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmEnumDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmEnumDecoder::OmmEnumDecoder() :
 _pRsslBuffer( 0 ),
 _rsslEnum( 0 ),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmEnumDecoder::~OmmEnumDecoder()
{
}

Data::DataCode OmmEnumDecoder::getCode() const
{
	return _dataCode;
}

bool OmmEnumDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmEnumDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmEnumDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeEnum( dIter, &_rsslEnum ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		_rsslEnum = 0;
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

const EmaString& OmmEnumDecoder::toString()
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
	RsslRet retCode = rsslPrimitiveToString( &_rsslEnum, RSSL_DT_ENUM, &tempBuffer );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString text( "Failed to convert OmmEnum to string. Reason: " );
		text += rsslRetCodeToString( retCode );
		throwIueException( text, retCode );
	}
	else
	{
		_toString.set( temp );
	}

	return _toString;
}

UInt16 OmmEnumDecoder::getEnum() const
{
	return _rsslEnum;
}

const EmaBuffer& OmmEnumDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmEnumDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmEnumDecoder::getErrorCode() const
{
	return _errorCode;
}
