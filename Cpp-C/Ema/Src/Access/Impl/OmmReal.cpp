/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmReal.h"
#include "OmmRealDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

const EmaString ExponentNeg14String( "Power of -14. Stringeration value is 0" );
const EmaString ExponentNeg13String( "Power of -13" );
const EmaString ExponentNeg12String( "Power of -12" );
const EmaString ExponentNeg11String( "Power of -11" );
const EmaString ExponentNeg10String( "Power of -10" );
const EmaString ExponentNeg9String( "Power of -9" );
const EmaString ExponentNeg8String( "Power of -8" );
const EmaString ExponentNeg7String( "Power of -7" );
const EmaString ExponentNeg6String( "Power of -6" );
const EmaString ExponentNeg5String( "Power of -5" );
const EmaString ExponentNeg4String( "Power of -4" );
const EmaString ExponentNeg3String( "Power of -3" );
const EmaString ExponentNeg2String( "Power of -2" );
const EmaString ExponentNeg1String( "Power of -1" );
const EmaString Exponent0String( "Power of 0" );
const EmaString ExponentPos1String( "Power of 1" );
const EmaString ExponentPos2String( "Power of 2" );
const EmaString ExponentPos3String( "Power of 3" );
const EmaString ExponentPos4String( "Power of 4" );
const EmaString ExponentPos5String( "Power of 5" );
const EmaString ExponentPos6String( "Power of 6" );
const EmaString ExponentPos7String( "Power of 7" );
const EmaString Divisor1String( "Divisor of 1" );
const EmaString Divisor2String( "Divisor of 2" );
const EmaString Divisor4String( "Divisor of 4" );
const EmaString Divisor8String( "Divisor of 8" );
const EmaString Divisor16String( "Divisor of 16" );
const EmaString Divisor32String( "Divisor of 32" );
const EmaString Divisor64String( "Divisor of 64" );
const EmaString Divisor128String( "Divisor of 128" );
const EmaString Divisor256String( "Divisor of 256" );
const EmaString InfinityString( "Inf" );
const EmaString NegativeInfinityString( "-Inf" );
const EmaString NanString( "NaN" );
EmaString TempMTString;

const EmaString& getMTypeAsString( OmmReal::MagnitudeType mType )
{
	switch ( mType )
	{
	case OmmReal::ExponentNeg14Enum :
		return ExponentNeg14String;
	case OmmReal::ExponentNeg13Enum :
		return ExponentNeg13String;
	case OmmReal::ExponentNeg12Enum :
		return ExponentNeg12String;
	case OmmReal::ExponentNeg11Enum :
		return ExponentNeg11String;
	case OmmReal::ExponentNeg10Enum :
		return ExponentNeg10String;
	case OmmReal::ExponentNeg9Enum :
		return ExponentNeg9String;
	case OmmReal::ExponentNeg8Enum :
		return ExponentNeg8String;
	case OmmReal::ExponentNeg7Enum :
		return ExponentNeg7String;
	case OmmReal::ExponentNeg6Enum :
		return ExponentNeg6String;
	case OmmReal::ExponentNeg5Enum :
		return ExponentNeg5String;
	case OmmReal::ExponentNeg4Enum :
		return ExponentNeg4String;
	case OmmReal::ExponentNeg3Enum :
		return ExponentNeg3String;
	case OmmReal::ExponentNeg2Enum :
		return ExponentNeg2String;
	case OmmReal::ExponentNeg1Enum :
		return ExponentNeg1String;
	case OmmReal::Exponent0Enum :
		return Exponent0String;
	case OmmReal::ExponentPos1Enum :
		return ExponentPos1String;
	case OmmReal::ExponentPos2Enum :
		return ExponentPos2String;
	case OmmReal::ExponentPos3Enum :
		return ExponentPos3String;
	case OmmReal::ExponentPos4Enum :
		return ExponentPos4String;
	case OmmReal::ExponentPos5Enum :
		return ExponentPos5String;
	case OmmReal::ExponentPos6Enum :
		return ExponentPos6String;
	case OmmReal::ExponentPos7Enum :
		return ExponentPos7String;
	case OmmReal::Divisor1Enum :
		return Divisor1String;
	case OmmReal::Divisor2Enum :
		return Divisor2String;
	case OmmReal::Divisor4Enum :
		return Divisor4String;
	case OmmReal::Divisor8Enum :
		return Divisor8String;
	case OmmReal::Divisor16Enum :
		return Divisor16String;
	case OmmReal::Divisor32Enum :
		return Divisor32String;
	case OmmReal::Divisor64Enum :
		return Divisor64String;
	case OmmReal::Divisor128Enum :
		return Divisor128String;
	case OmmReal::Divisor256Enum :
		return Divisor256String;
	case OmmReal::InfinityEnum :
		return InfinityString;
	case OmmReal::NegInfinityEnum :
		return NegativeInfinityString;
	case OmmReal::NotANumberEnum :
		return NanString;
	default :
		return TempMTString.set( "Unknown MagnitudeType value " ).append( (Int64)mType );
	}
}

OmmReal::OmmReal() :
 _pDecoder( new ( _space ) OmmRealDecoder() )
{
}

OmmReal::~OmmReal()
{
	_pDecoder->~OmmRealDecoder();
}

const EmaString& OmmReal::getMagnitudeTypeAsString() const
{
	return getMTypeAsString( getMagnitudeType() );
}

DataType::DataTypeEnum OmmReal::getDataType() const
{
	return DataType::RealEnum;
}

Data::DataCode OmmReal::getCode() const
{
	return _pDecoder->getCode();
}

Int64 OmmReal::getMantissa() const
{
	return _pDecoder->getMantissa();
}

OmmReal::MagnitudeType OmmReal::getMagnitudeType() const
{
	return _pDecoder->getMagnitudeType();
}

const EmaString& OmmReal::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmReal::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmReal::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmReal::getDecoder()
{
	return *_pDecoder;
}

bool OmmReal::hasDecoder() const
{
	return true;
}

double OmmReal::getAsDouble() const
{
	return _pDecoder->toDouble();
}

const Encoder& OmmReal::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmReal::hasEncoder() const
{
	return false;
}
