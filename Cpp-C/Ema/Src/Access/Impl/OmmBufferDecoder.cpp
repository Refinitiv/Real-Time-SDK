/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmBufferDecoder.h"
#include "ExceptionTranslator.h"
#include <stdlib.h>

using namespace refinitiv::ema::access;

OmmBufferDecoder::OmmBufferDecoder() :
 _rsslBuffer(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmBufferDecoder::~OmmBufferDecoder()
{
}

Data::DataCode OmmBufferDecoder::getCode() const
{
	return _dataCode;
}

bool OmmBufferDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmBufferDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmBufferDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	switch ( rsslDecodeBuffer( dIter, &_rsslBuffer ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
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

const EmaString& OmmBufferDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	const char* tempStr = getBuffer().operator const char *();
	UInt32 length = (UInt32)strlen( tempStr );
	_toString.setInt( tempStr, length, true );
	return _toString.toString();
}

const EmaBuffer& OmmBufferDecoder::getBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );
	
	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmBufferDecoder::getRsslBuffer() const
{
	return _rsslBuffer;
}

OmmError::ErrorCode OmmBufferDecoder::getErrorCode() const
{
	return _errorCode;
}
