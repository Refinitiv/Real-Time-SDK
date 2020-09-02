/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUtf8.h"
#include "OmmUtf8Decoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace rtsdk::ema::access;

OmmUtf8::OmmUtf8() :
  _pDecoder( new ( _space ) OmmUtf8Decoder() )
{
}

OmmUtf8::~OmmUtf8()
{
	_pDecoder->~OmmUtf8Decoder();
}

DataType::DataTypeEnum OmmUtf8::getDataType() const
{
	return DataType::Utf8Enum;
}

Data::DataCode OmmUtf8::getCode() const
{
	return _pDecoder->getCode();
}

const EmaBuffer& OmmUtf8::getUtf8() const
{
	return _pDecoder->getUtf8();
}

const EmaString& OmmUtf8::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmUtf8::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmUtf8::getAsHex() const
{
	return _pDecoder->getUtf8();
}

Decoder& OmmUtf8::getDecoder()
{
	return *_pDecoder;
}

bool OmmUtf8::hasDecoder() const
{
	return true;
}

const Encoder& OmmUtf8::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmUtf8::hasEncoder() const
{
	return false;
}
