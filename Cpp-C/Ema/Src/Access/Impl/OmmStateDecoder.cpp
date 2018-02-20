/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmStateDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmStateDecoder::OmmStateDecoder() :
 _pRsslBuffer( 0 ),
 _rsslState(),
 _toString(),
 _statusText(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmStateDecoder::~OmmStateDecoder()
{
}

Data::DataCode OmmStateDecoder::getCode() const
{
	return _dataCode;
}

bool OmmStateDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmStateDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmStateDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeState( dIter, &_rsslState ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		rsslClearState( &_rsslState );
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

void OmmStateDecoder::setRsslData( RsslState* rsslState )
{
	if ( rsslState )
	{
		_dataCode = Data::NoCodeEnum;

		_rsslState = *rsslState;
	}
	else
	{
		_dataCode = Data::BlankEnum;

		_rsslState.code = RSSL_SC_NONE;
		_rsslState.dataState = RSSL_DATA_OK;
		_rsslState.streamState = RSSL_STREAM_OPEN;
		rsslClearBuffer( &_rsslState.text );
	}
}

const EmaString& OmmStateDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}
	
	stateToString( &_rsslState, _toString );

	return _toString;
}

OmmState::StreamState OmmStateDecoder::getStreamState() const
{
	return static_cast< OmmState::StreamState >( _rsslState.streamState );
}

OmmState::DataState OmmStateDecoder::getDataState() const
{
	return static_cast< OmmState::DataState >( _rsslState.dataState );
}

UInt8 OmmStateDecoder::getStatusCode() const
{
	return _rsslState.code;
}

const EmaString& OmmStateDecoder::getStatusText()
{
	_statusText.setInt( _rsslState.text.data, _rsslState.text.length, false );

	return _statusText.toString();
}

const EmaBuffer& OmmStateDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );
	
	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmStateDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmStateDecoder::getErrorCode() const
{
	return _errorCode;
}
