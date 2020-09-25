/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmErrorDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmErrorDecoder::OmmErrorDecoder() :
 _pRsslBuffer( 0 ),
 _errorCode( OmmError::NoErrorEnum ),
 _toHex()
{
}

OmmErrorDecoder::~OmmErrorDecoder()
{
}

OmmError::ErrorCode OmmErrorDecoder::getErrorCode() const
{
	return _errorCode;
}

bool OmmErrorDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmErrorDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* pRsslBuffer, const RsslDataDictionary* error , void* )
{
	_pRsslBuffer = pRsslBuffer;

	_errorCode = (OmmError::ErrorCode)(UInt64)error;

	return true;
}

bool OmmErrorDecoder::setRsslData( RsslDecodeIterator* , RsslBuffer* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

const EmaBuffer& OmmErrorDecoder::getAsHex()
{
	_toHex.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _toHex.toBuffer();
}

const RsslBuffer& OmmErrorDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}
