/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "OmmInt.h"
#include "OmmIntDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace refinitiv::ema::access;

OmmInt::OmmInt() :
 _pDecoder( new ( _space ) OmmIntDecoder() )
{
}

OmmInt::~OmmInt()
{
	_pDecoder->~OmmIntDecoder();
}

DataType::DataTypeEnum OmmInt::getDataType() const
{
	return DataType::IntEnum;
}

Data::DataCode OmmInt::getCode() const
{
	return _pDecoder->getCode();
}

Int64 OmmInt::getInt() const
{
	return _pDecoder->getInt();
}

const EmaString& OmmInt::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmInt::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmInt::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmInt::getDecoder()
{
	return *_pDecoder;
}

bool OmmInt::hasDecoder() const
{
	return true;
}

const Encoder& OmmInt::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmInt::hasEncoder() const
{
	return false;
}
