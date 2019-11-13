/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmArrayEntry.h"
#include "ExceptionTranslator.h"
#include "OmmArrayDecoder.h"
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
#include "OmmError.h"
#include "OmmRmtes.h"
#include "OmmUtf8.h"
#include "RmtesBuffer.h" // not sure if needed
#include "OmmInvalidUsageException.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

OmmArrayEntry::OmmArrayEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _toString()
{
}

OmmArrayEntry::~OmmArrayEntry()
{
}

DataType::DataTypeEnum OmmArrayEntry::getLoadType() const
{
	return _pLoad->getDataType();
}

Data::DataCode OmmArrayEntry::getCode() const
{
	return _pLoad->getCode();
}

const Data& OmmArrayEntry::getLoad() const
{
	return *_pLoad;
}

Int64 OmmArrayEntry::getInt() const
{
	if ( _pLoad->getDataType() != DataType::IntEnum )
	{
		EmaString temp( "Attempt to getInt() while actual Entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getInt() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmInt&>( *_pLoad ).getInt();
}

UInt64 OmmArrayEntry::getUInt() const
{
	if ( _pLoad->getDataType() != DataType::UIntEnum )
	{
		EmaString temp( "Attempt to getUInt() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getUInt() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmUInt&>( *_pLoad ).getUInt();
}

const OmmReal& OmmArrayEntry::getReal() const
{
	if ( _pLoad->getDataType() != DataType::RealEnum )
	{
		EmaString temp( "Attempt to getReal() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getReal() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmReal&>( *_pLoad );
}

const OmmDate& OmmArrayEntry::getDate() const
{
	if ( _pLoad->getDataType() != DataType::DateEnum )
	{
		EmaString temp( "Attempt to getDate() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDate() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDate&>( *_pLoad );
}

const OmmDateTime& OmmArrayEntry::getDateTime() const
{
	if ( _pLoad->getDataType() != DataType::DateTimeEnum )
	{
		EmaString temp( "Attempt to getDateTime() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDateTime() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDateTime&>( *_pLoad );
}

const OmmTime& OmmArrayEntry::getTime() const
{
	if ( _pLoad->getDataType() != DataType::TimeEnum )
	{
		EmaString temp( "Attempt to getTime() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getTime() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmTime&>( *_pLoad );
}

const EmaString& OmmArrayEntry::getAscii() const
{
	if ( _pLoad->getDataType() != DataType::AsciiEnum )
	{
		EmaString temp( "Attempt to getAscii() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getAscii() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAscii&>( *_pLoad ).getAscii();
}

const EmaBuffer& OmmArrayEntry::getBuffer() const
{
	if ( _pLoad->getDataType() != DataType::BufferEnum )
	{
		EmaString temp( "Attempt to getBuffer() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getBuffer() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmBuffer&>( *_pLoad ).getBuffer();
}

float OmmArrayEntry::getFloat() const
{
	if ( _pLoad->getDataType() != DataType::FloatEnum )
	{
		EmaString temp( "Attempt to getFloat() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getFloat() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmFloat&>( *_pLoad ).getFloat();
}

double OmmArrayEntry::getDouble() const
{
	if ( _pLoad->getDataType() != DataType::DoubleEnum )
	{
		EmaString temp( "Attempt to getDouble() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDouble() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDouble&>( *_pLoad ).getDouble();
}

const RmtesBuffer& OmmArrayEntry::getRmtes() const
{
	if ( _pLoad->getDataType() != DataType::RmtesEnum )
	{
		EmaString temp( "Attempt to getRmtes() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getRmtes() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmRmtes&>( *_pLoad ).getRmtes();
}

const EmaBuffer& OmmArrayEntry::getUtf8() const
{
	if ( _pLoad->getDataType() != DataType::Utf8Enum )
	{
		EmaString temp( "Attempt to getUtf8() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getUtf8() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmUtf8&>( *_pLoad ).getUtf8();
}

const OmmState& OmmArrayEntry::getState() const
{
	if ( _pLoad->getDataType() != DataType::StateEnum )
	{
		EmaString temp( "Attempt to getState() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getState() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmState&>( *_pLoad );
}

UInt16 OmmArrayEntry::getEnum() const
{
	if ( _pLoad->getDataType() != DataType::EnumEnum )
	{
		EmaString temp( "Attempt to getEnum() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getEnum() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmEnum&>( *_pLoad ).getEnum();
}

const OmmQos& OmmArrayEntry::getQos() const
{
	if ( _pLoad->getDataType() != DataType::QosEnum )
	{
		EmaString temp( "Attempt to getQos() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( _pLoad->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getQos() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmQos&>( *_pLoad );
}

const OmmError& OmmArrayEntry::getError() const
{
	if ( _pLoad->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( *_pLoad );
}

const EmaString& OmmArrayEntry::toString() const
{
	return _toString.clear()
		.append( "OmmArrayEntry " )
		.append( " dataType=\"" ).append( getDTypeAsString( _pDecoder->getLoad().getDataType() ) ).append( "\"" )
		.append( " value=\"" ).append( _pLoad->toString() ).append( "\"\n" );
}
