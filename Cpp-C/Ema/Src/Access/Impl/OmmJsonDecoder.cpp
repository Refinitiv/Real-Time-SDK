/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmJsonDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmJsonDecoder::OmmJsonDecoder() :
 _rsslBuffer(),
 _toString(),
 _getString(),
 _getBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmJsonDecoder::~OmmJsonDecoder()
{
}

bool OmmJsonDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
{
	switch ( rsslDecodeBuffer( dIter, &_rsslBuffer ) )
	{
	case RSSL_RET_BLANK_DATA :
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
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

bool OmmJsonDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmJsonDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* pRsslBuffer,
								const RsslDataDictionary* , void* )
{
	RsslDecodeIterator decodeIterator;
	rsslClearDecodeIterator( &decodeIterator );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIterator, pRsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIterator, majVer, minVer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	switch ( rsslDecodeBuffer( &decodeIterator, &_rsslBuffer ) )
	{
	case RSSL_RET_BLANK_DATA :
		_dataCode = Data::BlankEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
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

Data::DataCode OmmJsonDecoder::getCode() const
{
	return _dataCode;
}

const EmaString& OmmJsonDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaString& OmmJsonDecoder::getString()
{
	_getString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _getString.toString();
}

const EmaBuffer& OmmJsonDecoder::getBuffer()
{
	_getBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _getBuffer.toBuffer();
}

const RsslBuffer& OmmJsonDecoder::getRsslBuffer() const
{
	return _rsslBuffer;
}

OmmError::ErrorCode OmmJsonDecoder::getErrorCode() const
{
	return _errorCode;
}
