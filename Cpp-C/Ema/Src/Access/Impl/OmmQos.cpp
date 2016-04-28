/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmQos.h"
#include "OmmQosDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

const EmaString TickByTickRateName( "TickByTick" );
const EmaString JustInTimeConflatedRateName( "JustInTimeConflated" );
EmaString UnknownQosRateName;

const EmaString RealTimeTimelinessName( "RealTime" );
const EmaString InexactDelayedTimelinessName( "InexactDelayed" );
EmaString UnknownQosTimelinessName;

OmmQos::OmmQos() :
 _pDecoder( new ( _space ) OmmQosDecoder() )
{
}

OmmQos::~OmmQos()
{
	_pDecoder->~OmmQosDecoder();
}

const EmaString& OmmQos::getRateAsString() const
{
	switch ( getRate() )
	{
	case TickByTickEnum :
		return TickByTickRateName;
	case JustInTimeConflatedEnum :
		return JustInTimeConflatedRateName;
	default :
		return UnknownQosRateName.set( "Rate: " ).append( (UInt64)getRate() );
	}
}

const EmaString& OmmQos::getTimelinessAsString() const
{
	switch ( getTimeliness() )
	{
	case RealTimeEnum :
		return RealTimeTimelinessName;
	case InexactDelayedEnum :
		return InexactDelayedTimelinessName;
	default :
		return UnknownQosTimelinessName.set( "Timeliness: " ).append( (UInt64)getTimeliness() );
	}
}

DataType::DataTypeEnum OmmQos::getDataType() const
{
	return DataType::QosEnum;
}

Data::DataCode OmmQos::getCode() const
{
	return _pDecoder->getCode();
}

UInt32 OmmQos::getTimeliness() const
{
	return _pDecoder->getTimeliness();
}

UInt32 OmmQos::getRate() const
{
	return _pDecoder->getRate();
}

const EmaString& OmmQos::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmQos::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmQos::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmQos::getDecoder()
{
	return *_pDecoder;
}

bool OmmQos::hasDecoder() const
{
	return true;
}

const Encoder& OmmQos::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmQos::hasEncoder() const
{
	return false;
}
