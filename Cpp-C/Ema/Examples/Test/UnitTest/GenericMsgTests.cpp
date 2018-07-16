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

TEST(GenericMsgTests, testGenericMsgwithRefreshMsg)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ElementList eList;

		eList.addInt( EmaString( "Int" ), 1234 )
		.addAscii( EmaString( "Ascii" ), "Ascii" )
		.complete();

		RefreshMsg refreshMsg;

		refreshMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		refreshMsg.attrib( eList );
		refreshMsg.payload( eList );

		StaticDecoder::setData( &refreshMsg, &dictionary );

		GenericMsg genMsg;
		genMsg.payload( refreshMsg );
		EXPECT_EQ( genMsg.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "GenericMsg.toString() == Decoding of just encoded object in the same application is not supported";

		StaticDecoder::setData( &genMsg, &dictionary );
		EXPECT_NE( genMsg.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "GenericMsg.toString() != Decoding of just encoded object in the same application is not supported";

		EXPECT_TRUE( true ) << "RefreshMsg as Payload of GenericMsg - exception NOT expected" ;
	}
	catch ( const OmmException& )
	{

		EXPECT_FALSE( true ) << "RefreshMsg as Payload of GenericMsg - exception NOT expected" ;
	}
}

TEST(GenericMsgTests, testGenericMsgWithOpaque)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_OPAQUE;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "GenericMsg::getPayload().getDataType() == DataType::OpaqueEnum" ;
	
		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "GenericMsg::getPayload().getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with Opaque payload - exception not expected" ;
	}
}

TEST(GenericMsgTests, testGenericMsgWithXml)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_XML;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::XmlEnum ) << "GenericMsg::getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getXml().getBuffer(), compareTo ) << "GenericMsg::getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with Xml payload - exception not expected" ;
	}
}

TEST(GenericMsgTests, testGenericMsgWithAnsiPage)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "GenericMsg::getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "GenericMsg::getPayload().getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with AnsiPage payload - exception not expected" ;
	}
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( flEnc );
		genMsg.payload( flEnc );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;


		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;

		//get FieldList that is in the attrib of genMsg
		const FieldList& flAttrib = genMsg.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getLoad().getDataType() == DataType::FieldListEnum" ;

		//get FieldList that is in the payload of genMsg
		const FieldList& flPayload = genMsg.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}


		EXPECT_TRUE( true ) << "GenericMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( elEnc );
		genMsg.payload( elEnc );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ElementListEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;

		//get ElementList that is in the attrib of genMsg
		const ElementList& elAttrib = genMsg.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ElementListEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ElementListEnum" ;

		//get ElementList that is in the payload of genMsg
		const ElementList& elPayload = genMsg.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}


		EXPECT_TRUE( true ) << "GenericMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( mapEnc );
		genMsg.payload( mapEnc );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::MapEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::MapEnum" ;

		//get Map that is in the attrib of genMsg
		const Map& mapAttrib = genMsg.getAttrib().getMap();
		//decode mapAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::MapEnum ) << "GenericMsg::getLoad().getDataType() == DataType::MapEnum" ;

		//get Map that is in the payload of genMsg
		const Map& mapPayload = genMsg.getPayload().getMap();
		//decode mapPayload
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}


		EXPECT_TRUE( true ) << "GenericMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgReqMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( flEnc );
		requestMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the attrib of genMsg
		const ReqMsg& reqMsgInAttrib = genMsg.getAttrib().getReqMsg();
		//get attrib FieldList from reqMsgInAttrib
		const FieldList& flAttrib = reqMsgInAttrib.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		//get payload FieldList from reqMsgInAttrib
		const FieldList& flPayload = reqMsgInAttrib.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the payload of genMsg
		const ReqMsg& reqMsgInPayload = genMsg.getPayload().getReqMsg();
		//get attrib FieldList from reqMsgInPayload
		const FieldList& flAttrib2 = reqMsgInPayload.getAttrib().getFieldList();
		//decode flAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib2 );
		}

		//get payload FieldList from reqMsgInPayload
		const FieldList& flPayload2 = reqMsgInPayload.getPayload().getFieldList();
		//decode flPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(GenericMsgTests, testGenericMsgReqMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( elEnc );
		requestMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;


		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the attrib of genMsg
		const ReqMsg& reqMsgInAttrib = genMsg.getAttrib().getReqMsg();
		//get attrib ElementList from reqMsgInAttrib
		const ElementList& elAttrib = reqMsgInAttrib.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		//get payload ElementList from reqMsgInAttrib
		const ElementList& elPayload = reqMsgInAttrib.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the payload of genMsg
		const ReqMsg& reqMsgInPayload = genMsg.getPayload().getReqMsg();
		//get attrib ElementList from reqMsgInPayload
		const ElementList& elAttrib2 = reqMsgInPayload.getAttrib().getElementList();
		//decode elAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib2 );
		}

		//get payload ElementList from reqMsgInPayload
		const ElementList& elPayload2 = reqMsgInPayload.getPayload().getElementList();
		//decode elPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgReqMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		Map mapEnc;
		EmaEncodeMapAll( mapEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( mapEnc );
		requestMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the attrib of genMsg
		const ReqMsg& reqMsgInAttrib = genMsg.getAttrib().getReqMsg();
		//get attrib Map from reqMsgInAttrib
		const Map& mapAttrib = reqMsgInAttrib.getAttrib().getMap();
		//decode mapAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		//get payload Map from reqMsgInAttrib
		const Map& mapPayload = reqMsgInAttrib.getPayload().getMap();
		//decode mapPayload
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the payload of genMsg
		const ReqMsg& reqMsgInPayload = genMsg.getPayload().getReqMsg();
		//get attrib Map from reqMsgInPayload
		const Map& mapAttrib2 = reqMsgInPayload.getAttrib().getMap();
		//decode mapAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib2 );
		}

		//get payload Map from reqMsgInPayload
		const Map& mapPayload2 = reqMsgInPayload.getPayload().getMap();
		//decode mapPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( flEnc );
		responseMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the attrib of genMsg
		const RefreshMsg& respMsgInAttrib = genMsg.getAttrib().getRefreshMsg();
		//get attrib FieldList from respMsgInAttrib
		const FieldList& flAttrib = respMsgInAttrib.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		//get payload FieldList from respMsgInAttrib
		const FieldList& flPayload = respMsgInAttrib.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getLoad().getDataType() == Data::RespMsgEnum" ;

		//get RespMsg that is in the payload of genMsg
		const RefreshMsg& respMsgInPayload = genMsg.getPayload().getRefreshMsg();
		//get attrib FieldList from respMsgInPayload
		const FieldList& flAttrib2 = respMsgInPayload.getAttrib().getFieldList();
		//decode flAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib2 );
		}

		//get payload FieldList from respMsgInPayload
		const FieldList& flPayload2 = respMsgInPayload.getPayload().getFieldList();
		//decode flPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg RespMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RespMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( elEnc );
		responseMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the attrib of genMsg
		const RefreshMsg& respMsgInAttrib = genMsg.getAttrib().getRefreshMsg();
		//get attrib ElementList from respMsgInAttrib
		const ElementList& elAttrib = respMsgInAttrib.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		//get payload ElementList from respMsgInAttrib
		const ElementList& elPayload = respMsgInAttrib.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getLoad().getDataType() == Data::RespMsgEnum" ;

		//get RespMsg that is in the payload of genMsg
		const RefreshMsg& respMsgInPayload = genMsg.getPayload().getRefreshMsg();
		//get attrib ElementList from respMsgInPayload
		const ElementList& elAttrib2 = respMsgInPayload.getAttrib().getElementList();
		//decode elAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib2 );
		}

		//get payload ElementList from respMsgInPayload
		const ElementList& elPayload2 = respMsgInPayload.getPayload().getElementList();
		//decode elPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg RespMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RespMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( mapEnc );
		responseMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the attrib of genMsg
		const RefreshMsg& respMsgInAttrib = genMsg.getAttrib().getRefreshMsg();
		//get attrib Map from respMsgInAttrib
		const Map& mapAttrib = respMsgInAttrib.getAttrib().getMap();
		//decode mapAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		//get payload Map from respMsgInAttrib
		const Map& mapPayload = respMsgInAttrib.getPayload().getMap();
		//decode mapPayload
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the payload of genMsg
		const RefreshMsg& respMsgInPayload = genMsg.getPayload().getRefreshMsg();
		//get attrib Map from respMsgInPayload
		const Map& mapAttrib2 = respMsgInPayload.getAttrib().getMap();
		//decode mapAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib2 );
		}

		//get payload Map from respMsgInPayload
		const Map& mapPayload2 = respMsgInPayload.getPayload().getMap();
		//decode mapPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg RespMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RespMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgReqMsgFieldListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( flEnc );
		requestMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg FieldList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg FieldList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgReqMsgElementListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;


		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( elEnc );
		requestMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg ElementList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg ElementList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgReqMsgMapDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;


		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( mapEnc );
		requestMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg Map Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg Map Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgFieldListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( flEnc );
		responseMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg RefreshMsg FieldList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RefreshMsg FieldList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgRefreshMsgElementListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( elEnc );
		responseMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg RefreshMsg ElementList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RefreshMsg ElementList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgRefreshMsgMapDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( mapEnc );
		responseMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg RefreshMsg Map Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RefreshMsg Map Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgHybrid)
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

		// set field list as attrib and payload of GenericMsg
		GenericMsg genMsg;

		genMsg.attrib( fl );
		genMsg.payload( fl );

		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getattrib()::getDataType()" ;

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getPayload()::getDataType()" ;

		EXPECT_TRUE( true ) << "GenericMsg Hybrid Usage - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Hybrid Usage - exception not expected" ;
	}
}

TEST(GenericMsgTests, testGenericMsgError)
{

	{
		try
		{
			GenericMsg msg;

			ElementList attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "GenericMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "GenericMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			GenericMsg msg;

			RefreshMsg attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "GenericMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "GenericMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

	{
		try
		{
			GenericMsg msg;

			ElementList load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "GenericMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "GenericMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			GenericMsg msg;

			RefreshMsg load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "GenericMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "GenericMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

}

