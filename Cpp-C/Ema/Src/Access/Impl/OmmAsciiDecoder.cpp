/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmAsciiDecoder.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmAsciiDecoder::OmmAsciiDecoder() :
 _rsslBuffer(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmAsciiDecoder::~OmmAsciiDecoder()
{
}

Data::DataCode OmmAsciiDecoder::getCode() const
{
	return _dataCode;
}

bool OmmAsciiDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmAsciiDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmAsciiDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
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

const EmaString& OmmAsciiDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaString& OmmAsciiDecoder::getAscii()
{
	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaBuffer& OmmAsciiDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmAsciiDecoder::getRsslBuffer() const
{
	return _rsslBuffer;
}

OmmError::ErrorCode OmmAsciiDecoder::getErrorCode() const
{
	return _errorCode;
}
