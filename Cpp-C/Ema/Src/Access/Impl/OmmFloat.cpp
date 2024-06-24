/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmFloat.h"
#include "OmmFloatDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace refinitiv::ema::access;

OmmFloat::OmmFloat() :
 _pDecoder( new ( _space ) OmmFloatDecoder() )
{
}

OmmFloat::~OmmFloat()
{
	_pDecoder->~OmmFloatDecoder();
}

DataType::DataTypeEnum OmmFloat::getDataType() const
{
	return DataType::FloatEnum;
}

Data::DataCode OmmFloat::getCode() const
{
	return _pDecoder->getCode();
}

float OmmFloat::getFloat() const
{
	return _pDecoder->getFloat();
}

const EmaString& OmmFloat::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmFloat::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmFloat::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmFloat::getDecoder()
{
	return *_pDecoder;
}

bool OmmFloat::hasDecoder() const
{
	return true;
}

const Encoder& OmmFloat::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmFloat::hasEncoder() const
{
	return false;
}
