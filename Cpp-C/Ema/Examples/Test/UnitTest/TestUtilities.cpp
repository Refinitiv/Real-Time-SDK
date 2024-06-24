/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace std;

// no google tests in this function
bool loadDictionaryFromFile( RsslDataDictionary* pDictionary )
{
	RsslBool fieldDictionaryLoaded = RSSL_FALSE, enumTableLoaded = RSSL_FALSE;
	char errTxt[256];
	RsslBuffer errorText = {255, ( char* )errTxt};

	const char* fieldDictionaryFileName = "RDMFieldDictionaryTest";
	const char* enumTableFileName = "enumtypeTest.def";

	rsslClearDataDictionary( pDictionary );

	if ( rsslLoadFieldDictionary( fieldDictionaryFileName, pDictionary, &errorText ) < 0 )
		cout << "\nUnable to load field dictionary.\n\tError Text: " << errorText.data << endl;
	else
		fieldDictionaryLoaded = RSSL_TRUE;

	if ( rsslLoadEnumTypeDictionary( enumTableFileName, pDictionary, &errorText ) < 0 )
		cout << "\nUnable to load enum type dictionary.\n\tError Text: " << errorText.data << endl;
	else
		enumTableLoaded = RSSL_TRUE;

	if ( fieldDictionaryLoaded && enumTableLoaded )
	{
		return true;
	}
	else
	{
		rsslDeleteDataDictionary( pDictionary );

		return false;
	}
}

// no google tests in this function
void RsslEncodeFieldListAll( RsslBuffer& rsslBuf )
{
	// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,

	RsslFieldList rsslFL;
	RsslEncodeIterator iter;

	rsslClearFieldList( &rsslFL );
	rsslClearEncodeIterator( &iter );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
	rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
	rsslFL.dictionaryId = 1;
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
	ascii.data = const_cast<char*>( "ABCDEF" );
	ascii.length = 6;
	rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&ascii );


	rsslBuf.length = rsslGetEncodedBufferLength( &iter );

	rsslEncodeFieldListComplete( &iter, RSSL_TRUE );
}

// no google tests in this function
void RsslEncodeElementListAll( RsslBuffer& rsslBuf )
{
	RsslElementList rsslEL;
	RsslEncodeIterator iter;

	rsslClearElementList( &rsslEL );
	rsslClearEncodeIterator( &iter );

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
	//RsslUInt rsslUInt = 17;
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
	ascii.data = const_cast<char*>( "ABCDEF" );
	ascii.length = 6;
	rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&ascii );

	//tenth entry
	rsslEEntry.name.data = ( char* )"Element - RmtesString";
	rsslEEntry.name.length = 21;
	rsslEEntry.dataType = RSSL_DT_RMTES_STRING;
	RsslBuffer news;
	news.data = const_cast<char*>( "ABCDEF" );
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
	buffer.data = const_cast<char*>( "ABCDEFGH" );
	buffer.length = 8;
	rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&buffer );

	//sixteenth entry
	rsslEEntry.name.data = ( char* )"Element - Utf8String";
	rsslEEntry.name.length = 20;
	rsslEEntry.dataType = RSSL_DT_UTF8_STRING;
	RsslBuffer buffer_utf8;
//		buffer_utf8.data = "KLMNOPQR";
	buffer_utf8.data = const_cast<char*>( "ABCDEFGH" );
	buffer_utf8.length = 8;
	rsslEncodeElementEntry( &iter, &rsslEEntry, ( void* )&buffer_utf8 );


	rsslEncodeElementListComplete( &iter, RSSL_TRUE );

}

// no google tests in this function
void RsslEncodeMapAll( RsslBuffer& mapBuffer )
{
	// encoding order:  SummaryData(with FieldList),
	//                  Buffer-Delete,
	//                  FieldList-Add,
	//                  Buffer-Add,
	//                  Buffer-Update,

	RsslMap rsslMap;
	RsslEncodeIterator mapEncodeIter;

	rsslClearMap( &rsslMap );

	rsslClearEncodeIterator( &mapEncodeIter );

	rsslSetEncodeIteratorRWFVersion( &mapEncodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );

	rsslSetEncodeIteratorBuffer( &mapEncodeIter, &mapBuffer );

	rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;

	rsslMap.containerType = RSSL_DT_FIELD_LIST;

	rsslMap.keyPrimitiveType = RSSL_DT_BUFFER;
	rsslMap.keyFieldId = 3426;

	// allocate buffer for the field list for SummaryData
	RsslBuffer rsslBuf;
	rsslBuf.length = 1000;
	rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

	RsslEncodeFieldListAll( rsslBuf );

	rsslMap.encSummaryData = rsslBuf;


	rsslEncodeMapInit( &mapEncodeIter, &rsslMap, 0, 0 );

	RsslMapEntry mapEntry;


	//first entry  //Delete Buffer
	rsslClearMapEntry( &mapEntry );

	mapEntry.flags = RSSL_MPEF_NONE;
	mapEntry.action = RSSL_MPEA_DELETE_ENTRY;

	RsslBuffer orderBuf;
	orderBuf.data = const_cast<char*>( "ABCD" );
	orderBuf.length = 4;

	rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );


	//second entry  //Add FieldList
	rsslClearMapEntry( &mapEntry );

	RsslBuffer rsslBuf1;
	rsslBuf1.length = 1000;
	rsslBuf1.data = ( char* )malloc( sizeof( char ) * 1000 );
	RsslEncodeFieldListAll( rsslBuf1 );

	mapEntry.flags = RSSL_MPEF_NONE;
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	mapEntry.encData = rsslBuf1;

	rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &rsslBuf1 );


	//third entry  //Add Buffer
	rsslClearMapEntry( &mapEntry );

	mapEntry.flags = RSSL_MPEF_NONE;
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	mapEntry.encData = rsslBuf;

	orderBuf.data = const_cast<char*>( "EFGHI" );
	orderBuf.length = 5;

	rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );


	//fourth entry  //Update Buffer
	rsslClearMapEntry( &mapEntry );

	mapEntry.flags = RSSL_MPEF_NONE;
	mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
	mapEntry.encData = rsslBuf;

	orderBuf.data = const_cast<char*>( "JKLMNOP" );
	orderBuf.length = 7;

	rsslEncodeMapEntry( &mapEncodeIter, &mapEntry, &orderBuf );

	mapBuffer.length = rsslGetEncodedBufferLength( &mapEncodeIter );
	rsslEncodeMapComplete( &mapEncodeIter, RSSL_TRUE );
}

//corresponding decoding is in TestUtilities::EmaDecodeFieldListAll(FieldList& fl)
// no google tests in this function
void EmaEncodeFieldListAll( FieldList& fl )
{
	// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,

	fl.info( 1, 65 );

	fl.addUInt( 1, 64 );

	fl.addReal( 6, 11, OmmReal::ExponentNeg2Enum );

	fl.addInt( -2, 32 );

	fl.addDate( 16, 1999, 11, 7 );

	fl.addTime( 18, 02, 03, 04, 005 );

	fl.addDateTime( -3, 1999, 11, 7, 01, 02, 03, 000 );

	fl.addQos( FID_QOS, OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );

	fl.addState( FID_STATE, OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );

	fl.addAscii( 235, EmaString( "ABCDEF" ) );

	fl.complete();
}

// no google tests in this function
void EmaEncodeElementListAll( ElementList& el )
{
	el.info( 5 );

	//first entry
	el.addUInt( EmaString( "Element - UInt" ), 64 );

	//second entry
	el.addReal( EmaString( "Element - Real" ), 11, OmmReal::ExponentNeg2Enum );

	//third entry
	el.addInt( EmaString( "Element - Int" ), 32 );

	//fourth entry
	el.addDate( EmaString( "Element - Date" ), 1999, 11, 7 );

	//fifth entry
	el.addTime( EmaString( "Element - Time" ), 02, 03, 04, 005 );

	//sixth entry
	el.addDateTime( EmaString( "Element - DateTime" ), 1999, 11, 7, 01, 02, 03, 000 );

	//seventh entry
	el.addQos( EmaString( "Element - Qos" ), OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );

	//eightth entry
	el.addState( EmaString( "Element - State" ), OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );

	//ninth entry
	el.addAscii( EmaString( "Element - AsciiString" ), EmaString( "ABCDEF" ) );

	//tenth entry
	char* s1 = const_cast<char*>( "ABCDEF" );
	EmaBuffer buf1( s1, 6 );
	el.addRmtes( EmaString( "Element - RmtesString" ), buf1 );

	//eleventh entry
	el.addEnum( EmaString( "Element - Enum" ), 29 );

	//twelfth entry
	el.addFloat( EmaString( "Element - Float" ), 11.11f );

	//thirteenth entry
	el.addDouble( EmaString( "Element - Double" ), 22.22f );

	//fourteenth entry
	el.addCodeReal( EmaString( "Element - RealBlank" ) );

	//fifteenth entry
	char* s2 = const_cast<char*>( "ABCDEFGH" );
	EmaBuffer buf2( s2, 8 );
	el.addBuffer( EmaString( "Element - Buffer" ), buf2 );

	//sixteenth entry
	char* s3 = const_cast<char*>( "ABCDEFGH");
	EmaBuffer buf3( s3, 8 );
	el.addUtf8( EmaString( "Element - Utf8String" ), buf3 );

	el.complete();
}

// no google tests in this function
void EmaEncodeMapAll( Map& map )
{
	// encoding order:  SummaryData(with FieldList),
	//                  Buffer-Delete,
	//                  FieldList-Add,
	//                  Buffer-Add,
	//                  Buffer-Update,

	map.totalCountHint( 5 );
	map.keyFieldId( 3426 );

	FieldList flEnc;
	EmaEncodeFieldListAll( flEnc );

	map.summaryData( flEnc );

	char* s1 = const_cast<char*>( "PERMISSION DATA" );
	EmaBuffer permission( s1, 15 );

	//first entry  //Delete Buffer
	char* orderBufData1 = const_cast<char*>( "ABCD" );
	EmaBuffer orderBuf1( orderBufData1, 4 );
	FieldList flEnc1;
	EmaEncodeFieldListAll( flEnc1 );
	map.addKeyBuffer( orderBuf1, MapEntry::DeleteEnum, flEnc1, permission );

	//second entry  //Add FieldList
	map.addKeyBuffer( orderBuf1, MapEntry::AddEnum, flEnc1, permission );

	//third entry  //Add FieldList
	char* orderBufData2 = const_cast<char*>( "EFGHI" );
	EmaBuffer orderBuf2( orderBufData2, 5 );
	map.addKeyBuffer( orderBuf2, MapEntry::AddEnum, flEnc1, permission );

	//fourth entry  //Update FieldList
	char* orderBufData3 = const_cast<char*>( "JKLMNOP" );
	EmaBuffer orderBuf3( orderBufData3, 7 );
	map.addKeyBuffer( orderBuf3, MapEntry::UpdateEnum, flEnc1, permission );

	map.complete();
}

//corresponds to TestUtilities::EmaEncodeFieldListAll(FieldList& fl)
// before calling this test, call SCOPED_TRACE("calling EmaDecodeFieldListAll")
void EmaDecodeFieldListAll( const FieldList& fl )
{
  EXPECT_TRUE( fl.hasInfo() ) << "Decode FieldList - hasInfo()";
  EXPECT_EQ( fl.getInfoDictionaryId(), 1 ) << "Decode FieldList - getInfoDictionaryId()";
  EXPECT_EQ( fl.getInfoFieldListNum(), 65 ) << "Decode FieldList - getInfoFieldListNum()";

	try
	{
		const FieldEntry& fe = fl.getEntry();
		EXPECT_FALSE( true ) << "Decode FieldList - exception expected";
	}
	catch ( OmmException& excp )
	{
	        EXPECT_TRUE( true ) << "Decode FieldList - exception expected: " << excp.getText();
	}

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - first fieldlist forth()";
	const FieldEntry& fe1 = fl.getEntry();
	EXPECT_EQ( fe1.getFieldId(), 1 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe1.getName(), "PROD_PERM" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe1.getLoadType(), DataType::UIntEnum ) << "FieldEntry::getLoadType() == DataType::UIntEnum";
	EXPECT_EQ( fe1.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe1.getUInt(), 64 ) << "FieldEntry::getUInt()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - second fieldlist forth()";
	const FieldEntry& fe2 = fl.getEntry();
	EXPECT_EQ( fe2.getFieldId(), 6 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe2.getName(), "TRDPRC_1" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe2.getLoadType(), DataType::RealEnum ) << "FieldEntry::getLoadType() == DataType::RealEnum";
	EXPECT_EQ( fe2.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe2.getReal().getMantissa(), 11 ) << "FieldEntry::getReal().getMantissa()";
	EXPECT_EQ( fe2.getReal().getMagnitudeType(), 12 ) << "FieldEntry::getReal().getMagnitudeType()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - third fieldlist forth()";
	const FieldEntry& fe3 = fl.getEntry();
	EXPECT_EQ( fe3.getFieldId(), -2 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe3.getName(), "INTEGER" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe3.getLoadType(), DataType::IntEnum ) << "FieldEntry::getLoadType() == DataType::IntEnum";
	EXPECT_EQ( fe3.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe3.getInt(), 32 ) << "FieldEntry::getInt()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - fourth fieldlist forth()";
	const FieldEntry& fe4 = fl.getEntry();
	EXPECT_EQ( fe4.getFieldId(), 16 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe4.getName(), "TRADE_DATE" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe4.getLoadType(), DataType::DateEnum ) << "FieldEntry::getLoadType() == DataType::DateEnum";
	EXPECT_EQ( fe4.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe4.getDate().getDay(), 7 ) << "FieldEntry::getDate().getDay()";
	EXPECT_EQ( fe4.getDate().getMonth(), 11 ) << "FieldEntry::getDate().getMonth()";
	EXPECT_EQ( fe4.getDate().getYear(), 1999 ) << "FieldEntry::getDate().getyear()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - fifth fieldlist forth()";
	const FieldEntry& fe5 = fl.getEntry();
	EXPECT_EQ( fe5.getFieldId(), 18 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe5.getName(), "TRDTIM_1" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe5.getLoadType(), DataType::TimeEnum ) << "FieldEntry::getLoadType() == DataType::TimeEnum";
	EXPECT_EQ( fe5.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe5.getTime().getHour(), 02 ) << "FieldEntry::getTime().getHour()";
	EXPECT_EQ( fe5.getTime().getMinute(), 03 ) << "FieldEntry::getTime().getMinute()";
	EXPECT_EQ( fe5.getTime().getSecond(), 04 ) << "FieldEntry::getTime().getSecond()";
	EXPECT_EQ( fe5.getTime().getMillisecond(), 005 ) << "FieldEntry::getTime().getMillisecond()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - sixth fieldlist forth()";
	const FieldEntry& fe6 = fl.getEntry();
	EXPECT_EQ( fe6.getFieldId(), -3 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe6.getName(), "TRADE_DATE" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe6.getLoadType(), DataType::DateTimeEnum ) << "FieldEntry::getLoadType() == DataType::DateTimeEnum";
	EXPECT_EQ( fe6.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe6.getDateTime().getDay(), 7 ) << "FieldEntry::getDateTime().getDay()";
	EXPECT_EQ( fe6.getDateTime().getMonth(), 11 ) << "FieldEntry::getDateTime().getMonth()";
	EXPECT_EQ( fe6.getDateTime().getYear(), 1999 ) << "FieldEntry::getDateTime().getYear()";
	EXPECT_EQ( fe6.getDateTime().getHour(), 01 ) << "FieldEntry::getDateTime().getHour()";
	EXPECT_EQ( fe6.getDateTime().getMinute(), 02 ) << "FieldEntry::getDateTime().getMinute()";
	EXPECT_EQ( fe6.getDateTime().getSecond(), 03 ) << "FieldEntry::getDateTime().getSecond()";
	EXPECT_EQ( fe6.getDateTime().getMillisecond(), 000 ) << "FieldEntry::getDateTime().getMillisecond()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - seventh fieldlist forth()";
	const FieldEntry& fe7 = fl.getEntry();
	EXPECT_EQ( fe7.getFieldId(), FID_QOS ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe7.getName(), "MY_QOS" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe7.getLoadType(), DataType::QosEnum ) << "FieldEntry::getLoadType() == DataType::QosEnum";
	EXPECT_EQ( fe7.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe7.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "FieldEntry::getQos().getTimeliness()";
	EXPECT_EQ( fe7.getQos().getRate(), OmmQos::TickByTickEnum ) << "FieldEntry::getQos().getRate()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - eigtth fieldlist forth()";
	const FieldEntry& fe8 = fl.getEntry();
	EXPECT_EQ( fe8.getFieldId(), FID_STATE ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe8.getName(), "MY_STATE" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe8.getLoadType(), DataType::StateEnum ) << "FieldEntry::getLoadType() == DataType::StateEnum";
	EXPECT_EQ( fe8.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( fe8.getState().getStreamState(), OmmState::OpenEnum ) << "FieldEntry::getState().getStreamState()";
	EXPECT_EQ( fe8.getState().getDataState(), OmmState::OkEnum ) << "FieldEntry::getState().getDataState()";
	EXPECT_EQ( fe8.getState().getStatusCode(), OmmState::NoneEnum ) << "FieldEntry::getState().getStatusCode()";
	EXPECT_STREQ( fe8.getState().getStatusText(), "Succeeded" ) << "FieldEntry::getState().getStatusText()";
	EXPECT_STREQ( fe8.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "FieldEntry::getState().toString()";

	EXPECT_TRUE( fl.forth() ) << "Decode FieldList - ninth fieldlist forth()";
	const FieldEntry& fe9 = fl.getEntry();
	EXPECT_EQ( fe9.getFieldId(), 235 ) << "FieldEntry::getFieldId()";
	EXPECT_STREQ( fe9.getName(), "PNAC" ) << "FieldEntry::getName()";
	EXPECT_EQ( fe9.getLoadType(), DataType::AsciiEnum ) << "FieldEntry::getLoadType() == DataType::AsciiEnum";
	EXPECT_EQ( fe9.getCode(), Data::NoCodeEnum ) << "FieldEntry::getCode() == Data::NoCodeEnum";
	EXPECT_STREQ( fe9.getAscii(), "ABCDEF" ) << "FieldEntry::getAscii()";

	EXPECT_TRUE( !fl.forth() ) << "Decode FieldList - final fieldlist forth()";
}


//corresponds to TestUtilities::EmaEncodeElementListAll(ElementList& el)
// before calling this test, call SCOPED_TRACE("calling EmaDecodeElementListAll")
void EmaDecodeElementListAll( const ElementList& el )
{
  EXPECT_TRUE( el.hasInfo() ) << "Decode ElementList - hasInfo()";
  EXPECT_EQ( el.getInfoElementListNum(), 5 ) << "MapEntry ElementList within map- getInfoElementListNum()";

	try
	{
		const ElementEntry& ee = el.getEntry();
		EXPECT_FALSE( true ) << "Decode ElementList - exception expected";
	}
	catch ( OmmException& excp )
	{
	        EXPECT_TRUE( true ) << "Decode ElementList - exception expected: " << excp.getText();
	}

	EXPECT_TRUE( el.forth() ) << "Decode ElementList within map - first elementlist forth()";
	const ElementEntry& ee1 = el.getEntry();
	EXPECT_STREQ( ee1.getName(), "Element - UInt" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee1.getLoadType(), DataType::UIntEnum ) << "ElementEntry::getLoadType() == DataType::UIntEnum";
	EXPECT_EQ( ee1.getUInt(), 64 ) << "ElementEntry::getUInt()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList within map - second elementlist forth()";
	const ElementEntry& ee2 = el.getEntry();
	EXPECT_STREQ( ee2.getName(), "Element - Real" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee2.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum";
	EXPECT_EQ( ee2.getReal().getMantissa(), 11 ) << "ElementEntry::getReal().getMantissa()";
	EXPECT_EQ( ee2.getReal().getMagnitudeType(), 12 ) << "ElementEntry::getReal().getMagnitudeType()";

	EXPECT_TRUE( el.forth() ) << "Decode Decode ElementList - third elementlist forth()";
	const ElementEntry& ee3 = el.getEntry();
	EXPECT_STREQ( ee3.getName(), "Element - Int" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee3.getLoadType(), DataType::IntEnum ) << "ElementEntry::getLoadType() == DataType::IntEnum";
	EXPECT_EQ( ee3.getLoad().getDataType(), DataType::IntEnum ) << "ElementEntry::getLoad().getDataType() == DataType::IntEnum";
	EXPECT_EQ( ee3.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee3.getInt(), 32 ) << "ElementEntry::getInt()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - fourth elementlist forth()";
	const ElementEntry& ee4 = el.getEntry();
	EXPECT_STREQ( ee4.getName(), "Element - Date" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee4.getLoadType(), DataType::DateEnum ) << "ElementEntry::getLoadType() == DataType::DateEnum";
	EXPECT_EQ( ee4.getLoad().getDataType(), DataType::DateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateEnum";
	EXPECT_EQ( ee4.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee4.getDate().getDay(), 7 ) << "ElementEntry::getDate().getDay()";
	EXPECT_EQ( ee4.getDate().getMonth(), 11 ) << "ElementEntry::getDate().getMonth()";
	EXPECT_EQ( ee4.getDate().getYear(), 1999 ) << "ElementEntry::getDate().getyear()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - fifth elementlist forth()";
	const ElementEntry& ee5 = el.getEntry();
	EXPECT_STREQ( ee5.getName(), "Element - Time" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee5.getLoadType(), DataType::TimeEnum ) << "ElementEntry::getLoadType() == DataType::TimeEnum";
	EXPECT_EQ( ee5.getLoad().getDataType(), DataType::TimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::TimeEnum";
	EXPECT_EQ( ee5.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee5.getTime().getHour(), 02 ) << "ElementEntry::getTime().getHour()";
	EXPECT_EQ( ee5.getTime().getMinute(), 03 ) << "ElementEntry::getTime().getMinute()";
	EXPECT_EQ( ee5.getTime().getSecond(), 04 ) << "ElementEntry::getTime().getSecond()";
	EXPECT_EQ( ee5.getTime().getMillisecond(), 005 ) << "ElementEntry::getTime().getMillisecond()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - sixth elementlist forth()";
	const ElementEntry& ee6 = el.getEntry();
	EXPECT_STREQ( ee6.getName(), "Element - DateTime" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee6.getLoadType(), DataType::DateTimeEnum ) << "ElementEntry::getLoadType() == DataType::DateTimeEnum";
	EXPECT_EQ( ee6.getLoad().getDataType(), DataType::DateTimeEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DateTimeEnum";
	EXPECT_EQ( ee6.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee6.getDateTime().getDay(), 7 ) << "ElementEntry::getDateTime().getDay()";
	EXPECT_EQ( ee6.getDateTime().getMonth(), 11 ) << "ElementEntry::getDateTime().getMonth()";
	EXPECT_EQ( ee6.getDateTime().getYear(), 1999 ) << "ElementEntry::getDateTime().getYear()";
	EXPECT_EQ( ee6.getDateTime().getHour(), 01 ) << "ElementEntry::getDateTime().getHour()";
	EXPECT_EQ( ee6.getDateTime().getMinute(), 02 ) << "ElementEntry::getDateTime().getMinute()";
	EXPECT_EQ( ee6.getDateTime().getSecond(), 03 ) << "ElementEntry::getDateTime().getSecond()";
	EXPECT_EQ( ee6.getDateTime().getMillisecond(), 000 ) << "ElementEntry::getDateTime().getMillisecond()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - seventh elementlist forth()";
	const ElementEntry& ee7 = el.getEntry();
	EXPECT_STREQ( ee7.getName(), "Element - Qos" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee7.getLoadType(), DataType::QosEnum ) << "ElementEntry::getLoadType() == DataType::QosEnum";
	EXPECT_EQ( ee7.getLoad().getDataType(), DataType::QosEnum ) << "ElementEntry::getLoad().getDataType() == DataType::QosEnum";
	EXPECT_EQ( ee7.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee7.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "ElementEntry::getTime().getTimeliness()";
	EXPECT_EQ( ee7.getQos().getRate(), OmmQos::TickByTickEnum ) << "ElementEntry::getQos().getRate()";
//	EXPECT_EQ( ee7.getQos().getRate(), 1 ) << "ElementEntry::getQos().getRate()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - eightth elementlist forth()";
	const ElementEntry& ee8 = el.getEntry();
	EXPECT_STREQ( ee8.getName(), "Element - State" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee8.getLoadType(), DataType::StateEnum ) << "ElementEntry::getLoadType() == DataType::StateEnum";
	EXPECT_EQ( ee8.getLoad().getDataType(), DataType::StateEnum ) << "ElementEntry::getLoad().getDataType() == DataType::StateEnum";
	EXPECT_EQ( ee8.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee8.getState().getStreamState(), OmmState::OpenEnum ) << "ElementEntry::getState().getStreamState()";
	EXPECT_EQ( ee8.getState().getDataState(), OmmState::OkEnum ) << "ElementEntry::getState().getDataState()";
	EXPECT_EQ( ee8.getState().getStatusCode(), OmmState::NoneEnum ) << "ElementEntry::getState().getStatusCode()";
	EXPECT_STREQ( ee8.getState().getStatusText(), "Succeeded" ) << "ElementEntry::getState().getStatusText()";
	EXPECT_STREQ( ee8.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "ElementEntry::getState().toString()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - ninth elementlist forth()";
	const ElementEntry& ee9 = el.getEntry();
	EXPECT_STREQ( ee9.getName(), "Element - AsciiString" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee9.getLoadType(), DataType::AsciiEnum ) << "ElementEntry::getLoadType() == DataType::AsciiEnum";
	EXPECT_EQ( ee9.getLoad().getDataType(), DataType::AsciiEnum ) << "ElementEntry::getLoad().getDataType() == DataType::AsciiEnum";
	EXPECT_EQ( ee9.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_STREQ( ee9.getAscii(), "ABCDEF" ) << "ElementEntry::getAscii()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - tenth elementlist forth()";
	const ElementEntry& ee10 = el.getEntry();
	EXPECT_STREQ( ee10.getName(), "Element - RmtesString" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee10.getLoadType(), DataType::RmtesEnum ) << "ElementEntry::getLoadType() == DataType::RmtesEnum";
	EXPECT_EQ( ee10.getLoad().getDataType(), DataType::RmtesEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RmtesEnum";
	EXPECT_EQ( ee10.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_TRUE( ee10.getRmtes().getAsUTF8() == EmaBuffer( "ABCDEF", 6 ) ) << "ElementEntry::getRmtes()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - eleventh elementlist forth()";
	const ElementEntry& ee11 = el.getEntry();
	EXPECT_STREQ( ee11.getName(), "Element - Enum" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee11.getLoadType(), DataType::EnumEnum ) << "ElementEntry::getLoadType() == DataType::EnumEnum";
	EXPECT_EQ( ee11.getLoad().getDataType(), DataType::EnumEnum ) << "ElementEntry::getLoad().getDataType() == DataType::EnumEnum";
	EXPECT_EQ( ee11.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee11.getEnum(), 29 ) << "ElementEntry::getEnum()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - twelfth elementlist forth()";
	const ElementEntry& ee12 = el.getEntry();
	EXPECT_STREQ( ee12.getName(), "Element - Float" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee12.getLoadType(), DataType::FloatEnum ) << "ElementEntry::getLoadType() == DataType::FloatEnum";
	EXPECT_EQ( ee12.getLoad().getDataType(), DataType::FloatEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FloatEnum";
	EXPECT_EQ( ee12.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee12.getFloat(), 11.11f ) << "ElementEntry::getFloat()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - thirteenth elementlist forth()";
	const ElementEntry& ee13 = el.getEntry();
	EXPECT_STREQ( ee13.getName(), "Element - Double" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee13.getLoadType(), DataType::DoubleEnum ) << "ElementEntry::getLoadType() == DataType::DoubleEnum";
	EXPECT_EQ( ee13.getLoad().getDataType(), DataType::DoubleEnum ) << "ElementEntry::getLoad().getDataType() == DataType::DoubleEnum";
	EXPECT_EQ( ee13.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_EQ( ee13.getDouble(), 22.22f ) << "ElementEntry::getDouble()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - fourteenth elementlist forth()";
	const ElementEntry& ee14 = el.getEntry();
	EXPECT_STREQ( ee14.getName(), "Element - RealBlank" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee14.getLoadType(), DataType::RealEnum ) << "ElementEntry::getLoadType() == DataType::RealEnum";
	EXPECT_EQ( ee14.getLoad().getDataType(), DataType::RealEnum ) << "ElementEntry::getLoad().getDataType() == DataType::RealEnum";
	EXPECT_EQ( ee14.getCode(), Data::BlankEnum ) << "ElementEntry::getCode() == Data::BlankEnum";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - fifteenth elementlist forth()";
	const ElementEntry& ee15 = el.getEntry();
	EXPECT_STREQ( ee15.getName(), "Element - Buffer" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee15.getLoadType(), DataType::BufferEnum ) << "ElementEntry::getLoadType() == DataType::BufferEnum";
	EXPECT_EQ( ee15.getLoad().getDataType(), DataType::BufferEnum ) << "ElementEntry::getLoad().getDataType() == DataType::BufferEnum";
	EXPECT_EQ( ee15.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_TRUE( ee15.getBuffer() == EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getBuffer()";

	EXPECT_TRUE( el.forth() ) << "Decode ElementList - sixteenth elementlist forth()";
	const ElementEntry& ee16 = el.getEntry();
	EXPECT_STREQ( ee16.getName(), "Element - Utf8String" ) << "ElementEntry::getName()";
	EXPECT_EQ( ee16.getLoadType(), DataType::Utf8Enum ) << "ElementEntry::getLoadType() == DataType::Utf8Enum";
	EXPECT_EQ( ee16.getLoad().getDataType(), DataType::Utf8Enum ) << "ElementEntry::getLoad().getDataType() == DataType::Utf8Enum";
	EXPECT_EQ( ee16.getCode(), Data::NoCodeEnum ) << "ElementEntry::getCode() == Data::NoCodeEnum";
	EXPECT_TRUE( ee16.getUtf8() == EmaBuffer( "ABCDEFGH", 8 ) ) << "ElementEntry::getUtf8()";

	EXPECT_TRUE( !el.forth() ) << "Decode ElementList - final elementlist forth()";
}

//corresponds to TestUtilities::EmaEncodeMapAll(Map& map)
// before calling this test, call SCOPED_TRACE("calling EmaDecodeMapAll")
void EmaDecodeMapAll( const Map& map )
{
        EXPECT_TRUE( map.hasKeyFieldId() ) << "Decode Map contains FieldList - hasKeyFieldId()";
	EXPECT_EQ( map.getKeyFieldId(), 3426 ) << "Decode Map contains FieldList - getKeyFieldId()";
	EXPECT_TRUE( map.hasTotalCountHint() ) << "Decode Map contains FieldList - hasTotalCountHint()";
	EXPECT_EQ( map.getTotalCountHint(), 5 ) << "Decode Map contains FieldList - getTotalCountHint()";

	switch ( map.getSummaryData().getDataType() )
	{
	case DataType::FieldListEnum :
	{
		const FieldList& fl = map.getSummaryData().getFieldList();
		EmaDecodeFieldListAll( fl );
	}
	break;
	default :
	        EXPECT_FALSE( true ) << "Decode Map Decode Summary FieldList - map.getSummaryType() not expected";
		break;
	}


	EXPECT_TRUE( map.forth() ) << "Map contains FieldList - first map forth()";

	const MapEntry& me1 = map.getEntry();

	EXPECT_EQ( me1.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum";
	{
		EmaBuffer Buf( "ABCD", 4 );
		EXPECT_TRUE( me1.getKey().getBuffer() == Buf ) << "MapEntry::getKey().getBuffer()";
	}
	EXPECT_EQ( me1.getAction(), MapEntry::DeleteEnum ) << "MapEntry::getAction() == MapEntry::DeleteEnum";
	EXPECT_EQ( me1.getLoad().getDataType(), DataType::NoDataEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum";


	EXPECT_TRUE( map.forth() ) << "Decode Map contains FieldList - second map forth()";

	const MapEntry& me2 = map.getEntry();

	EXPECT_EQ( me2.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum";
	{
		EmaBuffer Buf( "ABCD", 4 );
		EXPECT_TRUE( me2.getKey().getBuffer() == Buf ) << "MapEntry::getKey().getBuffer()";
	}
	EXPECT_EQ( me2.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum";
	EXPECT_EQ( me2.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum";
	{
		const FieldList& fl = me2.getFieldList();
		EmaDecodeFieldListAll( fl );
	}


	EXPECT_TRUE( map.forth() ) << "Map contains FieldList - third map forth()";

	const MapEntry& me3 = map.getEntry();

	EXPECT_EQ( me3.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum";
	{
		EmaBuffer Buf( "EFGHI", 5 );
		EXPECT_TRUE( me3.getKey().getBuffer() == Buf ) << "MapEntry::getKey().getBuffer()";
	}
	EXPECT_EQ( me3.getAction(), MapEntry::AddEnum ) << "MapEntry::getAction() == MapEntry::AddEnum";
	EXPECT_EQ( me3.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::NoDataEnum";
	{
		const FieldList& fl = me3.getFieldList();
		EmaDecodeFieldListAll( fl );
	}


	EXPECT_TRUE( map.forth() ) << "Map contains FieldList - fourth map forth()";

	const MapEntry& me4 = map.getEntry();

	EXPECT_EQ( me4.getKey().getDataType(), DataType::BufferEnum ) << "MapEntry::getKey().getDataType() == DataType::BufferEnum";
	{
		EmaBuffer Buf( "JKLMNOP", 7 );
		EXPECT_TRUE( me4.getKey().getBuffer() == Buf ) << "MapEntry::getKey().getBuffer()";
	}
	EXPECT_EQ( me4.getAction(), MapEntry::UpdateEnum ) << "MapEntry::getAction() == MapEntry::UpdateEnum";
	EXPECT_EQ( me4.getLoad().getDataType(), DataType::FieldListEnum ) << "MapEntry::getLoad().getDataType() == DataType::FieldListEnum";

	EXPECT_TRUE( !map.forth() ) << "Map contains FieldList - final map forth()";

	EXPECT_TRUE( true ) << "Map contains FieldList - exception not expected";
}

void encodeFieldList( RsslBuffer& rsslBuf, EmaString& inText )
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

void encodeNonRWFData( RsslBuffer* destRsslBuffer, RsslBuffer* srcRsslBuffer )
{
	RsslEncodeIterator iter;
	rsslClearEncodeIterator( &iter );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, destRsslBuffer );

	rsslEncodeNonRWFDataTypeInit( &iter, destRsslBuffer );

	memcpy( destRsslBuffer->data , srcRsslBuffer->data, srcRsslBuffer->length );
	destRsslBuffer->length = srcRsslBuffer->length;

	rsslEncodeNonRWFDataTypeComplete( &iter, destRsslBuffer, RSSL_TRUE );
}

void perfDecode( const ElementList& el )
{
	while ( el.forth() )
	{
		const ElementEntry& ee = el.getEntry();

		if ( ee.getCode() != Data::BlankEnum )
			switch ( ee.getLoadType() )
			{
			case DataType::IntEnum :
			{
				Int64 value = ee.getInt();
			}
			break;
			case DataType::UIntEnum :
			{
				UInt64 value = ee.getUInt();
			}
			break;
			case DataType::AsciiEnum :
			{
				const EmaString& text = ee.getAscii();
			}
			break;
			case DataType::BufferEnum :
			{
				const EmaBuffer& text = ee.getBuffer();
			}
			break;
			case DataType::RealEnum :
			{
				const OmmReal& r = ee.getReal();
				//OmmReal::MagnitudeType mt = r.getMagnitudeType();
				//Int64 m = r.getMantissa();
			}
			break;
			case DataType::DateEnum :
			{
				const OmmDate& d = ee.getDate();
				//UInt16 year = d.getYear();
				//UInt8 month = d.getMonth();
				//UInt8 day = d.getDay();
			}
			break;
			case DataType::DateTimeEnum :
			{
				const OmmDateTime& dt = ee.getDateTime();
				//UInt16 year = dt.getYear();
				//UInt8 month = dt.getMonth();
				//UInt8 day = dt.getDay();
				//UInt8 hour = dt.getHour();
				//UInt8 minute = dt.getMinute();
				//UInt8 second = dt.getSecond();
				//UInt16 millisecond = dt.getMillisecond();
			}
			break;
			case DataType::TimeEnum :
			{
				const OmmTime& dt = ee.getTime();
				//UInt8 hour = dt.getHour();
				//UInt8 minute = dt.getMinute();
				//UInt8 second = dt.getSecond();
				//UInt16 millisecond = dt.getMillisecond();
			}
			break;
			case DataType::FloatEnum :
			{
				float value = ee.getFloat();
			}
			break;
			case DataType::DoubleEnum :
			{
				double value = ee.getDouble();
			}
			break;
			case DataType::QosEnum :
			{
				const OmmQos& value = ee.getQos();
			}
			break;
			case DataType::StateEnum :
			{
				const OmmState& value = ee.getState();
			}
			break;
			case DataType::ArrayEnum :
			{
				const OmmArray& value = ee.getArray();
			}
			break;
			case DataType::FieldListEnum :
			{
				const FieldList& value = ee.getFieldList();
			}
			break;
			case DataType::ElementListEnum :
			{
				const ElementList& value = ee.getElementList();
			}
			break;
			case DataType::MapEnum :
			{
				const Map& value = ee.getMap();
			}
			break;
			case DataType::VectorEnum :
			{
				const Vector& value = ee.getVector();
			}
			break;
			case DataType::SeriesEnum :
			{
				const Series& value = ee.getSeries();
			}
			break;
			case DataType::FilterListEnum :
			{
				const FilterList& value = ee.getFilterList();
			}
			break;
			case DataType::RmtesEnum :
			{
				const RmtesBuffer& text = ee.getRmtes();
			}
			break;
			case DataType::Utf8Enum :
			{
				const EmaBuffer& text = ee.getUtf8();
			}
			break;
			case DataType::EnumEnum :
			{
				UInt16 value = ee.getEnum();
			}
			break;
			case DataType::OpaqueEnum :
			{
				const OmmOpaque& value = ee.getOpaque();
			}
			break;
			case DataType::XmlEnum :
			{
				const OmmXml& value = ee.getXml();
			}
			break;
			case DataType::AnsiPageEnum :
			{
				const OmmAnsiPage& value = ee.getAnsiPage();
			}
			break;
			case DataType::NoDataEnum :
			{

			} break;
			case DataType::ErrorEnum :
			{
				OmmError::ErrorCode error = ee.getError().getErrorCode();
			}
			break;
			default :
				break;
			}
	}
}

void perfDecode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		if ( fe.getCode() != Data::BlankEnum )
			switch ( fe.getLoadType() )
			{
			case DataType::IntEnum :
			{
				Int64 value = fe.getInt();
			}
			break;
			case DataType::UIntEnum :
			{
				UInt64 value = fe.getUInt();
			}
			break;
			case DataType::AsciiEnum :
			{
				const EmaString& text = fe.getAscii();
			}
			break;
			case DataType::BufferEnum :
			{
				const EmaBuffer& text = fe.getBuffer();
			}
			break;
			case DataType::RealEnum :
			{
				const OmmReal& r = fe.getReal();
				//OmmReal::MagnitudeType mt = r.getMagnitudeType();
				//Int64 m = r.getMantissa();
			}
			break;
			case DataType::DateEnum :
			{
				const OmmDate& d = fe.getDate();
				//UInt16 year = d.getYear();
				//UInt8 month = d.getMonth();
				//UInt8 day = d.getDay();
			}
			break;
			case DataType::DateTimeEnum :
			{
				const OmmDateTime& dt = fe.getDateTime();
				//UInt16 year = dt.getYear();
				//UInt8 month = dt.getMonth();
				//UInt8 day = dt.getDay();
				//UInt8 hour = dt.getHour();
				//UInt8 minute = dt.getMinute();
				//UInt8 second = dt.getSecond();
				//UInt16 millisecond = dt.getMillisecond();
			}
			break;
			case DataType::TimeEnum :
			{
				const OmmTime& dt = fe.getTime();
				//UInt8 hour = dt.getHour();
				//UInt8 minute = dt.getMinute();
				//UInt8 second = dt.getSecond();
				//UInt16 millisecond = dt.getMillisecond();
			}
			break;
			case DataType::FloatEnum :
			{
				float value = fe.getFloat();
			}
			break;
			case DataType::DoubleEnum :
			{
				double value = fe.getDouble();
			}
			break;
			case DataType::QosEnum :
			{
				const OmmQos& value = fe.getQos();
			}
			break;
			case DataType::StateEnum :
			{
				const OmmState& value = fe.getState();
			}
			break;
			case DataType::ArrayEnum :
			{
				const OmmArray& array = fe.getArray();
			}
			break;
			case DataType::FieldListEnum :
			{
				const FieldList& value = fe.getFieldList();
			}
			break;
			case DataType::ElementListEnum :
			{
				const ElementList& value = fe.getElementList();
			}
			break;
			case DataType::MapEnum :
			{
				const Map& value = fe.getMap();
			}
			break;
			case DataType::VectorEnum :
			{
				const Vector& value = fe.getVector();
			}
			break;
			case DataType::SeriesEnum :
			{
				const Series& value = fe.getSeries();
			}
			break;
			case DataType::FilterListEnum :
			{
				const FilterList& value = fe.getFilterList();
			}
			break;
			case DataType::OpaqueEnum :
			{
				const OmmOpaque& value = fe.getOpaque();
			}
			break;
			case DataType::XmlEnum :
			{
				const OmmXml& value = fe.getXml();
			}
			break;
			case DataType::AnsiPageEnum :
			{
				const OmmAnsiPage& value = fe.getAnsiPage();
			}
			break;
			case DataType::RmtesEnum :
			{
				const RmtesBuffer& text = fe.getRmtes();
			}
			break;
			case DataType::Utf8Enum :
			{
				const EmaBuffer& text = fe.getUtf8();
			}
			break;
			case DataType::EnumEnum :
			{
				UInt16 value = fe.getEnum();
			}
			break;
			case DataType::NoDataEnum :
			{

			} break;
			case DataType::ErrorEnum :
			{
				OmmError::ErrorCode error = fe.getError().getErrorCode();
			}
			break;
			default :
				break;
			}
	}
}

bool comparingData(RsslBuffer& rsslBuffer, const refinitiv::ema::access::EmaString& emaString)
{
	if ( ( rsslBuffer.length == emaString.length() ) && ( memcmp(rsslBuffer.data, emaString.c_str(), rsslBuffer.length) == 0 ) )
	{
		return true;
	}

	return false;
}

void prepareMsgToCopy(RsslEncodeIterator& encIter, RsslBuffer& msgBuf,
	RsslMsg* pRsslMsg, RsslDecodeIterator& decodeIter, RsslMsg* pRsslMsgDecode, Msg& respMsg,
	RsslDataDictionary const& dictionary
)
{
	RsslRet retval = 0;
	rsslClearEncodeIterator(&encIter);
	/* set version information of the connection on the encode iterator so proper versioning can be performed */
	rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	retval = rsslSetEncodeIteratorBuffer(&encIter, &msgBuf);
	ASSERT_EQ(RSSL_RET_SUCCESS, retval) << "rsslSetEncodeIteratorBuffer() failed with return code: " << retval << endl;

	retval = rsslEncodeMsg(&encIter, pRsslMsg);
	rsslClearDecodeIterator(&decodeIter);

	// Set the RWF version to decode with this iterator 
	rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	// Associates the RsslDecodeIterator with the RsslBuffer from which to decode.
	retval = rsslSetDecodeIteratorBuffer(&decodeIter, &msgBuf);
	ASSERT_EQ(RSSL_RET_SUCCESS, retval) << "rsslSetDecodeIteratorBuffer() failed with return code: " << retval << endl;

	// decode contents into the RsslMsg structure
	retval = rsslDecodeMsg(&decodeIter, pRsslMsgDecode);
	ASSERT_EQ(RSSL_RET_SUCCESS, retval) << "rsslDecodeMsg() failed with return code: " << retval << endl;
	StaticDecoder::setRsslData(&respMsg, pRsslMsgDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

	return;
}
