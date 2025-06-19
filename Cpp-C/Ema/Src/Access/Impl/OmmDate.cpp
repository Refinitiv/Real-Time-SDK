/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmDate.h"
#include "OmmDateDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace refinitiv::ema::access;

OmmDate::OmmDate() :
 _pDecoder( new ( _space ) OmmDateDecoder() )
{
}

OmmDate::~OmmDate()
{
	_pDecoder->~OmmDateDecoder();
}

DataType::DataTypeEnum OmmDate::getDataType() const
{
	return DataType::DateEnum;
}

Data::DataCode OmmDate::getCode() const
{
	return _pDecoder->getCode();
}

UInt16 OmmDate::getYear() const
{
	return _pDecoder->getYear();
}

UInt8 OmmDate::getMonth() const
{
	return _pDecoder->getMonth();
}

UInt8 OmmDate::getDay() const
{
	return _pDecoder->getDay();
}

const EmaString& OmmDate::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmDate::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmDate::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmDate::getDecoder()
{
	return *_pDecoder;
}

bool OmmDate::hasDecoder() const
{
	return true;
}

const Encoder& OmmDate::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmDate::hasEncoder() const
{
	return false;
}
