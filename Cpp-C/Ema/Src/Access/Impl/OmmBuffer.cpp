/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmBuffer.h"
#include "OmmBufferDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"

#include <new>

using namespace rtsdk::ema::access;

OmmBuffer::OmmBuffer() :
 _pDecoder( new ( _space ) OmmBufferDecoder() )
{
}

OmmBuffer::~OmmBuffer()
{
	_pDecoder->~OmmBufferDecoder();
}

DataType::DataTypeEnum OmmBuffer::getDataType() const
{
	return DataType::BufferEnum;
}

Data::DataCode OmmBuffer::getCode() const
{
	return _pDecoder->getCode();
}

const EmaBuffer& OmmBuffer::getBuffer() const
{
	return _pDecoder->getBuffer();
}

const EmaString& OmmBuffer::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmBuffer::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmBuffer::getAsHex() const
{
	return _pDecoder->getBuffer();
}

Decoder& OmmBuffer::getDecoder()
{
	return *_pDecoder;
}

bool OmmBuffer::hasDecoder() const
{
	return true;
}

const Encoder& OmmBuffer::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmBuffer::hasEncoder() const
{
	return false;
}
