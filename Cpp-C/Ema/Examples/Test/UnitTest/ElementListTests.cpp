/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace std;

TEST(ElementListTests, testElementListwithReal)
{

	try
	{
		ElementList el;
		el.addCodeReal( "blank real" );

		el.addReal( "real", 123, OmmReal::ExponentNeg1Enum );

		double d = 1456.789;

		el.addRealFromDouble( "real from double", d );

		el.addReal( "infinity", 1, OmmReal::InfinityEnum );

		el.addReal( "neg infinity", 1, OmmReal::NegInfinityEnum );

		el.addReal( "infinity", 1, OmmReal::NotANumberEnum );

		el.complete();

		StaticDecoder::setData( &el, 0 );


		EXPECT_TRUE( true ) << "ElementList with Real + Extensions - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with Real + Extensions - exception not expected" ;
	}
}

//encode ElementList with UPA and decode ElementList with EMA
TEST(ElementListTests, testElementListDecodeAll)
{

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,
		//                  RMTES_STRING, ENUM, FLOAT, DOUBLE, BLANK REAL, BUFFER, UTF8_STRING,
		//					OPAQUE, XML, ANSI_PAGE
		RsslElementList rsslEL;
		RsslEncodeIterator iter;

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

		//first entry
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		RsslUInt64 uint64 = 64;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );

		//second entry
		rsslEEntry.name.data = ( char* )"Element - Real";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_REAL;
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&real );

		//third entry
		rsslEEntry.name.data = ( char* )"Element - Int";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_INT;
		RsslInt64 int64 = 32;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&int64 );

		//fourth entry
		rsslEEntry.name.data = ( char* )"Element - Date";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_DATE;
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&date );

		//fifth entry
		rsslEEntry.name.data = ( char* )"Element - Time";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_TIME;
		//RsslTime rsslTime = {10, 21, 16, 777};
		RsslTime rsslTime = {02, 03, 04, 005};
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

		//sixth entry
		rsslEEntry.name.data = ( char* )"Element - DateTime";
		rsslEEntry.name.length = 18;
		rsslEEntry.dataType = RSSL_DT_DATETIME;
		RsslDateTime dateTime;
		dateTime.date.day = 7;
		dateTime.date.month = 11;
		dateTime.date.year = 1999;
		dateTime.time.hour = 01;
		dateTime.time.minute = 02;
		dateTime.time.second = 03;
		dateTime.time.millisecond = 000;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&dateTime );

		//seventh entry
		rsslEEntry.name.data = ( char* )"Element - Qos";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslQos );

		//eightth entry
		rsslEEntry.name.data = ( char* )"Element - State";
		rsslEEntry.name.length = 15;
		rsslEEntry.dataType = RSSL_DT_STATE;
		RsslState rsslState = RSSL_INIT_STATE;
		rsslState.streamState = RSSL_STREAM_OPEN;
		rsslState.dataState = RSSL_DATA_OK;
		rsslState.code = RSSL_SC_NONE;
		rsslState.text.data = ( char* )"Succeeded";
		rsslState.text.length = 9;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslState );

		//ninth entry
		rsslEEntry.name.data = ( char* )"Element - AsciiString";
		rsslEEntry.name.length = 21;
		rsslEEntry.dataType = RSSL_DT_ASCII_STRING;
		RsslBuffer ascii;
		ascii.data = const_cast<char*>("ABCDEF");
		ascii.length = 6;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&ascii );

		//tenth entry
		rsslEEntry.name.data = ( char* )"Element - RmtesString";
		rsslEEntry.name.length = 21;
		rsslEEntry.dataType = RSSL_DT_RMTES_STRING;
		RsslBuffer news;
		news.data = const_cast<char*>("ABCDEF");
		news.length = 6;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&news );

		//eleventh entry
		rsslEEntry.name.data = ( char* )"Element - Enum";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_ENUM;
		RsslEnum enumm = 29;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&enumm );

		//twelfth entry
		rsslEEntry.name.data = ( char* )"Element - Float";
		rsslEEntry.name.length = 15;
		rsslEEntry.dataType = RSSL_DT_FLOAT;
		RsslFloat floatValue = 11.11f;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&floatValue );

		//thirteenth entry
		rsslEEntry.name.data = ( char* )"Element - Double";
		rsslEEntry.name.length = 16;
		rsslEEntry.dataType = RSSL_DT_DOUBLE;
		RsslDouble doubleValue = 22.22f;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&doubleValue );

		//fourteenth entry
		rsslEEntry.name.data = ( char* )"Element - RealBlank";
		rsslEEntry.name.length = 19;
		rsslEEntry.dataType = RSSL_DT_REAL;
		RsslReal realValue;
		realValue.isBlank = RSSL_TRUE;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&realValue );

		//fifteenth entry
		rsslEEntry.name.data = ( char* )"Element - Buffer";
		rsslEEntry.name.length = 16;
		rsslEEntry.dataType = RSSL_DT_BUFFER;
		RsslBuffer buffer;
		buffer.data = const_cast<char*>("ABCDEFGH");
		buffer.length = 8;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&buffer );

		//sixteenth entry
		rsslEEntry.name.data = ( char* )"Element - Utf8String";
		rsslEEntry.name.length = 20;
		rsslEEntry.dataType = RSSL_DT_UTF8_STRING;
		RsslBuffer buffer_utf8;
		buffer_utf8.data = const_cast<char*>("KLMNOPQR");
		buffer_utf8.length = 8;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&buffer_utf8 );

		// seventeenth entry
		rsslEEntry.name.data = ( char* )"Element - Opaque";
		rsslEEntry.name.length = 16;
		rsslEEntry.dataType = RSSL_DT_OPAQUE;

		char opaqueBuffer[8];

		RsslBuffer buffer_opaque;
		buffer_opaque.data = opaqueBuffer;
		buffer_opaque.length = 8;

		rsslEncodeElementEntryInit( &iter, &rsslEEntry, 0 );
		rsslEncodeNonRWFDataTypeInit( &iter, &buffer_opaque );

		memcpy( buffer_opaque.data , "KLMNOPQR", 8 );
		buffer_opaque.length = 8;

		rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_opaque, RSSL_TRUE );
		rsslEncodeElementEntryComplete( &iter, RSSL_TRUE );

		// eightteenth entry
		rsslEEntry.name.data = ( char* )"Element - XML";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_XML;

		char xmlBuffer[25];

		RsslBuffer buffer_xml;
		buffer_xml.data = xmlBuffer;
		buffer_xml.length = 25;

		rsslEncodeElementEntryInit( &iter, &rsslEEntry, 0 );
		rsslEncodeNonRWFDataTypeInit( &iter, &buffer_xml );

		memcpy( buffer_xml.data , "<value> KLMNOPQR </value>", 25 );
		buffer_xml.length = 25;

		rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_xml, RSSL_TRUE );
		rsslEncodeElementEntryComplete( &iter, RSSL_TRUE );

		// nineteenth entry
		rsslEEntry.name.data = ( char* )"Element - AnsiPage";
		rsslEEntry.name.length = 18;
		rsslEEntry.dataType = RSSL_DT_ANSI_PAGE;

		char ansiPageBuffer[34];

		RsslBuffer buffer_ansiPage;
		buffer_ansiPage.data = ansiPageBuffer;
		buffer_ansiPage.length = 34;

		rsslEncodeElementEntryInit( &iter, &rsslEEntry, 0 );
		rsslEncodeNonRWFDataTypeInit( &iter, &buffer_ansiPage );

		memcpy( buffer_ansiPage.data , "328-srfsjkj43rouw-01-20ru2l24903$%", 34 );
		buffer_ansiPage.length = 34;

		rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_ansiPage, RSSL_TRUE );
		rsslEncodeElementEntryComplete( &iter, RSSL_TRUE );


		rsslEncodeElementListComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of ElementList
		ElementList el;
		StaticDecoder::setRsslData( &el, &rsslBuf, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_TRUE( el.hasInfo() ) << "ElementList with all data types - hasInfo()" ;
		EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with all data types- getInfoElementListNum()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - first forth()" ;

		const ElementEntry& ee1 = el.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - second forth()" ;

		const ElementEntry& ee2 = el.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		el.reset();
		{
			EXPECT_TRUE( el.hasInfo() ) << "ElementList with all data types - hasInfo()" ;
			EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with all data types- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = el.getEntry();
				EXPECT_FALSE( true ) << "ElementList with all data types- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with all data types- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( el.forth() ) << "ElementList with all data types- first forth() again" ;

			const ElementEntry& ee1 = el.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( el.forth() ) << "ElementList with all data types - second forth() again" ;

			const ElementEntry& ee2 = el.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}


		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - third forth()" ;

		const ElementEntry& ee3 = el.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - fourth forth()" ;

		const ElementEntry& ee4 = el.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - fifth forth()" ;

		const ElementEntry& ee5 = el.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - sixth forth()" ;

		const ElementEntry& ee6 = el.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types seventh forth()" ;

		const ElementEntry& ee7 = el.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee7.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - eightth forth()" ;

		const ElementEntry& ee8 = el.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - State" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::StateEnum ) << "ElementEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getState().getStreamState(), OmmState::OpenEnum ) << "ElementEntry::getState().getStreamState()" ;
		EXPECT_EQ( ee8.getState().getDataState(), OmmState::OkEnum ) << "ElementEntry::getState().getDataState()" ;
		EXPECT_EQ( ee8.getState().getStatusCode(), OmmState::NoneEnum ) << "ElementEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ee8.getState().getStatusText(), "Succeeded" ) << "ElementEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ee8.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "ElementEntry::getState().toString()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - ninth forth()" ;

		const ElementEntry& ee9 = el.getEntry();
		EXPECT_STREQ( ee9.getName(), "Element - AsciiString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee9.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( ee9.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee9.getAscii(), "ABCDEF" ) << "ElementEntry::getAscii()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - tenth forth()" ;

		const ElementEntry& ee10 = el.getEntry();
		EXPECT_STREQ( ee10.getName(), "Element - RmtesString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee10.getLoadType(), DataType::RmtesEnum ) << "ElementEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( ee10.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee10.getRmtes().getAsUTF8(), EmaBuffer( "ABCDEF", 6 ) ) << "ElementEntry::getRmtes()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - eleventh forth()" ;

		const ElementEntry& ee11 = el.getEntry();
		EXPECT_STREQ( ee11.getName(), "Element - Enum" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee11.getLoadType(), DataType::EnumEnum ) << "ElementEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ee11.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee11.getEnum(), 29 ) << "ElementEntry::getEnum()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - twelfth forth()" ;

		const ElementEntry& ee12 = el.getEntry();
		EXPECT_STREQ( ee12.getName(), "Element - Float" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee12.getLoadType(), DataType::FloatEnum ) << "ElementEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ee12.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee12.getFloat(), 11.11f ) << "ElementEntry::getFloat()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - thirteenth forth()" ;

		const ElementEntry& ee13 = el.getEntry();
		EXPECT_STREQ( ee13.getName(), "Element - Double" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee13.getLoadType(), DataType::DoubleEnum ) << "ElementEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ee13.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee13.getDouble(), 22.22f ) << "ElementEntry::getDouble()" ;


		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - fourteenth forth()" ;

		const ElementEntry& ee14 = el.getEntry();
		EXPECT_STREQ( ee14.getName(), "Element - RealBlank" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee14.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee14.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;


		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - fifteenth forth()" ;

		const ElementEntry& ee15 = el.getEntry();
		EXPECT_STREQ( ee15.getName(), "Element - Buffer" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee15.getLoadType(), DataType::BufferEnum ) << "ElementEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( ee15.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee15.getBuffer(), EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getBuffer()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - sixteenth forth()" ;

		const ElementEntry& ee16 = el.getEntry();
		EXPECT_STREQ( ee16.getName(), "Element - Utf8String" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee16.getLoadType(), DataType::Utf8Enum ) << "ElementEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( ee16.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee16.getUtf8(), EmaBuffer( "KLMNOPQR", 8 ) ) << "ElementEntry::getUtf8()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - seventeen forth()" ;

		const ElementEntry& ee17 = el.getEntry();
		EXPECT_STREQ( ee17.getName(), "Element - Opaque" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee17.getLoadType(), DataType::OpaqueEnum ) << "ElementEntry::getLoadType() == DataType::Opaque" ;
		EXPECT_EQ( ee17.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee17.getOpaque().getBuffer(), EmaBuffer( "KLMNOPQR", 8 ) ) << "ElementEntry::getOpaque()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - eightteen forth()" ;

		const ElementEntry& ee18 = el.getEntry();
		EXPECT_STREQ( ee18.getName(), "Element - XML" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee18.getLoadType(), DataType::XmlEnum ) << "ElementEntry::getLoadType() == DataType::Xml" ;
		EXPECT_EQ( ee18.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee18.getXml().getBuffer(), EmaBuffer( "<value> KLMNOPQR </value>", 25 ) ) << "ElementEntry::getXml()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with all data types - nineteen forth()" ;

		const ElementEntry& ee19 = el.getEntry();
		EXPECT_STREQ( ee19.getName(), "Element - AnsiPage" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee19.getLoadType(), DataType::AnsiPageEnum ) << "ElementEntry::getLoadType() == DataType::AnsiPage" ;
		EXPECT_EQ( ee19.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee19.getAnsiPage().getBuffer(), EmaBuffer( "328-srfsjkj43rouw-01-20ru2l24903$%", 34 ) ) << "ElementEntry::getXml()" ;

		EXPECT_TRUE( true ) << "ElementList with all data types - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with all data types - exception not expected" ;
	}
}

//encode with UPA and decode with EMA
TEST(ElementListTests, testElementListContainsFieldListDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, FieldList, QOS
		RsslElementList rsslEL;
		RsslEncodeIterator iter;

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

		//first entry
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		RsslUInt64 uint64 = 64;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );

		//second entry
		rsslEEntry.name.data = ( char* )"Element - Real";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_REAL;
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&real );

		//third entry
		rsslEEntry.name.data = ( char* )"Element - Int";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_INT;
		//RsslInt rsslInt = 13;
		RsslInt64 int64 = 32;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&int64 );

		//fourth entry
		rsslEEntry.name.data = ( char* )"Element - Date";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_DATE;
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&date );

		//fifth entry
		rsslEEntry.name.data = ( char* )"Element - Time";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_TIME;
		//RsslTime rsslTime = {10, 21, 16, 777};
		RsslTime rsslTime = {02, 03, 04, 005};
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

		//sixth entry
		rsslEEntry.name.data = ( char* )"Element - DateTime";
		rsslEEntry.name.length = 18;
		rsslEEntry.dataType = RSSL_DT_DATETIME;
		RsslDateTime dateTime;
		dateTime.date.day = 7;
		dateTime.date.month = 11;
		dateTime.date.year = 1999;
		dateTime.time.hour = 01;
		dateTime.time.minute = 02;
		dateTime.time.second = 03;
		dateTime.time.millisecond = 000;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&dateTime );

		//seventh entry (nested FieldList)
		rsslEEntry.name.data = ( char* )"Element - FieldList";
		rsslEEntry.name.length = 19;
		rsslEEntry.dataType = RSSL_DT_FIELD_LIST;

		RsslFieldList nestedFieldList = RSSL_INIT_FIELD_LIST;
		RsslFieldEntry rsslFEntry;
		rsslEncodeElementEntryInit( &iter, &rsslEEntry, 1000 );
		nestedFieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		rsslEncodeFieldListInit( &iter, &nestedFieldList, 0, 0 );
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;	 // PROD_PERM + UINT
		uint64 = 641;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, &uint64 );
		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );
		rsslEncodeElementEntryComplete( &iter, RSSL_TRUE );

		//eightth entry
		rsslEEntry.name.data = ( char* )"Element - Qos";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslQos );

		rsslEncodeElementListComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of ElementList
		ElementList el;
		StaticDecoder::setRsslData( &el, &rsslBuf, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( el.hasInfo() ) << "ElementList with primitives and FieldList - hasInfo()" ;
		EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with primitives and FieldList- getInfoElementListNum()" ;


		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - first forth()" ;

		const ElementEntry& ee1 = el.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - second forth()" ;

		const ElementEntry& ee2 = el.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		el.reset();
		{
			EXPECT_TRUE( el.hasInfo() ) << "ElementList with primitives and FieldList - hasInfo()" ;
			EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with primitives and FieldList- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = el.getEntry();
				EXPECT_FALSE( true ) << "ElementList with primitives and FieldList- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with primitives and FieldList- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList- first forth() again" ;

			const ElementEntry& ee1 = el.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - second forth() again" ;

			const ElementEntry& ee2 = el.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}


		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - third forth()" ;

		const ElementEntry& ee3 = el.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - fourth forth()" ;

		const ElementEntry& ee4 = el.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - fifth forth()" ;

		const ElementEntry& ee5 = el.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - sixth forth()" ;

		const ElementEntry& ee6 = el.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - seventh forth()" ;

		const ElementEntry& ee7 = el.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - FieldList" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::FieldListEnum ) << "ElementEntry::getLoadType() == DataType::FieldListEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::FieldListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& fl = ee7.getFieldList();

			EXPECT_FALSE( fl.hasInfo() ) << "ElementEntry FieldList within elementlist - hasInfo()" ;

			EXPECT_TRUE( fl.forth() ) << "ElementEntry FieldList within elementlist - first fieldlist forth()" ;
			const FieldEntry& fe1 = fl.getEntry();
			EXPECT_EQ( fe1.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe1.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe1.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe1.getUInt(), 641 ) << "FieldEntry::getUInt()" ;

			EXPECT_FALSE( fl.forth() ) << "ElementEntry FieldList within elementlist - second fieldlist forth()" ;
		}

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and FieldList - eightth forth()" ;

		const ElementEntry& ee8 = el.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee8.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_FALSE( el.forth() ) << "ElementList with primitives and FieldList - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with primitives and FieldList - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with primitives and FieldList - exception not expected" ;
	}
}

//encode with UPA and decode with EMA
TEST(ElementListTests, testElementListContainsElementListDecodeAll)
{

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, QOS
		RsslElementList rsslEL;
		RsslEncodeIterator iter;

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

		//first entry
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		RsslUInt64 uint64 = 64;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );

		//second entry
		rsslEEntry.name.data = ( char* )"Element - Real";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_REAL;
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&real );

		//third entry
		rsslEEntry.name.data = ( char* )"Element - Int";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_INT;
		//RsslInt rsslInt = 13;
		RsslInt64 int64 = 32;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&int64 );

		//fourth entry
		rsslEEntry.name.data = ( char* )"Element - Date";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_DATE;
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&date );

		//fifth entry
		rsslEEntry.name.data = ( char* )"Element - Time";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_TIME;
		//RsslTime rsslTime = {10, 21, 16, 777};
		RsslTime rsslTime = {02, 03, 04, 005};
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

		//sixth entry
		rsslEEntry.name.data = ( char* )"Element - DateTime";
		rsslEEntry.name.length = 18;
		rsslEEntry.dataType = RSSL_DT_DATETIME;
		RsslDateTime dateTime;
		dateTime.date.day = 7;
		dateTime.date.month = 11;
		dateTime.date.year = 1999;
		dateTime.time.hour = 01;
		dateTime.time.minute = 02;
		dateTime.time.second = 03;
		dateTime.time.millisecond = 000;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&dateTime );

		//seventh entry (nested ElementList)
		rsslEEntry.name.data = ( char* )"Element - ElementList";
		rsslEEntry.name.length = 21;
		rsslEEntry.dataType = RSSL_DT_ELEMENT_LIST;
		RsslElementList nestedElementList = RSSL_INIT_ELEMENT_LIST;
		rsslEncodeElementEntryInit( &iter, &rsslEEntry, 1000 );
		nestedElementList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
		nestedElementList.elementListNum = 5;
		rsslEncodeElementListInit( &iter, &nestedElementList, 0, 0 );
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		uint64 = 641;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );
		rsslEncodeElementListComplete( &iter, RSSL_TRUE );
		rsslEncodeElementEntryComplete( &iter, RSSL_TRUE );

		//eightth entry
		rsslEEntry.name.data = ( char* )"Element - Qos";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslQos );

		rsslEncodeElementListComplete( &iter, RSSL_TRUE );

		//Now do EMA decoding of ElementList
		ElementList el;
		StaticDecoder::setRsslData( &el, &rsslBuf, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_TRUE( el.hasInfo() ) << "ElementList with primitives and ElementList - hasInfo()" ;
		EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with primitives and ElementList - getInfoElementListNum()" ;


		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - first forth()" ;

		const ElementEntry& ee1 = el.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - second forth()" ;

		const ElementEntry& ee2 = el.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		el.reset();
		{
			EXPECT_TRUE( el.hasInfo() ) << "ElementList with primitives and ElementList - hasInfo()" ;
			EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with primitives and ElementList- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = el.getEntry();
				EXPECT_FALSE( true ) << "ElementList with primitives and ElementList- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with primitives and ElementList- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList- first forth() again" ;

			const ElementEntry& ee1 = el.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - second forth() again" ;

			const ElementEntry& ee2 = el.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - third forth()" ;

		const ElementEntry& ee3 = el.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - fourth forth()" ;

		const ElementEntry& ee4 = el.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - fifth forth()" ;

		const ElementEntry& ee5 = el.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - sixth forth()" ;

		const ElementEntry& ee6 = el.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - seventh forth()" ;

		const ElementEntry& ee7 = el.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - ElementList" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::ElementListEnum ) << "ElementEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::ElementListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& nestedEl = ee7.getElementList();

			EXPECT_TRUE( nestedEl.hasInfo() ) << "ElementEntry ElementList within elementlist - hasInfo()" ;
			EXPECT_EQ( nestedEl.getInfoElementListNum(), 5 ) << "ElementEntry ElementList within elementlist - getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = nestedEl.getEntry();
				EXPECT_FALSE( true ) << "ElementEntry ElementList within elementlist - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementEntry ElementList within elementlist - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( nestedEl.forth() ) << "ElementEntry ElementList within elementlist - first elementlist forth()" ;
			const ElementEntry& ee1 = nestedEl.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getUInt(), 641 ) << "ElementEntry::getUInt()" ;

			EXPECT_FALSE( nestedEl.forth() ) << "MapEntry ElementList within elementlist - final elementlist forth()" ;
		}

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and ElementList - eightth forth()" ;

		const ElementEntry& ee8 = el.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee8.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_FALSE( el.forth() ) << "ElementList with primitives and ElementList - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with primitives and ElementList - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with primitives and ElementList - exception not expected" ;
	}
}

//encode with UPA and decode with EMA
TEST(ElementListTests, testElementListContainsMapDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, Map, QOS
		RsslElementList rsslEL;
		RsslEncodeIterator iter;

		rsslClearElementList( &rsslEL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 4096 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslEL.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
		rsslEL.elementListNum = 5;

		rsslEncodeElementListInit( &iter, &rsslEL, 0, 0 );

		RsslElementEntry rsslEEntry = RSSL_INIT_ELEMENT_ENTRY;

		//first entry
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		RsslUInt64 uint64 = 64;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );

		//second entry
		rsslEEntry.name.data = ( char* )"Element - Real";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_REAL;
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&real );

		//third entry
		rsslEEntry.name.data = ( char* )"Element - Int";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_INT;
		//RsslInt rsslInt = 13;
		RsslInt64 int64 = 32;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&int64 );

		//fourth entry
		rsslEEntry.name.data = ( char* )"Element - Date";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_DATE;
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&date );

		//fifth entry
		rsslEEntry.name.data = ( char* )"Element - Time";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_TIME;
		//RsslTime rsslTime = {10, 21, 16, 777};
		RsslTime rsslTime = {02, 03, 04, 005};
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

		//sixth entry
		rsslEEntry.name.data = ( char* )"Element - DateTime";
		rsslEEntry.name.length = 18;
		rsslEEntry.dataType = RSSL_DT_DATETIME;
		RsslDateTime dateTime;
		dateTime.date.day = 7;
		dateTime.date.month = 11;
		dateTime.date.year = 1999;
		dateTime.time.hour = 01;
		dateTime.time.minute = 02;
		dateTime.time.second = 03;
		dateTime.time.millisecond = 000;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&dateTime );

		//seventh entry (nested Map)
		rsslEEntry.name.data = ( char* )"Element - Map";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_MAP;
		RsslMap nestedMap = RSSL_INIT_MAP;
		rsslEncodeElementEntryInit( &iter, &rsslEEntry, 2048 );
		nestedMap.flags = RSSL_MPF_HAS_KEY_FIELD_ID;
		nestedMap.containerType = RSSL_DT_FIELD_LIST;
		nestedMap.keyPrimitiveType = RSSL_DT_BUFFER;
		nestedMap.keyFieldId = 3426;
		RsslBuffer rsslBuf1;
		rsslBuf1.length = 1000;
		rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
		rsslEncodeMapInit( &iter, &nestedMap, 0, 0 );
		RsslMapEntry mapEntry;
		rsslClearMapEntry( &mapEntry );
		RsslEncodeFieldListAll( rsslBuf1 );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		RsslBuffer orderBuf;
		orderBuf.data = const_cast<char*>("ABCD");
		orderBuf.length = 4;
		rsslEncodeMapEntry( &iter, &mapEntry, &orderBuf );
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("EFGHI");
		orderBuf.length = 5;
		rsslEncodeMapEntry( &iter, &mapEntry, &orderBuf );
		rsslClearMapEntry( &mapEntry );
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		mapEntry.encData = rsslBuf1;
		orderBuf.data = const_cast<char*>("JKLMNOP");
		orderBuf.length = 7;
		rsslEncodeMapEntry( &iter, &mapEntry, &orderBuf );
		rsslEncodeMapComplete( &iter, RSSL_TRUE );
		rsslEncodeElementEntryComplete( &iter, RSSL_TRUE );

		//eightth entry
		rsslEEntry.name.data = ( char* )"Element - Qos";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslQos );

		rsslEncodeElementListComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of ElementList
		ElementList el;
		StaticDecoder::setRsslData( &el, &rsslBuf, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( el.hasInfo() ) << "ElementList with primitives and Map - hasInfo()" ;
		EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with primitives and Map- getInfoElementListNum()" ;


		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - first forth()" ;

		const ElementEntry& ee1 = el.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - second forth()" ;

		const ElementEntry& ee2 = el.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		el.reset();
		{
			EXPECT_TRUE( el.hasInfo() ) << "ElementList with primitives and Map - hasInfo()" ;
			EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "ElementList with primitives and Map- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = el.getEntry();
				EXPECT_FALSE( true ) << "ElementList with primitives and Map- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with primitives and Map- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map- first forth() again" ;

			const ElementEntry& ee1 = el.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - second forth() again" ;

			const ElementEntry& ee2 = el.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - third forth()" ;

		const ElementEntry& ee3 = el.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - fourth forth()" ;

		const ElementEntry& ee4 = el.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - fifth forth()" ;

		const ElementEntry& ee5 = el.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - sixth forth()" ;

		const ElementEntry& ee6 = el.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - seventh forth()" ;

		const ElementEntry& ee7 = el.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - Map" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::MapEnum ) << "ElementEntry::getLoadType() == DataType::MapEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::MapEnum ) << "ElementEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& map = ee7.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "MapEntry Map within elementlist - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "MapEntry Map within elementlist - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within elementlist - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within elementlist - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me2a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within elementlist - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "MapEntry Map within elementlist - fourth map forth()" ;
		}

		EXPECT_TRUE( el.forth() ) << "ElementList with primitives and Map - eightth forth()" ;

		const ElementEntry& ee8 = el.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee8.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_FALSE( el.forth() ) << "ElementList with primitives and Map - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with primitives and Map - exception not expected" ;

		free( rsslBuf.data );

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with primitives and Map - exception not expected" ;
	}
}

TEST(ElementListTests, testElementListDecodetoString)
{

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING, RMTES_STRING
		RsslElementList rsslEL;
		RsslEncodeIterator iter;

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
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&dateTime );

		rsslEEntry.name.data = ( char* )"Element - RsslQos";
		rsslEEntry.name.length = 17;
		rsslEEntry.dataType = RSSL_DT_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslQos );

		rsslEEntry.name.data = ( char* )"Element - RsslState";
		rsslEEntry.name.length = 19;
		rsslEEntry.dataType = RSSL_DT_STATE;
		RsslState rsslState = RSSL_INIT_STATE;
		rsslState.streamState = RSSL_STREAM_OPEN;
		rsslState.dataState = RSSL_DATA_OK;
		rsslState.code = RSSL_SC_NONE;
		rsslState.text.data = ( char* )"Succeeded";
		rsslState.text.length = 9;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslState );

		rsslEEntry.name.data = ( char* )"Element - RsslAsciiString";
		rsslEEntry.name.length = 25;
		rsslEEntry.dataType = RSSL_DT_ASCII_STRING;
		RsslBuffer ascii;
		ascii.data = const_cast<char*>("ABCDEF");
		ascii.length = 6;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&ascii );

		rsslEEntry.name.data = ( char* )"Element - RsslRmtesString";
		rsslEEntry.name.length = 25;
		rsslEEntry.dataType = RSSL_DT_RMTES_STRING;
		RsslBuffer news;
		news.data = const_cast<char*>("ABCDEF");
		news.length = 6;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&news );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		rsslEncodeElementListComplete( &iter, RSSL_TRUE );

		ElementList el;
		StaticDecoder::setRsslData( &el, &rsslBuf, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "ElementList toString Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList toString Decode - exception not expected" ;
	}
}

// ElementList encode & decode tests (encoding by EMA and decoding by EMA)
TEST(ElementListTests, testElementListEncodeDecodeAll)
{

	ElementList elEnc;
	elEnc.info( 5 );

	try
	{
		//EMA Encoding

		//first entry
		elEnc.addUInt( EmaString( "Element - UInt" ), 64 );

		//second entry
		elEnc.addReal( EmaString( "Element - Real" ), 11, OmmReal::ExponentNeg2Enum );

		//third entry
		elEnc.addInt( EmaString( "Element - Int" ), 32 );

		//fourth entry
		elEnc.addDate( EmaString( "Element - Date" ), 1999, 11, 7 );

		//fifth entry
		elEnc.addTime( EmaString( "Element - Time" ), 02, 03, 04, 005 );

		//sixth entry
		elEnc.addDateTime( EmaString( "Element - DateTime" ), 1999, 11, 7, 01, 02, 03, 000 );

		//seventh entry
		elEnc.addQos( EmaString( "Element - Qos" ), 45623, OmmQos::JustInTimeConflatedEnum );

		//eightth entry
		elEnc.addState( EmaString( "Element - State" ), OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );

		//ninth entry
		elEnc.addAscii( EmaString( "Element - AsciiString" ), EmaString( "ABCDEF" ) );

		//tenth entry
		char* s1 = const_cast<char*>("ABCDEF");
		EmaBuffer buf1( s1, 6 );
		elEnc.addRmtes( EmaString( "Element - RmtesString" ), buf1 );

		//eleventh entry
		elEnc.addEnum( EmaString( "Element - Enum" ), 29 );

		//twelfth entry
		elEnc.addFloat( EmaString( "Element - Float" ), 11.11f );

		//thirteenth entry
		elEnc.addDouble( EmaString( "Element - Double" ), 22.22f );

		//fourteenth entry
		elEnc.addCodeReal( EmaString( "Element - RealBlank" ) );

		//fifteenth entry
		char* s2 = const_cast<char*>("ABCDEFGH");
		EmaBuffer buf2( s2, 8 );
		elEnc.addBuffer( EmaString( "Element - Buffer" ), buf2 );

		//sixteenth entry
		char* s3 = const_cast<char*>("ABCDEFGH");
		EmaBuffer buf3( s3, 8 );
		elEnc.addUtf8( EmaString( "Element - Utf8String" ), buf3 );

		//seventeenth entry
		OmmArray ar1;
		ar1.addInt( 123 ).addInt( 234 ).addInt( 345 ).complete();
		elEnc.addArray( EmaString( "Element - OmmArray" ), ar1 );

		//eighteenth entry
		char* s4 = const_cast<char*>("OPQRST");
		EmaBuffer buf4( s4, 6 );
		OmmOpaque opaque;
		opaque.set( buf4 );
		elEnc.addOpaque( EmaString( "Element - Opaque" ), opaque );

		elEnc.complete();

		//Decoding
		StaticDecoder::setData( &elEnc, 0 );


		EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with all data types - hasInfo()" ;
		EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with all data types - getInfoElementListNum()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - first forth()" ;

		const ElementEntry& ee1 = elEnc.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - second forth()" ;

		const ElementEntry& ee2 = elEnc.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		elEnc.reset();
		{
			EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with all data types - hasInfo()" ;
			EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with all data types- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = elEnc.getEntry();
				EXPECT_FALSE( true ) << "ElementList with all data types- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with all data types- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types- first forth() again" ;
			const ElementEntry& ee1 = elEnc.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - second forth() again" ;
			const ElementEntry& ee2 = elEnc.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - third forth()" ;
		const ElementEntry& ee3 = elEnc.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - fourth forth()" ;
		const ElementEntry& ee4 = elEnc.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - fifth forth()" ;
		const ElementEntry& ee5 = elEnc.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - sixth forth()" ;
		const ElementEntry& ee6 = elEnc.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types seventh forth()" ;
		const ElementEntry& ee7 = elEnc.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getQos().getTimeliness(), 45623 ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee7.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - eightth forth()" ;
		const ElementEntry& ee8 = elEnc.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - State" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::StateEnum ) << "ElementEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ee8.getLoad().getDataType(), DataType::StateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getState().getStreamState(), OmmState::OpenEnum ) << "ElementEntry::getState().getStreamState()" ;
		EXPECT_EQ( ee8.getState().getDataState(), OmmState::OkEnum ) << "ElementEntry::getState().getDataState()" ;
		EXPECT_EQ( ee8.getState().getStatusCode(), OmmState::NoneEnum ) << "ElementEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ee8.getState().getStatusText(), "Succeeded" ) << "ElementEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ee8.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "ElementEntry::getState().toString()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - ninth forth()" ;
		const ElementEntry& ee9 = elEnc.getEntry();
		EXPECT_STREQ( ee9.getName(), "Element - AsciiString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee9.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( ee9.getLoad().getDataType(), DataType::AsciiEnum ) << "ElementEntry::getLoad().getDataType() == DataType::AsciiEnum" ;
		EXPECT_EQ( ee9.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee9.getAscii(), "ABCDEF" ) << "ElementEntry::getAscii()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - tenth forth()" ;
		const ElementEntry& ee10 = elEnc.getEntry();
		EXPECT_STREQ( ee10.getName(), "Element - RmtesString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee10.getLoadType(), DataType::RmtesEnum ) << "ElementEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( ee10.getLoad().getDataType(), DataType::RmtesEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( ee10.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee10.getRmtes().getAsUTF8(), EmaBuffer( "ABCDEF", 6 ) ) << "ElementEntry::getRmtes()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - eleventh forth()" ;
		const ElementEntry& ee11 = elEnc.getEntry();
		EXPECT_STREQ( ee11.getName(), "Element - Enum" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee11.getLoadType(), DataType::EnumEnum ) << "ElementEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ee11.getLoad().getDataType(), DataType::EnumEnum ) << "ElementEntry::getLoad().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( ee11.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee11.getEnum(), 29 ) << "ElementEntry::getEnum()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - twelfth forth()" ;
		const ElementEntry& ee12 = elEnc.getEntry();
		EXPECT_STREQ( ee12.getName(), "Element - Float" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee12.getLoadType(), DataType::FloatEnum ) << "ElementEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ee12.getLoad().getDataType(), DataType::FloatEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( ee12.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee12.getFloat(), 11.11f ) << "ElementEntry::getFloat()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - thirteenth forth()" ;
		const ElementEntry& ee13 = elEnc.getEntry();
		EXPECT_STREQ( ee13.getName(), "Element - Double" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee13.getLoadType(), DataType::DoubleEnum ) << "ElementEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ee13.getLoad().getDataType(), DataType::DoubleEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ee13.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee13.getDouble(), 22.22f ) << "ElementEntry::getDouble()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - fourteenth forth()" ;
		const ElementEntry& ee14 = elEnc.getEntry();
		EXPECT_STREQ( ee14.getName(), "Element - RealBlank" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee14.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee14.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( ee14.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - fifteenth forth()" ;
		const ElementEntry& ee15 = elEnc.getEntry();
		EXPECT_STREQ( ee15.getName(), "Element - Buffer" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee15.getLoadType(), DataType::BufferEnum ) << "ElementEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( ee15.getLoad().getDataType(), DataType::BufferEnum ) << "ElementEntry::getLoad().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( ee15.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee15.getBuffer(), EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getBuffer()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - sixteenth forth()" ;
		const ElementEntry& ee16 = elEnc.getEntry();
		EXPECT_STREQ( ee16.getName(), "Element - Utf8String" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee16.getLoadType(), DataType::Utf8Enum ) << "ElementEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( ee16.getLoad().getDataType(), DataType::Utf8Enum ) << "ElementEntry::getLoad().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( ee16.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee16.getUtf8(), EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getUtf8()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - seventeenth forth()" ;
		const ElementEntry& ee17 = elEnc.getEntry();
		EXPECT_STREQ( ee17.getName(), "Element - OmmArray" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee17.getLoadType(), DataType::ArrayEnum ) << "ElementEntry::getLoadType() == DataType::ArrayEnum" ;
		EXPECT_EQ( ee17.getLoad().getDataType(), DataType::ArrayEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ArrayEnum" ;
		EXPECT_EQ( ee17.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		const OmmArray& ar2 = ee17.getArray();
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within elemenlist - first forth()" ;
		const OmmArrayEntry& ae1 = ar2.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae1.getInt(), 123 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within elemenlist - second forth()" ;
		const OmmArrayEntry& ae2 = ar2.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 234 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within elemenlist - third forth()" ;
		const OmmArrayEntry& ae3 = ar2.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), 345 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_FALSE( ar2.forth() ) << "OmmArray within elemenlist - final forth()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with all data types - eighteenth forth()" ;
		const ElementEntry& ee18 = elEnc.getEntry();
		EXPECT_STREQ( ee18.getName(), "Element - Opaque" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee18.getLoadType(), DataType::OpaqueEnum ) << "ElementEntry::getLoadType() == DataType::OpaqueEnum" ;
		EXPECT_EQ( ee18.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		const OmmOpaque& opaque2 = ee18.getOpaque();
		char* sop = const_cast<char*>("OPQRST");
		EXPECT_STREQ( opaque2.getBuffer(), EmaBuffer( sop, 6 ) ) << "ElementEntry::getOpaque().getBuffer()::c_buf()" ;

		EXPECT_FALSE( elEnc.forth() ) << "ElementList with all data types - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with all data types - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with all data types - exception not expected" ;
	}

	elEnc.clear();
	elEnc.info( 6 );

	try
	{
		//EMA Encoding

		//first entry
		elEnc.addUInt( EmaString( "Element - UInt" ), 64 );
		//second entry
		elEnc.addCodeUInt( EmaString( "Element - Blank UInt" ) );

		//third entry
		elEnc.addReal( EmaString( "Element - Real" ), 11, OmmReal::ExponentNeg2Enum );
		//fourth entry
		elEnc.addCodeReal( EmaString( "Element - Blank Real" ) );

		//fifth entry
		elEnc.addInt( EmaString( "Element - Int" ), 32 );
		//sixth entry
		elEnc.addCodeInt( EmaString( "Element - Blank Int" ) );

		//seventh entry
		elEnc.addDate( EmaString( "Element - Date" ), 1999, 11, 7 );
		//eightth entry
		elEnc.addCodeDate( EmaString( "Element - Blank Date" ) );

		//ninth entry
		elEnc.addTime( EmaString( "Element - Time" ), 02, 03, 04, 005 );
		//tenth entry
		elEnc.addCodeTime( EmaString( "Element - Blank Time" ) );

		//eleventh entry
		elEnc.addDateTime( EmaString( "Element - DateTime" ), 1999, 11, 7, 01, 02, 03, 000 );
		//twelfth entry
		elEnc.addCodeDateTime( EmaString( "Element - Blank DateTime" ) );

		//thirteenth entry
		elEnc.addQos( EmaString( "Element - Qos" ), OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );
		//fourteenth entry
		elEnc.addCodeQos( EmaString( "Element - Blank Qos" ) );

		//fifteenth entry
		elEnc.addState( EmaString( "Element - State" ), OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );
		//sixteenth entry
		elEnc.addCodeState( EmaString( "Element - Blank State" ) );

		//seventeenth entry
		elEnc.addAscii( EmaString( "Element - AsciiString" ), EmaString( "ABCDEF" ) );
		//eighteenth entry
		elEnc.addCodeAscii( EmaString( "Element - Blank AsciiString" ) );

		//nineteenth entry
		char* s1 = const_cast<char*>("ABCDEF");
		elEnc.addRmtes( EmaString( "Element - RmtesString" ), EmaBuffer( s1, 6 ) );
		//twentieth entry
		elEnc.addCodeRmtes( EmaString( "Element - Blank RmtesString" ) );

		//21st entry
		elEnc.addEnum( EmaString( "Element - Enum" ), 29 );
		//22nd entry
		elEnc.addCodeEnum( EmaString( "Element - Blank Enum" ) );

		//23rd entry
		elEnc.addFloat( EmaString( "Element - Float" ), 11.11f );
		//24th entry
		elEnc.addCodeFloat( EmaString( "Element - Blank Float" ) );

		//25th entry
		elEnc.addDouble( EmaString( "Element - Double" ), 22.22f );
		//26th entry
		elEnc.addCodeDouble( EmaString( "Element - Blank Double" ) );

		//27th entry
		elEnc.addCodeReal( EmaString( "Element - RealBlank" ) );

		//28th entry
		char* s2 = const_cast<char*>("ABCDEFGH");
		elEnc.addBuffer( EmaString( "Element - Buffer" ), EmaBuffer( s2, 8 ) );
		//29th entry
		elEnc.addCodeBuffer( EmaString( "Element - Blank Buffer" ) );

		//30th entry
		char* s3 = const_cast<char*>("ABCDEFGH");
		elEnc.addUtf8( EmaString( "Element - Utf8String" ), EmaBuffer( s3, 8 ) );
		//31st entry
		elEnc.addCodeUtf8( EmaString( "Element - Blank Utf8String" ) );

		//32nd entry
		OmmArray ar1;
		ar1.addInt( 123 ).addInt( 234 ).addInt( 345 ).complete();
		elEnc.addArray( EmaString( "Element - OmmArray" ), ar1 );

		//33rd entry
		char* s4 = const_cast<char*>("OPQRST");
		EmaBuffer buf4( s4, 6 );
		OmmOpaque opaque;
		opaque.set( buf4 );
		elEnc.addOpaque( EmaString( "Element - Opaque" ), opaque );

		elEnc.complete();


		//Decoding
		StaticDecoder::setData( &elEnc, 0 );


		EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList after clear() - hasInfo()" ;
		EXPECT_EQ( elEnc.getInfoElementListNum(), 6 ) << "ElementList after clear() - getInfoElementListNum()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - first forth()" ;
		const ElementEntry& ee1 = elEnc.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - second forth()" ;
		const ElementEntry& ee1b = elEnc.getEntry();
		EXPECT_STREQ( ee1b.getName(), "Element - Blank UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1b.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - third forth()" ;
		const ElementEntry& ee2 = elEnc.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - fourth forth()" ;
		const ElementEntry& ee2b = elEnc.getEntry();
		EXPECT_STREQ( ee2b.getName(), "Element - Blank Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2b.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - fifth forth()" ;
		const ElementEntry& ee3 = elEnc.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - sixth forth()" ;
		const ElementEntry& ee3b = elEnc.getEntry();
		EXPECT_STREQ( ee3b.getName(), "Element - Blank Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3b.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - seventh forth()" ;
		const ElementEntry& ee4 = elEnc.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - eighth forth()" ;
		const ElementEntry& ee4b = elEnc.getEntry();
		EXPECT_STREQ( ee4b.getName(), "Element - Blank Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4b.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - ninth forth()" ;
		const ElementEntry& ee5 = elEnc.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - tenth forth()" ;
		const ElementEntry& ee5b = elEnc.getEntry();
		EXPECT_STREQ( ee5b.getName(), "Element - Blank Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5b.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - eleventh forth()" ;
		const ElementEntry& ee6 = elEnc.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - twelfth forth()" ;
		const ElementEntry& ee6b = elEnc.getEntry();
		EXPECT_STREQ( ee6b.getName(), "Element - Blank DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6b.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - thriteenth forth()" ;
		const ElementEntry& ee7 = elEnc.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee7.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - fourteenth forth()" ;
		const ElementEntry& ee7b = elEnc.getEntry();
		EXPECT_STREQ( ee7b.getName(), "Element - Blank Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7b.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee7b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - fifteenth forth()" ;
		const ElementEntry& ee8 = elEnc.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - State" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::StateEnum ) << "ElementEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ee8.getLoad().getDataType(), DataType::StateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getState().getStreamState(), OmmState::OpenEnum ) << "ElementEntry::getState().getStreamState()" ;
		EXPECT_EQ( ee8.getState().getDataState(), OmmState::OkEnum ) << "ElementEntry::getState().getDataState()" ;
		EXPECT_EQ( ee8.getState().getStatusCode(), OmmState::NoneEnum ) << "ElementEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ee8.getState().getStatusText(), "Succeeded" ) << "ElementEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ee8.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "ElementEntry::getState().toString()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - sixteenth forth()" ;
		const ElementEntry& ee8b = elEnc.getEntry();
		EXPECT_STREQ( ee8b.getName(), "Element - Blank State" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8b.getLoadType(), DataType::StateEnum ) << "ElementEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ee8b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - seventeenth forth()" ;
		const ElementEntry& ee9 = elEnc.getEntry();
		EXPECT_STREQ( ee9.getName(), "Element - AsciiString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee9.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( ee9.getLoad().getDataType(), DataType::AsciiEnum ) << "ElementEntry::getLoad().getDataType() == DataType::AsciiEnum" ;
		EXPECT_EQ( ee9.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee9.getAscii(), "ABCDEF" ) << "ElementEntry::getAscii()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - eighteenth forth()" ;
		const ElementEntry& ee9b = elEnc.getEntry();
		EXPECT_STREQ( ee9b.getName(), "Element - Blank AsciiString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee9b.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( ee9b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - nineteenth forth()" ;
		const ElementEntry& ee10 = elEnc.getEntry();
		EXPECT_STREQ( ee10.getName(), "Element - RmtesString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee10.getLoadType(), DataType::RmtesEnum ) << "ElementEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( ee10.getLoad().getDataType(), DataType::RmtesEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( ee10.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee10.getRmtes().getAsUTF8(), EmaBuffer( "ABCDEF", 6 ) ) << "ElementEntry::getRmtes()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - twentieth forth()" ;
		const ElementEntry& ee10b = elEnc.getEntry();
		EXPECT_STREQ( ee10b.getName(), "Element - Blank RmtesString" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee10b.getLoadType(), DataType::RmtesEnum ) << "ElementEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( ee10b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 21st forth()" ;
		const ElementEntry& ee11 = elEnc.getEntry();
		EXPECT_STREQ( ee11.getName(), "Element - Enum" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee11.getLoadType(), DataType::EnumEnum ) << "ElementEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ee11.getLoad().getDataType(), DataType::EnumEnum ) << "ElementEntry::getLoad().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( ee11.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee11.getEnum(), 29 ) << "ElementEntry::getEnum()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 22nd forth()" ;
		const ElementEntry& ee11b = elEnc.getEntry();
		EXPECT_STREQ( ee11b.getName(), "Element - Blank Enum" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee11b.getLoadType(), DataType::EnumEnum ) << "ElementEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ee11b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 23rd forth()" ;
		const ElementEntry& ee12 = elEnc.getEntry();
		EXPECT_STREQ( ee12.getName(), "Element - Float" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee12.getLoadType(), DataType::FloatEnum ) << "ElementEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ee12.getLoad().getDataType(), DataType::FloatEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( ee12.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee12.getFloat(), 11.11f ) << "ElementEntry::getFloat()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 24th forth()" ;
		const ElementEntry& ee12b = elEnc.getEntry();
		EXPECT_STREQ( ee12b.getName(), "Element - Blank Float" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee12b.getLoadType(), DataType::FloatEnum ) << "ElementEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ee12b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 25th forth()" ;
		const ElementEntry& ee13 = elEnc.getEntry();
		EXPECT_STREQ( ee13.getName(), "Element - Double" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee13.getLoadType(), DataType::DoubleEnum ) << "ElementEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ee13.getLoad().getDataType(), DataType::DoubleEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ee13.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee13.getDouble(), 22.22f ) << "ElementEntry::getDouble()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 26th forth()" ;
		const ElementEntry& ee13b = elEnc.getEntry();
		EXPECT_STREQ( ee13b.getName(), "Element - Blank Double" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee13b.getLoadType(), DataType::DoubleEnum ) << "ElementEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ee13b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 27th forth()" ;
		const ElementEntry& ee14 = elEnc.getEntry();
		EXPECT_STREQ( ee14.getName(), "Element - RealBlank" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee14.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee14.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( ee14.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 28th forth()" ;
		const ElementEntry& ee15 = elEnc.getEntry();
		EXPECT_STREQ( ee15.getName(), "Element - Buffer" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee15.getLoadType(), DataType::BufferEnum ) << "ElementEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( ee15.getLoad().getDataType(), DataType::BufferEnum ) << "ElementEntry::getLoad().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( ee15.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee15.getBuffer(), EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getBuffer()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 29th forth()" ;
		const ElementEntry& ee15b = elEnc.getEntry();
		EXPECT_STREQ( ee15b.getName(), "Element - Blank Buffer" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee15b.getLoadType(), DataType::BufferEnum ) << "ElementEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( ee15b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 30th forth()" ;
		const ElementEntry& ee16 = elEnc.getEntry();
		EXPECT_STREQ( ee16.getName(), "Element - Utf8String" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee16.getLoadType(), DataType::Utf8Enum ) << "ElementEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( ee16.getLoad().getDataType(), DataType::Utf8Enum ) << "ElementEntry::getLoad().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( ee16.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( ee16.getUtf8(), EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getUtf8()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 31st forth()" ;
		const ElementEntry& ee16b = elEnc.getEntry();
		EXPECT_STREQ( ee16b.getName(), "Element - Blank Utf8String" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee16b.getLoadType(), DataType::Utf8Enum ) << "ElementEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( ee16b.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 32nd forth()" ;
		const ElementEntry& ee17 = elEnc.getEntry();
		EXPECT_STREQ( ee17.getName(), "Element - OmmArray" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee17.getLoadType(), DataType::ArrayEnum ) << "ElementEntry::getLoadType() == DataType::ArrayEnum" ;
		EXPECT_EQ( ee17.getLoad().getDataType(), DataType::ArrayEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ArrayEnum" ;
		EXPECT_EQ( ee17.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		const OmmArray& ar2 = ee17.getArray();
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within elemenlist - first forth()" ;
		const OmmArrayEntry& ae1 = ar2.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae1.getInt(), 123 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within elemenlist - second forth()" ;
		const OmmArrayEntry& ae2 = ar2.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 234 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within elemenlist - third forth()" ;
		const OmmArrayEntry& ae3 = ar2.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), 345 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_FALSE( ar2.forth() ) << "OmmArray within elemenlist - final forth()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList after clear() - 33rd forth()" ;
		const ElementEntry& ee18 = elEnc.getEntry();
		EXPECT_STREQ( ee18.getName(), "Element - Opaque" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee18.getLoadType(), DataType::OpaqueEnum ) << "ElementEntry::getLoadType() == DataType::OpaqueEnum" ;
		EXPECT_EQ( ee18.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		const OmmOpaque& opaque2 = ee18.getOpaque();
		char* sop = const_cast<char*>("OPQRST");
		EXPECT_STREQ( opaque2.getBuffer(), EmaBuffer( sop, 6 ) ) << "ElementEntry::getOpaque().getBuffer()::c_buf()" ;

		EXPECT_FALSE( elEnc.forth() ) << "ElementList after clear() - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with all data types - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode ElementList after clear() - exception not expected" ;
	}
}

TEST(ElementListTests, testElementListContainsFieldListEncodeDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	ElementList elEnc;
	elEnc.info( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, FieldList, QOS

		//first entry
		elEnc.addUInt( EmaString( "Element - UInt" ), 64 );

		//second entry
		elEnc.addReal( EmaString( "Element - Real" ), 11, OmmReal::ExponentNeg2Enum );

		//third entry
		elEnc.addInt( EmaString( "Element - Int" ), 32 );

		//fourth entry
		elEnc.addDate( EmaString( "Element - Date" ), 1999, 11, 7 );

		//fifth entry
		elEnc.addTime( EmaString( "Element - Time" ), 02, 03, 04, 005 );

		//sixth entry
		elEnc.addDateTime( EmaString( "Element - DateTime" ), 1999, 11, 7, 01, 02, 03, 000 );

		//seventh entry (nested FieldList)
		FieldList flEnc;
		flEnc.addUInt( 1, 641 );
		flEnc.complete();
		elEnc.addFieldList( EmaString( "Element - FieldList" ), flEnc );

		//eightth entry
		elEnc.addQos( EmaString( "Element - Qos" ), OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );

		elEnc.complete();


		//Now do EMA decoding of ElementList
		StaticDecoder::setData( &elEnc, &dictionary );


		EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with primitives and FieldList - hasInfo()" ;
		EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with primitives and FieldList- getInfoElementListNum()" ;


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - first forth()" ;

		const ElementEntry& ee1 = elEnc.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - second forth()" ;

		const ElementEntry& ee2 = elEnc.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		elEnc.reset();
		{
			EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with primitives and FieldList - hasInfo()" ;
			EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with primitives and FieldList- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = elEnc.getEntry();
				EXPECT_FALSE( true ) << "ElementList with primitives and FieldList- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with primitives and FieldList- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList- first forth() again" ;

			const ElementEntry& ee1 = elEnc.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - second forth() again" ;

			const ElementEntry& ee2 = elEnc.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - third forth()" ;

		const ElementEntry& ee3 = elEnc.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - fourth forth()" ;

		const ElementEntry& ee4 = elEnc.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - fifth forth()" ;

		const ElementEntry& ee5 = elEnc.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - sixth forth()" ;

		const ElementEntry& ee6 = elEnc.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - seventh forth()" ;

		const ElementEntry& ee7 = elEnc.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - FieldList" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::FieldListEnum ) << "ElementEntry::getLoadType() == DataType::FieldListEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::FieldListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::FieldListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		{
			const FieldList& nestedFl = ee7.getFieldList();

			EXPECT_FALSE( nestedFl.hasInfo() ) << "ElementEntry FieldList within elementlist - hasInfo()" ;

			EXPECT_TRUE( nestedFl.forth() ) << "ElementEntry FieldList within elementlist - first fieldlist forth()" ;
			const FieldEntry& fe1 = nestedFl.getEntry();
			EXPECT_EQ( fe1.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe1.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe1.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe1.getUInt(), 641 ) << "FieldEntry::getUInt()" ;

			EXPECT_FALSE( nestedFl.forth() ) << "ElementEntry FieldList within elementlist - second fieldlist forth()" ;
		}

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and FieldList - eightth forth()" ;

		const ElementEntry& ee8 = elEnc.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee8.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_FALSE( elEnc.forth() ) << "ElementList with primitives and FieldList - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with primitives and FieldList - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList contains FieldList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(ElementListTests, testElementListContainsElementListEncodeDecodeAll)
{

	ElementList elEnc;
	elEnc.info( 5 );

	try
	{
		//EMA Encoding
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, QOS

		//first entry
		elEnc.addUInt( EmaString( "Element - UInt" ), 64 );

		//second entry
		elEnc.addReal( EmaString( "Element - Real" ), 11, OmmReal::ExponentNeg2Enum );

		//third entry
		elEnc.addInt( EmaString( "Element - Int" ), 32 );

		//fourth entry
		elEnc.addDate( EmaString( "Element - Date" ), 1999, 11, 7 );

		//fifth entry
		elEnc.addTime( EmaString( "Element - Time" ), 02, 03, 04, 005 );

		//sixth entry
		elEnc.addDateTime( EmaString( "Element - DateTime" ), 1999, 11, 7, 01, 02, 03, 000 );

		//seventh entry (nested ElementList)
		ElementList elEnc1;
		elEnc1.info( 5 );
		elEnc1.addUInt( EmaString( "Element - UInt" ), 641 );
		elEnc1.complete();
		elEnc.addElementList( EmaString( "Element - ElementList" ), elEnc1 );

		//eightth entry
		elEnc.addQos( EmaString( "Element - Qos" ), OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );

		elEnc.complete();


		//Now do EMA decoding of ElementList
		StaticDecoder::setData( &elEnc, 0 );


		EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with primitives and ElementList - hasInfo()" ;
		EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with primitives and ElementList - getInfoElementListNum()" ;


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - first forth()" ;

		const ElementEntry& ee1 = elEnc.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - second forth()" ;

		const ElementEntry& ee2 = elEnc.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		elEnc.reset();
		{
			EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with primitives and ElementList - hasInfo()" ;
			EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with primitives and ElementList- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = elEnc.getEntry();
				EXPECT_FALSE( true ) << "ElementList with primitives and ElementList- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with primitives and ElementList- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList- first forth() again" ;

			const ElementEntry& ee1 = elEnc.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - second forth() again" ;

			const ElementEntry& ee2 = elEnc.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - third forth()" ;

		const ElementEntry& ee3 = elEnc.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - fourth forth()" ;

		const ElementEntry& ee4 = elEnc.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - fifth forth()" ;

		const ElementEntry& ee5 = elEnc.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - sixth forth()" ;

		const ElementEntry& ee6 = elEnc.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - seventh forth()" ;

		const ElementEntry& ee7 = elEnc.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - ElementList" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::ElementListEnum ) << "ElementEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::ElementListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::ElementListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		{
			const ElementList& nestedEl = ee7.getElementList();

			EXPECT_TRUE( nestedEl.hasInfo() ) << "ElementEntry ElementList within elementlist - hasInfo()" ;
			EXPECT_EQ( nestedEl.getInfoElementListNum(), 5 ) << "ElementEntry ElementList within elementlist - getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = nestedEl.getEntry();
				EXPECT_FALSE( true ) << "ElementEntry ElementList within elementlist - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementEntry ElementList within elementlist - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( nestedEl.forth() ) << "ElementEntry ElementList within elementlist - first elementlist forth()" ;
			const ElementEntry& ee1 = nestedEl.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getUInt(), 641 ) << "ElementEntry::getUInt()" ;

			EXPECT_FALSE( nestedEl.forth() ) << "ElementEntry ElementList within elementlist - final elementlist forth()" ;
		}

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - eightth forth()" ;

		const ElementEntry& ee8 = elEnc.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee8.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_FALSE( elEnc.forth() ) << "ElementList with primitives and ElementList - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with primitives and ElementList - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with primitives and ElementList - exception not expected" ;
	}
}

TEST(ElementListTests, testElementListContainsMapEncodeDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	ElementList elEnc;
	EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

	elEnc.info( 5 );
	EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

	try
	{
		//EMA Encoding
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, Map, QOS

		//first entry
		elEnc.addUInt( EmaString( "Element - UInt" ), 64 );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//second entry
		elEnc.addReal( EmaString( "Element - Real" ), 11, OmmReal::ExponentNeg2Enum );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//third entry
		elEnc.addInt( EmaString( "Element - Int" ), 32 );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//fourth entry
		elEnc.addDate( EmaString( "Element - Date" ), 1999, 11, 7 );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//fifth entry
		elEnc.addTime( EmaString( "Element - Time" ), 02, 03, 04, 005 );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//sixth entry
		elEnc.addDateTime( EmaString( "Element - DateTime" ), 1999, 11, 7, 01, 02, 03, 000 );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//seventh entry (nested Map)
		Map mapEnc1;
		EmaEncodeMapAll( mapEnc1 );
		elEnc.addMap( EmaString( "Element - Map" ), mapEnc1 );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		//eightth entry
		elEnc.addQos( EmaString( "Element - Qos" ), OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";

		elEnc.complete();
		EXPECT_EQ( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() == Decoding of just encoded object in the same application is not supported";


		//Now do EMA decoding of ElementList
		StaticDecoder::setData( &elEnc, &dictionary );
		EXPECT_NE( elEnc.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ElementList.toString() != Decoding of just encoded object in the same application is not supported";


		EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with primitives and ElementList - hasInfo()" ;
		EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with primitives and ElementList - getInfoElementListNum()" ;


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - first forth()" ;

		const ElementEntry& ee1 = elEnc.getEntry();
		EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - second forth()" ;

		const ElementEntry& ee2 = elEnc.getEntry();
		EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;

		elEnc.reset();
		{
			EXPECT_TRUE( elEnc.hasInfo() ) << "ElementList with primitives and ElementList - hasInfo()" ;
			EXPECT_EQ( elEnc.getInfoElementListNum(), 5 ) << "ElementList with primitives and ElementList- getInfoElementListNum()" ;

			try
			{
				const ElementEntry& ee = elEnc.getEntry();
				EXPECT_FALSE( true ) << "ElementList with primitives and ElementList- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "ElementList with primitives and ElementList- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList- first forth() again" ;

			const ElementEntry& ee1 = elEnc.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()" ;

			EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - second forth() again" ;

			const ElementEntry& ee2 = elEnc.getEntry();
			EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()" ;
		}


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - third forth()" ;

		const ElementEntry& ee3 = elEnc.getEntry();
		EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - fourth forth()" ;

		const ElementEntry& ee4 = elEnc.getEntry();
		EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()" ;
		EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()" ;
		EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - fifth forth()" ;

		const ElementEntry& ee5 = elEnc.getEntry();
		EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()" ;
		EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()" ;
		EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()" ;
		EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - sixth forth()" ;

		const ElementEntry& ee6 = elEnc.getEntry();
		EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - seventh forth()" ;

		const ElementEntry& ee7 = elEnc.getEntry();
		EXPECT_STREQ( ee7.getName(), "Element - Map" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee7.getLoadType(), DataType::MapEnum ) << "ElementEntry::getLoadType() == DataType::MapEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::MapEnum ) << "ElementEntry::getLoad().getDataType() == DataType::MapEnum" ;
		EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee7.getLoad().getDataType(), DataType::MapEnum ) << "ElementEntry::getLoad().getDataType() == DataType::MapEnum" ;
		{
			const Map& nestedMap = ee7.getMap();
			EXPECT_TRUE( nestedMap.hasKeyFieldId() ) << "ElementEntry Map within elementlist - hasKeyFieldId()" ;
			EXPECT_EQ( nestedMap.getKeyFieldId(), 3426 ) << "ElementEntry Map within elementlist - getKeyFieldId()" ;

			EXPECT_TRUE( nestedMap.forth() ) << "ElementEntry Map within elementlist - first map forth()" ;
			const MapEntry& me1a = nestedMap.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "ABCD", 4 );
				EXPECT_STREQ( me1a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( nestedMap.forth() ) << "ElementEntry Map within elementlist - second map forth()" ;
			const MapEntry& me2a = nestedMap.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			        const FieldList& fl = me2a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}

			EXPECT_TRUE( nestedMap.forth() ) << "ElementEntry Map within elementlist - third map forth()" ;
			const MapEntry& me3a = nestedMap.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			{
				EmaBuffer Buf( "EFGHI", 5 );
				EXPECT_STREQ( me3a.getKey().getBuffer(), Buf ) << "MapEntry::getKey().getBuffer()" ;
			}
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
				const FieldList& fl = me3a.getFieldList();
				SCOPED_TRACE("calling EmaDecodeFieldListAll");
				EmaDecodeFieldListAll( fl );
			}
		}


		EXPECT_TRUE( elEnc.forth() ) << "ElementList with primitives and ElementList - eightth forth()" ;

		const ElementEntry& ee8 = elEnc.getEntry();
		EXPECT_STREQ( ee8.getName(), "Element - Qos" ) << "ElementEntry::getName()" ;
		EXPECT_EQ( ee8.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( ee8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( ee8.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getTime().getRate()" ;

		EXPECT_FALSE( elEnc.forth() ) << "ElementList with primitives and ElementList - final forth()" ;

		EXPECT_TRUE( true ) << "ElementList with primitives and ElementList - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList with primitives and ElementList - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(ElementListTests, testElementListPrePostBindElementList)
{

	try
	{

		// Encode ElementList via prebind
		ElementList elementList1;
		{
			ElementList elementList;
			elementList1.addElementList( EmaString( "ElementList Payload" ), elementList );
			EmaEncodeElementListAll( elementList );
			elementList1.complete();
			StaticDecoder::setData( &elementList1, 0 );

		}

		// Encode ElementList via postbind
		ElementList elementList2;
		{
			ElementList elementList;
			EmaEncodeElementListAll( elementList );
			elementList2.addElementList( EmaString( "ElementList Payload" ), elementList );
			elementList2.complete();
			StaticDecoder::setData( &elementList2, 0 );

		}

		EXPECT_STREQ( elementList1.toString(), elementList2.toString() ) << "Pre/Post-bound ElementLists are equal - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound ElementLists are equal - exception not expected" ;
	}
}

TEST(ElementListTests, testElementListPrePostBindFieldList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode FieldList via prebind
		ElementList elementList1;
		{
			FieldList fieldList;
			elementList1.addFieldList( EmaString( "FieldList Payload" ), fieldList );
			EmaEncodeFieldListAll( fieldList );
			elementList1.complete();
			StaticDecoder::setData( &elementList1, &dictionary );
		}

		// Encode FieldList via postbind
		ElementList elementList2;
		{
			FieldList fieldList;
			EmaEncodeFieldListAll( fieldList );
			elementList2.addFieldList( EmaString( "FieldList Payload" ), fieldList );
			elementList2.complete();
			StaticDecoder::setData( &elementList2, &dictionary );
		}

		EXPECT_STREQ( elementList1.toString(), elementList2.toString() ) << "Pre/Post-bound FieldLists are equal - exception not expected";

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound FieldLists are equal - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(ElementListTests, testElementListHybrid)
{

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME

		RsslElementList rsslEL;
		RsslEncodeIterator iter;

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

		//first entry
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		RsslUInt64 uint64 = 64;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&uint64 );

		//second entry
		rsslEEntry.name.data = ( char* )"Element - Real";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_REAL;
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&real );

		//third entry
		rsslEEntry.name.data = ( char* )"Element - Int";
		rsslEEntry.name.length = 13;
		rsslEEntry.dataType = RSSL_DT_INT;
		RsslInt64 int64 = 32;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&int64 );

		//fourth entry
		rsslEEntry.name.data = ( char* )"Element - Date";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_DATE;
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&date );

		//fifth entry
		rsslEEntry.name.data = ( char* )"Element - Time";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_TIME;
		//RsslTime rsslTime = {10, 21, 16, 777};
		RsslTime rsslTime = {02, 03, 04, 005};
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&rsslTime );

		//sixth entry
		rsslEEntry.name.data = ( char* )"Element - DateTime";
		rsslEEntry.name.length = 18;
		rsslEEntry.dataType = RSSL_DT_DATETIME;
		RsslDateTime dateTime;
		dateTime.date.day = 7;
		dateTime.date.month = 11;
		dateTime.date.year = 1999;
		dateTime.time.hour = 01;
		dateTime.time.minute = 02;
		dateTime.time.second = 03;
		dateTime.time.millisecond = 000;
		rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&dateTime );

		rsslEncodeElementListComplete( &iter, RSSL_TRUE );


		// Convert RsslElementList into EMA's ElementList
		ElementList decodedElementList, encodedElementList;
		StaticDecoder::setRsslData( &decodedElementList, &rsslBuf, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		// encode Series with the above EMA's ElementList
		Series series;
		series.summaryData( decodedElementList );
		series.add( decodedElementList );
		series.add( encodedElementList );

		encodedElementList.addUInt( "UINT", 123 ).complete();

		series.complete();

		StaticDecoder::setData( &series, 0 );


		EXPECT_EQ( series.getSummaryData().getDataType(), DataType::ElementListEnum ) << "Series::getSummaryData().getDataType()" ;

		EXPECT_TRUE( series.forth() ) << "first Series::forth()" ;
		const SeriesEntry& se1 = series.getEntry();

		EXPECT_EQ( se1.getLoadType(), DataType::ElementListEnum ) << "first Series::getEntry()::getLoadType()" ;

		EXPECT_TRUE( series.forth() ) << "second Series::forth()" ;
		const SeriesEntry& se2 = series.getEntry();

		EXPECT_EQ( se2.getLoadType(), DataType::ElementListEnum ) << "second Series::getEntry()::getLoadType()" ;

		EXPECT_FALSE( series.forth() ) << "third Series::forth()" ;

		EXPECT_TRUE( true ) << "ElementList Hybrid Usage - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList Hybrid Usage - exception not expected" ;
	}
}

TEST(ElementListTests, testElementListError)
{

	{
		try
		{
			ElementList el;
			el.complete();
			EXPECT_TRUE( true ) << "ElementList::complete() on empty element list - exception not expected" ;

			StaticDecoder::setData( &el, 0 );


			EXPECT_FALSE( el.forth() ) << "ElementList::forth()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "ElementList::complete() on empty element list - exception not expected" ;
		}
	}

	{
		try
		{
			ElementList el;
			el.info( 1 );
			el.complete();
			EXPECT_TRUE( true ) << "ElementList::complete() on empty element list with info - exception not expected" ;

			StaticDecoder::setData( &el, 0 );


			EXPECT_TRUE( el.hasInfo() ) << "ElementList::hasInfo()" ;

			EXPECT_EQ( el.getInfoElementListNum(), 1 ) << "ElementList::getInfoElementListNum()" ;

			EXPECT_FALSE( el.forth() ) << "ElementList::forth()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "ElementList::complete() on empty element list with info - exception not expected" ;
		}
	}

	{
		try
		{
			ElementList el;
			el.addAscii( "entry 1", "value 1" );
			el.addAscii( "entry 2", "value 2" );
			el.complete();

			el.addAscii( "entry 3", "value 3" );

			EXPECT_FALSE( true ) << "ElementList::addAscii() after complete() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addAscii() after complete() - exception expected" ;
		}
	}

	{
		try
		{
			ElementList el;
			el.addAscii( "entry 1", "value 1" );
			el.addAscii( "entry 2", "value 2" );
			el.complete();

			el.clear();

			el.addAscii( "entry 3", "value 3" ).complete();

			EXPECT_TRUE( true ) << "ElementList::addAscii() after complete() & clear() - exception not expected" ;

			StaticDecoder::setData( &el, 0 );


			EXPECT_TRUE( el.forth() ) << "ElementList::forth()" ;
			EXPECT_FALSE( el.forth() ) << "ElementList::forth()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "ElementList::addAscii() after complete() & clear() - exception not expected" ;
		}
	}

	{
		try
		{
			ElementList el;
			el.addAscii( "entry 1", "value 1" );
			el.addAscii( "entry 2", "value 2" );
			el.complete();

			OmmArray arr;
			arr.addAscii( "array entry" ).complete();

			el.addArray( "entry 3", arr ).complete();

			EXPECT_FALSE( true ) << "ElementList::addArray() after complete() - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addArray() after complete() - exception expected" ;
		}
	}

	{
		try
		{
			ElementList el;
			el.addAscii( "entry 1", "value 1" );
			el.addAscii( "entry 2", "value 2" );
			el.complete();

			OmmArray arr;

			el.addArray( "entry 3", arr ).complete();

			EXPECT_FALSE( true ) << "ElementList::addArray() with empty array after complete() - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addArray() with empty array after complete() - exception expected" ;
		}
	}

	{
		try
		{
			ElementList el;
			el.addAscii( "entry 1", "value 1" );
			el.addAscii( "entry 2", "value 2" );
			el.complete();

			el.clear();

			OmmArray arr;
			arr.addAscii( "array entry" ).complete();

			el.addArray( "entry 3", arr ).complete();

			EXPECT_TRUE( true ) << "ElementList::addArray() after complete() & clear() - exception not expected" ;

			StaticDecoder::setData( &el, 0 );


			EXPECT_TRUE( el.forth() ) << "ElementList::forth()" ;
			EXPECT_FALSE( el.forth() ) << "ElementList::forth()" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "ElementList::addArray() after complete() & clear() - exception not expected" ;
		}
	}

	{
		try
		{
			ElementList el;

			OmmArray arr;
			arr.addAscii( "array entry" );

			el.addArray( "entry 1", arr );

			el.complete();

			EXPECT_FALSE( true ) << "ElementList::addArray() without OmmArray::complete() - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addArray() without OmmArray::complete() - exception expected" ;
		}
	}


	{
		try
		{
			ElementList el;

			RefreshMsg msg;

			el.addRefreshMsg( "entry", msg );

			el.complete();

			EXPECT_FALSE( true ) << "ElementList::addRefreshMsg() while message is empty - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addRefreshMsg() while message is empty - exception expected" ;
		}
	}

	{
		try
		{
			ElementList el;

			RefreshMsg msg;

			msg.streamId( 1 );

			el.addRefreshMsg( "entry", msg );

			el.complete();

			StaticDecoder::setData( &el, 0 );


			EXPECT_TRUE( el.forth() ) << "ElementList::forth()" ;

			EXPECT_EQ( el.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "ElementEntry::getLoadType()" ;

			EXPECT_EQ( static_cast<const RefreshMsg&>( el.getEntry().getLoad() ).getStreamId(), 1 ) << "RefreshMsg::getStreamId()" ;

			EXPECT_TRUE( true ) << "ElementList::addRefreshMsg() while message is populated - exception not expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "ElementList::addRefreshMsg() while message is populated - exception not expected" ;
		}
	}

	{
		try
		{
			ElementList el;

			RefreshMsg msg;

			msg.streamId( 1 );

			msg.clear();

			el.addRefreshMsg( "entry", msg );

			el.complete();

			EXPECT_FALSE( true ) << "ElementList::addRefreshMsg() while message is populated then cleared - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addRefreshMsg() while message is populated then cleared - exception expected" ;
		}
	}

	{
		try
		{
			ElementList el;

			GenericMsg msg;

			el.addGenericMsg( "entry", msg );

			el.complete();

			EXPECT_FALSE( true ) << "ElementList::addGenericMsg() while message is empty - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addGenericMsg() while message is empty - exception expected" ;
		}
	}

	{
		try
		{
			ElementList el;

			GenericMsg msg;

			msg.streamId( 1 );

			el.addGenericMsg( "entry", msg );

			el.complete();

			StaticDecoder::setData( &el, 0 );


			EXPECT_TRUE( el.forth() ) << "ElementList::forth()" ;

			EXPECT_EQ( el.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "ElementEntry::getLoadType()" ;

			EXPECT_EQ( static_cast<const GenericMsg&>( el.getEntry().getLoad() ).getStreamId(), 1) << "GenericMsg::getStreamId()";

			EXPECT_TRUE( true ) << "ElementList::addGenericMsg() while message is populated - exception not expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "ElementList::addGenericMsg() while message is populated - exception not expected" ;
		}
	}

	{
		try
		{
			ElementList el;

			GenericMsg msg;

			msg.streamId( 1 );

			msg.clear();

			el.addGenericMsg( "entry", msg );

			el.complete();

			EXPECT_FALSE( true ) << "ElementList::addGenericMsg() while message is populated then cleared - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "ElementList::addGenericMsg() while message is populated then cleared - exception expected" ;
		}
	}

}

TEST(ElementListTests, testElementListEmpty_Encode_Decode)
{
	try
	{
		ElementList elementList;
		elementList.info(5).complete();

		Series series;
		series.add(elementList).complete();

		StaticDecoder::setData(&series, NULL);

		EXPECT_TRUE(series.forth()) << "Check the first Series";

		const ElementList& elementListDec = series.getEntry().getElementList();

		EXPECT_TRUE(elementListDec.hasInfo()) << "Check has info attribute";
		EXPECT_TRUE(elementListDec.getInfoElementListNum() == 5) << "Check the element list info num attribute";
		EXPECT_FALSE(elementListDec.forth()) << "Check to make sure that there is no element entry";

		EXPECT_FALSE(series.forth()) << "Check to make sure that there is no enty in Series";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode and decode empty ElementList - exception not expected with text" << exp.getText().c_str();
	}
}

TEST(ElementListTests, testElementEntryWithNoPayload_Encode_Decode)
{
	try
	{
		ElementList elementList;
		elementList.info(5)
			.add("Name1").add("Name2")
			.complete();

		StaticDecoder::setData(&elementList, NULL);

		EXPECT_TRUE(elementList.hasInfo()) << "Check has info attribute";
		EXPECT_TRUE(elementList.getInfoElementListNum() == 5) << "Check the element list info num attribute";

		EXPECT_TRUE(elementList.forth()) << "Get the first ElementList entry";
		EXPECT_TRUE(elementList.getEntry().getName() == "Name1" ) << "Check the element name the first entry";
		EXPECT_TRUE(elementList.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the first entry";

		EXPECT_TRUE(elementList.forth()) << "Get the second ElementList entry";
		EXPECT_TRUE(elementList.getEntry().getName() == "Name2") << "Check the element name the second entry";
		EXPECT_TRUE(elementList.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second entry";

		EXPECT_FALSE(elementList.forth()) << "Check to make sure that there is no more enty in ElementList";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode and decode ElementList - exception not expected with text" << exp.getText().c_str();
	}
}

TEST(ElementListTests, testElementListAddInfoAfterInitialized)
{
	try
	{
		ElementList elementList;
		elementList.add("ElementName").info(5).complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Encode info after ElementList is initialized - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Invalid attempt to call info() when container is initialized.", exp.getText().c_str());
		return;
	}

	EXPECT_TRUE(false) << "Encode total count hint after ElementList is initialized - exception expected";
}

TEST(ElementListTests, testElementListAddEntryAfterCallingComplete_Encode)
{
	try
	{
		ElementList elementList;
		elementList.info(1).complete();
		elementList.add("Name1");
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Fails to encode ElementList after the complete() is called - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Attempt to add an entry after complete() was called.", exp.getText().c_str());
	}
}

TEST(ElementListTests, testElementListClear_Encode_Decode)
{
	try
	{
		// load dictionary for decoding of the field list
		RsslDataDictionary dictionary;

		ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

		FieldList fieldList;
		fieldList.addUInt(1, 3056).complete();

		ElementList elementList;
		elementList.info(4)
			.addFieldList("1", fieldList)
			.clear().info(6)
			.addFieldList("2", fieldList)
			.add("3")
			.complete();

		StaticDecoder::setData(&elementList, &dictionary);

		EXPECT_TRUE(elementList.hasInfo()) << "Check has info attribute";
		EXPECT_TRUE(elementList.getInfoElementListNum() == 6) << "Check the info value attribute";

		EXPECT_TRUE(elementList.forth()) << "Get the first Element entry";
		EXPECT_TRUE(elementList.getEntry().getName() == "2") << "Check the name of the first entry";
		EXPECT_TRUE(elementList.getEntry().getLoadType() == DataType::FieldListEnum) << "Check the load type of the first entry";

		const FieldList& fieldListDec = elementList.getEntry().getFieldList();

		EXPECT_TRUE(fieldListDec.forth()) << "Check the first field entry";
		EXPECT_TRUE(fieldListDec.getEntry().getFieldId() == 1) << "Check the field ID of first entry";
		EXPECT_TRUE(fieldListDec.getEntry().getUInt() == 3056) << "Check the value of first entry";

		EXPECT_FALSE(fieldListDec.forth()) << "Check to make sure that there is no more entry";
		
		EXPECT_TRUE(elementList.forth()) << "Get the second Element entry";
		EXPECT_TRUE(elementList.getEntry().getName() == "3") << "Check the name of the second entry";
		EXPECT_TRUE(elementList.getEntry().getLoadType() == DataType::NoDataEnum) << "Check the load type of the second entry";

		EXPECT_FALSE(elementList.forth()) << "Check to make sure that there is no more enty in ElementList";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode and decode ElementList - exception not expected with text" << exp.getText().c_str();
	}
}
