/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ElementEntry.h"
#include "ExceptionTranslator.h"
#include "ElementListDecoder.h"
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
#include "FieldList.h"
#include "ElementList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "FilterList.h"
#include "OmmOpaque.h"
#include "OmmXml.h"
#include "OmmAnsiPage.h"
#include "ReqMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "PostMsg.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "OmmArray.h"
#include "OmmError.h"
#include "OmmRmtes.h"
#include "OmmUtf8.h"
#include "RmtesBuffer.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

ElementEntry::ElementEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _toString()
{
}

ElementEntry::~ElementEntry()
{
}

DataType::DataTypeEnum ElementEntry::getLoadType() const
{
	return (*_pLoad)->getDataType();
}

Data::DataCode ElementEntry::getCode() const
{
	return (*_pLoad)->getCode();
}

const Data& ElementEntry::getLoad() const
{
	return _pDecoder->getLoad();
}

const EmaString& ElementEntry::getName() const
{
	return _pDecoder->getName();
}

Int64 ElementEntry::getInt() const
{
	if ( (*_pLoad)->getDataType() != DataType::IntEnum )
	{
		EmaString temp( "Attempt to getInt() while actual Entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getInt() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmInt&>( **_pLoad ).getInt();
}

UInt64 ElementEntry::getUInt() const
{
	if ( (*_pLoad)->getDataType() != DataType::UIntEnum )
	{
		EmaString temp( "Attempt to getUInt() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getUInt() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmUInt&>( **_pLoad ).getUInt();
}

const OmmReal& ElementEntry::getReal() const
{
	if ( (*_pLoad)->getDataType() != DataType::RealEnum )
	{
		EmaString temp( "Attempt to getReal() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getReal() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmReal&>( **_pLoad );
}

const OmmDate& ElementEntry::getDate() const
{
	if ( (*_pLoad)->getDataType() != DataType::DateEnum )
	{
		EmaString temp( "Attempt to getDate() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDate() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDate&>( **_pLoad );
}

const OmmDateTime& ElementEntry::getDateTime() const
{
	if ( (*_pLoad)->getDataType() != DataType::DateTimeEnum )
	{
		EmaString temp( "Attempt to getDateTime() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDateTime() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDateTime&>( **_pLoad );
}

const OmmTime& ElementEntry::getTime() const
{
	if ( (*_pLoad)->getDataType() != DataType::TimeEnum )
	{
		EmaString temp( "Attempt to getTime() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getTime() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmTime&>( **_pLoad );
}

const EmaString& ElementEntry::getAscii() const
{
	if ( (*_pLoad)->getDataType() != DataType::AsciiEnum )
	{
		EmaString temp( "Attempt to getAscii() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getAscii() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmAscii&>( **_pLoad ).getAscii();
}

const EmaBuffer& ElementEntry::getBuffer() const
{
	if ( (*_pLoad)->getDataType() != DataType::BufferEnum )
	{
		EmaString temp( "Attempt to getBuffer() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getBuffer() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmBuffer&>( **_pLoad ).getBuffer();
}

float ElementEntry::getFloat() const
{
	if ( (*_pLoad)->getDataType() != DataType::FloatEnum )
	{
		EmaString temp( "Attempt to getFloat() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getFloat() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmFloat&>( **_pLoad ).getFloat();
}

double ElementEntry::getDouble() const
{
	if ( (*_pLoad)->getDataType() != DataType::DoubleEnum )
	{
		EmaString temp( "Attempt to getDouble() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDouble() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmDouble&>( **_pLoad ).getDouble();
}

const RmtesBuffer& ElementEntry::getRmtes() const
{
	if ( (*_pLoad)->getDataType() != DataType::RmtesEnum )
	{
		EmaString temp( "Attempt to getRmtes() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getRmtes() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmRmtes&>( **_pLoad ).getRmtes();
}

const EmaBuffer& ElementEntry::getUtf8() const
{
	if ( (*_pLoad)->getDataType() != DataType::Utf8Enum )
	{
		EmaString temp( "Attempt to getUtf8() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getUtf8() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmUtf8&>( **_pLoad ).getUtf8();
}

const OmmState& ElementEntry::getState() const
{
	if ( (*_pLoad)->getDataType() != DataType::StateEnum )
	{
		EmaString temp( "Attempt to getState() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getState() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmState&>( **_pLoad );
}

const OmmQos& ElementEntry::getQos() const
{
	if ( (*_pLoad)->getDataType() != DataType::QosEnum )
	{
		EmaString temp( "Attempt to getQos() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getQos() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmQos&>( **_pLoad );
}

UInt16 ElementEntry::getEnum() const
{
	if ( (*_pLoad)->getDataType() != DataType::EnumEnum )
	{
		EmaString temp( "Attempt to getEnum() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getEnum() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmEnum&>( **_pLoad ).getEnum();
}

const FieldList& ElementEntry::getFieldList() const
{
	if ( (*_pLoad)->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const FieldList&>( **_pLoad );
}

const ElementList& ElementEntry::getElementList() const
{
	if ( (*_pLoad)->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const ElementList&>( **_pLoad );
}

const Map& ElementEntry::getMap() const
{
	if ( (*_pLoad)->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const Map&>( **_pLoad );
}

const Vector& ElementEntry::getVector() const
{
	if ( (*_pLoad)->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const Vector&>( **_pLoad );
}

const Series& ElementEntry::getSeries() const
{
	if ( (*_pLoad)->getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const Series&>( **_pLoad );
}

const FilterList& ElementEntry::getFilterList() const
{
	if ( (*_pLoad)->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const FilterList&>( **_pLoad );
}

const OmmOpaque& ElementEntry::getOpaque() const
{
	if ( (*_pLoad)->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmOpaque&>( **_pLoad );
}

const OmmXml& ElementEntry::getXml() const
{
	if ( (*_pLoad)->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmXml&>( **_pLoad );
}

const OmmAnsiPage& ElementEntry::getAnsiPage() const
{
	if ( (*_pLoad)->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmAnsiPage&>( **_pLoad );
}

const ReqMsg& ElementEntry::getReqMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const ReqMsg&>( **_pLoad );
}

const RefreshMsg& ElementEntry::getRefreshMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const RefreshMsg&>( **_pLoad );
}

const UpdateMsg& ElementEntry::getUpdateMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const UpdateMsg&>( **_pLoad );
}

const StatusMsg& ElementEntry::getStatusMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const StatusMsg&>( **_pLoad );
}

const PostMsg& ElementEntry::getPostMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getRespMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const PostMsg&>( **_pLoad );
}

const AckMsg& ElementEntry::getAckMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const AckMsg&>( **_pLoad );
}

const GenericMsg& ElementEntry::getGenericMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const GenericMsg&>( **_pLoad );
}

const OmmArray& ElementEntry::getArray() const
{
	if ( (*_pLoad)->getDataType() != DataType::ArrayEnum )
	{
		EmaString temp( "Attempt to getArray() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmArray&>( **_pLoad );
}

const OmmError& ElementEntry::getError() const
{
	if ( (*_pLoad)->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmError&>( **_pLoad );
}

const EmaString& ElementEntry::toString() const
{
	_toString.clear()
		.append( "ElementEntry " )
		.append( " name=\"" ).append( _pDecoder->getName() ).append( "\"" )
		.append( " dataType=\"" ).append( getDTypeAsString( (*_pLoad)->getDataType() ) );
	
	if ( (*_pLoad)->getDataType() >= DataType::FieldListEnum || (*_pLoad)->getDataType() == DataType::ArrayEnum )
	{
		_toString.append( "\"\n" ).append( (*_pLoad)->toString( 1 ) );
		addIndent( _toString, 0 ).append( "ElementEntryEnd\n" );
	}
	else
		_toString.append( "\" value=\"" ).append( (*_pLoad)->toString() ).append( "\"\n" );

	return _toString;
}
