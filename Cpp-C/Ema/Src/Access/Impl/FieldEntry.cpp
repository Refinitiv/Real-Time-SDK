/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "FieldEntry.h"
#include "ExceptionTranslator.h"
#include "FieldListDecoder.h"
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
#include "RmtesBuffer.h"	// not sure if needed??
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

FieldEntry::FieldEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _toString()
{
}

FieldEntry::~FieldEntry()
{
}

DataType::DataTypeEnum FieldEntry::getLoadType() const
{
	return (*_pLoad)->getDataType();
}

Data::DataCode FieldEntry::getCode() const
{
	return (*_pLoad)->getCode();
}

const Data& FieldEntry::getLoad() const
{
	return **_pLoad;
}

const EmaString& FieldEntry::getName() const
{
	return _pDecoder->getName();
}

Int16 FieldEntry::getFieldId() const
{
	return _pDecoder->getFieldId();
}

Int16 FieldEntry::getRippleTo( Int16 fieldId ) const
{
	return _pDecoder->getRippleTo( fieldId );
}

const EmaString& FieldEntry::getRippleToName(Int16 fieldId) const
{
	return _pDecoder->getRippleToName(fieldId);
}

Int64 FieldEntry::getInt() const
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

UInt64 FieldEntry::getUInt() const
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

const OmmReal& FieldEntry::getReal() const
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

const OmmDate& FieldEntry::getDate() const
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

const OmmDateTime& FieldEntry::getDateTime() const
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

const OmmTime& FieldEntry::getTime() const
{
	if ( (*_pLoad)->getDataType() != DataType::TimeEnum )
	{
		EmaString temp( "Attempt to getTime() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getTime() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmTime&>( **_pLoad );
}

const EmaString& FieldEntry::getAscii() const
{
	if ( (*_pLoad)->getDataType() != DataType::AsciiEnum )
	{
		EmaString temp( "Attempt to getAscii() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getAscii() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAscii&>( **_pLoad ).getAscii();
}

const EmaBuffer& FieldEntry::getBuffer() const
{
	if ( (*_pLoad)->getDataType() != DataType::BufferEnum )
	{
		EmaString temp( "Attempt to getBuffer() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getBuffer() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmBuffer&>( **_pLoad ).getBuffer();
}

float FieldEntry::getFloat() const
{
	if ( (*_pLoad)->getDataType() != DataType::FloatEnum )
	{
		EmaString temp( "Attempt to getFloat() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getFloat() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmFloat&>( **_pLoad ).getFloat();
}

double FieldEntry::getDouble() const
{
	if ( (*_pLoad)->getDataType() != DataType::DoubleEnum )
	{
		EmaString temp( "Attempt to getDouble() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getDouble() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmDouble&>( **_pLoad ).getDouble();
}

const RmtesBuffer& FieldEntry::getRmtes() const
{
	if ( (*_pLoad)->getDataType() != DataType::RmtesEnum )
	{
		EmaString temp( "Attempt to getRmtes() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getRmtes() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmRmtes&>( **_pLoad ).getRmtes();
}

const EmaBuffer& FieldEntry::getUtf8() const
{
	if ( (*_pLoad)->getDataType() != DataType::Utf8Enum )
	{
		EmaString temp( "Attempt to getUtf8() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getUtf8() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmUtf8&>( **_pLoad ).getUtf8();
}

const OmmState& FieldEntry::getState() const
{
	if ( (*_pLoad)->getDataType() != DataType::StateEnum )
	{
		EmaString temp( "Attempt to getState() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getState() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmState&>( **_pLoad );
}

const OmmQos& FieldEntry::getQos() const
{
	if ( (*_pLoad)->getDataType() != DataType::QosEnum )
	{
		EmaString temp( "Attempt to getQos() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getQos() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmQos&>( **_pLoad );
}

UInt16 FieldEntry::getEnum() const
{
	if ( (*_pLoad)->getDataType() != DataType::EnumEnum )
	{
		EmaString temp( "Attempt to getEnum() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getEnum() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmEnum&>( **_pLoad ).getEnum();
}


bool FieldEntry::hasEnumDisplay() const
{
	if  ( ( (*_pLoad)->getDataType() == DataType::EnumEnum ) &&
		( (*_pLoad)->getCode() != Data::BlankEnum ) )
	{
		return _pDecoder->hasEnumDisplay( static_cast<const OmmEnum&>( **_pLoad ).getEnum() );
	}

	return false;
}

const EmaString& FieldEntry::getEnumDisplay() const
{
	if ( (*_pLoad)->getDataType() != DataType::EnumEnum )
	{
		EmaString temp( "Attempt to getEnumDisplay() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ( (*_pLoad)->getCode() == Data::BlankEnum )
	{
		EmaString temp( "Attempt to getEnumDisplay() while entry data is blank." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pDecoder->getEnumDisplay( static_cast<const OmmEnum&>( **_pLoad ).getEnum() );
}

const FieldList& FieldEntry::getFieldList() const
{
	if ( (*_pLoad)->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( **_pLoad );
}

const ElementList& FieldEntry::getElementList() const
{
	if ( (*_pLoad)->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( **_pLoad );
}

const Map& FieldEntry::getMap() const
{
	if ( (*_pLoad)->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( **_pLoad );
}

const Vector& FieldEntry::getVector() const
{
	if ( (*_pLoad)->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( **_pLoad );
}

const Series& FieldEntry::getSeries() const
{
	if ( (*_pLoad)->getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( **_pLoad );
}

const FilterList& FieldEntry::getFilterList() const
{
	if ( (*_pLoad)->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( **_pLoad );
}

const OmmOpaque& FieldEntry::getOpaque() const
{
	if ( (*_pLoad)->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( **_pLoad );
}

const OmmXml& FieldEntry::getXml() const
{
	if ( (*_pLoad)->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( **_pLoad );
}

const OmmAnsiPage& FieldEntry::getAnsiPage() const
{
	if ( (*_pLoad)->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( **_pLoad );
}

const ReqMsg& FieldEntry::getReqMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( **_pLoad );
}

const RefreshMsg& FieldEntry::getRefreshMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( **_pLoad );
}

const UpdateMsg& FieldEntry::getUpdateMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( **_pLoad );
}

const StatusMsg& FieldEntry::getStatusMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( **_pLoad );
}

const PostMsg& FieldEntry::getPostMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getRespMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( **_pLoad );
}

const AckMsg& FieldEntry::getAckMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( **_pLoad );
}

const GenericMsg& FieldEntry::getGenericMsg() const
{
	if ( (*_pLoad)->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( **_pLoad );
}

const OmmArray& FieldEntry::getArray() const
{
	if ( (*_pLoad)->getDataType() != DataType::ArrayEnum )
	{
		EmaString temp( "Attempt to getArray() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
	else if ((*_pLoad)->getCode() == Data::BlankEnum)
	{
		EmaString temp("Attempt to getArray() while entry data is blank.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return static_cast<const OmmArray&>( **_pLoad );
}

const OmmError& FieldEntry::getError() const
{
	if ( (*_pLoad)->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual entry data type is " );
		temp += getDTypeAsString( (*_pLoad)->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( **_pLoad );
}

const EmaString& FieldEntry::toString() const
{
	_toString.clear()
		.append( "FieldEntry " )
		.append( " fid=\"" ).append( _pDecoder->getFieldId() ).append( "\"" )
		.append( " name=\"" ).append( _pDecoder->getName() ).append( "\"" )
		.append( " dataType=\"" ).append( getDTypeAsString( (*_pLoad)->getDataType() ) );

	if ( (*_pLoad)->getDataType() >= DataType::FieldListEnum || (*_pLoad)->getDataType() == DataType::ArrayEnum )
	{
		_toString.append( "\"\n" ).append( (*_pLoad)->toString( 1 ) );
		addIndent( _toString, 0 ).append( "FieldEntryEnd\n" );
	}
	else
		_toString.append( "\" value=\"" ).append( (*_pLoad)->toString() ).append( "\"\n" );

	return _toString;
}
