/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmDateTime.h"
#include "OmmDateTimeDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

OmmDateTime::OmmDateTime() :
 _pDecoder( new ( _space ) OmmDateTimeDecoder() )
{
}

OmmDateTime::~OmmDateTime()
{
	_pDecoder->~OmmDateTimeDecoder();
}

DataType::DataTypeEnum OmmDateTime::getDataType() const
{
	return DataType::DateTimeEnum;
}

Data::DataCode OmmDateTime::getCode() const
{
	return _pDecoder->getCode();
}

UInt16 OmmDateTime::getYear() const
{
	return _pDecoder->getYear();
}

UInt8 OmmDateTime::getMonth() const
{
	return _pDecoder->getMonth();
}

UInt8 OmmDateTime::getDay() const
{
	return _pDecoder->getDay();
}

UInt8 OmmDateTime::getHour() const
{
	return _pDecoder->getHour();
}

UInt8 OmmDateTime::getMinute() const
{
	return _pDecoder->getMinute();
}

UInt8 OmmDateTime::getSecond() const
{
	return _pDecoder->getSecond();
}

UInt16 OmmDateTime::getMillisecond() const
{
	return _pDecoder->getMillisecond();
}

UInt16 OmmDateTime::getMicrosecond() const
{
	return _pDecoder->getMicrosecond();
}

UInt16 OmmDateTime::getNanosecond() const
{
	return _pDecoder->getNanosecond();
}

const EmaString& OmmDateTime::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmDateTime::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmDateTime::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmDateTime::getDecoder()
{
	return *_pDecoder;
}

bool OmmDateTime::hasDecoder() const
{
	return true;
}

const Encoder& OmmDateTime::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmDateTime::hasEncoder() const
{
	return false;
}
