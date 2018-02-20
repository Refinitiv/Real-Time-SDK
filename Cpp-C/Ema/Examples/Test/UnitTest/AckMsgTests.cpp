/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

TEST(AckMsgTests, testAckMsgwithElementList)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ElementList eList;

		eList.addInt( EmaString( "Int" ), 1234 )
		.addAscii( EmaString( "Ascii" ), "Ascii" )
		.complete();

		AckMsg ackMsg;
		ackMsg.attrib( eList );
		ackMsg.payload( eList );

		StaticDecoder::setData( &ackMsg, &dictionary );


		EXPECT_TRUE( true ) << "ElementList as Payload of AckMsg - exception NOT expected" ;
	}
	catch ( const OmmException& )
	{

		EXPECT_FALSE( true ) << "ElementList as Payload of AckMsg - exception NOT expected" ;
	}
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

		free( rsslBuf.data );
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

