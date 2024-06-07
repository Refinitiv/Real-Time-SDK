/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

TEST(FieldListTests, testInvalidDate)
{

	try
	{
		FieldList().addTime( 267, 10, 11, 22, 1000, 900, 900 ).complete();
		EXPECT_FALSE( true ) << "FieldList set invalid date - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "FieldList set invalid date - exception expected" ;
	}
}

TEST(FieldListTests, testFieldListDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,
		//                  RMTES_STRING, ENUM, FLOAT, DOUBLE, BLANK REAL, BUFFRER, UTF8_STRING,
		//					OPAUE, XML, ANSI_PAGE
		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;

		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		// fid not found case (first)
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = -100;
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		//second entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		//third entry
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

		//fourth entry
		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );

		//fifth entry
		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );

		//sixth entry
		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );

		//seventh entry
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
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );

		//eigthth entry
		rsslFEntry.dataType = RSSL_DT_QOS;
		rsslFEntry.fieldId = FID_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslQos );

		//ninth entry
		rsslFEntry.dataType = RSSL_DT_STATE;
		rsslFEntry.fieldId = FID_STATE;
		RsslState rsslState = RSSL_INIT_STATE;
		rsslState.streamState = RSSL_STREAM_OPEN;
		rsslState.dataState = RSSL_DATA_OK;
		rsslState.code = RSSL_SC_NONE;
		rsslState.text.data = ( char* )"Succeeded";
		rsslState.text.length = 9;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslState );

		//tenth entry
		rsslFEntry.dataType = RSSL_DT_ASCII_STRING;
		rsslFEntry.fieldId = 715;		// STORY_ID + ASCII
		RsslBuffer ascii;
		ascii.data = const_cast<char*>("ABCDEF");
		ascii.length = 6;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&ascii );

		//eleventh entry
		rsslFEntry.dataType = RSSL_DT_RMTES_STRING;
		rsslFEntry.fieldId = 28;		// NEWS + RMTES
		RsslBuffer news;
		news.data = const_cast<char*>("ABCDEF");
		news.length = 6;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&news );

		//twelfth entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 4;		// RDN_EXCHID + ENUM
		RsslEnum enumm = 29;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&enumm );

		//thirteenth entry
		rsslFEntry.dataType = RSSL_DT_FLOAT;
		rsslFEntry.fieldId = -9;
		RsslFloat floatValue = 11.11f;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&floatValue );

		//fourteenth entry
		rsslFEntry.dataType = RSSL_DT_DOUBLE;
		rsslFEntry.fieldId = -10;
		RsslDouble doubleValue = 22.22f;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&doubleValue );

		//fifteenth entry (blank)
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 7;		// TRDPRC_1  + REAL
		RsslReal realValue;
		realValue.isBlank = RSSL_TRUE;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&realValue );

		//sixteenth entry
		rsslFEntry.dataType = RSSL_DT_BUFFER;
		rsslFEntry.fieldId = -11;		// MY_BUFFER
		RsslBuffer buffer;
		buffer.data = const_cast<char*>("ABCDEFGH");
		buffer.length = 8;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&buffer );

		//seventeenth entry
		rsslFEntry.dataType = RSSL_DT_UTF8_STRING;
		rsslFEntry.fieldId = -12;		// MY_UTF8
		RsslBuffer buffer_utf8;
		buffer_utf8.data = const_cast<char*>("KLMNOPQR");
		buffer_utf8.length = 8;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&buffer_utf8 );

		// eighteen entry
		rsslFEntry.fieldId = -17;
		rsslFEntry.dataType = RSSL_DT_OPAQUE;

		char opaqueBuffer[8];

		RsslBuffer buffer_opaque;
		buffer_opaque.data = opaqueBuffer;
		buffer_opaque.length = 8;

		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 0 );
		rsslEncodeNonRWFDataTypeInit( &iter, &buffer_opaque );

		memcpy( buffer_opaque.data , "KLMNOPQR", 8 );
		buffer_opaque.length = 8;

		rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_opaque, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

		// nineteenth entry
		rsslFEntry.fieldId = -1;
		rsslFEntry.dataType = RSSL_DT_XML;

		char xmlBuffer[25];

		RsslBuffer buffer_xml;
		buffer_xml.data = xmlBuffer;
		buffer_xml.length = 25;

		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 0 );
		rsslEncodeNonRWFDataTypeInit( &iter, &buffer_xml );

		memcpy( buffer_xml.data , "<value> KLMNOPQR </value>", 25 );
		buffer_xml.length = 25;

		rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_xml, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

		// twentieth entry
		rsslFEntry.fieldId = -18;
		rsslFEntry.dataType = RSSL_DT_ANSI_PAGE;

		char ansiPageBuffer[34];

		RsslBuffer buffer_ansiPage;
		buffer_ansiPage.data = ansiPageBuffer;
		buffer_ansiPage.length = 34;

		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 0 );
		rsslEncodeNonRWFDataTypeInit( &iter, &buffer_ansiPage );

		memcpy( buffer_ansiPage.data , "328-srfsjkj43rouw-01-20ru2l24903$%", 34 );
		buffer_ansiPage.length = 34;

		rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_ansiPage, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

		// twenty first entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 115;		// BID_TICK_1 + ENUM
		enumm = 0; // " "
		rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

		// twenty second entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 115;		// BID_TICK_1 + ENUM
		enumm = 1; // #DE#
		rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

		// twenty third entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 115;		// BID_TICK_1 + ENUM
		enumm = 2; // #FE#
		rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

		// twenty fourth entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 270;		// ACT_TP_1 + ENUM
		enumm = 26; // #42DE#
		rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

		// twenty fifth entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 54;		// LOTSZUNITS + ENUM
		enumm = 70; // #4B673533BD#
		rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

		// twenty sixth entry
		rsslFEntry.dataType = RSSL_DT_ENUM;
		rsslFEntry.fieldId = 8960;		// GBLISS_IND + ENUM
		enumm = 2; // "  EURO  "
		rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );

		//Now do EMA decoding of FieldList
		FieldList fl;
		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasInfo() ) << "FieldList with all data types - hasInfo()" ;
		EXPECT_EQ( fl.getInfoDictionaryId(), dictionary.info_DictionaryId ) << "FieldList with all data types- getInfoDictionaryId()";
		EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "FieldList with all data types- getInfoFieldListNum()" ;

		try
		{
			const FieldEntry& fe = fl.getEntry();
			EXPECT_FALSE( true ) << "FieldList with all data types- exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FieldList with all data types- exception expected: " ).append( excp.getText() ) ;
		}


		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types- first forth()" ;

		const FieldEntry& fe1 = fl.getEntry();

		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		try
		{
			UInt64 intValue = fe1.getUInt();
			EXPECT_FALSE( true ) << "FieldList with all data types - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "FieldList with all data types - exception expected: " ).append( excp.getText() ) ;
		}


		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - second forth()" ;

		const FieldEntry& fe2 = fl.getEntry();

		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - third forth()" ;

		const FieldEntry& fe3 = fl.getEntry();

		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;


		fl.reset();

		{
			EXPECT_TRUE( fl.hasInfo() ) << "FieldList with all data types - hasInfo()" ;
			EXPECT_EQ( fl.getInfoDictionaryId(), dictionary.info_DictionaryId ) << "FieldList with all data types- getInfoDictionaryId()";
			EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "FieldList with all data types- getInfoFieldListNum()" ;

			try
			{
				const FieldEntry& fe = fl.getEntry();
				EXPECT_FALSE( true ) << "FieldList with all data types- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "FieldList with all data types- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( fl.forth() ) << "FieldList with all data types- first forth() again" ;

			const FieldEntry& fe1 = fl.getEntry();

			EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
			EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

			try
			{
				UInt64 intValue = fe1.getUInt();
				EXPECT_FALSE( true ) << "FieldList with all data types - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "FieldList with all data types - exception expected: " ).append( excp.getText() ) ;
			}


			EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - second forth() again" ;

			const FieldEntry& fe2 = fl.getEntry();

			EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( fe2.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

			EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - third forth() again" ;

			const FieldEntry& fe3 = fl.getEntry();

			EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( fe3.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
			EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;
		}

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - fourth forth()" ;

		const FieldEntry& fe4 = fl.getEntry();

		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - fifth forth()" ;

		const FieldEntry& fe5 = fl.getEntry();

		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - sixth forth()" ;

		const FieldEntry& fe6 = fl.getEntry();

		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - seventh forth()" ;

		const FieldEntry& fe7 = fl.getEntry();

		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types eightth forth()" ;

		const FieldEntry& fe8 = fl.getEntry();

		EXPECT_EQ(  fe8.getFieldId(), FID_QOS ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_QOS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::QosEnum ) << "FieldEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "FieldEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( fe8.getQos().getRate(), OmmQos::TickByTickEnum ) << "FieldEntry::getTime().getRate()" ;
		EXPECT_STREQ( fe8.getQos().toString(), "RealTime/TickByTick" ) << "fe8.getQos().toString()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - ninth forth()" ;

		const FieldEntry& fe9 = fl.getEntry();

		EXPECT_EQ(  fe9.getFieldId(), FID_STATE ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "MY_STATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::StateEnum ) << "FieldEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( fe9.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe9.getState().getStreamState(), OmmState::OpenEnum ) << "FieldEntry::getState().getStreamState()" ;
		EXPECT_EQ( fe9.getState().getDataState(), OmmState::OkEnum ) << "FieldEntry::getState().getDataState()" ;
		EXPECT_EQ( fe9.getState().getStatusCode(), OmmState::NoneEnum ) << "FieldEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( fe9.getState().getStatusText(), "Succeeded" ) << "FieldEntry::getState().getStatusText()" ;
		EXPECT_STREQ( fe9.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "FieldEntry::getState().toString()" ;


		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - tenth forth()" ;

		const FieldEntry& fe10 = fl.getEntry();

		EXPECT_EQ( fe10.getFieldId(), 715 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe10.getName(), "STORY_ID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe10.getLoadType(), DataType::AsciiEnum ) << "FieldEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( fe10.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe10.getAscii(), "ABCDEF" ) << "FieldEntry::getAscii()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - eleventh forth()" ;

		const FieldEntry& fe11 = fl.getEntry();

		EXPECT_EQ( fe11.getFieldId(), 28 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe11.getName(), "NEWS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe11.getLoadType(), DataType::RmtesEnum ) << "FieldEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( fe11.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe11.getRmtes().getAsUTF8(), EmaBuffer( "ABCDEF", 6 ) ) << "FieldEntry::getRmtes()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twelfth forth()" ;

		const FieldEntry& fe12 = fl.getEntry();

		EXPECT_EQ( fe12.getFieldId(), 4 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe12.getName(), "RDN_EXCHID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe12.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( fe12.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe12.getEnum(), 29 ) << "FieldEntry::getEnum()" ;
		EXPECT_EQ(fe12.hasEnumDisplay(), true) << "FieldEntry::hasEnumDisplay()";
		EXPECT_STREQ( fe12.getEnumDisplay(), "CSC" ) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - thirteenth forth()" ;

		const FieldEntry& fe13 = fl.getEntry();

		EXPECT_EQ(  fe13.getFieldId(), -9 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe13.getName(), "MY_FLOAT" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe13.getLoadType(), DataType::FloatEnum ) << "FieldEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( fe13.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe13.getFloat(), 11.11f ) << "FieldEntry::getFloat()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - fourteenth forth()" ;

		const FieldEntry& fe14 = fl.getEntry();

		EXPECT_EQ(  fe14.getFieldId(), -10 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe14.getName(), "MY_DOUBLE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe14.getLoadType(), DataType::DoubleEnum ) << "FieldEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( fe14.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe14.getDouble(), 22.22f ) << "FieldEntry::getFloat()" ;

		EXPECT_TRUE( true ) << "FieldList with all data types - exception not expected" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - fifteenth forth()" ;

		const FieldEntry& fe15 = fl.getEntry();

		EXPECT_EQ( fe15.getFieldId(), 7 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe15.getName(), "TRDPRC_2" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe15.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe15.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - sixteenth forth()" ;

		const FieldEntry& fe16 = fl.getEntry();

		EXPECT_EQ(  fe16.getFieldId(), -11 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe16.getName(), "MY_BUFFER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe16.getLoadType(), DataType::BufferEnum ) << "FieldEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( fe16.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe16.getBuffer(), EmaBuffer( "ABCDEFGH", 8 ) ) << "FieldEntry::getBuffer()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - seventeenth forth()" ;

		const FieldEntry& fe17 = fl.getEntry();

		EXPECT_EQ(  fe17.getFieldId(), -12 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe17.getName(), "MY_UTF8" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe17.getLoadType(), DataType::Utf8Enum ) << "FieldEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( fe17.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe17.getUtf8(), EmaBuffer( "KLMNOPQR", 8 ) ) << "FieldEntry::getUtf8()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - eighteenth forth()" ;

		const FieldEntry& fe18 = fl.getEntry();

		EXPECT_EQ(  fe18.getFieldId(), -17 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe18.getName(), "MY_OPAQUE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe18.getLoadType(), DataType::OpaqueEnum ) << "FieldEntry::getLoadType() == DataType::OpaqueEnum" ;
		EXPECT_EQ( fe18.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe18.getOpaque().getBuffer(), EmaBuffer( "KLMNOPQR", 8 ) ) << "FieldEntry::getOpaque()::getBuffer()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - nineteenth forth()" ;

		const FieldEntry& fe19 = fl.getEntry();

		EXPECT_EQ(  fe19.getFieldId(), -1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe19.getName(), "XML" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe19.getLoadType(), DataType::XmlEnum ) << "FieldEntry::getLoadType() == DataType::XmlEnum" ;
		EXPECT_EQ( fe19.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe19.getXml().getBuffer(), EmaBuffer( "<value> KLMNOPQR </value>", 25 ) ) << "FieldEntry::getXml()::getBuffer()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twentieth forth()" ;

		const FieldEntry& fe20 = fl.getEntry();

		EXPECT_EQ(  fe20.getFieldId(), -18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe20.getName(), "MY_ANSI_PAGE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe20.getLoadType(), DataType::AnsiPageEnum ) << "FieldEntry::getLoadType() == DataType::AnsiPageEnum" ;
		EXPECT_EQ( fe20.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe20.getAnsiPage().getBuffer(), EmaBuffer( "328-srfsjkj43rouw-01-20ru2l24903$%", 34 ) ) << "FieldEntry::getAnsiPage()::getBuffer()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twenty first forth()" ;

		const FieldEntry& fe21 = fl.getEntry();

		EXPECT_EQ( fe21.getFieldId(), 115 ) << "FieldEntry::getFieldId()";
		EXPECT_STREQ( fe21.getName(), "BID_TICK_1" ) << "FieldEntry::getName()";
		EXPECT_EQ( fe21.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
		EXPECT_EQ( fe21.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
		EXPECT_EQ( fe21.getEnum(), 0 ) << "FieldEntry::getEnum()";
		EXPECT_TRUE( fe21.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";
		EXPECT_STREQ( fe21.getEnumDisplay(), " " ) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twenty second forth()";

		const FieldEntry& fe22 = fl.getEntry();

		EXPECT_EQ( fe22.getFieldId(), 115 ) << "FieldEntry::getFieldId()";
		EXPECT_STREQ( fe22.getName(), "BID_TICK_1" ) << "FieldEntry::getName()";
		EXPECT_EQ( fe22.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
		EXPECT_EQ( fe22.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
		EXPECT_EQ( fe22.getEnum(), 1 ) << "FieldEntry::getEnum()";
		EXPECT_TRUE( fe22.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";

		unsigned char expected[5] = {};
		expected[0] = 222;

		EXPECT_EQ( memcmp(fe22.getEnumDisplay().c_str(), expected, fe22.getEnumDisplay().length()), 0) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twenty third forth()";

		const FieldEntry& fe23 = fl.getEntry();

		EXPECT_EQ( fe23.getFieldId(), 115 ) << "FieldEntry::getFieldId()";
		EXPECT_STREQ( fe23.getName(), "BID_TICK_1" ) << "FieldEntry::getName()";
		EXPECT_EQ( fe23.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
		EXPECT_EQ( fe23.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
		EXPECT_EQ( fe23.getEnum(), 2 ) << "FieldEntry::getEnum()";
		EXPECT_TRUE( fe23.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";

		expected[0] = 254;
		EXPECT_EQ( memcmp(fe23.getEnumDisplay().c_str(), expected, fe23.getEnumDisplay().length()), 0) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twenty fourth forth()";

		const FieldEntry& fe24 = fl.getEntry();

		EXPECT_EQ( fe24.getFieldId(), 270 ) << "FieldEntry::getFieldId()";
		EXPECT_STREQ( fe24.getName(), "ACT_TP_1" ) << "FieldEntry::getName()";
		EXPECT_EQ( fe24.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
		EXPECT_EQ( fe24.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
		EXPECT_EQ( fe24.getEnum(), 26 ) << "FieldEntry::getEnum()";
		EXPECT_TRUE( fe24.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";

		expected[0] = 66;
		expected[1] = 222;

		EXPECT_EQ( memcmp(fe24.getEnumDisplay().c_str(), expected, fe24.getEnumDisplay().length()), 0) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twenty fifth forth()";

		const FieldEntry& fe25 = fl.getEntry();

		EXPECT_EQ( fe25.getFieldId(), 54 ) << "FieldEntry::getFieldId()";
		EXPECT_STREQ( fe25.getName(), "LOTSZUNITS" ) << "FieldEntry::getName()";
		EXPECT_EQ( fe25.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
		EXPECT_EQ( fe25.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
		EXPECT_EQ( fe25.getEnum(), 70 ) << "FieldEntry::getEnum()";
		EXPECT_EQ( fe25.hasEnumDisplay(), true ) << "FieldEntry::hasEnumDisplay()";

		expected[0] = 75;
		expected[1] = 103;
		expected[2] = 53;
		expected[3] = 51;
		expected[4] = 189;

		EXPECT_TRUE( memcmp(fe25.getEnumDisplay().c_str(), expected, fe25.getEnumDisplay().length()) == 0 ) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( fl.forth() ) << "FieldList with all data types - twenty sixth forth()";

		const FieldEntry& fe26 = fl.getEntry();

		EXPECT_EQ( fe26.getFieldId(), 8960 ) << "FieldEntry::getFieldId()";
		EXPECT_STREQ( fe26.getName(), "GBLISS_IND" ) << "FieldEntry::getName()";
		EXPECT_EQ( fe26.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
		EXPECT_EQ( fe26.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
		EXPECT_EQ( fe26.getEnum(), 2 ) << "FieldEntry::getEnum()";
		EXPECT_TRUE( fe26.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";
		EXPECT_STREQ( fe26.getEnumDisplay(), "  EURO  " ) << "FieldEntry::getEnumDisplay()";

		EXPECT_FALSE( fl.forth() ) << "FieldList with all data types - twenty seventh forth()";

		EXPECT_TRUE( true ) << "FieldList with all data types - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with all data types - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListContainsFieldListDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, FieldList, UINT

		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;

		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		// fid not found case (first)
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = -100;
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		//second entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		//third entry
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );


		//fourth entry
		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );


		//fifth entry
		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );

		//sixth entry
		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );

		//seventh entry
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
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );

		//eightth entry (nested FieldList)
		rsslFEntry.fieldId = -13;		// MY_FIELDLIST
		RsslFieldList nestedFieldList = RSSL_INIT_FIELD_LIST;
		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 1000 );
		nestedFieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		rsslEncodeFieldListInit( &iter, &nestedFieldList, 0, 0 );
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;	 // PROD_PERM + UINT
		uint64 = 641;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, &uint64 );
		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

		//ninth entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 642;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );

		//Now do EMA decoding of FieldList
		FieldList fl;
		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasInfo() ) << "FieldList with primitives and FieldList - hasInfo()" ;
		EXPECT_EQ( fl.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives and FieldList - getInfoDictionaryId()";
		EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "FieldList with primitives and FieldList - getInfoFieldListNum()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - first forth()" ;
		const FieldEntry& fe1 = fl.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - second forth()" ;
		const FieldEntry& fe2 = fl.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - third forth()" ;
		const FieldEntry& fe3 = fl.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - fourth forth()" ;
		const FieldEntry& fe4 = fl.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - fifth forth()" ;
		const FieldEntry& fe5 = fl.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - sixth forth()" ;
		const FieldEntry& fe6 = fl.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - seventh forth()" ;
		const FieldEntry& fe7 = fl.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - eightth forth()" ;
		const FieldEntry& fe8 = fl.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), -13 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_FIELDLIST" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::FieldListEnum ) << "FieldEntry::getLoadType() == DataType::FieldListEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::FieldListEnum ) << "FieldEntry::getCode() == DataType::FieldListEnum" ;
		{
			const FieldList& nestedFl = fe8.getFieldList();

			EXPECT_FALSE( nestedFl.hasInfo() ) << "FieldEntry FieldList within fieldlist - hasInfo()" ;

			EXPECT_TRUE( nestedFl.forth() ) << "FieldEntry FieldList within fieldlist - first fieldlist forth()" ;
			const FieldEntry& fe1 = nestedFl.getEntry();
			EXPECT_EQ( fe1.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe1.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe1.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe1.getUInt(), 641 ) << "FieldEntry::getUInt()" ;

			EXPECT_FALSE( nestedFl.forth() ) << "FieldEntry FieldList within fieldlist - second fieldlist forth()" ;
		}

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and FieldList - ninth forth()" ;
		const FieldEntry& fe9 = fl.getEntry();
		EXPECT_EQ( fe9.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getUInt(), 642 ) << "FieldEntry::getUInt()" ;

		EXPECT_FALSE( fl.forth() ) << "FieldList with primitives and FieldList - tenth forth()" ;

		EXPECT_TRUE( true ) << "FieldList with primitives and FieldList - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with primitives and FieldList - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListContainsElementListDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;

		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		// fid not found case (first)
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = -100;
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		//second entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		//third entry
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );


		//fourth entry
		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );

		//fifth entry
		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );

		//sixth entry
		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );

		//seventh entry
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
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );

		//eightth entry (ElementList)
		rsslFEntry.fieldId = -15;		// MY_ELEMENTLIST
		RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;
		RsslElementEntry rsslEEntry;
		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 1000 );
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
		elementList.elementListNum = 5;
		rsslEncodeElementListInit( &iter, &elementList, 0, 0 );
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		uint64 = 641;
		rsslEncodeElementEntry( &iter, &rsslEEntry, &uint64 );
		rsslEncodeElementListComplete( &iter, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );


		//ninth entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 642;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of FieldList
		FieldList fl;
		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasInfo() ) << "FieldList with primitives and FieldList - hasInfo()" ;
		EXPECT_EQ( fl.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives and FieldList - getInfoDictionaryId()";
		EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "FieldList with primitives and FieldList - getInfoFieldListNum()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - first forth()" ;
		const FieldEntry& fe1 = fl.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - second forth()" ;
		const FieldEntry& fe2 = fl.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - third forth()" ;
		const FieldEntry& fe3 = fl.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - fourth forth()" ;
		const FieldEntry& fe4 = fl.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - fifth forth()" ;
		const FieldEntry& fe5 = fl.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - sixth forth()" ;
		const FieldEntry& fe6 = fl.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - seventh forth()" ;
		const FieldEntry& fe7 = fl.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - eightth forth()" ;
		const FieldEntry& fe8 = fl.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), -15 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_ELEMENTLIST" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::ElementListEnum ) << "FieldEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::ElementListEnum ) << "FieldEntry::getCode() == DataType::ElementListEnum" ;
		{
			const ElementList& el = fe8.getElementList();

			EXPECT_TRUE( el.hasInfo() ) << "FieldEntry ElementList within fieldlist - hasInfo()" ;

			EXPECT_TRUE( el.forth() ) << "ElementEntry ElementList within fieldlist - first elementlist forth()" ;
			const ElementEntry& ee1 = el.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 641 ) << "ElementEntry::getUInt()" ;

			EXPECT_FALSE( el.forth() ) << "ElementEntry ElementList within fieldlist - second elementlist forth()" ;
		}


		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and ElementList - ninth forth()" ;
		const FieldEntry& fe9 = fl.getEntry();
		EXPECT_EQ( fe9.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getUInt(), 642 ) << "FieldEntry::getUInt()" ;


		EXPECT_FALSE( fl.forth() ) << "FieldList with primitives and ElementList - tenth forth()" ;

		EXPECT_TRUE( true ) << "FieldList with primitives and ElementList - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with primitives and ElementList - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListContainsMapDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, Map, UINT

		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 4096 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;

		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		// fid not found case (first)
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = -100;
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		//second entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		//third entry
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );


		//fourth entry
		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );


		//fifth entry
		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );


		//sixth entry
		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );


		//seventh entry
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
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );


		//eightth entry (nested Map)
		rsslFEntry.fieldId = -14;		// MY_MAP
		RsslMap nestedMap = RSSL_INIT_MAP;
		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 2048 );
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
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );


		//ninth entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 642;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of FieldList
		FieldList fl;
		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( fl.hasInfo() ) << "FieldList with primitives and Map - hasInfo()" ;
		EXPECT_EQ( fl.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives and Map - getInfoDictionaryId()";
		EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "FieldList with primitives and Map - getInfoFieldListNum()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - first forth()" ;
		const FieldEntry& fe1 = fl.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - second forth()" ;
		const FieldEntry& fe2 = fl.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - third forth()" ;
		const FieldEntry& fe3 = fl.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - fourth forth()" ;
		const FieldEntry& fe4 = fl.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - fifth forth()" ;
		const FieldEntry& fe5 = fl.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - sixth forth()" ;
		const FieldEntry& fe6 = fl.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - seventh forth()" ;
		const FieldEntry& fe7 = fl.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - eightth forth()" ;
		const FieldEntry& fe8 = fl.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), -14 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_MAP" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::MapEnum ) << "FieldEntry::getLoadType() == DataType::MapEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::MapEnum ) << "FieldEntry::getCode() == DataType::MapEnum" ;
		{
			const Map& map = fe8.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "MapEntry Map within fieldlist - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "MapEntry Map within fieldlist - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within fieldlist - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within fieldlist - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "MapEntry Map within fieldlist - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "MapEntry Map within fieldlist - fourth map forth()" ;
		}

		EXPECT_TRUE( fl.forth() ) << "FieldList with primitives and Map - ninth forth()" ;
		const FieldEntry& fe9 = fl.getEntry();
		EXPECT_EQ( fe9.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getUInt(), 642 ) << "FieldEntry::getUInt()" ;

		EXPECT_FALSE( fl.forth() ) << "FieldList with primitives and Map - tenth forth()" ;

		EXPECT_TRUE( true ) << "FieldList with primitives and Map - exception not expected" ;


		free( rsslBuf.data );
		free( rsslBuf1.data );

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with primitives and Map - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(FieldListTests, testFieldListEncodeDecodeAll)
{
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	DataDictionary emaDataDictionary, emaDataDictionaryEmpty;

	const EmaString fieldListString =
		"FieldList FieldListNum=\"65\" DictionaryId=\"1\"\n"
		"    FieldEntry fid=\"-100\" name=\"\" dataType=\"Error\"\n"
		"        OmmError\n"
		"            ErrorCode=\"FieldIdNotFound\"\n"
		"        OmmErrorEnd\n"
		"    FieldEntryEnd\n"
		"    FieldEntry fid=\"1\" name=\"PROD_PERM\" dataType=\"UInt\" value=\"64\"\n"
		"    FieldEntry fid=\"6\" name=\"TRDPRC_1\" dataType=\"Real\" value=\"0.11\"\n"
		"    FieldEntry fid=\"-2\" name=\"INTEGER\" dataType=\"Int\" value=\"32\"\n"
		"    FieldEntry fid=\"16\" name=\"TRADE_DATE\" dataType=\"Date\" value=\"07 NOV 1999\"\n"
		"    FieldEntry fid=\"18\" name=\"TRDTIM_1\" dataType=\"Time\" value=\"02:03:04:005:000:000\"\n"
		"    FieldEntry fid=\"-3\" name=\"TRADE_DATE\" dataType=\"DateTime\" value=\"07 NOV 1999 01:02:03:000:000:000\"\n"
		"    FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"Timeliness: 5656/Rate: 2345\"\n"
		"    FieldEntry fid=\"-6\" name=\"MY_STATE\" dataType=\"State\" value=\"Open / Ok / None / 'Succeeded'\"\n"
		"    FieldEntry fid=\"715\" name=\"STORY_ID\" dataType=\"Ascii\" value=\"ABCDEF\"\n"
		"    FieldEntry fid=\"28\" name=\"NEWS\" dataType=\"Rmtes\" value=\"ABCDEF\"\n"
		"    FieldEntry fid=\"4\" name=\"RDN_EXCHID\" dataType=\"Enum\" value=\"29\"\n"
		"    FieldEntry fid=\"-9\" name=\"MY_FLOAT\" dataType=\"Float\" value=\"11.11\"\n"
		"    FieldEntry fid=\"-10\" name=\"MY_DOUBLE\" dataType=\"Double\" value=\"22.21999931335449\"\n"
		"    FieldEntry fid=\"7\" name=\"TRDPRC_2\" dataType=\"Real\" value=\"(blank data)\"\n"
		"    FieldEntry fid=\"-11\" name=\"MY_BUFFER\" dataType=\"Buffer\"\n"
		"4142 4344 4546 4748                        ABCDEFGH\n"
		"\n"
		"    FieldEntry fid=\"-12\" name=\"MY_UTF8\" dataType=\"Utf8\" value=\"KLMNOPQR\"\n"
		"    FieldEntry fid=\"-16\" name=\"MY_ARRAY\" dataType=\"OmmArray\"\n"
		"        OmmArray with entries of dataType=\"Int\"\n"
		"            value=\"123\"\n"
		"            value=\"234\"\n"
		"            value=\"345\"\n"
		"        OmmArrayEnd\n"
		"    FieldEntryEnd\n"
		"    FieldEntry fid=\"-17\" name=\"MY_OPAQUE\" dataType=\"Opaque\" value=\"Opaque\n"
		"\n"
		"4f50 5152 5354                             OPQRST\n"
		"\n"
		"OpaqueEnd\n"
		"\"\n"
		"    FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"InexactDelayed/JustInTimeConflated\"\n"
		"FieldListEnd\n";

	try {
		emaDataDictionary.loadFieldDictionary( "RDMFieldDictionaryTest" );
		emaDataDictionary.loadEnumTypeDictionary( "enumtypeTest.def" );
	}
	catch ( const OmmException& ) {
		ASSERT_TRUE( false ) << "DataDictionary::loadFieldDictionary() failed to load dictionary information";
	}

	FieldList flEnc, flEmpty;
	EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

	flEnc.info( dictionary.info_DictionaryId, 65 );
	EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

	try
	{
		//EMA Encoding

		//first entry (fid not found case)
		flEnc.addUInt( -100, 64 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//second entry
		flEnc.addUInt( 1, 64 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//third entry
		flEnc.addReal( 6, 11, OmmReal::ExponentNeg2Enum );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//fourth entry
		flEnc.addInt( -2, 32 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//fifth entry
		flEnc.addDate( 16, 1999, 11, 7 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n") << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//sixth entry
		flEnc.addTime( 18, 02, 03, 04, 005 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//seventh entry
		flEnc.addDateTime( -3, 1999, 11, 7, 01, 02, 03, 000 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//eightth entry
		flEnc.addQos( FID_QOS, 5656, 2345 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n") << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//ninth entry
		flEnc.addState( FID_STATE, OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//tenth entry
		flEnc.addAscii( 715, EmaString( "ABCDEF" ) );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//eleventh entry
		char* s1 = const_cast<char*>("ABCDEF");
		flEnc.addRmtes( 28, EmaBuffer( s1, 6 ) );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//twelfth entry
		flEnc.addEnum( 4, 29 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//thirteenth entry
		flEnc.addFloat( -9, 11.11f );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//fourteenth entry
		flEnc.addDouble( -10, 22.22f );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//fifteenth entry (blank real)
		flEnc.addCodeReal( 7 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//sixteenth entry
		char* s2 = const_cast<char*>("ABCDEFGH");
		flEnc.addBuffer( -11, EmaBuffer( s2, 8 ) );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//seventeenth entry
		char* s3 = const_cast<char*>("KLMNOPQR");
		flEnc.addUtf8( -12, EmaBuffer( s3, 8 ) );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//eighteenth entry
		OmmArray ar1;
		ar1.addInt( 123 ).addInt( 234 ).addInt( 345 ).complete();
		flEnc.addArray( -16, ar1 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//nineteenth entry
		char* s4 = const_cast<char*>("OPQRST");
		EmaBuffer buf4( s4, 6 );
		OmmOpaque opaque;
		opaque.set( buf4 );
		flEnc.addOpaque( -17 , opaque );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		//twentyth entry
		flEnc.addQos( FID_QOS, 756565, 1232365 );
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( flEnc.toString(emaDataDictionary), "\nUnable to decode not completed FieldList data.\n" ) << "FieldList.toString() == Unable to decode not completed FieldList data.";

		flEnc.complete();
		EXPECT_EQ( flEnc.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "FieldList.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( flEnc.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "FieldList.toString() == Dictionary is not loaded.";

		EXPECT_EQ( flEnc.toString( emaDataDictionary ), fieldListString ) << "FieldList.toString() == fieldListString";

		flEmpty.addOpaque( -17, opaque );
		flEmpty.complete();
		flEmpty.clear();
		EXPECT_EQ( flEmpty.toString( emaDataDictionary ), "\nUnable to decode not completed FieldList data.\n" ) << "FieldList.toString() == Unable to decode not completed FieldList data.";

		flEmpty.complete();
		EXPECT_EQ( flEmpty.toString( emaDataDictionary ), "FieldList\nFieldListEnd\n" ) << "FieldList.toString() == FieldList\nFieldListEnd\n";

		//Decoding
		StaticDecoder::setData( &flEnc, &dictionary );
		EXPECT_EQ( flEnc.toString(), fieldListString ) << "FieldList.toString() == fieldListString";

		EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with all data types - hasInfo()" ;
		EXPECT_EQ( flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with all data types - getInfoDictionaryId()";
		EXPECT_EQ( flEnc.getInfoFieldListNum(), 65 ) << "FieldList with all data types - getInfoFieldListNum()" ;

		try
		{
			const FieldEntry& fe = flEnc.getEntry();
			EXPECT_FALSE( true ) << "FieldList with all data types - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "FieldList with all data types - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types- first forth()" ;
		const FieldEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;
		try
		{
			UInt64 intValue = fe1.getUInt();
			EXPECT_FALSE( true ) << "FieldList with all data types - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "FieldList with all data types - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - second forth()" ;
		const FieldEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - third forth()" ;
		const FieldEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		flEnc.reset();
		{
			EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with all data types - hasInfo()" ;
			EXPECT_EQ( flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with all data types- getInfoDictionaryId()";
			EXPECT_EQ( flEnc.getInfoFieldListNum(), 65 ) << "FieldList with all data types- getInfoFieldListNum()" ;
			try
			{
				const FieldEntry& fe = flEnc.getEntry();
				EXPECT_FALSE( true ) << "FieldList with all data types- exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "FieldList with all data types- exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types- first forth() again" ;
			const FieldEntry& fe1 = flEnc.getEntry();
			EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
			EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

			try
			{
				UInt64 intValue = fe1.getUInt();
				EXPECT_FALSE( true ) << "FieldList with all data types - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "FieldList with all data types - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - second forth() again" ;
			const FieldEntry& fe2 = flEnc.getEntry();
			EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( fe2.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

			EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - third forth() again" ;
			const FieldEntry& fe3 = flEnc.getEntry();
			EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( fe3.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
			EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fourth forth()" ;
		const FieldEntry& fe4 = flEnc.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fifth forth()" ;
		const FieldEntry& fe5 = flEnc.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - sixth forth()" ;
		const FieldEntry& fe6 = flEnc.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - seventh forth()" ;
		const FieldEntry& fe7 = flEnc.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types eightth forth()" ;
		const FieldEntry& fe8 = flEnc.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), FID_QOS ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_QOS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::QosEnum ) << "FieldEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::QosEnum ) << "FieldEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getQos().getTimeliness(), 5656 ) << "FieldEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( fe8.getQos().getRate(), 2345 ) << "FieldEntry::getQos().getRate()" ;
		EXPECT_STREQ( fe8.getQos().getTimelinessAsString(), "Timeliness: 5656" ) << "FieldEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( fe8.getQos().getRateAsString(), "Rate: 2345" ) << "FieldEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( fe8.getQos().toString(), "Timeliness: 5656/Rate: 2345" ) << "FieldEntry::getQos().toString()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - ninth forth()" ;
		const FieldEntry& fe9 = flEnc.getEntry();
		EXPECT_EQ(  fe9.getFieldId(), FID_STATE ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "MY_STATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::StateEnum ) << "FieldEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( fe9.getLoad().getDataType(), DataType::StateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( fe9.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe9.getState().getStreamState(), OmmState::OpenEnum ) << "FieldEntry::getState().getStreamState()" ;
		EXPECT_EQ( fe9.getState().getDataState(), OmmState::OkEnum ) << "FieldEntry::getState().getDataState()" ;
		EXPECT_EQ( fe9.getState().getStatusCode(), OmmState::NoneEnum ) << "FieldEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( fe9.getState().getStatusText(), "Succeeded" ) << "FieldEntry::getState().getStatusText()" ;
		EXPECT_STREQ( fe9.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "FieldEntry::getState().toString()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - tenth forth()" ;
		const FieldEntry& fe10 = flEnc.getEntry();
		EXPECT_EQ( fe10.getFieldId(), 715 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe10.getName(), "STORY_ID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe10.getLoadType(), DataType::AsciiEnum ) << "FieldEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( fe10.getLoad().getDataType(), DataType::AsciiEnum ) << "ElementEntry::getLoad().getDataType() == DataType::AsciiEnum" ;
		EXPECT_EQ( fe10.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe10.getAscii(), "ABCDEF" ) << "FieldEntry::getAscii()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - eleventh forth()" ;
		const FieldEntry& fe11 = flEnc.getEntry();
		EXPECT_EQ( fe11.getFieldId(), 28 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe11.getName(), "NEWS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe11.getLoadType(), DataType::RmtesEnum ) << "FieldEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( fe11.getLoad().getDataType(), DataType::RmtesEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( fe11.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe11.getRmtes().getAsUTF8(), EmaBuffer( "ABCDEF", 6 ) ) << "FieldEntry::getRmtes()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - twelfth forth()" ;
		const FieldEntry& fe12 = flEnc.getEntry();
		EXPECT_EQ( fe12.getFieldId(), 4 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe12.getName(), "RDN_EXCHID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe12.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( fe12.getLoad().getDataType(), DataType::EnumEnum ) << "ElementEntry::getLoad().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( fe12.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe12.getEnum(), 29 ) << "FieldEntry::getEnum()" ;
		EXPECT_TRUE( fe12.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";
		EXPECT_STREQ( fe12.getEnumDisplay(), "CSC" ) << "FieldEntry::getEnumDisplay()";

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - thirteenth forth()" ;
		const FieldEntry& fe13 = flEnc.getEntry();
		EXPECT_EQ(  fe13.getFieldId(), -9 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe13.getName(), "MY_FLOAT" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe13.getLoadType(), DataType::FloatEnum ) << "FieldEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( fe13.getLoad().getDataType(), DataType::FloatEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( fe13.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe13.getFloat(), 11.11f ) << "FieldEntry::getFloat()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fourteenth forth()" ;
		const FieldEntry& fe14 = flEnc.getEntry();
		EXPECT_EQ(  fe14.getFieldId(), -10 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe14.getName(), "MY_DOUBLE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe14.getLoadType(), DataType::DoubleEnum ) << "FieldEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( fe14.getLoad().getDataType(), DataType::DoubleEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( fe14.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe14.getDouble(), 22.22f ) << "FieldEntry::getFloat()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fifteenth forth()" ;
		const FieldEntry& fe15 = flEnc.getEntry();
		EXPECT_EQ( fe15.getFieldId(), 7 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe15.getName(), "TRDPRC_2" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe15.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe15.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe15.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - sixteenth forth()" ;
		const FieldEntry& fe16 = flEnc.getEntry();
		EXPECT_EQ(  fe16.getFieldId(), -11 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe16.getName(), "MY_BUFFER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe16.getLoadType(), DataType::BufferEnum ) << "FieldEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( fe16.getLoad().getDataType(), DataType::BufferEnum ) << "ElementEntry::getLoad().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( fe16.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe16.getBuffer(), EmaBuffer( "ABCDEFGH", 8 ) ) << "FieldEntry::getBuffer()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - seventeenth forth()" ;
		const FieldEntry& fe17 = flEnc.getEntry();
		EXPECT_EQ(  fe17.getFieldId(), -12 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe17.getName(), "MY_UTF8" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe17.getLoadType(), DataType::Utf8Enum ) << "FieldEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( fe17.getLoad().getDataType(), DataType::Utf8Enum ) << "ElementEntry::getLoad().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( fe17.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe17.getUtf8(), EmaBuffer( "KLMNOPQR", 8 ) ) << "FieldEntry::getUtf8()" ;

		EXPECT_TRUE( flEnc.forth() ) << "ElementList with all data types - eighteenth forth()" ;
		const FieldEntry& fe18 = flEnc.getEntry();
		EXPECT_EQ(  fe18.getFieldId(), -16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe18.getName(), "MY_ARRAY" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe18.getLoadType(), DataType::ArrayEnum ) << "FieldEntry::getLoadType() == DataType::ArrayEnum" ;
		EXPECT_EQ( fe18.getLoad().getDataType(), DataType::ArrayEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ArrayEnum" ;
		EXPECT_EQ( fe18.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		const OmmArray& ar2 = fe18.getArray();
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within fieldlist - first forth()" ;
		const OmmArrayEntry& ae1 = ar2.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae1.getInt(), 123 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within fieldlist - second forth()" ;
		const OmmArrayEntry& ae2 = ar2.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 234 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within fieldlist - third forth()" ;
		const OmmArrayEntry& ae3 = ar2.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), 345 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_FALSE( ar2.forth() ) << "OmmArray within fieldlist - final forth()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - nineteenth forth()" ;
		const FieldEntry& fe19 = flEnc.getEntry();
		EXPECT_EQ(  fe19.getFieldId(), -17 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe19.getName(), "MY_OPAQUE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe19.getLoadType(), DataType::OpaqueEnum ) << "FieldEntry::getLoadType() == DataType::OpaqueEnum" ;
		EXPECT_EQ( fe19.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		const OmmOpaque& opaque2 = fe19.getOpaque();
		char* sop = const_cast<char*>("OPQRST");
		EXPECT_STREQ( opaque2.getBuffer(), EmaBuffer( sop, 6 ) ) << "FieldEntry::getOpaque().getBuffer()::c_buf()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types twentyth forth()" ;
		const FieldEntry& fe20 = flEnc.getEntry();
		EXPECT_EQ(  fe20.getFieldId(), FID_QOS ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe20.getName(), "MY_QOS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe20.getLoadType(), DataType::QosEnum ) << "FieldEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( fe20.getLoad().getDataType(), DataType::QosEnum ) << "FieldEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( fe20.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe20.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "FieldEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( fe20.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "FieldEntry::getTime().getRate()" ;
		EXPECT_STREQ( fe20.getQos().getTimelinessAsString(), "InexactDelayed" ) << "FieldEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( fe20.getQos().getRateAsString(), "JustInTimeConflated" ) << "FieldEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( fe20.getQos().toString(), "InexactDelayed/JustInTimeConflated" ) << "FieldEntry::getQos().toString()" ;

		EXPECT_FALSE( flEnc.forth() ) << "FieldList with all data types - final forth()" ;

		EXPECT_TRUE( true ) << "FieldList with all data types - exception not expected" ;
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "ElementList with all data types - exception not expected" ;
		cout << excp << endl;
	}


	flEnc.clear();
	flEnc.info( dictionary.info_DictionaryId + 1, 66 );

	try
	{
		//EMA Encoding

		//first entry (fid not found case)
		flEnc.addUInt( -100, 64 );

		//second entry
		flEnc.addUInt( 1, 64 );
		//third  entry
		flEnc.addCodeUInt( 1 );

		//fourth entry
		flEnc.addReal( 6, 11, OmmReal::ExponentNeg2Enum );
		//fifth  entry
		flEnc.addCodeReal( 6 );

		//sixth entry
		flEnc.addInt( -2, 32 );
		//seventh  entry
		flEnc.addCodeInt( -2 );

		//eighth entry
		flEnc.addDate( 16, 1999, 11, 7 );
		//ninth  entry
		flEnc.addCodeDate( 16 );

		//tenth entry
		flEnc.addTime( 18, 02, 03, 04, 005 );
		//eleventh  entry
		flEnc.addCodeTime( 18 );

		//twelfth entry
		flEnc.addDateTime( -3, 1999, 11, 7, 01, 02, 03, 000 );
		//thirteenth entry
		flEnc.addCodeDateTime( -3 );

		//fourteenth entry
		flEnc.addQos( FID_QOS, OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );
		//fifteenth  entry
		flEnc.addCodeQos( FID_QOS );

		//sixteenth entry
		flEnc.addState( FID_STATE, OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );
		//seventeenth  entry
		flEnc.addCodeState( FID_STATE );

		//eighteenth entry
		flEnc.addAscii( 715, EmaString( "ABCDEF" ) );
		//nineteenth  entry
		flEnc.addCodeAscii( 715 );

		//twentieth entry
		char* s1 = const_cast<char*>("ABCDEF");
		flEnc.addRmtes( 28, EmaBuffer( s1, 6 ) );
		//21st  entry
		flEnc.addCodeRmtes( 28 );

		//22nd entry
		flEnc.addEnum( 4, 29 );
		//23rd  entry
		flEnc.addCodeEnum( 4 );

		//24th entry
		flEnc.addFloat( -9, 11.11f );
		//25th  entry
		flEnc.addCodeFloat( -9 );

		//26th entry
		flEnc.addDouble( -10, 22.22f );
		//27th entry
		flEnc.addCodeDouble( -10 );

		//28th entry (blank real)
		flEnc.addCodeReal( 7 );

		//29th entry
		char* s2 = const_cast<char*>("ABCDEFGH");
		flEnc.addBuffer( -11, EmaBuffer( s2, 8 ) );
		//30th  entry
		flEnc.addCodeBuffer( -11 );

		//31st entry
		char* s3 = const_cast<char*>("KLMNOPQR");
		flEnc.addUtf8( -12, EmaBuffer( s3, 8 ) );
		//32nd  entry
		flEnc.addCodeUtf8( -12 );

		//33rd entry
		OmmArray ar1;
		ar1.addInt( 123 ).addInt( 234 ).addInt( 345 ).complete();
		flEnc.addArray( -16, ar1 );

		//34th entry
		char* s4 = const_cast<char*>("OPQRST");
		EmaBuffer buf4( s4, 6 );
		OmmOpaque opaque;
		opaque.set( buf4 );
		flEnc.addOpaque( -17, opaque );

		flEnc.complete();


		//Decoding
		StaticDecoder::setData( &flEnc, &dictionary );

		EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with all data types - hasInfo()" ;
		EXPECT_EQ( flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId + 1) << "FieldList with all data types - getInfoDictionaryId()";
		EXPECT_EQ( flEnc.getInfoFieldListNum(), 66 ) << "FieldList with all data types - getInfoFieldListNum()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types- first forth()" ;
		const FieldEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - second forth()" ;
		const FieldEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - third forth()" ;
		const FieldEntry& fe2b = flEnc.getEntry();
		EXPECT_EQ( fe2b.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2b.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2b.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fourth forth()" ;
		const FieldEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fifth forth()" ;
		const FieldEntry& fe3b = flEnc.getEntry();
		EXPECT_EQ( fe3b.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3b.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3b.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - sixth forth()" ;
		const FieldEntry& fe4 = flEnc.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - seventh forth()" ;
		const FieldEntry& fe4b = flEnc.getEntry();
		EXPECT_EQ(  fe4b.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4b.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4b.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - eightth forth()" ;
		const FieldEntry& fe5 = flEnc.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - ninth forth()" ;
		const FieldEntry& fe5b = flEnc.getEntry();
		EXPECT_EQ( fe5b.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5b.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5b.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - tenth forth()" ;
		const FieldEntry& fe6 = flEnc.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - eleventh forth()" ;
		const FieldEntry& fe6b = flEnc.getEntry();
		EXPECT_EQ( fe6b.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6b.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6b.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - twelfth forth()" ;
		const FieldEntry& fe7 = flEnc.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - thirteenth forth()" ;
		const FieldEntry& fe7b = flEnc.getEntry();
		EXPECT_EQ(  fe7b.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7b.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7b.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types fourteenth forth()" ;
		const FieldEntry& fe8 = flEnc.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), FID_QOS ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_QOS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::QosEnum ) << "FieldEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "FieldEntry::getTime().getTimeliness()" ;
		EXPECT_EQ( fe8.getQos().getRate(), OmmQos::TickByTickEnum ) << "FieldEntry::getTime().getRate()" ;
		EXPECT_STREQ( fe8.getQos().getTimelinessAsString(), "RealTime" ) << "FieldEntry::getTime().getTimelinessAsString()" ;
		EXPECT_STREQ( fe8.getQos().getRateAsString(), "TickByTick" ) << "FieldEntry::getTime().getRateAsString()" ;
		EXPECT_STREQ( fe8.getQos().toString(), "RealTime/TickByTick" ) << "FieldEntry::getTime().toString()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - fifteenth forth()" ;
		const FieldEntry& fe8ba = flEnc.getEntry();
		EXPECT_EQ(  fe8ba.getFieldId(), FID_QOS ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8ba.getName(), "MY_QOS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8ba.getLoadType(), DataType::QosEnum ) << "FieldEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( fe8ba.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - sixteenth forth()" ;
		const FieldEntry& fe9 = flEnc.getEntry();
		EXPECT_EQ(  fe9.getFieldId(), FID_STATE ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "MY_STATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::StateEnum ) << "FieldEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( fe9.getLoad().getDataType(), DataType::StateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::StateEnum" ;
		EXPECT_EQ( fe9.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe9.getState().getStreamState(), OmmState::OpenEnum ) << "FieldEntry::getState().getStreamState()" ;
		EXPECT_EQ( fe9.getState().getDataState(), OmmState::OkEnum ) << "FieldEntry::getState().getDataState()" ;
		EXPECT_EQ( fe9.getState().getStatusCode(), OmmState::NoneEnum ) << "FieldEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( fe9.getState().getStatusText(), "Succeeded" ) << "FieldEntry::getState().getStatusText()" ;
		EXPECT_STREQ( fe9.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "FieldEntry::getState().toString()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - seventeenth forth()" ;
		const FieldEntry& fe9b = flEnc.getEntry();
		EXPECT_EQ(  fe9b.getFieldId(), FID_STATE ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9b.getName(), "MY_STATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9b.getLoadType(), DataType::StateEnum ) << "FieldEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( fe9b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - eighteenth forth()" ;
		const FieldEntry& fe10 = flEnc.getEntry();
		EXPECT_EQ( fe10.getFieldId(), 715 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe10.getName(), "STORY_ID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe10.getLoadType(), DataType::AsciiEnum ) << "FieldEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( fe10.getLoad().getDataType(), DataType::AsciiEnum ) << "ElementEntry::getLoad().getDataType() == DataType::AsciiEnum" ;
		EXPECT_EQ( fe10.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe10.getAscii(), "ABCDEF" ) << "FieldEntry::getAscii()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - ninteenth forth()" ;
		const FieldEntry& fe10b = flEnc.getEntry();
		EXPECT_EQ( fe10b.getFieldId(), 715 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe10b.getName(), "STORY_ID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe10b.getLoadType(), DataType::AsciiEnum ) << "FieldEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_EQ( fe10b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - twentieth forth()" ;
		const FieldEntry& fe11 = flEnc.getEntry();
		EXPECT_EQ( fe11.getFieldId(), 28 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe11.getName(), "NEWS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe11.getLoadType(), DataType::RmtesEnum ) << "FieldEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( fe11.getLoad().getDataType(), DataType::RmtesEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RmtesEnum" ;
		EXPECT_EQ( fe11.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe11.getRmtes().getAsUTF8(), EmaBuffer( "ABCDEF", 6 ) ) << "FieldEntry::getRmtes()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 21st forth()" ;
		const FieldEntry& fe11b = flEnc.getEntry();
		EXPECT_EQ( fe11b.getFieldId(), 28 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe11b.getName(), "NEWS" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe11b.getLoadType(), DataType::RmtesEnum ) << "FieldEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_EQ( fe11b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 22nd forth()" ;
		const FieldEntry& fe12 = flEnc.getEntry();
		EXPECT_EQ( fe12.getFieldId(), 4 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe12.getName(), "RDN_EXCHID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe12.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( fe12.getLoad().getDataType(), DataType::EnumEnum ) << "ElementEntry::getLoad().getDataType() == DataType::EnumEnum" ;
		EXPECT_EQ( fe12.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe12.getEnum(), 29 ) << "FieldEntry::getEnum()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 23rd forth()" ;
		const FieldEntry& fe12b = flEnc.getEntry();
		EXPECT_EQ( fe12b.getFieldId(), 4 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe12b.getName(), "RDN_EXCHID" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe12b.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( fe12b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 24th forth()" ;
		const FieldEntry& fe13 = flEnc.getEntry();
		EXPECT_EQ(  fe13.getFieldId(), -9 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe13.getName(), "MY_FLOAT" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe13.getLoadType(), DataType::FloatEnum ) << "FieldEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( fe13.getLoad().getDataType(), DataType::FloatEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FloatEnum" ;
		EXPECT_EQ( fe13.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe13.getFloat(), 11.11f ) << "FieldEntry::getFloat()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 25th forth()" ;
		const FieldEntry& fe13b = flEnc.getEntry();
		EXPECT_EQ(  fe13b.getFieldId(), -9 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe13b.getName(), "MY_FLOAT" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe13b.getLoadType(), DataType::FloatEnum ) << "FieldEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( fe13b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 26th forth()" ;
		const FieldEntry& fe14 = flEnc.getEntry();
		EXPECT_EQ(  fe14.getFieldId(), -10 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe14.getName(), "MY_DOUBLE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe14.getLoadType(), DataType::DoubleEnum ) << "FieldEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( fe14.getLoad().getDataType(), DataType::DoubleEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DoubleEnum" ;
		EXPECT_EQ( fe14.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe14.getDouble(), 22.22f ) << "FieldEntry::getFloat()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 27th forth()" ;
		const FieldEntry& fe14b = flEnc.getEntry();
		EXPECT_EQ(  fe14b.getFieldId(), -10 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe14b.getName(), "MY_DOUBLE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe14b.getLoadType(), DataType::DoubleEnum ) << "FieldEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( fe14b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 28th forth()" ;
		const FieldEntry& fe15 = flEnc.getEntry();
		EXPECT_EQ( fe15.getFieldId(), 7 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe15.getName(), "TRDPRC_2" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe15.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe15.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe15.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 29th forth()" ;
		const FieldEntry& fe16 = flEnc.getEntry();
		EXPECT_EQ(  fe16.getFieldId(), -11 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe16.getName(), "MY_BUFFER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe16.getLoadType(), DataType::BufferEnum ) << "FieldEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( fe16.getLoad().getDataType(), DataType::BufferEnum ) << "ElementEntry::getLoad().getDataType() == DataType::BufferEnum" ;
		EXPECT_EQ( fe16.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe16.getBuffer(), EmaBuffer( "ABCDEFGH", 8 ) ) << "FieldEntry::getBuffer()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 3oth forth()" ;
		const FieldEntry& fe16b = flEnc.getEntry();
		EXPECT_EQ(  fe16b.getFieldId(), -11 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe16b.getName(), "MY_BUFFER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe16b.getLoadType(), DataType::BufferEnum ) << "FieldEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_EQ( fe16b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 31st forth()" ;
		const FieldEntry& fe17 = flEnc.getEntry();
		EXPECT_EQ(  fe17.getFieldId(), -12 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe17.getName(), "MY_UTF8" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe17.getLoadType(), DataType::Utf8Enum ) << "FieldEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( fe17.getLoad().getDataType(), DataType::Utf8Enum ) << "ElementEntry::getLoad().getDataType() == DataType::Utf8Enum" ;
		EXPECT_EQ( fe17.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_STREQ( fe17.getUtf8(), EmaBuffer( "KLMNOPQR", 8 ) ) << "FieldEntry::getUtf8()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 32nd forth()" ;
		const FieldEntry& fe17b = flEnc.getEntry();
		EXPECT_EQ(  fe17b.getFieldId(), -12 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe17b.getName(), "MY_UTF8" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe17b.getLoadType(), DataType::Utf8Enum ) << "FieldEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_EQ( fe17b.getCode(), Data::BlankEnum ) << "FieldEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "ElementList with all data types - 33rd forth()" ;
		const FieldEntry& fe18 = flEnc.getEntry();
		EXPECT_EQ(  fe18.getFieldId(), -16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe18.getName(), "MY_ARRAY" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe18.getLoadType(), DataType::ArrayEnum ) << "FieldEntry::getLoadType() == DataType::ArrayEnum" ;
		EXPECT_EQ( fe18.getLoad().getDataType(), DataType::ArrayEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ArrayEnum" ;
		EXPECT_EQ( fe18.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		const OmmArray& ar2 = fe18.getArray();
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within fieldlist - first forth()" ;
		const OmmArrayEntry& ae1 = ar2.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae1.getInt(), 123 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within fieldlist - second forth()" ;
		const OmmArrayEntry& ae2 = ar2.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 234 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_TRUE( ar2.forth() ) << "OmmArray within fieldlist - third forth()" ;
		const OmmArrayEntry& ae3 = ar2.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), 345 ) << "OmmArrayEntry::getInt()" ;
		EXPECT_FALSE( ar2.forth() ) << "OmmArray within fieldlist - final forth()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with all data types - 34th forth()" ;
		const FieldEntry& fe19 = flEnc.getEntry();
		EXPECT_EQ(  fe19.getFieldId(), -17 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe19.getName(), "MY_OPAQUE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe19.getLoadType(), DataType::OpaqueEnum ) << "FieldEntry::getLoadType() == DataType::OpaqueEnum" ;
		EXPECT_EQ( fe19.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		const OmmOpaque& opaque2 = fe19.getOpaque();
		char* sop = const_cast<char*>("OPQRST");
		EXPECT_STREQ( opaque2.getBuffer(), EmaBuffer( sop, 6 ) ) << "FieldEntry::getOpaque().getBuffer()::c_buf()" ;

		EXPECT_FALSE( flEnc.forth() ) << "FieldList after clear() - final forth()" ;

		EXPECT_TRUE( true ) << "FieldList with all data types - exception not expected" ;
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "Encode FieldList after clear() - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(FieldListTests, testFieldListContainsFieldListEncodeDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FieldList flEnc;
	flEnc.info( dictionary.info_DictionaryId, 65 );

	try
	{
		//EMA Encoding
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, FieldList, UINT

		//first entry (fid not found case)
		flEnc.addUInt( -100, 64 );

		//second entry
		flEnc.addUInt( 1, 64 );

		//third entry
		flEnc.addReal( 6, 11, OmmReal::ExponentNeg2Enum );

		//fourth entry
		flEnc.addInt( -2, 32 );

		//fifth entry
		flEnc.addDate( 16, 1999, 11, 7 );

		//sixth entry
		flEnc.addTime( 18, 02, 03, 04, 005 );

		//seventh entry
		flEnc.addDateTime( -3, 1999, 11, 7, 01, 02, 03, 000 );

		//eightth entry (nested FieldList)
		FieldList flEnc1;
		flEnc1.addUInt( 1, 641 );
		flEnc1.complete();
		flEnc.addFieldList( -13, flEnc1 );

		//ninth entry
		flEnc.addUInt( 1, 642 );

		flEnc.complete();


		//Now do EMA decoding of FieldList
		StaticDecoder::setData( &flEnc, &dictionary );

		EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with primitives and FieldList - hasInfo()" ;
		EXPECT_EQ( flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives and FieldList - getInfoDictionaryId()";
		EXPECT_EQ( flEnc.getInfoFieldListNum(), 65 ) << "FieldList with primitives and FieldList - getInfoFieldListNum()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - first forth()" ;
		const FieldEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - second forth()" ;
		const FieldEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - third forth()" ;
		const FieldEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - fourth forth()" ;
		const FieldEntry& fe4 = flEnc.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - fifth forth()" ;
		const FieldEntry& fe5 = flEnc.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - sixth forth()" ;
		const FieldEntry& fe6 = flEnc.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - seventh forth()" ;
		const FieldEntry& fe7 = flEnc.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - eightth forth()" ;
		const FieldEntry& fe8 = flEnc.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), -13 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_FIELDLIST" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::FieldListEnum ) << "FieldEntry::getLoadType() == DataType::FieldListEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::FieldListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::FieldListEnum ) << "FieldEntry::getCode() == DataType::FieldListEnum" ;
		{
			const FieldList& nestedFl = fe8.getFieldList();

			EXPECT_FALSE( nestedFl.hasInfo() ) << "FieldEntry FieldList within fieldlist - hasInfo()" ;

			EXPECT_TRUE( nestedFl.forth() ) << "FieldEntry FieldList within fieldlist - first fieldlist forth()" ;
			const FieldEntry& fe1 = nestedFl.getEntry();
			EXPECT_EQ( fe1.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
			EXPECT_STREQ( fe1.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
			EXPECT_EQ( fe1.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( fe1.getUInt(), 641 ) << "FieldEntry::getUInt()" ;

			EXPECT_FALSE( nestedFl.forth() ) << "FieldEntry FieldList within fieldlist - second fieldlist forth()" ;
		}


		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and FieldList - ninth forth()" ;
		const FieldEntry& fe9 = flEnc.getEntry();
		EXPECT_EQ( fe9.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getUInt(), 642 ) << "FieldEntry::getUInt()" ;


		EXPECT_FALSE( flEnc.forth() ) << "FieldList with primitives and FieldList - tenth forth()" ;

		EXPECT_TRUE( true ) << "FieldList with primitives and FieldList - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with primitives and FieldList - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListContainsElementListEncodeDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FieldList flEnc;
	flEnc.info( dictionary.info_DictionaryId, 65 );

	try
	{
		//EMA Encoding
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

		//first entry (fid not found case)
		flEnc.addUInt( -100, 64 );

		//second entry
		flEnc.addUInt( 1, 64 );

		//third entry
		flEnc.addReal( 6, 11, OmmReal::ExponentNeg2Enum );

		//fourth entry
		flEnc.addInt( -2, 32 );

		//fifth entry
		flEnc.addDate( 16, 1999, 11, 7 );

		//sixth entry
		flEnc.addTime( 18, 02, 03, 04, 005 );

		//seventh entry
		flEnc.addDateTime( -3, 1999, 11, 7, 01, 02, 03, 000 );

		//eightth entry (nested ElementList)
		ElementList elEnc;
		elEnc.info( 5 );
		elEnc.addUInt( EmaString( "Element - UInt" ), 641 );
		elEnc.complete();
		flEnc.addElementList( -15, elEnc );

		//ninth entry
		flEnc.addUInt( 1, 642 );

		flEnc.complete();


		//Now do EMA decoding of FieldList
		StaticDecoder::setData( &flEnc, &dictionary );

		EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with primitives and FieldList - hasInfo()" ;
		EXPECT_EQ( flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives and FieldList - getInfoDictionaryId()";
		EXPECT_EQ( flEnc.getInfoFieldListNum(), 65 ) << "FieldList with primitives and FieldList - getInfoFieldListNum()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - first forth()" ;
		const FieldEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - second forth()" ;
		const FieldEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - third forth()" ;
		const FieldEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - fourth forth()" ;
		const FieldEntry& fe4 = flEnc.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - fifth forth()" ;
		const FieldEntry& fe5 = flEnc.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - sixth forth()" ;
		const FieldEntry& fe6 = flEnc.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - seventh forth()" ;
		const FieldEntry& fe7 = flEnc.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - eightth forth()" ;
		const FieldEntry& fe8 = flEnc.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), -15 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_ELEMENTLIST" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::ElementListEnum ) << "FieldEntry::getLoadType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::ElementListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::ElementListEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::ElementListEnum ) << "FieldEntry::getCode() == DataType::ElementListEnum" ;
		{
			const ElementList& nestedEl = fe8.getElementList();

			EXPECT_TRUE( nestedEl.hasInfo() ) << "FieldEntry ElementList within fieldlist - hasInfo()" ;

			EXPECT_TRUE( nestedEl.forth() ) << "ElementEntry ElementList within fieldlist - first elementlist forth()" ;
			const ElementEntry& ee1 = nestedEl.getEntry();
			EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()" ;
			EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ee1.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum" ;
			EXPECT_EQ( ee1.getUInt(), 641 ) << "ElementEntry::getUInt()" ;

			EXPECT_FALSE( nestedEl.forth() ) << "ElementEntry ElementList within fieldlist - second elementlist forth()" ;
		}


		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and ElementList - ninth forth()" ;
		const FieldEntry& fe9 = flEnc.getEntry();
		EXPECT_EQ( fe9.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getUInt(), 642 ) << "FieldEntry::getUInt()" ;


		EXPECT_FALSE( flEnc.forth() ) << "FieldList with primitives and ElementList - tenth forth()" ;

		EXPECT_TRUE( true ) << "FieldList with primitives and ElementList - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with primitives and ElementList - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListContainsMapEncodeDecodeAll)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	FieldList flEnc;
	flEnc.info( dictionary.info_DictionaryId, 65 );

	try
	{
		//EMA Encoding
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, Map, UINT

		//first entry (fid not found case)
		flEnc.addUInt( -100, 64 );

		//second entry
		flEnc.addUInt( 1, 64 );

		//third entry
		flEnc.addReal( 6, 11, OmmReal::ExponentNeg2Enum );

		//fourth entry
		flEnc.addInt( -2, 32 );

		//fifth entry
		flEnc.addDate( 16, 1999, 11, 7 );

		//sixth entry
		flEnc.addTime( 18, 02, 03, 04, 005 );

		//seventh entry
		flEnc.addDateTime( -3, 1999, 11, 7, 01, 02, 03, 000 );

		//eightth entry (nested Map)
		Map mapEnc1;
		EmaEncodeMapAll( mapEnc1 );
		flEnc.addMap( -14, mapEnc1 );

		//ninth entry
		flEnc.addUInt( 1, 642 );

		flEnc.complete();


		//Now do EMA decoding of FieldList
		StaticDecoder::setData( &flEnc, &dictionary );

		EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with primitives and Map - hasInfo()" ;
		EXPECT_EQ( flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives and Map - getInfoDictionaryId()";
		EXPECT_EQ( flEnc.getInfoFieldListNum(), 65 ) << "FieldList with primitives and Map - getInfoFieldListNum()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - first forth()" ;
		const FieldEntry& fe1 = flEnc.getEntry();
		EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;
		EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - second forth()" ;
		const FieldEntry& fe2 = flEnc.getEntry();
		EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - third forth()" ;
		const FieldEntry& fe3 = flEnc.getEntry();
		EXPECT_EQ( fe3.getFieldId(), 6 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe3.getName(), "TRDPRC_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe3.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum" ;
		EXPECT_EQ( fe3.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()" ;
		EXPECT_EQ( fe3.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - fourth forth()" ;
		const FieldEntry& fe4 = flEnc.getEntry();
		EXPECT_EQ(  fe4.getFieldId(), -2 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe4.getName(), "INTEGER" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe4.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum" ;
		EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe4.getInt(), 32 ) << "FieldEntry::getInt()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - fifth forth()" ;
		const FieldEntry& fe5 = flEnc.getEntry();
		EXPECT_EQ( fe5.getFieldId(), 16 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe5.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe5.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum" ;
		EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe5.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()" ;
		EXPECT_EQ( fe5.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()" ;
		EXPECT_EQ( fe5.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - sixth forth()" ;
		const FieldEntry& fe6 = flEnc.getEntry();
		EXPECT_EQ( fe6.getFieldId(), 18 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe6.getName(), "TRDTIM_1" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe6.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum" ;
		EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe6.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()" ;
		EXPECT_EQ( fe6.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()" ;
		EXPECT_EQ( fe6.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()" ;
		EXPECT_EQ( fe6.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - seventh forth()" ;
		const FieldEntry& fe7 = flEnc.getEntry();
		EXPECT_EQ(  fe7.getFieldId(), -3 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe7.getName(), "TRADE_DATE" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe7.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe7.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()" ;
		EXPECT_EQ( fe7.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( fe7.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()" ;
		EXPECT_EQ( fe7.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()" ;
		EXPECT_EQ( fe7.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( fe7.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( fe7.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - eightth forth()" ;
		const FieldEntry& fe8 = flEnc.getEntry();
		EXPECT_EQ(  fe8.getFieldId(), -14 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe8.getName(), "MY_MAP" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe8.getLoadType(), DataType::MapEnum ) << "FieldEntry::getLoadType() == DataType::MapEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::MapEnum ) << "ElementEntry::getLoad().getDataType() == DataType::MapEnum" ;
		EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum" ;
		EXPECT_EQ( fe8.getLoad().getDataType(), DataType::MapEnum ) << "FieldEntry::getCode() == DataType::MapEnum" ;
		{
			const Map& map = fe2.getMap();
			EXPECT_TRUE( map.hasKeyFieldId() ) << "FieldEntry Map within fieldList - hasKeyFieldId()" ;
			EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "FieldEntry Map within map - getKeyFieldId()" ;

			EXPECT_TRUE( map.forth() ) << "FieldEntry Map within fieldList - first map forth()" ;
			const MapEntry& me1a = map.getEntry();
			EXPECT_EQ( me1a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me1a.getKey().getBuffer(), EmaBuffer( "ABCD", 4 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me1a.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum" ;
			EXPECT_EQ( me1a.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum" ;

			EXPECT_TRUE( map.forth() ) << "FieldEntry Map within fieldList - second map forth()" ;
			const MapEntry& me2a = map.getEntry();
			EXPECT_EQ( me2a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;

			//me2a.getKey().getBuffer() is empty
			//..

			EXPECT_EQ( me2a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me2a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me2a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FieldEntry Map within fieldList - third map forth()" ;
			const MapEntry& me3a = map.getEntry();
			EXPECT_EQ( me3a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me3a.getKey().getBuffer(), EmaBuffer( "EFGHI", 5 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me3a.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum" ;
			EXPECT_EQ( me3a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me3a.getFieldList() );
			}

			EXPECT_TRUE( map.forth() ) << "FieldEntry Map within fieldList - fourth map forth()" ;
			const MapEntry& me4a = map.getEntry();
			EXPECT_EQ( me4a.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum" ;
			EXPECT_STREQ( me4a.getKey().getBuffer(), EmaBuffer( "JKLMNOP", 7 ) ) << "MapEntry::getKey().getBuffer()" ;
			EXPECT_EQ( me4a.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum" ;
			EXPECT_EQ( me4a.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
			{
			  SCOPED_TRACE("calling EmaDecodeFieldListAll");
			  EmaDecodeFieldListAll( me4a.getFieldList() );
			}

			EXPECT_FALSE( map.forth() ) << "FieldEntry Map within fieldList - fifth map forth()" ;
		}

		EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - ninth forth()" ;
		const FieldEntry& fe9 = flEnc.getEntry();
		EXPECT_EQ( fe9.getFieldId(), 1 ) << "FieldEntry::getFieldId()" ;
		EXPECT_STREQ( fe9.getName(), "PROD_PERM" ) << "FieldEntry::getName()" ;
		EXPECT_EQ( fe9.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum" ;
		EXPECT_EQ( fe9.getUInt(), 642 ) << "FieldEntry::getUInt()" ;

		EXPECT_FALSE( flEnc.forth() ) << "FieldList with primitives and Map - tenth forth()" ;

		EXPECT_TRUE( true ) << "FieldList with primitives and Map - exception not expected" ;
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList with primitives and Map - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}


void encodeErrorFieldList( RsslBuffer& rsslBuf )

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

	// fid not found case (first)
	rsslFEntry.dataType = RSSL_DT_UINT;
	rsslFEntry.fieldId = -100;
	RsslUInt64 uint64 = 64;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

	// corect fid found case (second)
	rsslFEntry.dataType = RSSL_DT_UINT;
	rsslFEntry.fieldId = 1;
	uint64 = 64;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

	// incorrect data type fid longer than expected (third)
	rsslFEntry.dataType = RSSL_DT_REAL;
	rsslFEntry.fieldId = 1;
	RsslReal real;
	real.isBlank = RSSL_FALSE;
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 11;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

	// correct data type fid (fourth)
	rsslFEntry.dataType = RSSL_DT_REAL;
	rsslFEntry.fieldId = 6;
	real.isBlank = RSSL_FALSE;
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 11;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

	// incorrect data type fid shorter than expected (fifth)
	rsslFEntry.dataType = RSSL_DT_UINT;
	rsslFEntry.fieldId = 6;
	uint64 = 67;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

	// correct data type fid
	rsslFEntry.dataType = RSSL_DT_REAL;
	rsslFEntry.fieldId = 6;
	real.isBlank = RSSL_FALSE;
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 11;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

	// Enum value is not found in the enumerated type dictionary
	rsslFEntry.dataType = RSSL_DT_ENUM;
	rsslFEntry.fieldId = 4;		// RDN_EXCHID
	RsslEnum enumm = 2999;
	rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)&enumm);

	// Enum value is not blank
	rsslClearFieldEntry(&rsslFEntry);
	rsslFEntry.dataType = RSSL_DT_ENUM;
	rsslFEntry.fieldId = 4;		// RDN_EXCHID
	rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)0);

	// Array value is blank
	rsslClearFieldEntry(&rsslFEntry);
	rsslFEntry.dataType = RSSL_DT_ARRAY;
	rsslFEntry.fieldId = 30013;		// HDLN_PE
	rsslEncodeFieldEntry(&iter, &rsslFEntry, (void*)0);

	rsslBuf.length = rsslGetEncodedBufferLength( &iter );

	rsslEncodeFieldListComplete( &iter, RSSL_TRUE );
}

TEST(FieldListTests, testErrorFieldListDecode)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		encodeErrorFieldList( rsslBuf );

		FieldList fl;

		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		// first entry "fid not found"
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() first" ;

			const FieldEntry& fe1 = fl.getEntry();

			EXPECT_EQ(  fe1.getFieldId(), -100 ) << "FieldEntry::getFieldId() == -100" ;

			EXPECT_EQ( fe1.getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType() == DataType::ErrorEnum" ;

			EXPECT_EQ( fe1.getError().getErrorCode(), OmmError::FieldIdNotFoundEnum ) << "FieldEntry::getErrorCode() == FieldIdNotFoundEnum" ;

			EXPECT_FALSE( fe1.getLoad().toString().empty() ) << "FieldEntry::getLoad()::toString() = empty" ;

			EXPECT_FALSE( fe1.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";

			try
			{
				fe1.getEnumDisplay();
				EXPECT_FALSE( true ) << "Call FieldEntry::getEnumDisplay() from the Error type - exception expected";
			}
			catch (const OmmException& excp)
			{
				EXPECT_TRUE( true ) << "Call FieldEntry::getEnumDisplay() from the Error type - exception expected";
				EXPECT_STREQ( excp.getText(), "Attempt to getEnumDisplay() while actual entry data type is Error" ) << "FieldEntry::getEnumDisplay()";
			}
		}

		// second entry fid found and correct
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() second" ;

			const FieldEntry& fe2 = fl.getEntry();

			EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId() == 1" ;

			EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;

			EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt() == 64" ;
		}

		// third entry fid found but not corrct data type (longer)
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() third" ;

			const FieldEntry& fe3 = fl.getEntry();

			EXPECT_EQ( fe3.getFieldId(), 1 ) << "FieldEntry::getFieldId() == 1" ;

			EXPECT_EQ( fe3.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum" ;

			EXPECT_FALSE( fe3.getLoad().toString().empty() ) << "Fid Not Found FieldEntry::getLoad()::toString() output: " << fe3.getLoad().toString() << "\n";
		}

		// fourth entry fid found and correct data type
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() fourth" ;

			const FieldEntry& fe4 = fl.getEntry();

			EXPECT_EQ( fe4.getFieldId(), 6 ) << "FieldEntry::getFieldId() == 6" ;

			EXPECT_EQ( fe4.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;

			EXPECT_EQ( fe4.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal()";
			EXPECT_EQ( fe4.getReal().getMantissa(), 11 ) << "FieldEntry::getReal()";

		}

		// fifth entry fid found and not correct (shorter)
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() fifth" ;

			const FieldEntry& fe5 = fl.getEntry();

			EXPECT_EQ( fe5.getFieldId(), 6 ) << "FieldEntry::getFieldId() == 6" ;

			EXPECT_EQ( fe5.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_FALSE( fe5.getLoad().toString().empty() ) << "Fid Not Found FieldEntry::getLoad()::toString() output: " << fe5.getLoad().toString() << "\n";
		}

		// sixth entry fid found and correct data type
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() sixth" ;

			const FieldEntry& fe6 = fl.getEntry();

			EXPECT_EQ( fe6.getFieldId(), 6 ) << "FieldEntry::getFieldId() == 6" ;

			EXPECT_EQ( fe6.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum" ;

			EXPECT_EQ( fe6.getReal().getMagnitudeType(), 12) << "FieldEntry::getReal()";
			EXPECT_EQ( fe6.getReal().getMantissa(), 11 ) << "FieldEntry::getReal()";

			try
			{
				fe6.getEnumDisplay();
				EXPECT_FALSE( true ) << "Call FieldEntry::getEnumDisplay() from the Real type - exception expected";
			}
			catch (const OmmException& excp)
			{
				EXPECT_TRUE( true ) << "Call FieldEntry::getEnumDisplay() from the Real type - exception expected";
				EXPECT_STREQ(excp.getText(), "Attempt to getEnumDisplay() while actual entry data type is Real" ) << "FieldEntry::getEnumDisplay()";
			}
		}

		// seventh entry Enum value is not found in the enumerated type dictionary
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() seventh";

			const FieldEntry& fe7 = fl.getEntry();

			EXPECT_EQ( fe7.getFieldId(), 4 ) << "FieldEntry::getFieldId() == 4";

			EXPECT_EQ( fe7.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";

			EXPECT_FALSE( fe7.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";

			try
			{
				fe7.getEnumDisplay();
				EXPECT_FALSE( true ) << "Enum dispaly does not found - exception expected";
			}
			catch (const OmmException& excp)
			{
				EXPECT_TRUE( true ) << "Enum dispaly does not found - exception expected";
				EXPECT_STREQ(excp.getText(), "The enum value 2999 for the field Id 4 does not exist in the enumerated type dictionary" ) << "FieldEntry::getEnumDisplay()";
			}
		}

		// eighth entry Enum value is blank
		{
			EXPECT_TRUE( fl.forth() ) << "FieldList::forth() eighth";

			const FieldEntry& fe8 = fl.getEntry();

			EXPECT_EQ( fe8.getFieldId(), 4 ) << "FieldEntry::getFieldId() == 4";

			EXPECT_EQ( fe8.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";

			EXPECT_FALSE( fe8.hasEnumDisplay() ) << "FieldEntry::hasEnumDisplay()";

			try
			{
				fe8.getEnumDisplay();
				EXPECT_FALSE( true ) << "Enum value is blank - exception expected";
			}
			catch (const OmmException& excp)
			{
				EXPECT_TRUE( true ) << "Enum value is blank - exception expected";
				EXPECT_STREQ(excp.getText(), "Attempt to getEnumDisplay() while entry data is blank.") << "FieldEntry::getEnumDisplay()";
			}
		}
		// nine entry Array value is blank
		{
			EXPECT_TRUE(fl.forth()) << "FieldList::forth() nine";

			const FieldEntry& fe9 = fl.getEntry();

			EXPECT_EQ(fe9.getFieldId(), 30013) << "FieldEntry::getFieldId() == 30013";

			EXPECT_EQ(fe9.getLoadType(), DataType::ArrayEnum) << "FieldEntry::getLoadType() == DataType::ArrayEnum";

			try
			{
				fe9.getArray();
				EXPECT_FALSE(true) << "Enum value is blank - exception expected";
			}
			catch (const OmmException& excp)
			{
				EXPECT_TRUE(true) << "Enum value is blank - exception expected";
				EXPECT_STREQ(excp.getText(), "Attempt to getArray() while entry data is blank.") << "FieldEntry::getArray()";
			}
		}

		EXPECT_FALSE( fl.forth() ) << "FieldList::forth() ninth";

		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "Error FieldList decoding - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "Error FieldList decoding - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListDecodetoString)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING, RMTES_STRING
		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;
		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		double d;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );


		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );


		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );


		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
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
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );


		rsslFEntry.fieldId = FID_QOS;
		rsslFEntry.dataType = RSSL_DT_QOS;
		RsslQos rsslQos;
		rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslQos.dynamic = 1;
		rsslQos.rateInfo = 0;
		rsslQos.timeInfo = 0;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslQos );


		rsslFEntry.dataType = RSSL_DT_STATE;
		rsslFEntry.fieldId = FID_STATE;
		RsslState rsslState = RSSL_INIT_STATE;
		rsslState.streamState = RSSL_STREAM_OPEN;
		rsslState.dataState = RSSL_DATA_OK;
		rsslState.code = RSSL_SC_NONE;
		rsslState.text.data = ( char* )"Succeeded";
		rsslState.text.length = 9;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslState );


		rsslFEntry.dataType = RSSL_DT_ASCII_STRING;
		rsslFEntry.fieldId = 235;		// ASCII
		RsslBuffer ascii;
		ascii.data = const_cast<char*>("ABCDEF");
		ascii.length = 6;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&ascii );


		// rmtes is treated as buffer for now... so no decoding
		rsslFEntry.dataType = RSSL_DT_RMTES_STRING;
		rsslFEntry.fieldId = 28;		// NEWS + RMTES
		RsslBuffer news;
		news.data = const_cast<char*>("ABCDEF");
		news.length = 6;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&news );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );

		FieldList fl;

		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "Fieldlist toString Decode - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "Fieldlist toString Decode - exception not expected" ;
		cout << excp << endl;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListPrePostBindElementList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode ElementList via prebind
		FieldList fieldList1;
		{
			ElementList elementList;
			fieldList1.addElementList( -15, elementList );
			EmaEncodeElementListAll( elementList );
			fieldList1.complete();
			StaticDecoder::setData( &fieldList1, &dictionary );
		}

		// Encode ElementList via postbind
		FieldList fieldList2;
		{
			ElementList elementList;
			EmaEncodeElementListAll( elementList );
			fieldList2.addElementList( -15, elementList );
			fieldList2.complete();
			StaticDecoder::setData( &fieldList2, &dictionary );
		}

		EXPECT_STREQ( fieldList1.toString(), fieldList2.toString() ) << "Pre/Post-bound ElementLists are equal - exception not expected";

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound ElementLists are equal - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(FieldListTests, testFieldListPrePostBindFieldList)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{

		// Encode FieldList via prebind
		FieldList fieldList1;
		{
			FieldList fieldList;
			fieldList1.addFieldList( -13, fieldList );
			EmaEncodeFieldListAll( fieldList );
			fieldList1.complete();
			StaticDecoder::setData( &fieldList1, &dictionary );
		}

		// Encode FieldList via postbind
		FieldList fieldList2;
		{
			FieldList fieldList;
			EmaEncodeFieldListAll( fieldList );
			fieldList2.addFieldList( -13, fieldList );
			fieldList2.complete();
			StaticDecoder::setData( &fieldList2, &dictionary );
		}

		EXPECT_STREQ( fieldList1.toString(), fieldList2.toString() ) << "Pre/Post-bound FieldLists are equal - exception not expected";

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "Pre/Post-bound FieldLists are equal - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(FieldListTests, testFieldListHybrid)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;

		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		// first entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		// second entry
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

		// third entry
		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );

		// fourth entry
		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );

		// fifth entry
		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );

		// sixth entry
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
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );

		// seventh entry (ElementList)
		rsslFEntry.fieldId = -15;		// MY_ELEMENTLIST
		RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;
		RsslElementEntry rsslEEntry;
		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 1000 );
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
		elementList.elementListNum = 5;
		rsslEncodeElementListInit( &iter, &elementList, 0, 0 );
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		uint64 = 641;
		rsslEncodeElementEntry( &iter, &rsslEEntry, &uint64 );
		rsslEncodeElementListComplete( &iter, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

		// eigth entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 642;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );

		// Convert RsslFieldList into EMA's FieldList
		FieldList decodedFieldList, encodedFieldList;
		StaticDecoder::setRsslData( &decodedFieldList, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		// encode Map with the above EMA's FieldList
		Map map;
		map.summaryData( decodedFieldList );
		map.addKeyAscii( "decodedFieldlist", MapEntry::AddEnum, decodedFieldList );
		map.addKeyAscii( "encodedFieldList", MapEntry::AddEnum, encodedFieldList );

		encodedFieldList.addUInt( 1, 123 ).complete();

		map.complete();

		StaticDecoder::setData( &map, &dictionary );

		EXPECT_EQ( map.getSummaryData().getDataType(), DataType::FieldListEnum ) << "Map::getSummaryData().getDataType()" ;

		EXPECT_TRUE( map.forth() ) << "first Map::forth()" ;
		const MapEntry& me1 = map.getEntry();

		EXPECT_EQ( me1.getLoadType(), DataType::FieldListEnum ) << "first Map::getEntry()::getLoadType()" ;

		EXPECT_TRUE( map.forth() ) << "second Map::forth()" ;
		const MapEntry& me2 = map.getEntry();

		EXPECT_EQ( me2.getLoadType(), DataType::FieldListEnum ) << "second Map::getEntry()::getLoadType()" ;

		EXPECT_FALSE( map.forth() ) << "third Map::forth()" ;

		EXPECT_TRUE( true ) << "FieldList Hybrid Usage - exception not expected" ;
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "FieldList Hybrid Usage - exception not expected" ;
		cout << excp << endl;
	}
}

TEST(FieldListTests, testFieldListError)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	{
		try
		{
			FieldList fl;
			fl.complete();
			EXPECT_TRUE( true ) << "FieldList::complete() on empty field list - exception not expected" ;

			StaticDecoder::setData( &fl, &dictionary );

			EXPECT_FALSE( fl.forth() ) << "FieldList::forth()" ;

		}
		catch ( const OmmException& excp )
		{
			cout << excp << endl;
			EXPECT_FALSE( true ) << "FieldList::complete() on empty field list - exception not expected" ;
		}
	}

	{
		try
		{
			FieldList fl;
			fl.info( 1, 65 );
			fl.complete();
			EXPECT_TRUE( true ) << "FieldList::complete() on empty field list with info - exception not expected" ;

			StaticDecoder::setData( &fl, &dictionary );

			EXPECT_TRUE( fl.hasInfo() ) << "FieldList::hasInfo()" ;

			EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "FieldList::getInfoFieldListNum()" ;

			EXPECT_EQ( fl.getInfoDictionaryId(), 1 ) << "FieldList::getInfoDictionaryId()" ;

			EXPECT_FALSE( fl.forth() ) << "FieldList::forth()" ;

		}
		catch ( const OmmException& excp )
		{
			cout << excp << endl;
			EXPECT_FALSE( true ) << "FieldList::complete() on empty field list with info - exception not expected" ;
		}
	}

	{
		try
		{
			FieldList fl;
			fl.addAscii( 235, "value 1" );
			fl.addAscii( 240, "value 2" );
			fl.complete();

			fl.addAscii( 242, "value 3" );

			EXPECT_FALSE( true ) << "FieldList::addAscii() after complete() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addAscii() after complete() - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;
			fl.addUInt( 1, 1 );
			fl.addEnum( 196, 1 );
			fl.complete();

			fl.clear();

			fl.addDouble( -10, 1.0 ).complete();

			EXPECT_TRUE( true ) << "FieldList::addDouble() after complete() & clear() - exception not expected" ;

			StaticDecoder::setData( &fl, &dictionary );

			EXPECT_TRUE( fl.forth() ) << "FieldList::forth()" ;
			EXPECT_FALSE( fl.forth() ) << "FieldList::forth()" ;

		}
		catch ( const OmmException& excp )
		{
			EXPECT_FALSE( true ) << "FieldList::addDouble() after complete() & clear() - exception not expected" << excp ;
		}
	}

	{
		try
		{
			FieldList fl;
			fl.addUInt( 1, 1 );
			fl.addEnum( 196, 1 );
			fl.complete();

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			fl.addElementList( -15, el ).complete();

			EXPECT_FALSE( true ) << "FieldList::addElementList() after complete() - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addElementList() after complete() - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;
			fl.addUInt( 1, 1 );
			fl.addEnum( 196, 1 );
			fl.complete();

			ElementList el;

			fl.addElementList( -15, el ).complete();

			EXPECT_FALSE( true ) << "FieldList::addElementList() with empty element list after complete() - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addElementList() with empty element list after complete() - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;
			fl.addUInt( 1, 1 );
			fl.addEnum( 196, 1 );
			fl.complete();

			fl.clear();

			ElementList el;
			el.addAscii( "entry", "value" ).complete();

			fl.addElementList( -15, el ).complete();

			EXPECT_TRUE( true ) << "FieldList::addElementList() after complete() & clear() - exception not expected" ;

			StaticDecoder::setData( &fl, &dictionary );

			EXPECT_TRUE( fl.forth() ) << "FieldList::forth()" ;
			EXPECT_FALSE( fl.forth() ) << "FieldList::forth()" ;

		}
		catch ( const OmmException& excp )
		{
			EXPECT_FALSE( true ) << "FieldList::addElementList() after complete() & clear() - exception not expected" << excp ;
		}
	}

	{
		try
		{
			FieldList fl;

			ElementList el;
			el.addAscii( "entry", "value" );

			fl.addElementList( -15, el ).complete();

			EXPECT_FALSE( true ) << "FieldList::addElementList() without ElementList::complete() - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addElementList() without ElementList::complete() - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;

			RefreshMsg msg;

			fl.addRefreshMsg( -19, msg );

			fl.complete();

			EXPECT_FALSE( true ) << "FieldList::addRefreshMsg() while message is empty - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addRefreshMsg() while message is empty - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;

			RefreshMsg msg;

			msg.streamId( 1 );

			fl.addRefreshMsg( -19, msg );

			fl.complete();

			StaticDecoder::setData( &fl, &dictionary );

			EXPECT_TRUE( fl.forth() ) << "FieldList::forth()" ;

			EXPECT_EQ( fl.getEntry().getLoadType(), DataType::RefreshMsgEnum ) << "ElementEntry::getLoadType()" ;

			EXPECT_EQ( static_cast<const RefreshMsg&>( fl.getEntry().getLoad() ).getStreamId(), 1 ) << "RefreshMsg::getStreamId()" ;

			EXPECT_TRUE( true ) << "FieldList::addRefreshMsg() while message is populated - exception not expected" ;

		}
		catch ( const OmmException& excp )
		{
			EXPECT_FALSE( true ) << "FieldList::addRefreshMsg() while message is populated - exception not expected" << excp;
		}
	}

	{
		try
		{
			FieldList fl;

			RefreshMsg msg;

			msg.streamId( 1 );

			msg.clear();

			fl.addRefreshMsg( -19, msg );

			fl.complete();

			EXPECT_FALSE( true ) << "FieldList::addRefreshMsg() while message is populated then cleared - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addRefreshMsg() while message is populated then cleared - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;

			GenericMsg msg;

			fl.addGenericMsg( -19, msg );

			fl.complete();

			EXPECT_FALSE( true ) << "FieldList::addGenericMsg() while message is empty - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addGenericMsg() while message is empty - exception expected" ;
		}
	}

	{
		try
		{
			FieldList fl;

			GenericMsg msg;

			msg.streamId( 1 );

			fl.addGenericMsg( -19, msg );

			fl.complete();

			StaticDecoder::setData( &fl, &dictionary );

			EXPECT_TRUE( fl.forth() ) << "FieldList::forth()" ;

			EXPECT_EQ( fl.getEntry().getLoadType(), DataType::GenericMsgEnum ) << "FieldEntry::getLoadType()" ;

			EXPECT_EQ( static_cast<const GenericMsg&>( fl.getEntry().getLoad() ).getStreamId(), 1 ) << "GenericMsg::getStreamId()" ;

			EXPECT_TRUE( true ) << "FieldList::addGenericMsg() while message is populated - exception not expected" ;

		}
		catch ( const OmmException& excp )
		{
			EXPECT_FALSE( true ) << "FieldList::addGenericMsg() while message is populated - exception not expected" << excp;
		}
	}

	{
		try
		{
			FieldList fl;

			GenericMsg msg;

			msg.streamId( 1 );

			msg.clear();

			fl.addGenericMsg( -19, msg );

			fl.complete();

			EXPECT_FALSE( true ) << "FieldList::addGenericMsg() while message is populated then cleared - exception expected" ;

		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "FieldList::addGenericMsg() while message is populated then cleared - exception expected" ;
		}
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListPrimitiveDecodingError)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	{
		try
		{
			// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,
			//                  RMTES_STRING, ENUM, FLOAT, DOUBLE, BLANK REAL, BUFFRER, UTF8_STRING,
			//					OPAUE, XML, ANSI_PAGE
			RsslFieldList rsslFL;
			RsslEncodeIterator iter;

			rsslClearFieldList( &rsslFL );
			rsslClearEncodeIterator( &iter );

			RsslBuffer rsslBuf;
			rsslBuf.length = 1000;
			rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

			rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
			rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
			rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
			rsslFL.dictionaryId = dictionary.info_DictionaryId;
			rsslFL.fieldListNum = 65;

			rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

			RsslFieldEntry rsslFEntry;

			// fid not found case (first)
			rsslFEntry.dataType = RSSL_DT_UINT;
			rsslFEntry.fieldId = -100;
			RsslUInt64 uint64 = 64;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


			//second entry
			rsslFEntry.dataType = RSSL_DT_UINT;
			rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
			uint64 = 64;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


			//third entry
			rsslFEntry.dataType = RSSL_DT_REAL;
			rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
			RsslReal real;
			double d;
			real.isBlank = RSSL_FALSE;
			real.hint = RSSL_RH_EXPONENT_2;
			real.value = 11;
			rsslRealToDouble( &d, &real );
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );


			//fourth entry
			rsslFEntry.dataType = RSSL_DT_INT;
			rsslFEntry.fieldId = -2;		// INTEGER + INT
			RsslInt64 int64 = 32;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );


			//fifth entry
			rsslFEntry.dataType = RSSL_DT_DATE;
			rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
			RsslDate date;
			date.day = 7;
			date.month = 11;
			date.year = 1999;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );


			//sixth entry
			rsslFEntry.dataType = RSSL_DT_TIME;
			rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
			RsslTime time;
			time.hour = 02;
			time.minute = 03;
			time.second = 04;
			time.millisecond = 005;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );


			//seventh entry
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
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );


			//eigthth entry
			rsslFEntry.dataType = RSSL_DT_QOS;
			rsslFEntry.fieldId = FID_QOS;
			RsslQos rsslQos;
			rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
			rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
			rsslQos.dynamic = 1;
			rsslQos.rateInfo = 0;
			rsslQos.timeInfo = 0;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslQos );

			//ninth entry
			rsslFEntry.dataType = RSSL_DT_STATE;
			rsslFEntry.fieldId = FID_STATE;
			RsslState rsslState = RSSL_INIT_STATE;
			rsslState.streamState = RSSL_STREAM_OPEN;
			rsslState.dataState = RSSL_DATA_OK;
			rsslState.code = RSSL_SC_NONE;
			rsslState.text.data = ( char* )"Succeeded";
			rsslState.text.length = 9;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&rsslState );

			//tenth entry
			rsslFEntry.dataType = RSSL_DT_ASCII_STRING;
			rsslFEntry.fieldId = 715;		// STORY_ID + ASCII
			RsslBuffer ascii;
			ascii.data = const_cast<char*>("ABCDEF");
			ascii.length = 6;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&ascii );

			//eleventh entry
			rsslFEntry.dataType = RSSL_DT_RMTES_STRING;
			rsslFEntry.fieldId = 28;		// NEWS + RMTES
			RsslBuffer news;
			news.data = const_cast<char*>("ABCDEF");
			news.length = 6;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&news );

			//twelfth entry
			rsslFEntry.dataType = RSSL_DT_ENUM;
			rsslFEntry.fieldId = 4;		// RDN_EXCHID + ENUM
			RsslEnum enumm = 29;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&enumm );

			//thirteenth entry
			rsslFEntry.dataType = RSSL_DT_FLOAT;
			rsslFEntry.fieldId = -9;
			RsslFloat floatValue = 11.11f;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&floatValue );

			//fourteenth entry
			rsslFEntry.dataType = RSSL_DT_DOUBLE;
			rsslFEntry.fieldId = -10;
			RsslDouble doubleValue = 22.22f;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&doubleValue );

			//fifteenth entry (blank)
			rsslFEntry.dataType = RSSL_DT_REAL;
			rsslFEntry.fieldId = 7;		// TRDPRC_1  + REAL
			RsslReal realValue;
			realValue.isBlank = RSSL_TRUE;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&realValue );

			//sixteenth entry
			rsslFEntry.dataType = RSSL_DT_BUFFER;
			rsslFEntry.fieldId = -11;		// MY_BUFFER
			RsslBuffer buffer;
			buffer.data = const_cast<char*>("ABCDEFGH");
			buffer.length = 8;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&buffer );

			//seventeenth entry
			rsslFEntry.dataType = RSSL_DT_UTF8_STRING;
			rsslFEntry.fieldId = -12;		// MY_UTF8
			RsslBuffer buffer_utf8;
			buffer_utf8.data = const_cast<char*>("KLMNOPQR");
			buffer_utf8.length = 8;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&buffer_utf8 );

			// eighteen entry
			rsslFEntry.fieldId = -17;
			rsslFEntry.dataType = RSSL_DT_OPAQUE;

			char opaqueBuffer[8];

			RsslBuffer buffer_opaque;
			buffer_opaque.data = opaqueBuffer;
			buffer_opaque.length = 8;

			rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 0 );
			rsslEncodeNonRWFDataTypeInit( &iter, &buffer_opaque );

			memcpy( buffer_opaque.data , "KLMNOPQR", 8 );
			buffer_opaque.length = 8;

			rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_opaque, RSSL_TRUE );
			rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

			// nineteenth entry
			rsslFEntry.fieldId = -1;
			rsslFEntry.dataType = RSSL_DT_XML;

			char xmlBuffer[25];

			RsslBuffer buffer_xml;
			buffer_xml.data = xmlBuffer;
			buffer_xml.length = 25;

			rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 0 );
			rsslEncodeNonRWFDataTypeInit( &iter, &buffer_xml );

			memcpy( buffer_xml.data , "<value> KLMNOPQR </value>", 25 );
			buffer_xml.length = 25;

			rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_xml, RSSL_TRUE );
			rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

			// nineteenth entry
			rsslFEntry.fieldId = -18;
			rsslFEntry.dataType = RSSL_DT_ANSI_PAGE;

			char ansiPageBuffer[34];

			RsslBuffer buffer_ansiPage;
			buffer_ansiPage.data = ansiPageBuffer;
			buffer_ansiPage.length = 34;

			rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 0 );
			rsslEncodeNonRWFDataTypeInit( &iter, &buffer_ansiPage );

			memcpy( buffer_ansiPage.data , "328-srfsjkj43rouw-01-20ru2l24903$%", 34 );
			buffer_ansiPage.length = 34;

			rsslEncodeNonRWFDataTypeComplete( &iter, &buffer_ansiPage, RSSL_TRUE );
			rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );


			rsslEncodeFieldListComplete( &iter, RSSL_TRUE );


			{
				Data* pData = new FieldList();;

				StaticDecoder::setRsslData( pData, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

				EXPECT_EQ( pData->getDataType(), DataType::ErrorEnum) << "Decoding FieldList without dictionary";
				EXPECT_EQ( reinterpret_cast<const OmmError*>( pData )->getErrorCode(), OmmError::NoDictionaryEnum) << "OmmError::NoDictionary";

				delete pData;
			}

			{
				Data* pData = new FieldList();;

				StaticDecoder::setRsslData( pData, &rsslBuf, RSSL_DT_FIELD_LIST, 20, RSSL_RWF_MINOR_VERSION, &dictionary );

				EXPECT_EQ( pData->getDataType(), DataType::ErrorEnum) << "Decoding FieldList wrong major version";
				EXPECT_EQ( reinterpret_cast<const OmmError*>( pData )->getErrorCode(), OmmError::IteratorSetFailureEnum) << "OmmError::NoDictionary";

				delete pData;
			}

			EXPECT_TRUE( true ) << "FieldList primitive decoding error - exception not expected" ;

		}
		catch ( const OmmException& excp )
		{
			EXPECT_FALSE( true ) << "FieldList primitive decoding error - exception not expected" << excp;
		}
		catch ( ... )
		{
			EXPECT_FALSE( true ) << "FieldList primitive decoding error + unknown exception - exception not expected" ;
		}
	}

	{
		try
		{
			// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,
			//                  RMTES_STRING, ENUM, FLOAT, DOUBLE, BLANK REAL, BUFFRER, UTF8_STRING,
			//					OPAUE, XML, ANSI_PAGE
			RsslFieldList rsslFL;
			RsslEncodeIterator iter;

			rsslClearFieldList( &rsslFL );
			rsslClearEncodeIterator( &iter );

			RsslBuffer rsslBuf;
			rsslBuf.length = 1000;
			rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

			rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
			rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
			rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
			rsslFL.dictionaryId = dictionary.info_DictionaryId;
			rsslFL.fieldListNum = 65;

			rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

			RsslFieldEntry rsslFEntry;

			// fid not found case (first)
			rsslFEntry.dataType = RSSL_DT_UINT;
			rsslFEntry.fieldId = -100;
			RsslUInt64 uint64 = 64;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );


			// encode UInt while data type is Date
			rsslFEntry.dataType = RSSL_DT_UINT;
			rsslFEntry.fieldId = 16;
			uint64 = 0xEFFFFFFFFFFFFFFF;
			rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

			rsslEncodeFieldListComplete( &iter, RSSL_TRUE );

			{
				FieldList fl;

				StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

				EXPECT_TRUE( fl.forth() ) << "FieldList::forth()" ;

				EXPECT_EQ( fl.getEntry().getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType()" ;

				EXPECT_EQ( static_cast<const OmmError&>( fl.getEntry().getLoad() ).getErrorCode(), OmmError::FieldIdNotFoundEnum) << "OmmError::getErrorCode()";

				EXPECT_TRUE( fl.forth() ) << "FieldList::forth()" ;

				EXPECT_EQ( fl.getEntry().getLoadType(), DataType::ErrorEnum ) << "FieldEntry::getLoadType()" ;

				EXPECT_EQ( static_cast<const OmmError&>( fl.getEntry().getLoad() ).getErrorCode(), OmmError::IncompleteDataEnum) << "OmmError::getErrorCode()";

				EXPECT_FALSE( fl.forth() ) << "FieldList::forth()" ;
			}

			EXPECT_TRUE( true ) << "FieldList primitive decoding error - exception not expected" ;
		}
		catch ( const OmmException& excp )
		{
		  EXPECT_FALSE( true ) << "FieldList primitive decoding error - exception not expected" << excp;
		}
		catch ( ... )
		{
			EXPECT_FALSE( true ) << "FieldList primitive decoding error + unknown exception - exception not expected" ;
		}
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(FieldListTests, testFieldListEncodeEMADecodeEMARippleToRippleToName)
{
    // load dictionary for decoding of the field list
    RsslDataDictionary dictionary;

    ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

    FieldList flEnc;
    flEnc.info(dictionary.info_DictionaryId, 65);

    try
    {
        //EMA Encoding for ripple fields

        //first entry (fid not found case)
        flEnc.addReal(6, 5236, OmmReal::ExponentNeg2Enum); // TRDPRC_1

        //second entry
        flEnc.addUInt(1, 64); // PROD_PERM

        //third entry
        flEnc.addTime(286, 23, 59, 60); // HIGH_TIME

        //fourth entry
        flEnc.addEnum(270, 5); // ACT_TP_1

        flEnc.complete();

        //Now do EMA decoding of FieldList
        StaticDecoder::setData(&flEnc, &dictionary);

        EXPECT_TRUE( flEnc.hasInfo() ) << "FieldList with primitives - hasInfo()";
		EXPECT_EQ(flEnc.getInfoDictionaryId(), dictionary.info_DictionaryId) << "FieldList with primitives - getInfoDictionaryId()";
        EXPECT_EQ( flEnc.getInfoFieldListNum(), 65 ) << "FieldList with primitives - getInfoFieldListNum()";

        EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives and Map - first forth()";
        const FieldEntry& fe1 = flEnc.getEntry();
        EXPECT_EQ( fe1.getFieldId(), 6 ) << "FieldEntry::getFieldId()";
        EXPECT_STREQ( fe1.getName(), "TRDPRC_1" ) << "FieldEntry::getName()";
        EXPECT_EQ( fe1.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum";
        EXPECT_EQ( fe1.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum";
        EXPECT_EQ( fe1.getReal().getMantissa(), 5236 ) << "FieldEntry::getReal().getMantissa()";
        EXPECT_EQ( fe1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "FieldEntry::getReal().getMagnitudeType()";
        EXPECT_EQ( fe1.getRippleTo(), 7 ) << "FieldEntry::getRippleTo()";
        EXPECT_STREQ( fe1.getRippleToName(), "TRDPRC_2" ) << "FieldEntry::getRippleToName()";

        EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives - second forth()";
        const FieldEntry& fe2 = flEnc.getEntry();
        EXPECT_EQ( fe2.getFieldId(), 1 ) << "FieldEntry::getFieldId()";
        EXPECT_STREQ( fe2.getName(), "PROD_PERM" ) << "FieldEntry::getName()";
        EXPECT_EQ( fe2.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum";
        EXPECT_EQ( fe2.getLoad().getDataType(), DataType::UIntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::UIntEnum";
        EXPECT_EQ( fe2.getUInt(), 64 ) << "FieldEntry::getUInt()";
        EXPECT_EQ( fe2.getRippleTo(), 0 ) << "FieldEntry::getRippleTo()";
		EXPECT_STREQ(fe2.getRippleToName(), "") << "FieldEntry::getRippleToName()";

        EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives - third forth()";
        const FieldEntry& fe3 = flEnc.getEntry();
        EXPECT_EQ( fe3.getFieldId(), 286 ) << "FieldEntry::getFieldId()";
        EXPECT_STREQ( fe3.getName(), "HIGH_TIME" ) << "FieldEntry::getName()";
        EXPECT_EQ( fe3.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum";
        EXPECT_EQ( fe3.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum";
        EXPECT_EQ( fe3.getTime().getHour(), 23 ) << "FieldEntry::getTime().getHour()";
        EXPECT_EQ( fe3.getTime().getMinute(), 59 ) << "FieldEntry::getReal().getMagnitudeType()";
        EXPECT_EQ( fe3.getTime().getSecond(), 60 ) << "FieldEntry::getReal().getMagnitudeType()";
        EXPECT_EQ( fe2.getRippleTo(), 3625 ) << "FieldEntry::getRippleTo()";
        EXPECT_STREQ( fe2.getRippleToName(), "HIGH_TIME2" ) << "FieldEntry::getRippleToName()";

        EXPECT_TRUE( flEnc.forth() ) << "FieldList with primitives - fourth forth()";
        const FieldEntry& fe4 = flEnc.getEntry();
        EXPECT_EQ( fe4.getFieldId(), 270 ) << "FieldEntry::getFieldId()";
        EXPECT_STREQ( fe4.getName(), "ACT_TP_1" ) << "FieldEntry::getName()";
        EXPECT_EQ( fe4.getLoadType(), DataType::EnumEnum ) << "FieldEntry::getLoadType() == DataType::EnumEnum";
        EXPECT_EQ( fe4.getLoad().getDataType(), DataType::EnumEnum ) << "ElementEntry::getLoad().getDataType() == DataType::EnumEnum";
        EXPECT_EQ( fe4.getEnum(), 5 ) << "FieldEntry::getEnum()";
        EXPECT_EQ( fe4.getRippleTo(), 271 ) << "FieldEntry::getRippleTo()";
        EXPECT_STREQ( fe4.getRippleToName(), "ACT_TP_2" ) << "FieldEntry::getRippleToName()";

        // Searching for others ripple fields
        EXPECT_EQ( fe4.getRippleTo(23), 24 ) << "FieldEntry::getRippleTo(int)";
        EXPECT_STREQ( fe4.getRippleToName(26), "ASK_2" ) << "FieldEntry::getRippleToName(int)";
        EXPECT_EQ( fe4.getRippleTo(18), 0 ) << "FieldEntry::getRippleTo(int)";
        EXPECT_STREQ( fe4.getRippleToName(29), "") << "FieldEntry::getRippleToName(int)"; //NEWS_TIME

        EXPECT_FALSE( flEnc.forth() ) << "FieldList with primitives - fifth forth()";

        EXPECT_TRUE( true ) << "FieldList with primitives - exception not expected";
    }
    catch (const OmmException& excp)
    {
                    EXPECT_FALSE( true ) << "FieldList with primitives and Map - exception not expected" << excp << endl;
    }

    rsslDeleteDataDictionary(&dictionary);
}

TEST(FieldListTests, testFieldListAddInfoAfterInitialized)
{
	try
	{
		FieldList fieldList;
		fieldList.addCodeUInt(1).info(1,1).complete();
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(false) << "Encode info after FieldList is initialized - exception expected with text" << exp.getText().c_str();
		EXPECT_STREQ("Invalid attempt to call info() when container is initialized.", exp.getText().c_str());
		return;
	}

	EXPECT_TRUE(false) << "Encode total count hint after FieldList is initialized - exception expected";
}

TEST(FieldListTests, testFieldListClear_Encode_Decode)
{
	try
	{
		// load dictionary for decoding of the field list
		RsslDataDictionary dictionary;

		ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

		FieldList fieldList;
		fieldList.info(1, 2).addUInt(1, 555)
			.clear()
			.info(3, 4).addUInt(1, 666).complete();

		StaticDecoder::setData(&fieldList, &dictionary);

		EXPECT_TRUE(fieldList.hasInfo()) << "Check has info attribute";
		EXPECT_TRUE(fieldList.getInfoDictionaryId() == 3) << "Check the info dictionary ID attribute";
		EXPECT_TRUE(fieldList.getInfoFieldListNum() == 4) << "Check the info field list num attribute";

		EXPECT_TRUE(fieldList.forth()) << "Get the first Field entry";
		EXPECT_TRUE(fieldList.getEntry().getFieldId() == 1) << "Check the field ID of the first entry";
		EXPECT_TRUE(fieldList.getEntry().getLoadType() == DataType::UIntEnum) << "Check the load type of the first entry";
		EXPECT_TRUE(fieldList.getEntry().getUInt() == 666) << "Check the Uint value of the first entry";

		EXPECT_FALSE(fieldList.forth()) << "Check to make sure that there is no more enty in FieldList";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "Fails to encode and decode FieldList - exception not expected with text" << exp.getText().c_str();
	}
}

TEST(FieldListTests, testFieldListAddNotCompletedContainer)
{
	try
	{
		FieldList fieldList;
		ElementList elementList;
		fieldList.addElementList(1, elementList);
		fieldList.complete();

		EXPECT_FALSE(true) << "FieldList complete while ElementList is not completed  - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "FieldList complete while ElementList is not completed  - exception expected";
	}

	try
	{
		FieldList fieldList;
		ElementList elementList;
		fieldList.addElementList(1, elementList);
		elementList.addUInt("test", 64);
		fieldList.complete();

		EXPECT_FALSE(true) << "FieldList complete while ElementList with data is not completed  - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "FieldList complete while ElementList with data is not completed  - exception expected";
	}

	try
	{
		FieldList fieldList;
		ElementList elementList;
		fieldList.addElementList(1, elementList);
		fieldList.addElementList(2, elementList);

		EXPECT_FALSE(true) << "FieldList add two not completed ElementLists - exception expected";
	}
	catch (const OmmException& )
	{
		EXPECT_TRUE(true) << "FieldList add two not completed ElementLists - exception expected";
	}

	try
	{
		FieldList fieldList;
		ElementList elementList, elementList1;
		fieldList.addElementList(1, elementList);
		elementList.complete();
		fieldList.addElementList(2, elementList1);
		fieldList.complete();

		EXPECT_FALSE(true) << "FieldList add first completed and second not completed ElementList - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "FieldList add first completed and second not completed ElementList - exception expected";
	}

	try
	{
		FieldList fieldList;
		ElementList elementList, elementList1;
		fieldList.addElementList(1, elementList);
		elementList1.complete();
		fieldList.addElementList(2, elementList1);
		fieldList.complete();

		EXPECT_FALSE(true) << "FieldList add first not completed and second completed ElementList - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "FieldList add first not completed and second completed ElementList - exception expected";
	}

	try
	{
		FieldList fieldList;
		ElementList elementList, elementList1;
		fieldList.addElementList(1, elementList);
		elementList.complete();
		fieldList.complete();
		fieldList.addElementList(2, elementList1);
		fieldList.complete();

		EXPECT_FALSE(true) << "FieldList add first completed then compleat map and add some second ElementList - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "FieldList add first completed then compleat map and add some second ElementList - exception expected";
	}

	try
	{
		FieldList fieldList, fieldList1;
		ElementList elementList;
		fieldList1.addElementList(1, elementList);
		elementList.complete();
		fieldList1.complete();
		fieldList.addFieldList(2, fieldList1);
		fieldList.complete();

		EXPECT_TRUE(true) << "FieldList add completed FieldList with nested ElementList - exception not expected";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "FieldList add completed FieldList with nested ElementList - exception not expected with test: "<< exp.getText();
	}

	try
	{
		FieldList fieldList;
		fieldList.addElementList(1, ElementList().addInt("text1", 1).complete());
		fieldList.addElementList(2, ElementList().addInt("text2", 2).complete());
		fieldList.complete();

		EXPECT_TRUE(true) << "FieldList add completed FieldList with nested ElementList - exception not expected";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "FieldList add completed FieldList with nested ElementList - exception not expected with test: " << exp.getText();
	}

	try
	{
		FieldList filedList;
		GenericMsg genericMsg;

		genericMsg.streamId(1);

		filedList.addGenericMsg(1, genericMsg);
		filedList.complete();

		EXPECT_TRUE(true) << "FieldList add not completed GenericMsg - exception not expected";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "FieldList add not completed GenericMsg - exception not expected with text: " << exp.getText();
	}

	try
	{
		FieldList fieldtList;
		OmmOpaque opaque;

		char* string = const_cast<char*>("OPQRST");
		EmaBuffer buffer(string, 6);
		opaque.set(buffer);

		fieldtList.addOpaque(1, opaque);
		fieldtList.complete();

		EXPECT_TRUE(true) << "ElementList add OmmOpaque - exception not expected";
	}
	catch (const OmmException& exp)
	{
		EXPECT_FALSE(true) << "ElementList add OmmOpaque - exception not expected with text: " << exp.getText();
	}

	try
	{
		FieldList fieldtList;
		ElementList elementList;
		GenericMsg genericMsg;

		genericMsg.streamId(1);

		fieldtList.addGenericMsg(1, genericMsg);
		fieldtList.addElementList(1, elementList);
		fieldtList.complete();

		EXPECT_FALSE(true) << "FieldList add not completed ElementList after GenericMsg - exception expected";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "FieldList add not completed ElementList after GenericMsg - exception expected";
	}
}
