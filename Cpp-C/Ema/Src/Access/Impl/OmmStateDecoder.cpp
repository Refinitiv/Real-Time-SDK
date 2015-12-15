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
 _dataCode( Data::BlankEnum )
{
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

void OmmStateDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	if ( rsslDecodeState( dIter, &_rsslState ) == RSSL_RET_SUCCESS )
		_dataCode = Data::NoCodeEnum;
	else
	{
		_dataCode = Data::BlankEnum;
		rsslClearState( &_rsslState );
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
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );
	
	return _hexBuffer.toBuffer();
}
