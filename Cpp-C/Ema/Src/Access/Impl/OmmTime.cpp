/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmTime.h"
#include "OmmTimeDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace rtsdk::ema::access;

OmmTime::OmmTime() :
 _pDecoder( new ( _space ) OmmTimeDecoder() )
{
}

OmmTime::~OmmTime()
{
	_pDecoder->~OmmTimeDecoder();
}

DataType::DataTypeEnum OmmTime::getDataType() const
{
	return DataType::TimeEnum;
}

Data::DataCode OmmTime::getCode() const
{
	return _pDecoder->getCode();
}

UInt8 OmmTime::getHour() const
{
	return _pDecoder->getHour();
}

UInt8 OmmTime::getMinute() const
{
	return _pDecoder->getMinute();
}

UInt8 OmmTime::getSecond() const
{
	return _pDecoder->getSecond();
}

UInt16 OmmTime::getMillisecond() const
{
	return _pDecoder->getMillisecond();
}

UInt16 OmmTime::getMicrosecond() const
{
	return _pDecoder->getMicrosecond();
}

UInt16 OmmTime::getNanosecond() const
{
	return _pDecoder->getNanosecond();
}

const EmaString& OmmTime::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmTime::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmTime::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmTime::getDecoder()
{
	return *_pDecoder;
}

bool OmmTime::hasDecoder() const
{
	return true;
}

const Encoder& OmmTime::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmTime::hasEncoder() const
{
	return false;
}
