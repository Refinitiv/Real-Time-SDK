/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmOpaqueDecoder.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmOpaqueDecoder::OmmOpaqueDecoder() :
 _rsslBuffer(),
 _toString(),
 _getString(),
 _getBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmOpaqueDecoder::~OmmOpaqueDecoder()
{
}

Data::DataCode OmmOpaqueDecoder::getCode() const
{
	return _dataCode;
}

bool OmmOpaqueDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* )
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

bool OmmOpaqueDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmOpaqueDecoder::setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* pRsslBuffer,
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

const EmaString& OmmOpaqueDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}
	
	_toString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _toString.toString();
}

const EmaString& OmmOpaqueDecoder::getString()
{
	_getString.setInt( _rsslBuffer.data, _rsslBuffer.length, false );

	return _getString.toString();
}

const EmaBuffer& OmmOpaqueDecoder::getBuffer()
{
	_getBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );

	return _getBuffer.toBuffer();
}

const RsslBuffer& OmmOpaqueDecoder::getRsslBuffer() const
{
	return _rsslBuffer;
}

OmmError::ErrorCode OmmOpaqueDecoder::getErrorCode() const
{
	return _errorCode;
}
