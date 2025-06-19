/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmRealDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmRealDecoder::OmmRealDecoder() :
 _pRsslBuffer( 0 ),
 _rsslReal(),
 _toDouble( 0 ),
 _toString(),
 _hexBuffer(),
 _toDoubleSet( false ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmRealDecoder::~OmmRealDecoder()
{
}

Data::DataCode OmmRealDecoder::getCode() const
{
	return _rsslReal.isBlank == RSSL_TRUE ? Data::BlankEnum : Data::NoCodeEnum;
}

Int8 OmmRealDecoder::getRsslIsBlank() const
{
	return _rsslReal.isBlank;
}

UInt8 OmmRealDecoder::getRsslHint() const
{
	return _rsslReal.hint;
}

bool OmmRealDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	rsslClearReal( &_rsslReal );
	_rsslReal.isBlank = RSSL_TRUE;
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmRealDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	rsslClearReal( &_rsslReal );
	_rsslReal.isBlank = RSSL_TRUE;
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmRealDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	_toDoubleSet = false;

	switch ( rsslDecodeReal( dIter, &_rsslReal ) )
	{
	case RSSL_RET_SUCCESS :
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		rsslClearReal( &_rsslReal );
		_rsslReal.isBlank = RSSL_TRUE;
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

const EmaString& OmmRealDecoder::toString()
{
	if ( _rsslReal.isBlank == RSSL_TRUE )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	char realString[256];
	RsslBuffer tempRsslBuffer;
	tempRsslBuffer.data = realString;
	tempRsslBuffer.length = 256;
	RsslRet retCode = rsslRealToString( &tempRsslBuffer, &_rsslReal );

	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "Failed to convert OmmReal to string. Reason: " );
		temp += rsslRetCodeToString( retCode );
		throwIueException( temp, retCode );
	}
	else
	{
		_toString.set( realString );
	}

	return _toString;
}

Int64 OmmRealDecoder::getMantissa() const
{
	return _rsslReal.value;
}

OmmReal::MagnitudeType OmmRealDecoder::getMagnitudeType() const
{
	return static_cast< OmmReal::MagnitudeType >( _rsslReal.hint );
}

double OmmRealDecoder::toDouble()
{
	if ( !_toDoubleSet )
	{
		_toDoubleSet = true;

		rsslRealToDouble( &_toDouble, &_rsslReal );
	}

	return _toDouble;
}

const EmaBuffer& OmmRealDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmRealDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmRealDecoder::getErrorCode() const
{
	return _errorCode;
}
