/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmUInt.h"
#include "OmmUIntDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

OmmUInt::OmmUInt() :
 _pDecoder( new ( _space ) OmmUIntDecoder() )
{
}

OmmUInt::~OmmUInt()
{
	_pDecoder->~OmmUIntDecoder();
}

DataType::DataTypeEnum OmmUInt::getDataType() const
{
	return DataType::UIntEnum;
}

Data::DataCode OmmUInt::getCode() const
{
	return _pDecoder->getCode();
}

UInt64 OmmUInt::getUInt() const
{
	return _pDecoder->getUInt();
}

const EmaString& OmmUInt::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmUInt::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmUInt::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmUInt::getDecoder()
{
	return *_pDecoder;
}

const Encoder& OmmUInt::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}
