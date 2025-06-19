/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

//encoding by ETA and decoding by EMA
TEST(RequestMsgTests, testRequestMsgDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslRequestMsg request;

		rsslClearRequestMsg( &request );

		RsslMsgKey msgKey;

		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		msgKey.filter = 8;
		rsslMsgKeyApplyHasFilter( &msgKey );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		EmaString inText;
		encodeFieldList( rsslBuf, inText );

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib( &msgKey );

		request.msgBase.msgKey = msgKey;

		request.msgBase.encDataBody = rsslBuf;
		request.msgBase.containerType = RSSL_DT_FIELD_LIST;

		ReqMsg reqMsg;

		StaticDecoder::setRsslData( &reqMsg, ( RsslMsg* )&request, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( reqMsg.hasMsgKey() ) << "ReqMsg::hasMsgKey() == true" ;

		EXPECT_TRUE( reqMsg.hasName() ) << "ReqMsg::hasName() == true" ;

		EXPECT_STREQ( reqMsg.getName(), "ABCDEF" ) << "ReqMsg::getName()" ;

		EXPECT_TRUE( reqMsg.hasNameType() ) << "ReqMsg::hasNameType() == true" ;

		EXPECT_EQ( reqMsg.getNameType(), 1 ) << "ReqMsg::getNameType()" ;

		EXPECT_TRUE( reqMsg.hasServiceId() ) << "ReqMsg::hasServiceId() == true" ;

		EXPECT_EQ( reqMsg.getServiceId(), 2 ) << "ReqMsg::getServiceId()" ;

		EXPECT_TRUE( reqMsg.hasId() ) << "ReqMsg::hasId() == true" ;

		EXPECT_EQ( reqMsg.getId(), 4 ) << "ReqMsg::getId()" ;

		EXPECT_TRUE( reqMsg.hasFilter() ) << "ReqMsg::hasFilter() == true" ;

		EXPECT_EQ( reqMsg.getFilter(), 8 ) << "ReqMsg::getFilter()" ;

		EXPECT_EQ( reqMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "ReqMsg::getAttribType()" ;

		EXPECT_EQ( reqMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "ReqMsg::getPayloadType()" ;

		EXPECT_TRUE( true ) << "RequestMsg Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(RequestMsgTests, testRequestMsgWithOpaqueDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslRequestMsg rsslReqMsg;
		rsslClearRequestMsg( &rsslReqMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslReqMsg.msgBase.msgKey = msgKey;
		rsslReqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		rsslReqMsg.msgBase.encDataBody = rsslBuf;
		rsslReqMsg.msgBase.containerType = RSSL_DT_OPAQUE;

		ReqMsg reqMsg;

		StaticDecoder::setRsslData( &reqMsg, ( RsslMsg* )&rsslReqMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( reqMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "RequestMsg::getPayload().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( reqMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "RequestMsg::getPayload().getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg Decode with Opaque payload - exception not expected" ;
	}
}

TEST(RequestMsgTests, testRequestMsgWithXmlDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslRequestMsg rsslReqMsg;
		rsslClearRequestMsg( &rsslReqMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslReqMsg.msgBase.msgKey = msgKey;
		rsslReqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		rsslReqMsg.msgBase.encDataBody = rsslBuf;
		rsslReqMsg.msgBase.containerType = RSSL_DT_XML;

		ReqMsg reqMsg;

		StaticDecoder::setRsslData( &reqMsg, ( RsslMsg* )&rsslReqMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( reqMsg.getPayload().getDataType(), DataType::XmlEnum ) << "RequestMsg::getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( reqMsg.getPayload().getXml().getBuffer(), compareTo ) << "RequestMsg::getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg Decode with Xml payload - exception not expected" ;
	}
}

TEST(RequestMsgTests, testRequestMsgWithJsonDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslRequestMsg rsslReqMsg;
		rsslClearRequestMsg( &rsslReqMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslReqMsg.msgBase.msgKey = msgKey;
		rsslReqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer jsonValue;
		jsonValue.data = ( char* )"{\"consumerList\":{\"consumer\":{\"name\":\"\",\"dataType\":\"Ascii\",\"value\":\"Consumer_1\"}}}";
		jsonValue.length = static_cast<rtrUInt32>( strlen( jsonValue.data ) );

		encodeNonRWFData( &rsslBuf, &jsonValue );

		rsslReqMsg.msgBase.encDataBody = rsslBuf;
		rsslReqMsg.msgBase.containerType = RSSL_DT_JSON;

		ReqMsg reqMsg;

		StaticDecoder::setRsslData( &reqMsg, ( RsslMsg* )&rsslReqMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( reqMsg.getPayload().getDataType(), DataType::JsonEnum ) << "RequestMsg::getPayload().getDataType() == DataType::JsonEnum" ;

		EmaBuffer compareTo( jsonValue.data, jsonValue.length );
		EXPECT_STREQ( reqMsg.getPayload().getJson().getBuffer(), compareTo ) << "RequestMsg::getPayload().getJson().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg Decode with Json payload - exception not expected" ;
	}
}

TEST(RequestMsgTests, testRequestMsgWithAnsiPageDecodeAll)
{


	RsslDataDictionary dictionary;

	try
	{
		RsslRequestMsg rsslReqMsg;
		rsslClearRequestMsg( &rsslReqMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslReqMsg.msgBase.msgKey = msgKey;
		rsslReqMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		rsslReqMsg.msgBase.encDataBody = rsslBuf;
		rsslReqMsg.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		ReqMsg reqMsg;

		StaticDecoder::setRsslData( &reqMsg, ( RsslMsg* )&rsslReqMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( reqMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "RequestMsg::getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( reqMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "RequestMsg::getPayload().getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg Decode with AnsiPage payload - exception not expected" ;
	}
}

//encoding by EMA and decoding by EMA
TEST(RequestMsgTests, testRequestMsgEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	DataDictionary emaDataDictionary, emaDataDictionaryEmpty;

	try {
		emaDataDictionary.loadFieldDictionary( "RDMFieldDictionaryTest" );
		emaDataDictionary.loadEnumTypeDictionary( "enumtypeTest.def" );
	}
	catch ( const OmmException& ) {
		ASSERT_TRUE( false ) << "DataDictionary::loadFieldDictionary() failed to load dictionary information";
	}

	try
	{
		ReqMsg reqMsg, reqMsgEmpty;

		const EmaString refreshMsgString =
			"ReqMsg\n"
			"    streamId=\"1\"\n"
			"    domain=\"MarketPrice Domain\"\n"
			"    InitialImage\n"
			"    InterestAfterRefresh\n"
			"    name=\"ABCDEF\"\n"
			"    nameType=\"1\"\n"
			"    filter=\"8\"\n"
			"    id=\"4\"\n"
			"    Attrib dataType=\"FieldList\"\n"
			"        FieldList FieldListNum=\"65\" DictionaryId=\"1\"\n"
			"            FieldEntry fid=\"1\" name=\"PROD_PERM\" dataType=\"UInt\" value=\"64\"\n"
			"            FieldEntry fid=\"6\" name=\"TRDPRC_1\" dataType=\"Real\" value=\"0.11\"\n"
			"            FieldEntry fid=\"-2\" name=\"INTEGER\" dataType=\"Int\" value=\"32\"\n"
			"            FieldEntry fid=\"16\" name=\"TRADE_DATE\" dataType=\"Date\" value=\"07 NOV 1999\"\n"
			"            FieldEntry fid=\"18\" name=\"TRDTIM_1\" dataType=\"Time\" value=\"02:03:04:005:000:000\"\n"
			"            FieldEntry fid=\"-3\" name=\"TRADE_DATE\" dataType=\"DateTime\" value=\"07 NOV 1999 01:02:03:000:000:000\"\n"
			"            FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"RealTime/TickByTick\"\n"
			"            FieldEntry fid=\"-6\" name=\"MY_STATE\" dataType=\"State\" value=\"Open / Ok / None / 'Succeeded'\"\n"
			"            FieldEntry fid=\"235\" name=\"PNAC\" dataType=\"Ascii\" value=\"ABCDEF\"\n"
			"        FieldListEnd\n"
			"\n"
			"    AttribEnd\n"
			"    ExtendedHeader\n"
			"        65 78 74 65 6E 64\n"
			"    ExtendedHeaderEnd\n"
			"    Payload dataType=\"FieldList\"\n"
			"        FieldList FieldListNum=\"65\" DictionaryId=\"1\"\n"
			"            FieldEntry fid=\"1\" name=\"PROD_PERM\" dataType=\"UInt\" value=\"64\"\n"
			"            FieldEntry fid=\"6\" name=\"TRDPRC_1\" dataType=\"Real\" value=\"0.11\"\n"
			"            FieldEntry fid=\"-2\" name=\"INTEGER\" dataType=\"Int\" value=\"32\"\n"
			"            FieldEntry fid=\"16\" name=\"TRADE_DATE\" dataType=\"Date\" value=\"07 NOV 1999\"\n"
			"            FieldEntry fid=\"18\" name=\"TRDTIM_1\" dataType=\"Time\" value=\"02:03:04:005:000:000\"\n"
			"            FieldEntry fid=\"-3\" name=\"TRADE_DATE\" dataType=\"DateTime\" value=\"07 NOV 1999 01:02:03:000:000:000\"\n"
			"            FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"RealTime/TickByTick\"\n"
			"            FieldEntry fid=\"-6\" name=\"MY_STATE\" dataType=\"State\" value=\"Open / Ok / None / 'Succeeded'\"\n"
			"            FieldEntry fid=\"235\" name=\"PNAC\" dataType=\"Ascii\" value=\"ABCDEF\"\n"
			"        FieldListEnd\n"
			"\n"
			"    PayloadEnd\n"
			"ReqMsgEnd\n";

		const EmaString refreshMsgEmptyString =
			"ReqMsg\n"
			"    streamId=\"0\"\n"
			"    domain=\"MarketPrice Domain\"\n"
			"    InitialImage\n"
			"    InterestAfterRefresh\n"
			"ReqMsgEnd\n";

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaString name( "ABCDEF" );
		EmaString serviceName( "SERVICE" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		reqMsg.streamId( 1 );

		reqMsg.domainType( MMT_MARKET_PRICE );
		reqMsg.name( name );
//serviceName is only kept on the encoded RsslMsg
		reqMsg.serviceName( serviceName );
		reqMsg.nameType( 1 );
		reqMsg.id( 4 );
		reqMsg.filter( 8 );

		reqMsg.extendedHeader( extendedHeader );
		reqMsg.attrib( flEnc );
		reqMsg.payload( flEnc );
		EXPECT_EQ( reqMsg.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "ReqMsg.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( reqMsg.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "ReqMsg.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( reqMsgEmpty.toString(emaDataDictionary), refreshMsgEmptyString ) << "ReqMsg.toString() == refreshMsgEmptyString";

		EXPECT_EQ( reqMsg.toString( emaDataDictionary ), refreshMsgString ) << "ReqMsg.toString() == refreshMsgString";

		StaticDecoder::setData(&reqMsg, &dictionary);

		ReqMsg reqClone( reqMsg );
		reqClone.clear();
		EXPECT_EQ( reqClone.toString( emaDataDictionary ), refreshMsgEmptyString ) << "ReqMsg.toString() == refreshMsgEmptyString";

		EXPECT_EQ( reqMsg.toString(), refreshMsgString ) << "ReqMsg.toString() == refreshMsgString";

		EXPECT_TRUE( reqMsg.hasMsgKey() ) << "ReqMsg::hasMsgKey() == true" ;

		EXPECT_EQ( reqMsg.getStreamId(), 1 ) << "ReqMsg::getStreamId()" ;

		EXPECT_EQ( reqMsg.getDomainType(), MMT_MARKET_PRICE ) << "ReqMsg::getDomainType()" ;

		EXPECT_TRUE( reqMsg.hasName() ) << "ReqMsg::hasName() == true" ;
		EXPECT_STREQ( reqMsg.getName(), "ABCDEF" ) << "ReqMsg::getName()" ;

		EXPECT_FALSE( reqMsg.hasServiceName() ) << "ReqMsg::hasServiceName() == false" ;

		EXPECT_TRUE( reqMsg.hasNameType() ) << "ReqMsg::hasNameType() == true" ;
		EXPECT_EQ( reqMsg.getNameType(), 1 ) << "ReqMsg::getNameType()" ;

		EXPECT_FALSE( reqMsg.hasServiceId() ) << "ReqMsg::hasServiceId() == false" ;

		EXPECT_TRUE( reqMsg.hasId() ) << "ReqMsg::hasId() == true" ;
		EXPECT_EQ( reqMsg.getId(), 4 ) << "ReqMsg::getId()" ;

		EXPECT_TRUE( reqMsg.hasFilter() ) << "ReqMsg::hasFilter() == true" ;
		EXPECT_EQ( reqMsg.getFilter(), 8 ) << "ReqMsg::getFilter()" ;

		EXPECT_TRUE( reqMsg.hasExtendedHeader() ) << "ReqMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( reqMsg.getExtendedHeader(), extendedHeader ) << "ReqMsg::getExtendedHeader()" ;

		EXPECT_EQ( reqMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "ElementEntry::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from reqMsg
		const FieldList& flAttrib = static_cast<const FieldList&>( reqMsg.getAttrib().getData() );
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( reqMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "ElementEntry::getLoad().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in payload) from reqMsg
		const FieldList& flPayload = static_cast<const FieldList&>( reqMsg.getPayload().getData() );
		//decode FieldList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_TRUE( true ) << "RequestMsg Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(RequestMsgTests, testRequestMsgtoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslRequestMsg request;

		rsslClearRequestMsg( &request );

		RsslMsgKey msgKey;

		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		msgKey.filter = 8;
		rsslMsgKeyApplyHasFilter( &msgKey );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		EmaString inText;
		encodeFieldList( rsslBuf, inText );

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib( &msgKey );

		request.msgBase.msgKey = msgKey;

		request.msgBase.encDataBody = rsslBuf;
		request.msgBase.containerType = RSSL_DT_FIELD_LIST;

		ReqMsg reqMsg;

		StaticDecoder::setRsslData( &reqMsg, ( RsslMsg* )&request, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		EXPECT_TRUE( true ) << "RequestMsg toString Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RequestMsg toString Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(RequestMsgTests, testDomainTypeUnsupported)
{

	try
	{
		ReqMsg reqMsg;

		reqMsg.domainType( 300 );

		EXPECT_FALSE( true ) << "Setting unsupported domain type. Exception expected." ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Setting unsupported domain type. Exception expected." ;
	}
}

TEST(RequestMsgTests, testReqMsgWithServiceIdAndNameAndServiceList)
{

	try
	{
		ReqMsg reqMsg;

		reqMsg.serviceName( EmaString( "DIRECT_FEED" ) ).serviceId( 10 );

		EXPECT_FALSE( true ) << "Setting ServiceId while Name is set. Exception expected." ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Setting ServiceId while Name is set. Exception expected." ;
	}

	try
	{
		ReqMsg reqMsg;

		reqMsg.serviceListName(EmaString("SVG1")).serviceId(10);

		EXPECT_FALSE(true) << "Setting ServiceId while ServiceListName is set. Exception expected.";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Setting ServiceId while ServiceListName is set. Exception expected.";
	}

	try
	{
		ReqMsg reqMsg;

		reqMsg.serviceId( 10 ).serviceName( EmaString( "DIRECT_FEED" ) );

		EXPECT_FALSE( true ) << "Setting ServiceName while id is set. Exception expected." ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Setting ServiceName while id is set. Exception expected." ;
	}

	try
	{
		ReqMsg reqMsg;

		reqMsg.serviceListName(EmaString("SVG1")).serviceName(EmaString("DIRECT_FEED"));

		EXPECT_FALSE(true) << "Setting ServiceName while ServiceListName is set. Exception expected.";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Setting ServiceName while ServiceListName is set. Exception expected.";
	}

	try
	{
		ReqMsg reqMsg;

		reqMsg.serviceName(EmaString("DIRECT_FEED")).serviceListName(EmaString("SVG1"));

		EXPECT_FALSE(true) << "Setting ServiceListName while ServiceName is set. Exception expected.";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Setting ServiceListName while ServiceName is set. Exception expected.";
	}

	try
	{
		ReqMsg reqMsg;

		reqMsg.serviceId(10).serviceListName(EmaString("SVG1"));

		EXPECT_FALSE(true) << "Setting ServiceListName while id is set. Exception expected.";
	}
	catch (const OmmException&)
	{
		EXPECT_TRUE(true) << "Setting ServiceListName while id is set. Exception expected.";
	}
}



TEST(RequestMsgTests, testRequestMsgEncodeDecodeQos)
{

	ReqMsg reqMsg;

	reqMsg.qos( 5000, 6000 );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), 5000 ) << "ReqMsg::getQosTimeliness()== 5000" ;
	EXPECT_EQ( reqMsg.getQosRate(), 6000 ) << "ReqMsg::getQosRate()== 6000 " ;

	// Clone message
	ReqMsg cloneReqMsg(reqMsg);

	EXPECT_EQ(cloneReqMsg.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg.getQosRate(), reqMsg.getQosRate()) << "Compare copy of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( 65539, 78895 ); // Out of range
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum ) << "ReqMsg::getQosTimeliness() == ReqMsg::BestDelayedTimelinessEnum " ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum ) << "ReqMsg::getQosRate() == ReqMsg::JustInTimeConflatedEnum" ;

	// Clone message
	ReqMsg cloneReqMsg1(reqMsg);

	EXPECT_EQ(cloneReqMsg1.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy1 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg1.getQosRate(), reqMsg.getQosRate()) << "Compare copy1 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum " ;

	// Clone message
	ReqMsg cloneReqMsg2(reqMsg);

	EXPECT_EQ(cloneReqMsg2.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy2 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg2.getQosRate(), reqMsg.getQosRate()) << "Compare copy2 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, ReqMsg::BestConflatedRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestConflatedRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestConflatedRateEnum " ;

	// Clone message
	ReqMsg cloneReqMsg3(reqMsg);

	EXPECT_EQ(cloneReqMsg3.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy3 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg3.getQosRate(), reqMsg.getQosRate()) << "Compare copy3 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum " ;

	// Clone message
	ReqMsg cloneReqMsg4(reqMsg);

	EXPECT_EQ(cloneReqMsg4.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy4 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg4.getQosRate(), reqMsg.getQosRate()) << "Compare copy4 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::BestConflatedRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestConflatedRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestConflatedRateEnum " ;

	// Clone message
	ReqMsg cloneReqMsg5(reqMsg);

	EXPECT_EQ(cloneReqMsg5.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy5 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg5.getQosRate(), reqMsg.getQosRate()) << "Compare copy5 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::TickByTickEnum ) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum " ;

	// Clone message
	ReqMsg cloneReqMsg6(reqMsg);

	EXPECT_EQ(cloneReqMsg6.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy6 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg6.getQosRate(), reqMsg.getQosRate()) << "Compare copy6 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, ReqMsg::JustInTimeConflatedEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum ) << "ReqMsg::getQosRate()== ReqMsg::JustInTimeConflatedEnum " ;

	// Clone message
	ReqMsg cloneReqMsg7(reqMsg);

	EXPECT_EQ(cloneReqMsg7.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy7 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg7.getQosRate(), reqMsg.getQosRate()) << "Compare copy7 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( 23, ReqMsg::JustInTimeConflatedEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), 23 ) << "ReqMsg::getQosTimeliness()== 23" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum ) << "ReqMsg::getQosRate()== ReqMsg::JustInTimeConflatedEnum " ;

	// Clone message
	ReqMsg cloneReqMsg8(reqMsg);

	EXPECT_EQ(cloneReqMsg8.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy8 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg8.getQosRate(), reqMsg.getQosRate()) << "Compare copy8 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, 5623 );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), 5623 ) << "ReqMsg::getQosRate()== 5623 " ;

	// Clone message
	ReqMsg cloneReqMsg9(reqMsg);

	EXPECT_EQ(cloneReqMsg9.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy9 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg9.getQosRate(), reqMsg.getQosRate()) << "Compare copy9 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( 9999, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), 9999 ) << "ReqMsg::getQosTimeliness()== 9999" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::TickByTickEnum ) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum " ;

	// Clone message
	ReqMsg cloneReqMsg10(reqMsg);

	EXPECT_EQ(cloneReqMsg10.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy10 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg10.getQosRate(), reqMsg.getQosRate()) << "Compare copy10 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::TickByTickEnum) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum";

	// Clone message
	ReqMsg cloneReqMsg11(reqMsg);

	EXPECT_EQ(cloneReqMsg11.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy11 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg11.getQosRate(), reqMsg.getQosRate()) << "Compare copy11 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::TickByTickEnum) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum";
	
	// Clone message
	ReqMsg cloneReqMsg12(reqMsg);

	EXPECT_EQ(cloneReqMsg12.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy12 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg12.getQosRate(), reqMsg.getQosRate()) << "Compare copy12 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, 4455 );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), 4455) << "ReqMsg::getQosRate()== 4455";

	// Clone message
	ReqMsg cloneReqMsg13(reqMsg);

	EXPECT_EQ(cloneReqMsg13.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy13 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg13.getQosRate(), reqMsg.getQosRate()) << "Compare copy13 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, 5555 );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), 5555) << "ReqMsg::getQosRate()== 5555";

	// Clone message
	ReqMsg cloneReqMsg14(reqMsg);

	EXPECT_EQ(cloneReqMsg14.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy14 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg14.getQosRate(), reqMsg.getQosRate()) << "Compare copy14 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::BestRateEnum) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum";
		
	// Clone message
	ReqMsg cloneReqMsg15(reqMsg);

	EXPECT_EQ(cloneReqMsg15.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy15 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg15.getQosRate(), reqMsg.getQosRate()) << "Compare copy15 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( 1235, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), 1235) << "ReqMsg::getQosTimeliness()== 1235";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::BestRateEnum) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum";
	
	// Clone message
	ReqMsg cloneReqMsg16(reqMsg);

	EXPECT_EQ(cloneReqMsg16.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy16 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg16.getQosRate(), reqMsg.getQosRate()) << "Compare copy16 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( 5678, ReqMsg::BestConflatedRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), 5678) << "ReqMsg::getQosTimeliness()== 5678";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::BestConflatedRateEnum) << "ReqMsg::getQosRate()== ReqMsg::BestConflatedRateEnum";

	// Clone message
	ReqMsg cloneReqMsg17(reqMsg);

	EXPECT_EQ(cloneReqMsg17.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy17 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg17.getQosRate(), reqMsg.getQosRate()) << "Compare copy17 of ReqMsg::getQosRate()";
	//////////////////////

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::JustInTimeConflatedEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum) << "ReqMsg::getQosRate()== ReqMsg::JustInTimeConflatedEnum";

	// Clone message
	ReqMsg cloneReqMsg18(reqMsg);

	EXPECT_EQ(cloneReqMsg18.getQosTimeliness(), reqMsg.getQosTimeliness()) << "Compare copy18 of ReqMsg::getQosTimeliness()";
	EXPECT_EQ(cloneReqMsg18.getQosRate(), reqMsg.getQosRate()) << "Compare copy18 of ReqMsg::getQosRate()";
}

TEST(RequestMsgTests, testRequestMsgClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslRequestMsg request;

		rsslClearRequestMsg(&request);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType(&msgKey);

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId(&msgKey);

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier(&msgKey);

		msgKey.filter = 8;
		rsslMsgKeyApplyHasFilter(&msgKey);

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = (char*)malloc(sizeof(char) * 1000);

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		request.msgBase.msgKey = msgKey;

		request.msgBase.encDataBody = rsslBuf;
		request.msgBase.containerType = RSSL_DT_FIELD_LIST;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&request);

		RsslMsg requestDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&requestDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		ReqMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&requestDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		ReqMsg cloneReqMsg(respMsg);

		EXPECT_TRUE(cloneReqMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneReqMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneReqMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneReqMsg.toString()) << "Check equal toString()";

		EXPECT_TRUE(true) << "ReqMsg Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "ReqMsg Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(RequestMsgTests, testRequestMsgEditClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslRequestMsg request;

		rsslClearRequestMsg(&request);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType(&msgKey);

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId(&msgKey);

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier(&msgKey);

		msgKey.filter = 8;
		rsslMsgKeyApplyHasFilter(&msgKey);

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = (char*)malloc(sizeof(char) * 1000);

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		request.msgBase.msgKey = msgKey;

		request.msgBase.encDataBody = rsslBuf;
		request.msgBase.containerType = RSSL_DT_FIELD_LIST;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&request);

		RsslMsg requestDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&requestDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		ReqMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&requestDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		ReqMsg cloneReqMsg(respMsg);

		EXPECT_TRUE(cloneReqMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneReqMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneReqMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneReqMsg.toString()) << "Check equal toString()";

		// Edit message
		cloneReqMsg.streamId(10);

		StaticDecoder::setData(&cloneReqMsg, &dictionary);

		EXPECT_FALSE(cloneReqMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(respMsg.toString(), cloneReqMsg.toString()) << "Check not equal toString()";
		EXPECT_TRUE(true) << "ReqMsg Edit Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "ReqMsg Edit Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(RequestMsgTests, testRequestMsgCloneMsg)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	RsslUInt16 flagsTest[] = {
		RSSL_RQMF_NONE,
		RSSL_RQMF_HAS_PRIORITY,
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
			RsslRequestMsg request;
			rsslClearRequestMsg(&request);

			request.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
			request.msgBase.streamId = 1;

			request.flags = flag;

			RsslBuffer rsslBuf = RSSL_INIT_BUFFER;

			RsslMsgKey msgKey;
			rsslClearMsgKey(&msgKey);

			RsslBuffer nameBuffer;
			nameBuffer.data = const_cast<char*>("ABCDEF");
			nameBuffer.length = 6;

			msgKey.name = nameBuffer;
			rsslMsgKeyApplyHasName(&msgKey);

			msgKey.nameType = 1;
			rsslMsgKeyApplyHasNameType(&msgKey);

			msgKey.serviceId = 2;
			rsslMsgKeyApplyHasServiceId(&msgKey);

			msgKey.identifier = 4;
			rsslMsgKeyApplyHasIdentifier(&msgKey);

			msgKey.filter = 8;
			rsslMsgKeyApplyHasFilter(&msgKey);

			rsslBuf.length = sizeof(rsslBufferData) / sizeof(char);
			rsslBuf.data = rsslBufferData;

			EmaString inText;
			encodeFieldList(rsslBuf, inText);

			msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
			msgKey.encAttrib = rsslBuf;
			rsslMsgKeyApplyHasAttrib(&msgKey);

			request.msgBase.msgKey = msgKey;

			request.msgBase.encDataBody = rsslBuf;
			request.msgBase.containerType = RSSL_DT_FIELD_LIST;

			/* Add Priority Info */
			if (flag & RSSL_RQMF_HAS_PRIORITY)
			{
				request.priorityClass = 242;
				request.priorityCount = 3178;
			}


			RsslBuffer msgBuf;
			msgBuf.length = sizeof(msgBufData) / sizeof(char);
			msgBuf.data = msgBufData;

			RsslMsg requestDecode;
			ReqMsg respMsg;

			prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&request, decodeIter, (RsslMsg*)&requestDecode, respMsg, dictionary);

			// Clone message
			ReqMsg cloneReqMsg(respMsg);

			EXPECT_EQ(cloneReqMsg.getDomainType(), respMsg.getDomainType()) << "Compare domainType";
			EXPECT_EQ(cloneReqMsg.getDomainType(), RSSL_DMT_MARKET_PRICE) << "Compare domainType: should be equal to " << RSSL_DMT_MARKET_PRICE;

			EXPECT_EQ(cloneReqMsg.getStreamId(), respMsg.getStreamId()) << "Compare streamId";
			EXPECT_EQ(cloneReqMsg.getStreamId(), 1) << "Compare streamId: should be equal to 1";

			EXPECT_EQ(cloneReqMsg.hasMsgKey(), respMsg.hasMsgKey()) << "Compare hasMsgKey";
			EXPECT_EQ(cloneReqMsg.hasName(), respMsg.hasName()) << "Compare hasName";
			EXPECT_EQ(cloneReqMsg.hasNameType(), respMsg.hasNameType()) << "Compare hasNameType";
			EXPECT_EQ(cloneReqMsg.hasServiceId(), respMsg.hasServiceId()) << "Compare hasServiceId";
			EXPECT_EQ(cloneReqMsg.hasId(), respMsg.hasId()) << "Compare hasId";
			EXPECT_EQ(cloneReqMsg.hasFilter(), respMsg.hasFilter()) << "Compare hasFilter";
			EXPECT_EQ(cloneReqMsg.hasExtendedHeader(), respMsg.hasExtendedHeader()) << "Compare hasExtendedHeader";

			EXPECT_EQ(cloneReqMsg.hasPriority(), respMsg.hasPriority()) << "Compare hasPriority";
			EXPECT_EQ(cloneReqMsg.hasQos(), respMsg.hasQos()) << "Compare hasQos";
			EXPECT_EQ(cloneReqMsg.hasView(), respMsg.hasView()) << "Compare hasView";
			EXPECT_EQ(cloneReqMsg.hasBatch(), respMsg.hasBatch()) << "Compare hasBatch";
			EXPECT_EQ(cloneReqMsg.hasServiceName(), respMsg.hasServiceName()) << "Compare hasServiceName";

			EXPECT_STREQ(respMsg.toString(), cloneReqMsg.toString()) << "Check equal toString()";

			EXPECT_EQ(cloneReqMsg.hasMsgKey(), true) << "Compare hasMsgKey: true";
			if (cloneReqMsg.hasMsgKey() && respMsg.hasMsgKey())
			{
				if (cloneReqMsg.hasServiceId())
				{
					EXPECT_EQ(cloneReqMsg.getServiceId(), respMsg.getServiceId()) << "Compare serviceId";
				}
				if (cloneReqMsg.hasName())
				{
					EXPECT_EQ(cloneReqMsg.getName(), respMsg.getName()) << "Compare name";
				}
				if (cloneReqMsg.hasNameType())
				{
					EXPECT_EQ(cloneReqMsg.getNameType(), respMsg.getNameType()) << "Compare nameType";
				}
				if (cloneReqMsg.hasId())
				{
					EXPECT_EQ(cloneReqMsg.getId(), respMsg.getId()) << "Compare id";
				}
				if (cloneReqMsg.hasFilter())
				{
					EXPECT_EQ(cloneReqMsg.getFilter(), respMsg.getFilter()) << "Compare filter";
				}
			}

			EXPECT_EQ(cloneReqMsg.hasPriority(), (flag & RSSL_RQMF_HAS_PRIORITY) > 0) << "Compare hasPriority: " << (flag & RSSL_RQMF_HAS_PRIORITY);
			if (cloneReqMsg.hasPriority())
			{
				EXPECT_EQ(cloneReqMsg.getPriorityClass(), respMsg.getPriorityClass()) << "Compare getPriorityClass";
				EXPECT_EQ(cloneReqMsg.getPriorityClass(), 242) << "Compare getPriorityClass: " << 242;
				EXPECT_EQ(cloneReqMsg.getPriorityCount(), respMsg.getPriorityCount()) << "Compare getPriorityCount";
				EXPECT_EQ(cloneReqMsg.getPriorityCount(), 3178) << "Compare getPriorityCount: " << 3178;
			}

			EXPECT_TRUE(true) << "ReqMsg Clone Success";
		}
		catch (const OmmException&)
		{
			EXPECT_FALSE(true) << "ReqMsg Clone - exception not expected";
		}
	}
	rsslDeleteDataDictionary(&dictionary);
}
