/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUtf8Decoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmUtf8Decoder::OmmUtf8Decoder() :
 _rsslBuffer(),
 _toString(),
 _utf8Buffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmUtf8Decoder::~OmmUtf8Decoder()
{
}

Data::DataCode OmmUtf8Decoder::getCode() const
{
	return _dataCode;
}

bool OmmUtf8Decoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmUtf8Decoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmUtf8Decoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
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

const EmaString& OmmUtf8Decoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}
	
	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaBuffer& OmmUtf8Decoder::getUtf8()
{
	_utf8Buffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _utf8Buffer.toBuffer();
}

const RsslBuffer& OmmUtf8Decoder::getRsslBuffer() const
{
	return _rsslBuffer;
}

OmmError::ErrorCode OmmUtf8Decoder::getErrorCode() const
{
	return _errorCode;
}
