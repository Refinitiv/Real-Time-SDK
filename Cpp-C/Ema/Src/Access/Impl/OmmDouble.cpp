/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmDouble.h"
#include "OmmDoubleDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace refinitiv::ema::access;

OmmDouble::OmmDouble() :
 _pDecoder( new ( _space ) OmmDoubleDecoder() )
{
}

OmmDouble::~OmmDouble()
{
	_pDecoder->~OmmDoubleDecoder();
}

DataType::DataTypeEnum OmmDouble::getDataType() const
{
	return DataType::DoubleEnum;
}

Data::DataCode OmmDouble::getCode() const
{
	return _pDecoder->getCode();
}

double OmmDouble::getDouble() const
{
	return _pDecoder->getDouble();
}

const EmaString& OmmDouble::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmDouble::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmDouble::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmDouble::getDecoder()
{
	return *_pDecoder;
}

bool OmmDouble::hasDecoder() const
{
	return true;
}

const Encoder& OmmDouble::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmDouble::hasEncoder() const
{
	return false;
}
