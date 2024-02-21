/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019, 2024 Refinitiv. All rights reserved.           --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

TEST(AckMsgTests, testAckMsgwithElementList)
{

	RsslDataDictionary dictionary;
	DataDictionary emaDataDictionary, emaDataDictionaryEmpty;

	const EmaString ackMsgString =
		"AckMsg\n"
		"    streamId=\"0\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    ackId=\"0\"\n"
		"    Attrib dataType=\"ElementList\"\n"
		"        ElementList\n"
		"            ElementEntry name=\"Int\" dataType=\"Int\" value=\"1234\"\n"
		"            ElementEntry name=\"Ascii\" dataType=\"Ascii\" value=\"Ascii\"\n"
		"        ElementListEnd\n"
		"    AttribEnd\n"
		"    Payload dataType=\"ElementList\"\n"
		"        ElementList\n"
		"            ElementEntry name=\"Int\" dataType=\"Int\" value=\"1234\"\n"
		"            ElementEntry name=\"Ascii\" dataType=\"Ascii\" value=\"Ascii\"\n"
		"        ElementListEnd\n"
		"    PayloadEnd\n"
		"AckMsgEnd\n";

	const EmaString ackMsgEmptyString =
		"AckMsg\n"
		"    streamId=\"0\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    ackId=\"0\"\n"
		"AckMsgEnd\n";

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary) ) << "Failed to load dictionary";
	try {
		emaDataDictionary.loadFieldDictionary( "RDMFieldDictionaryTest" );
		emaDataDictionary.loadEnumTypeDictionary( "enumtypeTest.def" );
	}
	catch ( const OmmException& ) {
		ASSERT_TRUE( false ) << "DataDictionary::loadFieldDictionary() failed to load dictionary information";
	}

	try
	{
		ElementList eList;

		eList.addInt( EmaString( "Int" ), 1234 )
		.addAscii( EmaString( "Ascii" ), "Ascii" )
		.complete();

		AckMsg ackMsg, ackMsgEmpty;
		ackMsg.attrib( eList );
		ackMsg.payload( eList );

		EXPECT_EQ( ackMsg.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "AckMsg.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( ackMsg.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "AckMsg.toString() == Dictionary is not loaded.";

		EXPECT_EQ( ackMsgEmpty.toString(emaDataDictionary), ackMsgEmptyString ) << "AckMsg.toString() == ackMsgEmptyString";

		EXPECT_EQ( ackMsg.toString( emaDataDictionary ), ackMsgString ) << "AckMsg.toString() == AckMsgString";

		StaticDecoder::setData(&ackMsg, &dictionary);

		AckMsg ackClone( ackMsg );
		ackClone.clear();
		EXPECT_EQ( ackClone.toString( emaDataDictionary ), ackMsgEmptyString ) << "AckMsg.toString() == ackMsgEmptyString";

		EXPECT_EQ( ackMsg.toString(), ackMsgString ) << "AckMsg.toString() == AckMsgString";
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ElementList as Payload of AckMsg - exception NOT expected" ;
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(AckMsgTests, testAckMsgwithRefreshMsg)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ElementList eList;

		eList.addInt( EmaString( "Int" ), 1234 )
		.addAscii( EmaString( "Ascii" ), "Ascii" )
		.complete();

		RefreshMsg respMsg;

		respMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		respMsg.attrib( eList );
		respMsg.payload( eList );

		StaticDecoder::setData( &respMsg, &dictionary );

		AckMsg ackMsg;
		ackMsg.payload( respMsg );

		StaticDecoder::setData( &ackMsg, &dictionary );


		EXPECT_TRUE( true ) << "RefreshMsg as Payload of AckMsg - exception NOT expected" ;
	}
	catch ( const OmmException& )
	{

		EXPECT_FALSE( true ) << "RefreshMsg as Payload of AckMsg - exception NOT expected" ;
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(AckMsgTests, testAckMsgWithOpaque)
{

	try
	{
		RsslAckMsg rsslAckMsg;
		rsslClearAckMsg( &rsslAckMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;

		rsslAckMsg.ackId = 1;
		rsslAckMsgApplyHasMsgKey( &rsslAckMsg );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>(strlen( opaqueValue.data ));

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		rsslAckMsg.msgBase.encDataBody = rsslBuf;
		rsslAckMsg.msgBase.containerType = RSSL_DT_OPAQUE;

		AckMsg ackMsg;

		StaticDecoder::setRsslData( &ackMsg, ( RsslMsg* )&rsslAckMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( ackMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "ackMsg.getPayload().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( ackMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "ackMsg.getPayload().getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ackMsg Decode with Opaque payload - exception not expected" ;
	}

}

TEST(AckMsgTests, testAckMsgWithXml)
{

	try
	{
		RsslAckMsg rsslAckMsg;
		rsslClearAckMsg( &rsslAckMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;

		rsslAckMsg.ackId = 1;
		rsslAckMsgApplyHasMsgKey( &rsslAckMsg );

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>(strlen( xmlValue.data ));

		encodeNonRWFData( &rsslBuf, &xmlValue );

		rsslAckMsg.msgBase.encDataBody = rsslBuf;
		rsslAckMsg.msgBase.containerType = RSSL_DT_XML;

		AckMsg ackMsg;

		StaticDecoder::setRsslData( &ackMsg, ( RsslMsg* )&rsslAckMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( ackMsg.getPayload().getDataType(), DataType::XmlEnum ) << "ackMsg.getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( ackMsg.getPayload().getXml().getBuffer(), compareTo ) << "ackMsg.getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ackMsg Decode with Xml payload - exception not expected" ;
	}
}

TEST(AckMsgTests, testAckMsgWithAnsiPage)
{

	try
	{
		RsslAckMsg rsslAckMsg;
		rsslClearAckMsg( &rsslAckMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;

		rsslAckMsg.ackId = 1;
		rsslAckMsgApplyHasMsgKey( &rsslAckMsg );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>(strlen( ansiPageValue.data ));

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		rsslAckMsg.msgBase.encDataBody = rsslBuf;
		rsslAckMsg.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		AckMsg ackMsg;

		StaticDecoder::setRsslData( &ackMsg, ( RsslMsg* )&rsslAckMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( ackMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "ackMsg.getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( ackMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "ackMsg.getPayload().getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "ackMsg Decode with Xml payload - exception not expected" ;
	}
}

// encoding by EMA and decoding by EMA
TEST(AckMsgTests, testAckMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		AckMsg ackMsg;
		ackMsg.ackId( 99 );
		ackMsg.nackCode( AckMsg::AccessDeniedEnum );
		EmaString nameG( "ACKMSG" );
		EmaBuffer headerG( "headerG", 7 );
		ackMsg.streamId( 3 );
		ackMsg.name( nameG );
		ackMsg.nameType( 1 );
		ackMsg.serviceId( 2 );
		ackMsg.id( 4 );
		ackMsg.filter( 8 );
		ackMsg.extendedHeader( headerG );
		ackMsg.attrib( flEnc );
		ackMsg.payload( flEnc );


		//Now do EMA decoding of AckMsg
		StaticDecoder::setData( &ackMsg, &dictionary );

		EXPECT_EQ( ackMsg.getAckId(), 99 ) << "AckMsg::getAckId()" ;
		EXPECT_TRUE( ackMsg.hasNackCode() ) << "AckMsg::hasNackCode() == true" ;
		EXPECT_EQ( ackMsg.getNackCode(), AckMsg::AccessDeniedEnum ) << "AckMsg::getNackCode()" ;

		EXPECT_TRUE( ackMsg.hasMsgKey() ) << "AckMsg::hasMsgKey() == true" ;

		EXPECT_EQ( ackMsg.getStreamId(), 3 ) << "AckMsg::getStreamId()" ;

		EXPECT_EQ( ackMsg.getDomainType(), MMT_MARKET_PRICE ) << "AckMsg::getDomainType()" ;

		EXPECT_TRUE( ackMsg.hasName() ) << "AckMsg::hasName() == true" ;
		EXPECT_STREQ( ackMsg.getName(), "ACKMSG" ) << "AckMsg::getName()" ;

		EXPECT_TRUE( ackMsg.hasNameType() ) << "AckMsg::hasNameType() == true" ;
		EXPECT_EQ( ackMsg.getNameType(), 1 ) << "AckMsg::getNameType()" ;

		EXPECT_TRUE( ackMsg.hasServiceId() ) << "AckMsg::hasServiceId() == true" ;
		EXPECT_EQ( ackMsg.getServiceId(), 2 ) << "AckMsg::getServiceId()" ;

		EXPECT_TRUE( ackMsg.hasId() ) << "AckMsg::hasId() == true" ;
		EXPECT_EQ( ackMsg.getId(), 4 ) << "AckMsg::getId()" ;

		EXPECT_TRUE( ackMsg.hasFilter() ) << "AckMsg::hasFilter() == true" ;
		EXPECT_EQ( ackMsg.getFilter(), 8 ) << "AckMsg::getFilter()" ;

		EXPECT_TRUE( ackMsg.hasExtendedHeader() ) << "AckMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( ackMsg.getExtendedHeader(), headerG ) << "AckMsg::getExtendedHeader()" ;

		EXPECT_EQ( ackMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "AckMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;

		//get FieldList that is in the attrib of ackMsg
		const FieldList& flAttrib = ackMsg.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( ackMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "AckMsg::getLoad().getDataType() == DataType::FieldListEnum" ;

		//get FieldList that is in the payload of ackMsg
		const FieldList& flPayload = ackMsg.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}


		EXPECT_TRUE( true ) << "AckMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "AckMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

// encoding by EMA and decoding by EMA
TEST(AckMsgTests, testAckMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		AckMsg ackMsg;
		ackMsg.ackId( 99 );
		ackMsg.nackCode( AckMsg::AccessDeniedEnum );
		EmaString nameG( "ACKMSG" );
		EmaBuffer headerG( "headerG", 7 );
		ackMsg.streamId( 3 );
		ackMsg.name( nameG );
		ackMsg.nameType( 1 );
		ackMsg.serviceId( 2 );
		ackMsg.id( 4 );
		ackMsg.filter( 8 );
		ackMsg.extendedHeader( headerG );
		ackMsg.attrib( elEnc );
		ackMsg.payload( elEnc );


		//Now do EMA decoding of AckMsg
		StaticDecoder::setData( &ackMsg, &dictionary );

		EXPECT_EQ( ackMsg.getAckId(), 99 ) << "AckMsg::getAckId()" ;
		EXPECT_TRUE( ackMsg.hasNackCode() ) << "AckMsg::hasNackCode() == true" ;
		EXPECT_EQ( ackMsg.getNackCode(), AckMsg::AccessDeniedEnum ) << "AckMsg::getNackCode()" ;

		EXPECT_TRUE( ackMsg.hasMsgKey() ) << "AckMsg::hasMsgKey() == true" ;

		EXPECT_EQ( ackMsg.getStreamId(), 3 ) << "AckMsg::getStreamId()" ;

		EXPECT_EQ( ackMsg.getDomainType(), MMT_MARKET_PRICE ) << "AckMsg::getDomainType()" ;

		EXPECT_TRUE( ackMsg.hasName() ) << "AckMsg::hasName() == true" ;
		EXPECT_STREQ( ackMsg.getName(), "ACKMSG" ) << "AckMsg::getName()" ;

		EXPECT_TRUE( ackMsg.hasNameType() ) << "AckMsg::hasNameType() == true" ;
		EXPECT_EQ( ackMsg.getNameType(), 1 ) << "AckMsg::getNameType()" ;

		EXPECT_TRUE( ackMsg.hasServiceId() ) << "AckMsg::hasServiceId() == true" ;
		EXPECT_EQ( ackMsg.getServiceId(), 2 ) << "AckMsg::getServiceId()" ;

		EXPECT_TRUE( ackMsg.hasId() ) << "AckMsg::hasId() == true" ;
		EXPECT_EQ( ackMsg.getId(), 4 ) << "AckMsg::getId()" ;

		EXPECT_TRUE( ackMsg.hasFilter() ) << "AckMsg::hasFilter() == true" ;
		EXPECT_EQ( ackMsg.getFilter(), 8 ) << "AckMsg::getFilter()" ;

		EXPECT_TRUE( ackMsg.hasExtendedHeader() ) << "AckMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( ackMsg.getExtendedHeader(), headerG ) << "AckMsg::getExtendedHeader()" ;

		EXPECT_EQ( ackMsg.getAttrib().getDataType(), DataType::ElementListEnum ) << "AckMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;

		//get ElementList that is in the attrib of ackMsg
		const ElementList& elAttrib = ackMsg.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		EXPECT_EQ( ackMsg.getPayload().getDataType(), DataType::ElementListEnum ) << "AckMsg::getLoad().getDataType() == DataType::ElementListEnum" ;

		//get ElementList that is in the payload of ackMsg
		const ElementList& elPayload = ackMsg.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}


		EXPECT_TRUE( true ) << "AckMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "AckMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(AckMsgTests, testAckMsgHybrid)
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
		FieldList fl;
		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		// set field list as attrib and payload of AckMsg
		AckMsg ackMsg;

		ackMsg.attrib( fl );
		ackMsg.payload( fl );

		StaticDecoder::setData( &ackMsg, &dictionary );


		EXPECT_EQ( ackMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "AckMsg::getattrib()::getDataType()" ;

		EXPECT_EQ( ackMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "AckMsg::getPayload()::getDataType()" ;

		EXPECT_TRUE( true ) << "AckMsg Hybrid Usage - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "AckMsg Hybrid Usage - exception not expected" ;
	}
}

TEST(AckMsgTests, testAckMsgError)
{

	{
		try
		{
			AckMsg msg;

			ElementList attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "AckMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "AckMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			AckMsg msg;

			RefreshMsg attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "AckMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "AckMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

	{
		try
		{
			AckMsg msg;

			ElementList load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "AckMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "AckMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			AckMsg msg;

			RefreshMsg load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "AckMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "AckMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

}

TEST(AckMsgTests, testAckMsgtoString)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslAckMsg rsslAckMsg;
		rsslClearAckMsg(&rsslAckMsg);

		RsslMsgKey msgKey;
		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;

		rsslAckMsg.ackId = 1;
		rsslAckMsgApplyHasMsgKey(&rsslAckMsg);

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		rsslAckMsg.msgBase.encDataBody = rsslBuf;
		rsslAckMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

		AckMsg ackMsg;

		StaticDecoder::setRsslData(&ackMsg, (RsslMsg*)&rsslAckMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		EXPECT_TRUE(true) << "AckMsg toString Decode - exception not expected";
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "AckMsg toString Decode - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(AckMsgTests, testAckMsgClone)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslAckMsg rsslAckMsg;
		rsslClearAckMsg(&rsslAckMsg);

		RsslMsgKey msgKey;
		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		rsslAckMsg.ackId = 1;

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = (char*)malloc(sizeof(char) * 1000);

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		rsslAckMsg.msgBase.encDataBody = rsslBuf;
		rsslAckMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		rsslAckMsg.msgBase.msgKey = msgKey;
		rsslAckMsgApplyHasMsgKey(&rsslAckMsg);

		RsslEncodeIterator encIter;

		rsslClearEncodeIterator(&encIter);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		int retval = 0;

		RsslBuffer msgBuf;
		msgBuf.length = 2048;
		msgBuf.data = (char*)malloc(sizeof(char) * 2048);

		/* set the buffer on an RsslEncodeIterator */
		if ((retval = rsslSetEncodeIteratorBuffer(&encIter, &msgBuf)) < RSSL_RET_SUCCESS)
		{
			//rsslReleaseBuffer(msgBuf, &error);
			EXPECT_FALSE(true) << "rsslSetEncodeIteratorBuffer() failed with return code: " << retval << endl;
		}

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&rsslAckMsg);

		RsslMsg ackDecode;
		RsslDecodeIterator decodeIter;

		rsslClearDecodeIterator(&decodeIter);

		// Set the RWF version to decode with this iterator 
		rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		// Associates the RsslDecodeIterator with the RsslBuffer from which to decode.
		if ((retval = rsslSetDecodeIteratorBuffer(&decodeIter, &msgBuf)) != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslSetDecodeIteratorBuffer() failed with return code: " << retval << endl;
		}

		// decode contents into the RsslMsg structure
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&ackDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		AckMsg ackMsg;

		StaticDecoder::setRsslData(&ackMsg, (RsslMsg*)&ackDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message

		AckMsg cloneAckMsg(ackMsg);

		EXPECT_TRUE(cloneAckMsg.getDomainType() == ackMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneAckMsg.getStreamId() == ackMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneAckMsg.hasMsgKey() == ackMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(cloneAckMsg.toString(), ackMsg.toString()) << "Check equal toString()";

		EXPECT_TRUE(true) << "AckMsg Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

		
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "AckMsg Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(AckMsgTests, testAckMsgEditClone)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslAckMsg rsslAckMsg;
		rsslClearAckMsg(&rsslAckMsg);

		RsslMsgKey msgKey;
		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		rsslAckMsg.ackId = 1;

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = (char*)malloc(sizeof(char) * 1000);
		EmaString inText;

		encodeFieldList(rsslBuf, inText);

		rsslAckMsg.msgBase.encDataBody = rsslBuf;
		rsslAckMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		rsslAckMsg.msgBase.msgKey = msgKey;
		rsslAckMsgApplyHasMsgKey(&rsslAckMsg);

		RsslEncodeIterator encIter;

		rsslClearEncodeIterator(&encIter);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		int retval = 0;

		RsslBuffer msgBuf;
		msgBuf.length = 2048;
		msgBuf.data = (char*)malloc(sizeof(char) * 2048);

		/* set the buffer on an RsslEncodeIterator */
		if ((retval = rsslSetEncodeIteratorBuffer(&encIter, &msgBuf)) < RSSL_RET_SUCCESS)
		{
			//rsslReleaseBuffer(msgBuf, &error);
			EXPECT_FALSE(true) << "rsslSetEncodeIteratorBuffer() failed with return code: " << retval << endl;
		}

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&rsslAckMsg);

		RsslMsg ackDecode;
		RsslDecodeIterator decodeIter;

		rsslClearDecodeIterator(&decodeIter);

		// Set the RWF version to decode with this iterator 
		rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		// Associates the RsslDecodeIterator with the RsslBuffer from which to decode.
		if ((retval = rsslSetDecodeIteratorBuffer(&decodeIter, &msgBuf)) != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslSetDecodeIteratorBuffer() failed with return code: " << retval << endl;
		}

		// decode contents into the RsslMsg structure
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&ackDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		AckMsg ackMsg;

		StaticDecoder::setRsslData(&ackMsg, (RsslMsg*)&ackDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message

		AckMsg cloneAckMsg(ackMsg);

		EXPECT_TRUE(cloneAckMsg.getDomainType() == ackMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneAckMsg.getStreamId() == ackMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneAckMsg.hasMsgKey() == ackMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(cloneAckMsg.toString(), ackMsg.toString()) << "Check equal toString()";

		// Edit message
		cloneAckMsg.streamId(10);

		StaticDecoder::setData(&cloneAckMsg, &dictionary);

		EXPECT_FALSE(cloneAckMsg.getStreamId() == ackMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(ackMsg.toString(), cloneAckMsg.toString()) << "Check not equal toString()";
		EXPECT_TRUE(true) << "AckMsg Edit Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "AckMsg Edit Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(AckMsgTests, testAckMsgCloneMsgKey)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	RsslUInt32 seqNum;

	RsslUInt16 flagsTest[] = {
		RSSL_AKMF_NONE,
		RSSL_AKMF_HAS_MSG_KEY,
		RSSL_AKMF_HAS_SEQ_NUM,
		RSSL_AKMF_HAS_NAK_CODE,
		RSSL_AKMF_HAS_MSG_KEY | RSSL_AKMF_HAS_SEQ_NUM | RSSL_AKMF_HAS_NAK_CODE,
	};
	const size_t nFlags = sizeof(flagsTest) / sizeof(RsslUInt16);
	RsslUInt16 flag;

	char rsslBufferData[1000];
	char msgBufData[2048];

	RsslEncodeIterator encIter;
	RsslDecodeIterator decodeIter;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	for (RsslUInt32 i = 0; i < nFlags; ++i)
	{
		flag = flagsTest[i];
		try
		{
			RsslAckMsg rsslAckMsg;
			rsslClearAckMsg(&rsslAckMsg);

			RsslBuffer rsslBuf = RSSL_INIT_BUFFER;

			RsslMsgKey msgKey;
			rsslClearMsgKey(&msgKey);

			rsslAckMsg.msgBase.streamId = 2;
			rsslAckMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
			rsslAckMsg.ackId = i + 123;

			rsslAckMsg.flags = flag;

			if (flag & RSSL_AKMF_HAS_MSG_KEY)
			{
				RsslBuffer nameBuffer;
				nameBuffer.data = const_cast<char*>("ABCDEF");
				nameBuffer.length = 6;

				msgKey.name = nameBuffer;
				rsslMsgKeyApplyHasName(&msgKey);

				rsslBuf.length = sizeof(rsslBufferData) / sizeof(char);
				rsslBuf.data = rsslBufferData;

				EmaString inText;
				encodeFieldList(rsslBuf, inText);

				rsslAckMsg.msgBase.encDataBody = rsslBuf;
				rsslAckMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

				msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
				msgKey.encAttrib = rsslBuf;
				rsslMsgKeyApplyHasAttrib(&msgKey);

				rsslAckMsg.msgBase.msgKey = msgKey;
				rsslAckMsgApplyHasMsgKey(&rsslAckMsg);
			}
			else {
				rsslAckMsg.msgBase.encDataBody.data = 0;
				rsslAckMsg.msgBase.encDataBody.length = 0;
				rsslAckMsg.msgBase.containerType = RSSL_DT_NO_DATA;
			}

			/* Add Item Sequence Number */
			if (flag & RSSL_AKMF_HAS_SEQ_NUM)
			{
				seqNum = i;
				rsslAckMsg.seqNum = seqNum;
			}

			/* Add nakCode */
			if (flag & RSSL_AKMF_HAS_NAK_CODE)
			{
				rsslAckMsg.nakCode = 112;
			}

			RsslBuffer msgBuf;
			msgBuf.length = sizeof(msgBufData) / sizeof(char);
			msgBuf.data = msgBufData;

			RsslMsg ackDecode;
			AckMsg ackMsg;

			prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&rsslAckMsg, decodeIter, (RsslMsg*)&ackDecode, ackMsg, dictionary);

			// Clone message

			AckMsg cloneAckMsg(ackMsg);

			EXPECT_EQ(cloneAckMsg.getDomainType(), ackMsg.getDomainType()) << "Compare domainType";
			EXPECT_EQ(cloneAckMsg.getDomainType(), RSSL_DMT_TRANSACTION) << "Compare domainType: should be equal to " << RSSL_DMT_TRANSACTION;

			EXPECT_EQ(cloneAckMsg.getStreamId(), ackMsg.getStreamId()) << "Compare streamId";
			EXPECT_EQ(cloneAckMsg.getStreamId(), 2) << "Compare streamId: should be equal to 2";

			EXPECT_EQ(cloneAckMsg.hasMsgKey(), ackMsg.hasMsgKey()) << "Compare hasMsgKey";
			EXPECT_EQ(cloneAckMsg.hasName(), ackMsg.hasName()) << "Compare hasName";
			EXPECT_EQ(cloneAckMsg.hasNameType(), ackMsg.hasNameType()) << "Compare hasNameType";
			EXPECT_EQ(cloneAckMsg.hasServiceId(), ackMsg.hasServiceId()) << "Compare hasServiceId";
			EXPECT_EQ(cloneAckMsg.hasId(), ackMsg.hasId()) << "Compare hasId";
			EXPECT_EQ(cloneAckMsg.hasFilter(), ackMsg.hasFilter()) << "Compare hasFilter";
			EXPECT_EQ(cloneAckMsg.hasExtendedHeader(), ackMsg.hasExtendedHeader()) << "Compare hasExtendedHeader";

			EXPECT_EQ(cloneAckMsg.hasSeqNum(), ackMsg.hasSeqNum()) << "Compare hasSeqNum";
			EXPECT_EQ(cloneAckMsg.hasNackCode(), ackMsg.hasNackCode()) << "Compare hasNackCode";
			EXPECT_EQ(cloneAckMsg.hasText(), ackMsg.hasText()) << "Compare hasText";
			EXPECT_EQ(cloneAckMsg.hasServiceName(), ackMsg.hasServiceName()) << "Compare hasServiceName";

			EXPECT_STREQ(cloneAckMsg.toString(), ackMsg.toString()) << "Check equal toString()";

			EXPECT_EQ(cloneAckMsg.hasMsgKey(), (flag & RSSL_AKMF_HAS_MSG_KEY) > 0) << "Compare hasMsgKey: " << (flag & RSSL_AKMF_HAS_MSG_KEY);
			if (cloneAckMsg.hasMsgKey() && ackMsg.hasMsgKey())
			{
				if (cloneAckMsg.hasServiceId())
				{
					EXPECT_EQ(cloneAckMsg.getServiceId(), ackMsg.getServiceId()) << "Compare serviceId";
				}
				if (cloneAckMsg.hasName())
				{
					EXPECT_EQ(cloneAckMsg.getName(), ackMsg.getName()) << "Compare name";
				}
				if (cloneAckMsg.hasNameType())
				{
					EXPECT_EQ(cloneAckMsg.getNameType(), ackMsg.getNameType()) << "Compare nameType";
				}
				if (cloneAckMsg.hasId())
				{
					EXPECT_EQ(cloneAckMsg.getId(), ackMsg.getId()) << "Compare id";
				}
				if (cloneAckMsg.hasFilter())
				{
					EXPECT_EQ(cloneAckMsg.getFilter(), ackMsg.getFilter()) << "Compare filter";
				}
			}

			EXPECT_EQ(cloneAckMsg.hasSeqNum(), (flag & RSSL_AKMF_HAS_SEQ_NUM) > 0) << "Compare hasSeqNum: " << (flag & RSSL_AKMF_HAS_SEQ_NUM);
			if (cloneAckMsg.hasSeqNum() && ackMsg.hasSeqNum())
			{
				EXPECT_EQ(cloneAckMsg.getSeqNum(), ackMsg.getSeqNum()) << "Compare SeqNum";
				EXPECT_EQ(seqNum, cloneAckMsg.getSeqNum()) << "Compare SeqNum: " << seqNum;
			}

			EXPECT_EQ(cloneAckMsg.hasNackCode(), (flag & RSSL_AKMF_HAS_NAK_CODE) > 0) << "Compare hasNackCode: " << (flag & RSSL_AKMF_HAS_NAK_CODE);
			if (cloneAckMsg.hasNackCode() && ackMsg.hasNackCode())
			{
				EXPECT_EQ(cloneAckMsg.getNackCode(), ackMsg.getNackCode()) << "Compare getNackCode";
				EXPECT_EQ(112, cloneAckMsg.getNackCode()) << "Compare getNackCode: " << 112;
			}

			EXPECT_TRUE(true) << "AckMsg Clone Success";
		}
		catch (const OmmException&)
		{
			EXPECT_FALSE(true) << "AckMsg Clone - exception not expected";
		}
	}

	rsslDeleteDataDictionary(&dictionary);
}
