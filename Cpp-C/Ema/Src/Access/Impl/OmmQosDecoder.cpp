/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmQosDecoder.h"
#include "OmmQos.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmQosDecoder::OmmQosDecoder() :
 _pRsslBuffer( 0 ),
 _rsslQos(),
 _toString(),
 _hexBuffer(),
 _dataCode( Data::BlankEnum ),
 _errorCode( OmmError::NoErrorEnum )
{
}

OmmQosDecoder::~OmmQosDecoder()
{
}

Data::DataCode OmmQosDecoder::getCode() const
{
	return _dataCode;
}

bool OmmQosDecoder::setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmQosDecoder::setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* )
{
	_errorCode = OmmError::UnknownErrorEnum;
	return false;
}

bool OmmQosDecoder::setRsslData( RsslDecodeIterator* dIter, RsslBuffer* pRsslBuffer )
{
	_pRsslBuffer = pRsslBuffer;

	switch ( rsslDecodeQos( dIter, &_rsslQos ) )
	{
	case RSSL_RET_SUCCESS :
		_dataCode = Data::NoCodeEnum;
		_errorCode = OmmError::NoErrorEnum;
		return true;
	case RSSL_RET_BLANK_DATA :
		rsslClearQos( &_rsslQos );
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

void OmmQosDecoder::setRsslData( RsslQos* rsslQos )
{
	if ( rsslQos )
	{
		_rsslQos = *rsslQos;
		_dataCode = Data::NoCodeEnum;
	}
	else
	{
		rsslClearQos( &_rsslQos );
		_dataCode = Data::BlankEnum;
	}
}

const EmaString& OmmQosDecoder::toString()
{
	if ( _dataCode == Data::BlankEnum )
	{
		static const EmaString blankData( "(blank data)" );
		return blankData;
	}

	switch( _rsslQos.timeliness )
	{
	case RSSL_QOS_TIME_REALTIME:
		_toString.set( "RealTime" );
		break;
	case RSSL_QOS_TIME_DELAYED_UNKNOWN :
		_toString.set( "InexactDelayed" );
		break;
	case RSSL_QOS_TIME_DELAYED:
		_toString.set( "Timeliness: " ).append( _rsslQos.timeInfo );
		break;
	}

	_toString.append("/");

	switch( _rsslQos.rate )
	{
	case RSSL_QOS_RATE_TICK_BY_TICK:
		_toString.append( "TickByTick" );
		break;
	case RSSL_QOS_RATE_JIT_CONFLATED:
		_toString.append( "JustInTimeConflated" );
		break;
	case RSSL_QOS_RATE_TIME_CONFLATED:
		_toString.append("Rate: ").append( _rsslQos.rateInfo );
		break;
	}

	return _toString;
}

UInt32 OmmQosDecoder::getTimeliness() const
{
	UInt32 timeliness = OmmQos::InexactDelayedEnum;

	switch( _rsslQos.timeliness )
	{
	case RSSL_QOS_TIME_REALTIME:
		timeliness = OmmQos::RealTimeEnum;
		break;
	case RSSL_QOS_TIME_DELAYED_UNKNOWN :
		timeliness = OmmQos::InexactDelayedEnum;
		break;
	case RSSL_QOS_TIME_DELAYED:
		timeliness = _rsslQos.timeInfo;
		break;
	}

	return timeliness;
}

UInt32 OmmQosDecoder::getRate() const
{
	UInt32 rate = OmmQos::JustInTimeConflatedEnum;

	switch( _rsslQos.rate )
	{
	case RSSL_QOS_RATE_TICK_BY_TICK:
		rate = OmmQos::TickByTickEnum;
		break;
	case RSSL_QOS_RATE_JIT_CONFLATED:
		rate = OmmQos::JustInTimeConflatedEnum;
		break;
	case RSSL_QOS_RATE_TIME_CONFLATED:
		rate = _rsslQos.rateInfo;
		break;
	}

	return rate;
}

void OmmQosDecoder::convertToRssl( RsslQos* pQos, UInt32 timeliness, UInt32 rate )
{
	if ( !pQos ) return;

	pQos->dynamic = RSSL_FALSE;

	if ( rate == OmmQos::TickByTickEnum )
		pQos->rate = RSSL_QOS_RATE_TICK_BY_TICK;
	else if ( rate == OmmQos::JustInTimeConflatedEnum )
		pQos->rate = RSSL_QOS_RATE_JIT_CONFLATED;
	else
	{
		if ( rate <= 65535 )
		{
			pQos->rate = RSSL_QOS_RATE_TIME_CONFLATED;
			pQos->rateInfo = rate;
		}
		else
		{
			pQos->rate = RSSL_QOS_RATE_JIT_CONFLATED;
		}
	}

	if ( timeliness == OmmQos::RealTimeEnum )
		pQos->timeliness = RSSL_QOS_TIME_REALTIME;
	else if ( timeliness == OmmQos::InexactDelayedEnum )
		pQos->timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	else
	{
		if ( timeliness <= 65535 )
		{
			pQos->timeliness = RSSL_QOS_TIME_DELAYED;
			pQos->timeInfo = timeliness;
		}
		else
		{
			pQos->timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		}
	}
}

const EmaBuffer& OmmQosDecoder::getHexBuffer()
{
	_hexBuffer.setFromInt( _pRsslBuffer->data, _pRsslBuffer->length );

	return _hexBuffer.toBuffer();
}

const RsslBuffer& OmmQosDecoder::getRsslBuffer() const
{
	return *_pRsslBuffer;
}

OmmError::ErrorCode OmmQosDecoder::getErrorCode() const
{
	return _errorCode;
}
