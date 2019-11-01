/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Key.h"
#include "ExceptionTranslator.h"
#include "OmmInt.h"
#include "OmmUInt.h"
#include "OmmAscii.h"
#include "OmmEnum.h"
#include "OmmFloat.h"
#include "OmmDouble.h"
#include "OmmDate.h"
#include "OmmDateTime.h"
#include "OmmTime.h"
#include "OmmReal.h"
#include "OmmState.h"
#include "OmmQos.h"
#include "OmmBuffer.h"
#include "OmmRmtes.h"
#include "OmmUtf8.h"
#include "OmmError.h"
#include "RmtesBuffer.h" // not sure if needed
#include "OmmInvalidUsageException.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Key::Key() :
 _pData( 0 )
{
}

Key::~Key()
{
}

const Data& Key::getData() const
{
	return *_pData;
}

DataType::DataTypeEnum Key::getDataType() const
{
	return _pData->getDataType();
}

Int64 Key::getInt() const
{
	if ( _pData->getDataType() != DataType::IntEnum )
	{
		EmaString temp( "Attempt to getInt() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmInt&>( *_pData ).getInt();
}

UInt64 Key::getUInt() const
{
	if ( _pData->getDataType() != DataType::UIntEnum )
	{
		EmaString temp( "Attempt to getUInt() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmUInt&>( *_pData ).getUInt();
}

const OmmReal& Key::getReal() const
{
	if ( _pData->getDataType() != DataType::RealEnum )
	{
		EmaString temp( "Attempt to getReal() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmReal&>( *_pData );
}

const OmmDate& Key::getDate() const
{
	if ( _pData->getDataType() != DataType::DateEnum )
	{
		EmaString temp( "Attempt to getDate() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDate&>( *_pData );
}

const OmmDateTime& Key::getDateTime() const
{
	if ( _pData->getDataType() != DataType::DateTimeEnum )
	{
		EmaString temp( "Attempt to getDateTime() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDateTime&>( *_pData );
}

const OmmTime& Key::getTime() const
{
	if ( _pData->getDataType() != DataType::TimeEnum )
	{
		EmaString temp( "Attempt to getTime() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmTime&>( *_pData );
}

const OmmQos& Key::getQos() const
{
	if ( _pData->getDataType() != DataType::QosEnum )
	{
		EmaString temp( "Attempt to getQos() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmQos&>( *_pData );
}

const OmmState& Key::getState() const
{
	if ( _pData->getDataType() != DataType::StateEnum )
	{
		EmaString temp( "Attempt to getState() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmState&>( *_pData );
}

UInt16 Key::getEnum() const
{
	if ( _pData->getDataType() != DataType::EnumEnum )
	{
		EmaString temp( "Attempt to getEnum() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmEnum&>( *_pData ).getEnum();
}

const EmaBuffer& Key::getBuffer() const
{
	if ( _pData->getDataType() != DataType::BufferEnum )
	{
		EmaString temp( "Attempt to getBuffer() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmBuffer&>( *_pData ).getBuffer();
}

const EmaString& Key::getAscii() const
{
	if ( _pData->getDataType() != DataType::AsciiEnum )
	{
		EmaString temp( "Attempt to getAscii() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAscii&>( *_pData ).getAscii();
}

float Key::getFloat() const
{
	if ( _pData->getDataType() != DataType::FloatEnum )
	{
		EmaString temp( "Attempt to getFloat() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmFloat&>( *_pData ).getFloat();
}

double Key::getDouble() const
{
	if ( _pData->getDataType() != DataType::DoubleEnum )
	{
		EmaString temp( "Attempt to getDouble() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDouble&>( *_pData ).getDouble();
}

const RmtesBuffer& Key::getRmtes() const
{
	if ( _pData->getDataType() != DataType::RmtesEnum )
	{
		EmaString temp( "Attempt to getRmtes() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmRmtes&>( *_pData ).getRmtes();
}

const EmaBuffer& Key::getUtf8() const
{
	if ( _pData->getDataType() != DataType::Utf8Enum )
	{
		EmaString temp( "Attempt to get*_pDataUtf8() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmUtf8&>( *_pData ).getUtf8();
}

const OmmError& Key::getError() const
{
	if ( _pData->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual data type is " );
		temp += getDTypeAsString( _pData->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( *_pData );
}
