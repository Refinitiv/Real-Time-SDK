/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmEnum.h"
#include "OmmEnumDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

OmmEnum::OmmEnum() :
 _pDecoder( new ( _space ) OmmEnumDecoder() )
{
}

OmmEnum::~OmmEnum()
{
	_pDecoder->~OmmEnumDecoder();
}

DataType::DataTypeEnum OmmEnum::getDataType() const
{
	return DataType::EnumEnum;
}

Data::DataCode OmmEnum::getCode() const
{
	return _pDecoder->getCode();
}

UInt16 OmmEnum::getEnum() const
{
	return _pDecoder->getEnum();
}

const EmaString& OmmEnum::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmEnum::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmEnum::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmEnum::getDecoder()
{
	return *_pDecoder;
}

bool OmmEnum::hasDecoder() const
{
	return true;
}

const Encoder& OmmEnum::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmEnum::hasEncoder() const
{
	return false;
}
