/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmAscii.h"
#include "OmmAsciiDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace rtsdk::ema::access;

OmmAscii::OmmAscii() :
  _pDecoder( new ( _space ) OmmAsciiDecoder() )
{
}

OmmAscii::~OmmAscii()
{
	_pDecoder->~OmmAsciiDecoder();
}

DataType::DataTypeEnum OmmAscii::getDataType() const
{
	return DataType::AsciiEnum;
}

Data::DataCode OmmAscii::getCode() const
{
	return _pDecoder->getCode();
}

const EmaString& OmmAscii::getAscii() const
{
	return _pDecoder->getAscii();
}

const EmaString& OmmAscii::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmAscii::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmAscii::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmAscii::getDecoder()
{
	return *_pDecoder;
}

bool OmmAscii::hasDecoder() const
{
	return true;
}

const Encoder& OmmAscii::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmAscii::hasEncoder() const
{
	return false;
}
