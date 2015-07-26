/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Data.h"

using namespace thomsonreuters::ema::access;

const EmaString ReqMsgString( "ReqMsg" );
const EmaString RefreshMsgString( "RefreshMsg" );
const EmaString UpdateMsgString( "UpdateMsg" );
const EmaString StatusMsgString( "StatusMsg" );
const EmaString PostMsgString( "PostMsg" );
const EmaString AckMsgString( "AckMsg" );
const EmaString GenericMsgString( "GenericMsg" );
const EmaString FieldListString( "FieldList" );
const EmaString ElementListString( "ElementList" );
const EmaString MapString( "Map" );
const EmaString VectorString( "Vector" );
const EmaString SeriesString( "Series" );
const EmaString FilterListString( "FilterList" );
const EmaString OpaqueString( "Opaque" );
const EmaString XmlString( "Xml" );
const EmaString AnsiPageString( "AnsiPage" );
const EmaString OmmArrayString( "OmmArray" );
const EmaString IntString( "Int" );
const EmaString UIntString( "UInt" );
const EmaString RealString( "Real" );
const EmaString FloatString( "Float" );
const EmaString DoubleString( "Double" );
const EmaString DateString( "Date" );
const EmaString TimeString( "Time" );
const EmaString DateTimeString( "DateTime" );
const EmaString QosString( "Qos" );
const EmaString StateString( "State" );
const EmaString EnumString( "Enum" );
const EmaString BufferString( "Buffer" );
const EmaString AsciiString( "Ascii" );
const EmaString Utf8String( "Utf8" );
const EmaString RmtesString( "Rmtes" );
const EmaString NoDataString( "NoData" );
const EmaString ErrorString( "Error" );
EmaString TempDTString;

const EmaString NoCodeString( "NoCode" );
const EmaString BlankString( "Blank" );
EmaString TempDCString;

const EmaString& getDTypeAsString( DataType::DataTypeEnum dType )
{
	switch ( dType )
	{
	case DataType::ReqMsgEnum :
		return ReqMsgString;
	case DataType::RefreshMsgEnum :
		return RefreshMsgString;
	case DataType::StatusMsgEnum :
		return StatusMsgString;
	case DataType::UpdateMsgEnum :
		return UpdateMsgString;
	case DataType::PostMsgEnum :
		return PostMsgString;
	case DataType::AckMsgEnum :
		return AckMsgString;
	case DataType::GenericMsgEnum :
		return GenericMsgString;
	case DataType::FieldListEnum :
		return FieldListString;
	case DataType::ElementListEnum :
		return ElementListString;
	case DataType::MapEnum :
		return MapString;
	case DataType::VectorEnum :
		return VectorString;
	case DataType::SeriesEnum :
		return SeriesString;
	case DataType::FilterListEnum :
		return FilterListString;
	case DataType::OpaqueEnum :
		return OpaqueString;
	case DataType::XmlEnum :
		return XmlString;
	case DataType::AnsiPageEnum :
		return AnsiPageString;
	case DataType::ArrayEnum :
		return OmmArrayString;
	case DataType::IntEnum :
		return IntString;
	case DataType::UIntEnum :
		return UIntString;
	case DataType::RealEnum :
		return RealString;
	case DataType::FloatEnum :
		return FloatString;
	case DataType::DoubleEnum :
		return DoubleString;
	case DataType::DateEnum :
		return DateString;
	case DataType::TimeEnum :
		return TimeString;
	case DataType::DateTimeEnum :
		return DateTimeString;
	case DataType::QosEnum :
		return QosString;
	case DataType::StateEnum :
		return StateString;
	case DataType::EnumEnum :
		return EnumString;
	case DataType::BufferEnum :
		return BufferString;
	case DataType::AsciiEnum :
		return AsciiString;
	case DataType::Utf8Enum :
		return Utf8String;
	case DataType::RmtesEnum :
		return RmtesString;
	case DataType::NoDataEnum :
		return NoDataString;
	case DataType::ErrorEnum :
		return ErrorString;
	default :
		return TempDTString.set( "Unknown DataType value " ).append( (UInt64)dType );
	}
}

Data::Data() 
{
}

Data::~Data()
{
}

const EmaString& Data::getCodeAsString() const
{
	switch ( getCode() )
	{
	case NoCodeEnum :
		return NoCodeString;
	case BlankEnum :
		return BlankString;
	default :
		return TempDCString.set( "Unknown DataCode value " ).append( (Int64)getCode() );
	}
}

Data::operator const char* () const
{
	return toString().c_str();
}
