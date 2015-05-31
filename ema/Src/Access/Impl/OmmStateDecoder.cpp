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
 _rsslBuffer(),
 _rsslState(),
 _toString(),
 _statusText(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _toStringSet( false )
{
	rsslClearState( & _rsslState );
}

OmmStateDecoder::~OmmStateDecoder()
{
}

Data::DataCode OmmStateDecoder::getCode() const
{
	return _dataCode;
}

void OmmStateDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
}

void OmmStateDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
}

void OmmStateDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* rsslBuffer )
{
	_rsslBuffer = *rsslBuffer;

	_toStringSet = false;

	RsslRet retCode = rsslDecodeState( dIter, &_rsslState );

	switch ( retCode )
	{
	case RSSL_RET_BLANK_DATA :
		_dataCode = Data::BlankEnum;
		break;
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		break;
	case RSSL_RET_INCOMPLETE_DATA :
	default :
		{
			_dataCode = Data::BlankEnum;
			EmaString temp( "Failed to decode OmmState. Reason: " );
			temp += rsslRetCodeToString( retCode );
			throwIueException( temp );
		}
		break;
	}
}

void OmmStateDecoder::setRsslData( RsslState* rsslState )
{
	_toStringSet = false;

	if ( rsslState )
	{
		_dataCode = Data::NoCodeEnum;

		_rsslState = *rsslState;
	}
	else
	{
		_dataCode = Data::BlankEnum;

		_rsslState.code = RSSL_SC_NONE;
		_rsslState.code = RSSL_DATA_OK;
		_rsslState.streamState = RSSL_STREAM_OPEN;
		rsslClearBuffer( &_rsslState.text );
	}
}

const EmaString& OmmStateDecoder::toString()
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		if ( _dataCode == Data::BlankEnum )
			_toString.clear().set( "(blank data)" );
		else
			stateToString( &_rsslState, _toString );
	}

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

UInt16 OmmStateDecoder::getStatusCode() const
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
	_hexBuffer.setFromInt( _rsslBuffer.data, _rsslBuffer.length );
	
	return _hexBuffer.toBuffer();
}
