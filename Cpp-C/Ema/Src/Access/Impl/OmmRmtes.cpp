/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmRmtes.h"
#include "OmmRmtesDecoder.h"
#include <new>

using namespace rtsdk::ema::access;

OmmRmtes::OmmRmtes() :
 _pDecoder( new ( _space ) OmmRmtesDecoder() )
{
}

OmmRmtes::~OmmRmtes()
{
	_pDecoder->~OmmRmtesDecoder();
}

DataType::DataTypeEnum OmmRmtes::getDataType() const
{
	return DataType::RmtesEnum;
}

Data::DataCode OmmRmtes::getCode() const
{
	return _pDecoder->getCode();
}

const RmtesBuffer& OmmRmtes::getRmtes() const
{
	return _pDecoder->getRmtes();
}

const EmaString& OmmRmtes::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmRmtes::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmRmtes::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmRmtes::getDecoder()
{
	return *_pDecoder;
}

bool OmmRmtes::hasDecoder() const
{
	return true;
}

Decoder& OmmRmtes::setDecoder( Decoder& decoder )
{
	_pDecoder = static_cast<OmmRmtesDecoder*>( &decoder );
	return *_pDecoder;
}

const Encoder& OmmRmtes::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmRmtes::hasEncoder() const
{
	return false;
}
