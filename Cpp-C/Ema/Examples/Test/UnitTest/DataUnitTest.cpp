/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Access/Impl/DirectoryServiceStore.h"

using namespace refinitiv::ema::access;
using namespace std;

extern void testHeader(const EmaString& title)
{
	cout << endl
		<< "******************************************************************************"
		<< endl << endl
		<< title
		<< endl << endl;
}

int checkResultCalls(0);

class MyEnvironment : public ::testing::Environment {
public:
  virtual void TearDown() {
    if (checkResultCalls)
      cout << "checkResult called " << checkResultCalls
	   << (checkResultCalls == 1 ? " time" : " times")
	   << "; checkResult calls should be replaced with equivalent google test macros" << endl;
  }
};

void decodeData( const Data& data, EmaString& outText );

void checkResult( bool result, const EmaString& description )
{
  ++checkResultCalls;
  EXPECT_TRUE( result ) << description;
  return;
}

void decodeArray( const OmmArray& ar, EmaString& outText )
{
	while ( ar.forth() )
	{
		const OmmArrayEntry& ae = ar.getEntry();
		if ( ae.getCode() == Data::BlankEnum )
			outText.append( "blank entry\n" );
		else
			decodeData( ae.getLoad(), outText );
	}
}

void decodeFieldList( const FieldList& fl, EmaString& outText )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		if ( fe.getCode() == Data::BlankEnum )
			outText.append( "blank entry\n" );
		else
			decodeData( fe.getLoad(), outText );
	}
}

void decodeElementList( const ElementList& el, EmaString& outText )
{
	while ( el.forth() )
	{
		const ElementEntry& ee = el.getEntry();

		if ( ee.getCode() == Data::BlankEnum )
			outText.append( "blank entry\n" );
		else
			decodeData( ee.getLoad(), outText );
	}
}

void decodeFilterList( const FilterList& fl, EmaString& outText )
{
	while ( fl.forth() )
	{
		const FilterEntry& fe = fl.getEntry();

		decodeData( fe.getLoad(), outText );
	}
}

void decodeMap( const Map& map, EmaString& outText )
{
	decodeData( map.getSummaryData().getData(), outText );

	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		decodeData( me.getLoad(), outText );
	}
}

void decodeSeries( const Series& s, EmaString& outText )
{
	while ( s.forth() )
	{
		const SeriesEntry& se = s.getEntry();

		decodeData( se.getLoad(), outText );
	}
}

void decodeData( const Data& data, EmaString& outText )
{
	switch ( data.getDataType() )
	{
	case DataType::IntEnum :
		outText.append( "Int: " ).append( static_cast<const OmmInt&>( data ).getInt() ).append( "\n" );
		break;
	case DataType::UIntEnum :
		outText.append( "UInt: " ).append( static_cast<const OmmUInt&>( data ).getUInt() ).append( "\n" );
		break;
	case DataType::AsciiEnum :
		outText.append( "Ascii: " ).append( static_cast<const OmmAscii&>( data ).getAscii() ).append( "\n" );
		break;
	case DataType::BufferEnum :
		outText.append( "Buffer: " ).append( static_cast<const OmmBuffer&>( data ).getBuffer() ).append( "\n" );
		break;
	case DataType::RealEnum :
		outText.append( "Real: " ).append( static_cast<const OmmReal&>( data ).getAsDouble() ).append( "\n" );
		break;
	case DataType::DateEnum :
		outText.append( "Date: " ).append( static_cast<const OmmDate&>( data ).toString() ).append( "\n" );
		break;
	case DataType::DateTimeEnum :
		outText.append( "DateTime: " ).append( static_cast<const OmmDateTime&>( data ).toString() ).append( "\n" );
		break;
	case DataType::TimeEnum :
		outText.append( "Time: " ).append( static_cast<const OmmTime&>( data ).toString() ).append( "\n" );
		break;
	case DataType::FloatEnum :
		outText.append( "Float: " ).append( static_cast<const OmmFloat&>( data ).getFloat() ).append( "\n" );
		break;
	case DataType::DoubleEnum :
		outText.append( "Double: " ).append( static_cast<const OmmDouble&>( data ).getDouble() ).append( "\n" );
		break;
	case DataType::RmtesEnum :
//				outText.append( "Rmtes: " ).append( ae.getRmtes().cs_buf() ).append( "\n" );
		break;
	case DataType::Utf8Enum :
//				outText.append( "Utf8: " ).append( ae.getUtf8().cs_buf() ).append( "\n" );
		break;
	case DataType::EnumEnum :
		outText.append( "Enum: " ).append( static_cast<const OmmEnum&>( data ).getEnum() ).append( "\n" );
		break;
	case DataType::StateEnum :
		outText.append( "State: " ).append( static_cast<const OmmState&>( data ).toString() ).append( "\n" );
		break;
	case DataType::QosEnum :
		outText.append( "Qos: " ).append( static_cast<const OmmQos&>( data ).toString() ).append( "\n" );
		break;
	case DataType::ArrayEnum :
		outText.append( "Array:\n" );
		decodeArray( static_cast<const OmmArray&>( data ), outText );
		break;
	case DataType::FieldListEnum :
		outText.append( "FieldList:\n" );
		decodeFieldList( static_cast<const FieldList&>( data ), outText );
		break;
	case DataType::ElementListEnum :
		outText.append( "ElementList:\n" );
		decodeElementList( static_cast<const ElementList&>( data ), outText );
		break;
	case DataType::FilterListEnum :
		outText.append( "FilterList:\n" );
		decodeFilterList( static_cast<const FilterList&>( data ), outText );
		break;
	case DataType::MapEnum :
		outText.append( "Map:\n" );
		decodeMap( static_cast<const Map&>( data ), outText );
		break;
	case DataType::SeriesEnum :
		outText.append( "Series:\n" );
		decodeSeries( static_cast<const Series&>( data ), outText );
		break;
	case DataType::NoDataEnum :
		outText.append( "NoData" );
		break;
	default :
		outText.append( "unknown data type " ).append( data.getDataType() ).append( "\n" );
		break;
	}
}

void decode( const OmmArray& ar, EmaString& outText )
{
	if ( ar.hasFixedWidth() )
		outText.append( "Fixed Width: " ).append( ar.getFixedWidth() ).append( "\n" );

	while ( ar.forth() )
	{
		const OmmArrayEntry& ae = ar.getEntry();

		if ( ae.getCode() == Data::BlankEnum )
			outText.append( "blank entry\n" );
		else
			switch ( ae.getLoadType() )
			{
			case DataType::IntEnum :
				outText.append( "Int: " ).append( ae.getInt() ).append( "\n" );
				break;
			case DataType::UIntEnum :
				outText.append( "UInt: " ).append( ae.getUInt() ).append( "\n" );
				break;
			case DataType::AsciiEnum :
				outText.append( "Ascii: " ).append( ae.getAscii() ).append( "\n" );
				break;
			case DataType::BufferEnum :
				outText.append( "Buffer: " ).append( ae.getBuffer() ).append( "\n" );
				break;
			case DataType::RealEnum :
				outText.append( "Real: " ).append( ae.getReal().getAsDouble() ).append( "\n" );
				break;
			case DataType::DateEnum :
				outText.append( "Date: " ).append( ae.getDate().toString() ).append( "\n" );
				break;
			case DataType::DateTimeEnum :
				outText.append( "DateTime: " ).append( ae.getDateTime().toString() ).append( "\n" );
				break;
			case DataType::TimeEnum :
				outText.append( "Time: " ).append( ae.getTime().toString() ).append( "\n" );
				break;
			case DataType::FloatEnum :
				outText.append( "Float: " ).append( ae.getFloat() ).append( "\n" );
				break;
			case DataType::DoubleEnum :
				outText.append( "Double: " ).append( ae.getDouble() ).append( "\n" );
				break;
			case DataType::RmtesEnum :
//				outText.append( "Rmtes: " ).append( ae.getRmtes().cs_buf() ).append( "\n" );
				break;
			case DataType::Utf8Enum :
//				outText.append( "Utf8: " ).append( ae.getUtf8().cs_buf() ).append( "\n" );
				break;
			case DataType::EnumEnum :
				outText.append( "Enum: " ).append( ae.getEnum() ).append( "\n" );
				break;
			case DataType::StateEnum :
				outText.append( "State: " ).append( ae.getState().toString() ).append( "\n" );
				break;
			case DataType::QosEnum :
				outText.append( "Qos: " ).append( ae.getQos().toString() ).append( "\n" );
				break;
			case DataType::ErrorEnum :
				outText.append( "Error: " ).append( ae.getError().getErrorCodeAsString() ).append( "\n" );
				break;
			default :
				outText.append( "array entry with unknown data type " ).append( ae.getLoad().getDataType() ).append( "\n" );
				break;
			}
	}
}

//tesing with EmaBuffer::operator==
void decodeBufferCheckValues( const OmmArray& ar, EmaString& outText )
{
	int arrayEntryCount = 0;
	while ( ar.forth() )
	{
		const OmmArrayEntry& ae = ar.getEntry();

		if ( ae.getCode() == Data::BlankEnum )
			outText.append( "blank entry\n" );
		else
			switch ( ae.getLoadType() )
			{
			case DataType::BufferEnum :
			{
				arrayEntryCount++;
				char* s;
				int length = 0;
				if ( arrayEntryCount == 1 )
				{
					//1st array entry
				        s = const_cast<char*>( "ABC" );
					length = 3;
				}
				else if ( arrayEntryCount == 2 )
				{
					//2nd array entry
				        s = const_cast<char*>( "DEFGH" );
					length = 5;
				}
				else
				{
					//3rd array entry
				        s = const_cast<char*>( "KLMNOPQRS" );
					length = 9;
				}

				if ( EmaBuffer( s, length ) == ae.getBuffer() )
					outText.append( "Buffer: " ).append( s ).append( "\n" );
			}
			break;
			default :
				break;
			}
	}
}


void perfDecode( const OmmArray& ar )
{
	while ( ar.forth() )
	{
		const OmmArrayEntry& ae = ar.getEntry();

		if ( ae.getCode() != Data::BlankEnum )
			switch ( ae.getLoadType() )
			{
			case DataType::IntEnum :
			{
				Int64 value = ae.getInt();
			}
			break;
			case DataType::UIntEnum :
			{
				UInt64 value = ae.getUInt();
			}
			break;
			case DataType::AsciiEnum :
			{
				const EmaString& text = ae.getAscii();
			}
			break;
			case DataType::BufferEnum :
			{
				const EmaBuffer& text = ae.getBuffer();
			}
			break;
			case DataType::RealEnum :
			{
				const OmmReal& r = ae.getReal();
				OmmReal::MagnitudeType mt = r.getMagnitudeType();
				Int64 m = r.getMantissa();
			}
			break;
			case DataType::DateEnum :
			{
				const OmmDate& d = ae.getDate();
				UInt16 year = d.getYear();
				UInt8 month = d.getMonth();
				UInt8 day = d.getDay();
			}
			break;
			case DataType::DateTimeEnum :
			{
				const OmmDateTime& dt = ae.getDateTime();
				UInt16 year = dt.getYear();
				UInt8 month = dt.getMonth();
				UInt8 day = dt.getDay();
				UInt8 hour = dt.getHour();
				UInt8 minute = dt.getMinute();
				UInt8 second = dt.getSecond();
				UInt16 millisecond = dt.getMillisecond();
			}
			break;
			case DataType::TimeEnum :
			{
				const OmmTime& dt = ae.getTime();
				UInt8 hour = dt.getHour();
				UInt8 minute = dt.getMinute();
				UInt8 second = dt.getSecond();
				UInt16 millisecond = dt.getMillisecond();
			}
			break;
			case DataType::FloatEnum :
			{
				float value = ae.getFloat();
			}
			break;
			case DataType::DoubleEnum :
			{
				double value = ae.getDouble();
			}
			break;
			case DataType::RmtesEnum :
			{
				const RmtesBuffer& text = ae.getRmtes();
			}
			break;
			case DataType::Utf8Enum :
			{
				const EmaBuffer& text = ae.getUtf8();
			}
			break;
			case DataType::EnumEnum :
			{
				UInt16 en = ae.getEnum();
			}
			break;
			case DataType::StateEnum :
			{
				const OmmState& state = ae.getState();
				state.getStreamState();
				state.getDataState();
				state.getStatusCode();
				const EmaString& text = state.getStatusText();
			}
			break;
			case DataType::QosEnum :
			{
				const OmmQos& qos = ae.getQos();
				UInt16 r = qos.getRate();
				UInt16 timeliness = qos.getTimeliness();
			}
			break;

			default :
				break;
			}
	}
}

void decode( const FieldList& fl, EmaString& outText )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		if ( fe.getCode() == Data::BlankEnum )
			outText.append( "blank entry\n" );
		else
			switch ( fe.getLoadType() )
			{
			case DataType::IntEnum :
				outText.append( "Int: " ).append( fe.getInt() ).append( "\n" );
				break;
			case DataType::UIntEnum :
				outText.append( "UInt: " ).append( fe.getUInt() ).append( "\n" );
				break;
			case DataType::AsciiEnum :
				outText.append( "Ascii: " ).append( fe.getAscii() ).append( "\n" );
				break;
			case DataType::BufferEnum :
				outText.append( "Buffer: " ).append( fe.getBuffer() ).append( "\n" );
				break;
			case DataType::RealEnum :
				outText.append( "Real: " ).append( fe.getReal().getAsDouble() ).append( "\n" );
				break;
			case DataType::DateEnum :
				outText.append( "Date: " ).append( fe.getDate().toString() ).append( "\n" );
				break;
			case DataType::DateTimeEnum :
				outText.append( "DateTime: " ).append( fe.getDateTime().toString() ).append( "\n" );
				break;
			case DataType::TimeEnum :
				outText.append( "Time: " ).append( fe.getTime().toString() ).append( "\n" );
				break;
			case DataType::FloatEnum :
				outText.append( "Float: " ).append( fe.getFloat() ).append( "\n" );
				break;
			case DataType::DoubleEnum :
				outText.append( "Double: " ).append( fe.getDouble() ).append( "\n" );
				break;
			case DataType::RmtesEnum :
				break;
			case DataType::Utf8Enum :
				break;
			case DataType::EnumEnum :
				outText.append( "Enum: " ).append( fe.getEnum() ).append( "\n" );
				break;
			case DataType::StateEnum :
				outText.append( "State: " ).append( fe.getState().toString() ).append( "\n" );
				break;
			case DataType::QosEnum :
				outText.append( "Qos: " ).append( fe.getQos().toString() ).append( "\n" );
				break;
			case DataType::ArrayEnum :
				outText.append( "OmmArray: " ).append( fe.getArray().toString() ).append( "\n" );
				break;
			case DataType::FieldListEnum :
				outText.append( "FieldList: " ).append( fe.getFieldList().toString() ).append( "\n" );
				break;
			case DataType::ElementListEnum :
				outText.append( "ElementList: " ).append( fe.getElementList().toString() ).append( "\n" );
				break;
			case DataType::MapEnum :
				outText.append( "Map: " ).append( fe.getMap().toString() ).append( "\n" );
				break;
			case DataType::VectorEnum :
				outText.append( "Vector: " ).append( fe.getVector().toString() ).append( "\n" );
				break;
			case DataType::SeriesEnum :
				outText.append( "Series: " ).append( fe.getSeries().toString() ).append( "\n" );
				break;
			case DataType::FilterListEnum :
				outText.append( "FilterList: " ).append( fe.getFilterList().toString() ).append( "\n" );
				break;
			case DataType::OpaqueEnum :
				outText.append( "Opaque: " ).append( fe.getOpaque().toString() ).append( "\n" );
				break;
			case DataType::XmlEnum :
				outText.append( "Xml: " ).append( fe.getXml().toString() ).append( "\n" );
				break;
			case DataType::JsonEnum :
				outText.append( "Json: " ).append( fe.getJson().toString() ).append( "\n" );
				break;
			case DataType::AnsiPageEnum :
				outText.append( "AnsiPage: " ).append( fe.getAnsiPage().toString() ).append( "\n" );
				break;
			case DataType::NoDataEnum :
				outText.append( "NoData" ).append( "\n" );
				break;
			case DataType::ErrorEnum :
				outText.append( "Error: " ).append( fe.getError().getErrorCodeAsString() ).append( "\n" );
				break;
			default :
				outText.append( "field entry with unknown data type " ).append( fe.getLoadType() ).append( "\n" );
				break;
			}
	}
}

void decode( const ElementList& el, EmaString& outText )
{
	if ( el.hasInfo() )
		outText.append( "ElementListNum: " ).append( el.getInfoElementListNum() ).append( "\n" );

	while ( el.forth() )
	{
		const ElementEntry& ee = el.getEntry();

		if ( ee.getCode() == Data::BlankEnum )
			outText.append( ee.getName() ).append( " blank entry\n" );
		else
			switch ( ee.getLoadType() )
			{
			case DataType::IntEnum :
				outText.append( ee.getName() ).append( " Int: " ).append( ee.getInt() ).append( "\n" );
				break;
			case DataType::UIntEnum :
				outText.append( ee.getName() ).append( " UInt: " ).append( ee.getUInt() ).append( "\n" );
				break;
			case DataType::AsciiEnum :
				outText.append( ee.getName() ).append( " Ascii: " ).append( ee.getAscii() ).append( "\n" );
				break;
			case DataType::BufferEnum :
				outText.append( ee.getName() ).append( " Buffer: " ).append( ee.getBuffer() ).append( "\n" );
				break;
			case DataType::RealEnum :
				outText.append( ee.getName() ).append( " Real: " ).append( ee.getReal().getAsDouble() ).append( "\n" );
				break;
			case DataType::DateEnum :
				outText.append( ee.getName() ).append( " Date: " ).append( ee.getDate().toString() ).append( "\n" );
				break;
			case DataType::DateTimeEnum :
				outText.append( ee.getName() ).append( " DateTime: " ).append( ee.getDateTime().toString() ).append( "\n" );
				break;
			case DataType::TimeEnum :
				outText.append( ee.getName() ).append( " Time: " ).append( ee.getTime().toString() ).append( "\n" );
				break;
			case DataType::FloatEnum :
				outText.append( ee.getName() ).append( " Float: " ).append( ee.getFloat() ).append( "\n" );
				break;
			case DataType::DoubleEnum :
				outText.append( ee.getName() ).append( " Double: " ).append( ee.getDouble() ).append( "\n" );
				break;
			case DataType::RmtesEnum :
				break;
			case DataType::Utf8Enum :
				break;
			case DataType::EnumEnum :
				outText.append( ee.getName() ).append( " Enum: " ).append( ee.getEnum() ).append( "\n" );
				break;
			case DataType::StateEnum :
				outText.append( ee.getName() ).append( " State: " ).append( ee.getState().toString() ).append( "\n" );
				break;
			case DataType::QosEnum :
				outText.append( ee.getName() ).append( " Qos: " ).append( ee.getQos().toString() ).append( "\n" );
				break;
			case DataType::ArrayEnum :
				outText.append( ee.getName() ).append( " OmmArray: " ).append( ee.getArray().toString() ).append( "\n" );
				break;
			case DataType::FieldListEnum :
				outText.append( ee.getName() ).append( " FieldList: " ).append( ee.getFieldList().toString() ).append( "\n" );
				break;
			case DataType::ElementListEnum :
				outText.append( ee.getName() ).append( " ElementList: " ).append( ee.getElementList().toString() ).append( "\n" );
				break;
			case DataType::MapEnum :
				outText.append( ee.getName() ).append( " Map: " ).append( ee.getMap().toString() ).append( "\n" );
				break;
			case DataType::VectorEnum :
				outText.append( ee.getName() ).append( " Vector: " ).append( ee.getVector().toString() ).append( "\n" );
				break;
			case DataType::SeriesEnum :
				outText.append( ee.getName() ).append( " Series: " ).append( ee.getSeries().toString() ).append( "\n" );
				break;
			case DataType::FilterListEnum :
				outText.append( ee.getName() ).append( " FilterList: " ).append( ee.getFilterList().toString() ).append( "\n" );
				break;
			case DataType::OpaqueEnum :
				outText.append( ee.getName() ).append( " Opaque: " ).append( ee.getOpaque().toString() ).append( "\n" );
				break;
			case DataType::XmlEnum :
				outText.append( ee.getName() ).append( " Xml: " ).append( ee.getXml().toString() ).append( "\n" );
				break;
			case DataType::JsonEnum :
				outText.append( ee.getName() ).append( " Json: " ).append( ee.getJson().toString() ).append( "\n" );
				break;
			case DataType::AnsiPageEnum :
				outText.append( ee.getName() ).append( " AnsiPage: " ).append( ee.getAnsiPage().toString() ).append( "\n" );
				break;
			case DataType::NoDataEnum :
				outText.append( ee.getName() ).append( " NoData" ).append( "\n" );
				break;
			case DataType::ErrorEnum :
				outText.append( ee.getName() ).append( " Error: " ).append( ee.getError().getErrorCodeAsString() ).append( "\n" );
				break;
			default :
				outText.append( ee.getName() ).append( " field entry with unknown data type " ).append( ee.getLoadType() ).append( "\n" );
				break;
			}
	}
}

void decode( const Map& map, EmaString& outText )
{
	switch ( map.getSummaryData().getDataType() )
	{
	case DataType::FieldListEnum :
		decode( map.getSummaryData().getFieldList(), outText );
		break;
	}

	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		// decode key
		switch ( me.getKey().getDataType() )
		{
		case DataType::IntEnum :
			outText.append( "Int: " ).append( me.getKey().getInt() ).append( "\n" );
			break;
		case DataType::UIntEnum :
			outText.append( "UInt: " ).append( me.getKey().getUInt() ).append( "\n" );
			break;
		case DataType::AsciiEnum :
			outText.append( "Ascii: " ).append( me.getKey().getAscii() ).append( "\n" );
			break;
		case DataType::BufferEnum :
			outText.append( "Buffer: " ).append( me.getKey().getBuffer() ).append( "\n" );
			break;
		case DataType::RealEnum :
			outText.append( "Real: " ).append( me.getKey().getReal().getAsDouble() ).append( "\n" );
			break;
		case DataType::DateEnum :
			outText.append( "Date: " ).append( me.getKey().getDate().toString() ).append( "\n" );
			break;
		case DataType::DateTimeEnum :
			outText.append( "DateTime: " ).append( me.getKey().getDateTime().toString() ).append( "\n" );
			break;
		case DataType::TimeEnum :
			outText.append( "Time: " ).append( me.getKey().getTime().toString() ).append( "\n" );
			break;
		case DataType::FloatEnum :
			outText.append( "Float: " ).append( me.getKey().getFloat() ).append( "\n" );
			break;
		case DataType::DoubleEnum :
			outText.append( "Double: " ).append( me.getKey().getDouble() ).append( "\n" );
			break;
		case DataType::RmtesEnum :
//			outText.append( "Rmtes: " ).append( me.getKeyRmtes().cs_buf() ).append( "\n" );
			break;
		case DataType::Utf8Enum :
			outText.append( "Utf8: " ).append( me.getKey().getUtf8() ).append( "\n" );
			break;
		case DataType::ErrorEnum :
			outText.append( "Error: " ).append( me.getError().getErrorCodeAsString() ).append( "\n" );
			break;
		default :
			outText.append( "map key with unknown data type " ).append( me.getKey().getDataType() ).append( "\n" );
			break;
		}

		// decode action
		outText.append( "Action: " ).append( me.getMapActionAsString() ).append( "\n" );

		// decode load
		switch ( me.getLoad().getDataType() )
		{
		case DataType::FieldListEnum :
			decode( me.getFieldList(), outText );
			break;
		case DataType::ElementListEnum :
			break;
		case DataType::FilterListEnum :
			break;
		case DataType::SeriesEnum :
			break;
		case DataType::VectorEnum :
			break;
		case DataType::MapEnum :
			decode( me.getMap(), outText );
			break;
		case DataType::NoDataEnum :
			break;
		case DataType::AckMsgEnum :
			break;
		case DataType::GenericMsgEnum :
			break;
		case DataType::PostMsgEnum :
			break;
		case DataType::RefreshMsgEnum :
			break;
		case DataType::StatusMsgEnum :
			break;
		case DataType::UpdateMsgEnum :
			break;
		case DataType::ReqMsgEnum :
			break;
		default :
			outText.append( "map entry with unknown data type " ).append( me.getLoadType() ).append( "\n" );
			break;
		}
	}
}

void perfDecode( const Map& map )
{
	switch ( map.getSummaryData().getDataType() )
	{
	case DataType::FieldListEnum :
		perfDecode( map.getSummaryData().getFieldList() );
		break;
	case DataType::MapEnum :
		perfDecode( map.getSummaryData().getMap() );
		break;
	}

	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		// decode key
		switch ( me.getKey().getDataType() )
		{
		case DataType::IntEnum :
		{
			Int64 value = me.getKey().getInt();
		}
		break;
		case DataType::UIntEnum :
		{
			UInt64 value = me.getKey().getUInt();
		}
		break;
		case DataType::AsciiEnum :
		{
			const EmaString& value = me.getKey().getAscii();
		}
		break;
		case DataType::BufferEnum :
		{
			const EmaBuffer& value = me.getKey().getBuffer();
		}
		break;
		case DataType::RealEnum :
		{
			const OmmReal& value = me.getKey().getReal();
			OmmReal::MagnitudeType mt = value.getMagnitudeType();
			Int64 m = value.getMantissa();
		}
		break;
		case DataType::DateEnum :
		{
			const OmmDate& d = me.getKey().getDate();
			UInt16 year = d.getYear();
			UInt8 month = d.getMonth();
			UInt8 day = d.getDay();
		}
		break;
		case DataType::DateTimeEnum :
		{
			const OmmDateTime& dt = me.getKey().getDateTime();
			UInt16 year = dt.getYear();
			UInt8 month = dt.getMonth();
			UInt8 day = dt.getDay();
			UInt8 hour = dt.getHour();
			UInt8 minute = dt.getMinute();
			UInt8 second = dt.getSecond();
			UInt16 millisecond = dt.getMillisecond();
		}
		break;
		case DataType::TimeEnum :
		{
			const OmmTime& dt = me.getKey().getTime();
			UInt8 hour = dt.getHour();
			UInt8 minute = dt.getMinute();
			UInt8 second = dt.getSecond();
			UInt16 millisecond = dt.getMillisecond();
		}
		break;
		case DataType::FloatEnum :
		{
			float f = me.getKey().getFloat();
		}
		break;
		case DataType::DoubleEnum :
		{
			double d = me.getKey().getDouble();
		}
		break;
		case DataType::RmtesEnum :
		{
			const RmtesBuffer& text = me.getKey().getRmtes();
		}
		break;
		case DataType::Utf8Enum :
		{
			const EmaBuffer& text = me.getKey().getUtf8();
		}
		break;
		default :
			break;
		}

		// decode action
		MapEntry::MapAction action = me.getAction();

		// decode load
		switch ( me.getLoadType() )
		{
		case DataType::FieldListEnum :
			perfDecode( me.getFieldList() );
			break;
		case DataType::ElementListEnum :
//			perfDecode( me.getElementList() );
			break;
		case DataType::FilterListEnum :
			break;
		case DataType::SeriesEnum :
			break;
		case DataType::VectorEnum :
			break;
		case DataType::MapEnum :
			perfDecode( me.getMap() );
			break;
		case DataType::NoDataEnum :
			break;
		case DataType::AckMsgEnum :
			break;
		case DataType::GenericMsgEnum :
			break;
		case DataType::PostMsgEnum :
			break;
		case DataType::RefreshMsgEnum :
			break;
		case DataType::StatusMsgEnum :
			break;
		case DataType::UpdateMsgEnum :
			break;
		case DataType::ReqMsgEnum :
			break;
		default :
			break;
		}
	}
}

TEST(DataUnitTest, testUInt64ToUInt32Compare)
{
	size_t x0 = 0xFFFFFFFFFFFFFFFF;
	size_t x1 = 0x0000000100000000;
	size_t x2 = 0x00000000FFFFFFFF;
	size_t x3 = 0x00000000FFFFFFFE;
	const UInt32 y = -1;

	EXPECT_TRUE( x0 >= y ) << "0xFFFFFFFFFFFFFFFF >= -1";
	EXPECT_TRUE( x1 >= y ) << "0x0000000100000000 >= -1";
	EXPECT_TRUE( x2 >= y ) << "0x00000000FFFFFFF >= -1";
	EXPECT_TRUE( x3 < y ) << "0x00000000FFFFFFE < -1";
}

TEST(DataUnitTest, testPerfArrayWithReal)
{
	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;
		RsslBuffer rsslBuf;
		RsslReal real;

		char* buffer = ( char* )malloc( sizeof( char ) * 1000 );

		Int64 loopCount = 1000;

		for ( int i = 0; i < 3; ++i )
		{

			for ( Int64 value = 0; value < loopCount; ++value )
			{
				rsslClearArray( &rsslArray );
				rsslClearEncodeIterator( &iter );

				rsslBuf.length = 1000;
				rsslBuf.data = buffer;

				rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
				rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

				rsslArray.itemLength = 0;
				rsslArray.primitiveType = RSSL_DT_REAL;

				rsslEncodeArrayInit( &iter, &rsslArray );

				real.hint = RSSL_RH_EXPONENT_2;
				real.isBlank = RSSL_FALSE;
				real.value = value;
				rsslEncodeArrayEntry( &iter, 0, &real );

				real.hint = RSSL_RH_FRACTION_2;
				real.isBlank = RSSL_FALSE;
				real.value = value + 1;
				rsslEncodeArrayEntry( &iter, 0, &real );

				real.hint = RSSL_RH_FRACTION_2;
				real.isBlank = RSSL_FALSE;
				real.value = value + 2;
				rsslEncodeArrayEntry( &iter, 0, &real );

				rsslBuf.length = rsslGetEncodedBufferLength( &iter );

				rsslEncodeArrayComplete( &iter, RSSL_TRUE );

				OmmArray ar;

				StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

				perfDecode( ar );

				ar.reset();

				perfDecode( ar );
			}

			loopCount *= 5;
		}

		free( buffer );

		EXPECT_TRUE( true ) <<  "Perf Test OmmArray with Real - exception not expected";

	}
	catch ( const OmmException& excp )
	{
	  EXPECT_FALSE( true ) << "Perf Test OmmArray with Real - exception not expected "
			      << excp << endl;
	}
}

TEST(DataUnitTest, testPerfArrayWithDateTime)
{
	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;
		RsslBuffer rsslBuf;
		RsslDateTime dateTime;

		char* buffer = ( char* )malloc( sizeof( char ) * 1000 );

		Int64 loopCount = 1000;

		for ( int i = 0; i < 3; ++i )
		{
			for ( Int64 value = 0; value < loopCount; ++value )
			{
				rsslClearArray( &rsslArray );
				rsslClearEncodeIterator( &iter );

				rsslBuf.length = 1000;
				rsslBuf.data = buffer;

				rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
				rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

				rsslArray.itemLength = 0;
				rsslArray.primitiveType = RSSL_DT_DATETIME;

				rsslEncodeArrayInit( &iter, &rsslArray );

				dateTime.date.year = 3333;
				dateTime.date.month = 11;
				dateTime.date.day = 1;
				dateTime.time.hour = 14;
				dateTime.time.minute = 15;
				dateTime.time.second = 16;
				dateTime.time.millisecond = 17;
				rsslEncodeArrayEntry( &iter, 0, &dateTime );

				dateTime.date.year = 1111;
				dateTime.date.month = 11;
				dateTime.date.day = 1;
				dateTime.time.hour = 14;
				dateTime.time.minute = 15;
				dateTime.time.second = 16;
				dateTime.time.millisecond = 17;
				rsslEncodeArrayEntry( &iter, 0, &dateTime );

				dateTime.date.year = 2222;
				dateTime.date.month = 11;
				dateTime.date.day = 1;
				dateTime.time.hour = 14;
				dateTime.time.minute = 15;
				dateTime.time.second = 16;
				dateTime.time.millisecond = 17;
				rsslEncodeArrayEntry( &iter, 0, &dateTime );

				rsslBuf.length = rsslGetEncodedBufferLength( &iter );

				rsslEncodeArrayComplete( &iter, RSSL_TRUE );

				OmmArray ar;

				StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

				perfDecode( ar );

				ar.reset();

				perfDecode( ar );
			}


			loopCount *= 5;
		}

		free( buffer );

		EXPECT_TRUE( true ) << "Perf Test OmmArray with DateTime - exception not expected";

	}
	catch ( const OmmException& excp )
	{
	  EXPECT_FALSE( true ) << "Perf Test OmmArray with DateTime - exception not expected " << excp << endl;
	}
}

// no checkResult calls in this function
void encodeFieldListWithMapInside( RsslBuffer& rsslBuf, EmaString& inText )
{
	RsslFieldList rsslFL;
	RsslEncodeIterator iter;

	rsslClearFieldList( &rsslFL );
	rsslClearEncodeIterator( &iter );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
	rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
	rsslFL.dictionaryId = 1;
	rsslFL.fieldListNum = 65;

	rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

	RsslFieldEntry rsslFEntry;

	rsslFEntry.dataType = RSSL_DT_UINT;
	rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
	RsslUInt64 uint64 = 64;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );
	inText.append( "UInt: " ).append( uint64 ).append( "\n" );

	rsslFEntry.dataType = RSSL_DT_REAL;
	rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
	RsslReal real;
	double d;
	real.isBlank = RSSL_FALSE;
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 11;
	rsslRealToDouble( &d, &real );
	inText.append( "Real: " ).append( d ).append( "\n" );
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

	rsslFEntry.dataType = RSSL_DT_INT;
	rsslFEntry.fieldId = -2;		// INTEGER + INT
	RsslInt64 int64 = 32;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );
	inText.append( "Int: " ).append( int64 ).append( "\n" );

	rsslFEntry.dataType = RSSL_DT_DATE;
	rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
	RsslDate date;
	date.day = 7;
	date.month = 11;
	date.year = 1999;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );
	inText.append( "Date: " ).append( "07 NOV 1999" ).append( "\n" );	// HARDCODED !!!

	rsslFEntry.dataType = RSSL_DT_TIME;
	rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
	RsslTime time;
	time.hour = 02;
	time.minute = 03;
	time.second = 04;
	time.millisecond = 005;
	inText.append( "Time: " ).append( "02:03:04:005" ).append( "\n" );	// HARDCODED !!!
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );

	rsslFEntry.dataType = RSSL_DT_DATETIME;
	rsslFEntry.fieldId = -3;		// TRADE_DATE + DATE
	RsslDateTime dateTime;
	dateTime.date.day = 7;
	dateTime.date.month = 11;
	dateTime.date.year = 1999;
	dateTime.time.hour = 01;
	dateTime.time.minute = 02;
	dateTime.time.second = 03;
	dateTime.time.millisecond = 000;
	inText.append( "DateTime: " ).append( "07 NOV 1999 01:02:03:000" ).append( "\n" );	// HARDCODED !!!
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );

	rsslFEntry.dataType = RSSL_DT_STATE;
	rsslFEntry.fieldId = FID_STATE;
	RsslState rsslState = RSSL_INIT_STATE;
	rsslState.streamState = RSSL_STREAM_OPEN;
	rsslState.dataState = RSSL_DATA_OK;
	rsslState.code = RSSL_SC_NONE;
	rsslState.text.data = ( char* )"Succeeded";
	rsslState.text.length = 9;
	inText.append( "State: State='Open / Ok / None / 'Succeeded''" ).append( "\n" );
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslState );

	rsslFEntry.dataType = RSSL_DT_ASCII_STRING;
	rsslFEntry.fieldId = 235;		// ASCII
	RsslBuffer ascii;
	ascii.data = const_cast<char*>( "ABCDEF" );
	ascii.length = 6;
	inText.append( "Ascii: " ).append( ascii.data ).append( "\n" );
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&ascii );


	rsslBuf.length = rsslGetEncodedBufferLength( &iter );

	rsslEncodeFieldListComplete( &iter, RSSL_TRUE );
}

TEST(DataUnitTest, testPerfElementList)
{
	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING, RMTES_STRING
		RsslElementList rsslEL;
		RsslEncodeIterator iter;

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		Int64 loopCount = 1000;

		for ( int i = 0; i < 3; ++i )
		{

			for ( Int64 value = 0; value < loopCount; ++value )
			{
				rsslClearElementList( &rsslEL );
				rsslClearEncodeIterator( &iter );

				RsslBuffer rsslBuf;
				rsslBuf.length = 1000;
				rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

				rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
				rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
				rsslEL.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
				rsslEL.elementListNum = 5;

				rsslEncodeElementListInit( &iter, &rsslEL, 0, 0 );

				RsslElementEntry rsslEEntry = RSSL_INIT_ELEMENT_ENTRY;

				rsslEEntry.name.data = ( char* )"Element - RsslUInt";
				rsslEEntry.name.length = 18;
				rsslEEntry.dataType = RSSL_DT_UINT;
				//RsslUInt rsslUInt = 17;
				RsslUInt64 uint64 = 64;
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );

				rsslClearElementEntry( &rsslEEntry );	// clear this to ensure a blank field
				rsslEEntry.name.data = ( char* )"Element - RsslReal - Blank";
				rsslEEntry.name.length = 26;
				rsslEEntry.dataType = RSSL_DT_REAL;
				rsslEncodeElementEntry( &iter, &rsslEEntry, NULL );		/* this encodes a blank */

				rsslEEntry.name.data = ( char* )"Element - RsslInt";
				rsslEEntry.name.length = 17;
				rsslEEntry.dataType = RSSL_DT_INT;
				//RsslInt rsslInt = 13;
				RsslInt64 int64 = 32;
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&int64 );

				rsslEEntry.name.data = ( char* )"Element - RsslDate";
				rsslEEntry.name.length = 18;
				rsslEEntry.dataType = RSSL_DT_DATE;
				RsslDate date;
				date.day = 7;
				date.month = 11;
				date.year = 1999;
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&date );

				rsslEEntry.name.data = ( char* )"Element - RsslTime";
				rsslEEntry.name.length = 18;
				rsslEEntry.dataType = RSSL_DT_TIME;
				//RsslTime rsslTime = {10, 21, 16, 777};
				RsslTime rsslTime = {02, 03, 04, 005};
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

				rsslEEntry.name.data = ( char* )"Element - RsslDateTime";
				rsslEEntry.name.length = 22;
				rsslEEntry.dataType = RSSL_DT_DATETIME;
				RsslDateTime dateTime;
				dateTime.date.day = 7;
				dateTime.date.month = 11;
				dateTime.date.year = 1999;
				dateTime.time.hour = 01;
				dateTime.time.minute = 02;
				dateTime.time.second = 03;
				dateTime.time.millisecond = 000;
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

				rsslEEntry.name.data = ( char* )"Element - RsslState";
				rsslEEntry.name.length = 19;
				RsslState rsslState = RSSL_INIT_STATE;
				rsslState.streamState = RSSL_STREAM_OPEN;
				rsslState.dataState = RSSL_DATA_OK;
				rsslState.code = RSSL_SC_NONE;
				rsslState.text.data = ( char* )"Succeeded";
				rsslState.text.length = 9;
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslState );

				rsslEEntry.name.data = ( char* )"Element - RsslAsciiString";
				rsslEEntry.name.length = 25;
				RsslBuffer ascii;
				ascii.data = const_cast<char*>( "ABCDEF" );
				ascii.length = 6;
				rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&ascii );

				rsslBuf.length = rsslGetEncodedBufferLength( &iter );

				rsslEncodeElementListComplete( &iter, RSSL_TRUE );
			}

			loopCount *= 5;
		}

		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "ElementList decode - exception not expected";

	}
	catch ( const OmmException& excp )
	{
	  EXPECT_FALSE( true ) << "ElementList decode - exception not expected" << excp << endl;
	}

}

TEST(DataUnitTest, testDoNothing)
{
  InfoFilter infofilter;
  class AppClient : public OmmConsumerClient {};
  AppClient ac;
  OmmConsumerConfig config;
}

EmaString g_userName;
EmaString g_password;

EmaString g_proxyHost;
EmaString g_proxyPort;

int main(int argc, char** argv) {
  int i = 1;
  int retVal = 0;

  try {
	  for (; i < argc; i++)
	  {
		  if (0 == strcmp("-uname", argv[i]))
		  {
			  if (++i == argc)
				  break;
			  g_userName.set(argv[i]);
		  }
		  else if (0 == strcmp("-passwd", argv[i]))
		  {
			  if (++i == argc)
				  break;
			  g_password.set(argv[i]);
		  }
		  else if (0 == strcmp("-ph", argv[i]))
		  {
			  if (++i == argc)
				  break;
			  g_proxyHost.set(argv[i]);
		  }
		  else if (0 == strcmp("-pp", argv[i]))
		  {
			  if (++i == argc)
				  break;
			  g_proxyPort.set(argv[i]);
		  }
	  }
  } catch (const OmmException& excp) {
	  cout << excp << endl;
  }

  try {
	  ::testing::AddGlobalTestEnvironment(new MyEnvironment);
	  ::testing::InitGoogleTest(&argc, argv);

	  /* Skipping the test cases for the EmaConfigTest.testLoadingConfigurationFromProgrammaticConfigForSessionManagement when a login credential isn't available */
	  if ((g_userName.length() == 0) || (g_password.length() == 0))
		  testing::GTEST_FLAG(filter) += "-EmaConfigTest.testLoadingConfigurationFromProgrammaticConfigForSessionManagement";

	  retVal = RUN_ALL_TESTS();
  } catch (const std::exception& e) {
	  std::cout << "GoogleTest failed: %s\n" << e.what() << std::endl;
	  return 1;
  } catch (...) {
	  std::cout << "GoogleTest failed: unknown error\n" << std::endl;
	  return 1;
  }

  return retVal;
}
