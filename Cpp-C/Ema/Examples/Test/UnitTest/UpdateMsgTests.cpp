/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019, 2020, 2024 LSEG. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "EmaUnitTestConnect.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

//encoding by ETA and decoding by EMA
TEST(UpdateMsgTests, testUpdateMsgDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslUpdateMsg update;

		rsslClearUpdateMsg( &update );

		RsslMsgKey msgKey;

		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>( "ABCDEF" );
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

		update.msgBase.msgKey = msgKey;
		rsslUpdateMsgApplyHasMsgKey( &update );

		update.msgBase.encDataBody = rsslBuf;
		update.msgBase.containerType = RSSL_DT_FIELD_LIST;


		UpdateMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&update, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( respMsg.hasMsgKey() ) << "UpdateMsg::hasMsgKey() == true" ;

		EXPECT_TRUE( respMsg.hasName() ) << "UpdateMsg::hasName() == true" ;

		EXPECT_STREQ( respMsg.getName(), "ABCDEF" ) << "UpdateMsg::getName()" ;

		EXPECT_TRUE( respMsg.hasNameType() ) << "UpdateMsg::hasNameType() == true" ;

		EXPECT_EQ( respMsg.getNameType(), 1 ) << "UpdateMsg::getNameType()" ;

		EXPECT_TRUE( respMsg.hasServiceId() ) << "UpdateMsg::hasServiceId() == true" ;

		EXPECT_EQ( respMsg.getServiceId(), 2 ) << "UpdateMsg::getServiceId()" ;

		EXPECT_TRUE( respMsg.hasId() ) << "UpdateMsg::hasId() == true" ;

		EXPECT_EQ( respMsg.getId(), 4 ) << "UpdateMsg::getId()" ;

		EXPECT_TRUE( respMsg.hasFilter() ) << "UpdateMsg::hasFilter() == true" ;

		EXPECT_EQ( respMsg.getFilter(), 8 ) << "UpdateMsg::getFilter()" ;

		EXPECT_EQ( respMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getAttribType()" ;

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getPayloadType()" ;

		EXPECT_TRUE( true ) << "UpdateMsg Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(UpdateMsgTests, testUpdateMsgWithOpaque)
{

	try
	{
		RsslUpdateMsg rsslUpdateMsg;
		rsslClearUpdateMsg( &rsslUpdateMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslUpdateMsg.msgBase.msgKey = msgKey;
		rsslUpdateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		rsslUpdateMsg.msgBase.encDataBody = rsslBuf;
		rsslUpdateMsg.msgBase.containerType = RSSL_DT_OPAQUE;

		UpdateMsg updateMsg;

		StaticDecoder::setRsslData( &updateMsg, ( RsslMsg* )&rsslUpdateMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( updateMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "UpdateMsg::getPayload().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( updateMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "UpdateMsg::getPayload().getOpaque().getBuffer()" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Decode with Opaque payload - exception not expected" ;
	}
}

TEST(UpdateMsgTests, testUpdateMsgWithXml)
{

	try
	{
		RsslUpdateMsg rsslUpdateMsg;
		rsslClearUpdateMsg( &rsslUpdateMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslUpdateMsg.msgBase.msgKey = msgKey;
		rsslUpdateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		rsslUpdateMsg.msgBase.encDataBody = rsslBuf;
		rsslUpdateMsg.msgBase.containerType = RSSL_DT_XML;

		UpdateMsg updateMsg;

		StaticDecoder::setRsslData( &updateMsg, ( RsslMsg* )&rsslUpdateMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( updateMsg.getPayload().getDataType(), DataType::XmlEnum ) << "UpdateMsg::getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( updateMsg.getPayload().getXml().getBuffer(), compareTo ) << "UpdateMsg::getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Decode with Xml payload - exception not expected" ;
	}
}

TEST(UpdateMsgTests, testUpdateMsgWithAnsiPage)
{

	try
	{
		RsslUpdateMsg rsslUpdateMsg;
		rsslClearUpdateMsg( &rsslUpdateMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslUpdateMsg.msgBase.msgKey = msgKey;
		rsslUpdateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		rsslUpdateMsg.msgBase.encDataBody = rsslBuf;
		rsslUpdateMsg.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		UpdateMsg updateMsg;

		StaticDecoder::setRsslData( &updateMsg, ( RsslMsg* )&rsslUpdateMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( updateMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "UpdateMsg::getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( updateMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "UpdateMsg::getPayload().getAnsiPage().getBuffer()" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Decode with AnsiPage payload - exception not expected" ;
	}
}


//encoding by EMA and decoding by EMA
TEST(UpdateMsgTests, testUpdateMsgEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	const EmaString updateMsgString =
		"UpdateMsg\n"
		"    streamId=\"3\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    updateTypeNum=\"0\"\n"
		"    conflatedCount=\"5\"\n"
		"    conflatedTime=\"20\"\n"
		"    name=\"TRI.N\"\n"
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
		"UpdateMsgEnd\n";

	const EmaString updateMsgEmptyString =
		"UpdateMsg\n"
		"    streamId=\"0\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    updateTypeNum=\"0\"\n"
		"UpdateMsgEnd\n";

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	DataDictionary emaDataDictionary, emaDataDictionaryEmpty;

	try {
		emaDataDictionary.loadFieldDictionary("RDMFieldDictionaryTest");
		emaDataDictionary.loadEnumTypeDictionary("enumtypeTest.def");
	}
	catch (const OmmException&) {
		ASSERT_TRUE(false) << "DataDictionary::loadFieldDictionary() failed to load dictionary information";
	}

	try
	{
		UpdateMsg update, updateEmpty;

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		update.streamId( 3 );

		update.domainType( MMT_MARKET_PRICE );
		update.name( name );
//serviceName is only kept on the encoded RsslMsg
		update.serviceName( serviceName );
		update.nameType( 1 );
		update.id( 4 );
		update.filter( 8 );

		update.conflated( 5, 20 );  //conflated is ok for update

		update.extendedHeader( extendedHeader );
		update.attrib( flEnc );
		update.payload( flEnc );
		EXPECT_EQ( update.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "UpdateMsg.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( update.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "UpdateMsg.toString() == Dictionary is not loaded.";

		EXPECT_EQ( updateEmpty.toString(emaDataDictionary), updateMsgEmptyString ) << "UpdateMsg.toString() == updateMsgEmptyString";

		EXPECT_EQ( update.toString( emaDataDictionary ), updateMsgString ) << "UpdateMsg.toString() == updateMsgString";

        StaticDecoder::setData(&update, &dictionary);

		UpdateMsg updateMsgClone( update );
		updateMsgClone.clear();
		EXPECT_EQ( updateMsgClone.toString( emaDataDictionary ), updateMsgEmptyString ) << "UpdateMsg.toString() == updateMsgEmptyString";

		EXPECT_EQ( update.toString(), updateMsgString ) << "UpdateMsg.toString() == updateMsgString";

		EXPECT_TRUE( update.hasMsgKey() ) << "UpdateMsg::hasMsgKey() == true" ;

		EXPECT_EQ( update.getStreamId(), 3 ) << "UpdateMsg::getStreamId()" ;

		EXPECT_EQ( update.getDomainType(), MMT_MARKET_PRICE ) << "UpdateMsg::getDomainType()" ;

		EXPECT_TRUE( update.hasName() ) << "UpdateMsg::hasName() == true" ;
		EXPECT_STREQ( update.getName(), "TRI.N" ) << "UpdateMsg::getName()" ;

		EXPECT_FALSE( update.hasServiceName() ) << "UpdateMsg::hasServiceName() == false" ;

		EXPECT_TRUE( update.hasNameType() ) << "UpdateMsg::hasNameType() == true" ;
		EXPECT_EQ( update.getNameType(), 1 ) << "UpdateMsg::getNameType()" ;

		EXPECT_FALSE( update.hasServiceId() ) << "UpdateMsg::hasServiceId() == false" ;

		EXPECT_TRUE( update.hasId() ) << "UpdateMsg::hasId() == true" ;
		EXPECT_EQ( update.getId(), 4 ) << "UpdateMsg::getId()" ;

		EXPECT_TRUE( update.hasFilter() ) << "UpdateMsg::hasFilter() == true" ;
		EXPECT_EQ( update.getFilter(), 8 ) << "UpdateMsg::getFilter()" ;

		EXPECT_TRUE( update.hasConflated() ) << "UpdateMsg::hasConflated() == true" ;
		EXPECT_EQ( update.getConflatedCount(), 5 ) << "UpdateMsg::getConflatedCount()" ;
		EXPECT_EQ( update.getConflatedTime(), 20 ) << "UpdateMsg::getConflatedTime()" ;

		EXPECT_TRUE( update.hasExtendedHeader() ) << "UpdateMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( update.getExtendedHeader(), extendedHeader ) << "UpdateMsg::getExtendedHeader()" ;

		EXPECT_EQ( update.getAttrib().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from respMsg
		const FieldList& flAttrib = static_cast<const FieldList&>( update.getAttrib().getData() );
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( update.getPayload().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getLoad().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in payload) from respMsg
		const FieldList& flPayload = static_cast<const FieldList&>( update.getPayload().getData() );
		//decode FieldList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_TRUE( true ) << "UpdateMsg Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(UpdateMsgTests, testUpdateMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		UpdateMsg update;

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		update.streamId( 3 );

		update.domainType( MMT_MARKET_PRICE );
		update.name( name );
//serviceName is only kept on the encoded RsslMsg
		update.serviceName( serviceName );
		update.nameType( 1 );
		update.id( 4 );
		update.filter( 8 );

		update.conflated( 5, 20 );  //conflated is ok for update

		update.extendedHeader( extendedHeader );
		update.attrib( flEnc );
		update.payload( flEnc );


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &update, &dictionary );


		EXPECT_TRUE( update.hasMsgKey() ) << "UpdateMsg::hasMsgKey() == true" ;

		EXPECT_EQ( update.getStreamId(), 3 ) << "UpdateMsg::getStreamId()" ;

		EXPECT_EQ( update.getDomainType(), MMT_MARKET_PRICE ) << "UpdateMsg::getDomainType()" ;

		EXPECT_TRUE( update.hasName() ) << "UpdateMsg::hasName() == true" ;
		EXPECT_STREQ( update.getName(), "TRI.N" ) << "UpdateMsg::getName()" ;

		EXPECT_FALSE( update.hasServiceName() ) << "UpdateMsg::hasServiceName() == false" ;

		EXPECT_TRUE( update.hasNameType() ) << "UpdateMsg::hasNameType() == true" ;
		EXPECT_EQ( update.getNameType(), 1 ) << "UpdateMsg::getNameType()" ;

		EXPECT_FALSE( update.hasServiceId() ) << "UpdateMsg::hasServiceId() == false" ;

		EXPECT_TRUE( update.hasId() ) << "UpdateMsg::hasId() == true" ;
		EXPECT_EQ( update.getId(), 4 ) << "UpdateMsg::getId()" ;

		EXPECT_TRUE( update.hasFilter() ) << "UpdateMsg::hasFilter() == true" ;
		EXPECT_EQ( update.getFilter(), 8 ) << "UpdateMsg::getFilter()" ;

		EXPECT_TRUE( update.hasConflated() ) << "UpdateMsg::hasConflated() == true" ;
		EXPECT_EQ( update.getConflatedCount(), 5 ) << "UpdateMsg::getConflatedCount()" ;
		EXPECT_EQ( update.getConflatedTime(), 20 ) << "UpdateMsg::getConflatedTime()" ;

		EXPECT_TRUE( update.hasExtendedHeader() ) << "UpdateMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( update.getExtendedHeader(), extendedHeader ) << "UpdateMsg::getExtendedHeader()" ;

		EXPECT_EQ( update.getAttrib().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from updateMsg
		const FieldList& flAttrib = update.getAttrib().getFieldList();
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( update.getPayload().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getLoad().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in payload) from updateMsg
		const FieldList& flPayload = update.getPayload().getFieldList();
		//decode FieldList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_TRUE( true ) << "UpdateMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(UpdateMsgTests, testUpdateMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		UpdateMsg update;

		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		update.streamId( 3 );

		update.domainType( MMT_MARKET_PRICE );
		update.name( name );
//serviceName is only kept on the encoded RsslMsg
		update.serviceName( serviceName );
		update.nameType( 1 );
		update.id( 4 );
		update.filter( 8 );

		update.conflated( 5, 20 );  //conflated is ok for update

		update.extendedHeader( extendedHeader );
		update.attrib( elEnc );
		update.payload( elEnc );


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &update, &dictionary );


		EXPECT_TRUE( update.hasMsgKey() ) << "UpdateMsg::hasMsgKey() == true" ;

		EXPECT_EQ( update.getStreamId(), 3 ) << "UpdateMsg::getStreamId()" ;

		EXPECT_EQ( update.getDomainType(), MMT_MARKET_PRICE ) << "UpdateMsg::getDomainType()" ;

		EXPECT_TRUE( update.hasName() ) << "UpdateMsg::hasName() == true" ;
		EXPECT_STREQ( update.getName(), "TRI.N" ) << "UpdateMsg::getName()" ;

		EXPECT_FALSE( update.hasServiceName() ) << "UpdateMsg::hasServiceName() == false" ;

		EXPECT_TRUE( update.hasNameType() ) << "UpdateMsg::hasNameType() == true" ;
		EXPECT_EQ( update.getNameType(), 1 ) << "UpdateMsg::getNameType()" ;

		EXPECT_FALSE( update.hasServiceId() ) << "UpdateMsg::hasServiceId() == false" ;

		EXPECT_TRUE( update.hasId() ) << "UpdateMsg::hasId() == true" ;
		EXPECT_EQ( update.getId(), 4 ) << "UpdateMsg::getId()" ;

		EXPECT_TRUE( update.hasFilter() ) << "UpdateMsg::hasFilter() == true" ;
		EXPECT_EQ( update.getFilter(), 8 ) << "UpdateMsg::getFilter()" ;

		EXPECT_TRUE( update.hasConflated() ) << "UpdateMsg::hasConflated() == true" ;
		EXPECT_EQ( update.getConflatedCount(), 5 ) << "UpdateMsg::getConflatedCount()" ;
		EXPECT_EQ( update.getConflatedTime(), 20 ) << "UpdateMsg::getConflatedTime()" ;

		EXPECT_TRUE( update.hasExtendedHeader() ) << "UpdateMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( update.getExtendedHeader(), extendedHeader ) << "UpdateMsg::getExtendedHeader()" ;

		EXPECT_EQ( update.getAttrib().getDataType(), DataType::ElementListEnum ) << "UpdateMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;
		//get ElementList (in attrib) from updateMsg
		const ElementList& flAttrib = update.getAttrib().getElementList();
		//decode ElementList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( flAttrib );
		}

		EXPECT_EQ( update.getPayload().getDataType(), DataType::ElementListEnum ) << "UpdateMsg::getLoad().getDataType() == DataType::ElementListEnum" ;
		//get ElementList (in payload) from updateMsg
		const ElementList& flPayload = update.getPayload().getElementList();
		//decode ElementList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( flPayload );
		}

		EXPECT_TRUE( true ) << "UpdateMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(UpdateMsgTests, testUpdateMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		UpdateMsg update;

		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		update.streamId( 3 );

		update.domainType( MMT_MARKET_PRICE );
		update.name( name );
//serviceName is only kept on the encoded RsslMsg
		update.serviceName( serviceName );
		update.nameType( 1 );
		update.id( 4 );
		update.filter( 8 );

		update.conflated( 5, 20 );  //conflated is ok for update

		update.extendedHeader( extendedHeader );
		update.attrib( mapEnc );
		update.payload( mapEnc );


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &update, &dictionary );


		EXPECT_TRUE( update.hasMsgKey() ) << "UpdateMsg::hasMsgKey() == true" ;

		EXPECT_EQ( update.getStreamId(), 3 ) << "UpdateMsg::getStreamId()" ;

		EXPECT_EQ( update.getDomainType(), MMT_MARKET_PRICE ) << "UpdateMsg::getDomainType()" ;

		EXPECT_TRUE( update.hasName() ) << "UpdateMsg::hasName() == true" ;
		EXPECT_STREQ( update.getName(), "TRI.N" ) << "UpdateMsg::getName()" ;

		EXPECT_FALSE( update.hasServiceName() ) << "UpdateMsg::hasServiceName() == false" ;

		EXPECT_TRUE( update.hasNameType() ) << "UpdateMsg::hasNameType() == true" ;
		EXPECT_EQ( update.getNameType(), 1 ) << "UpdateMsg::getNameType()" ;

		EXPECT_FALSE( update.hasServiceId() ) << "UpdateMsg::hasServiceId() == false" ;

		EXPECT_TRUE( update.hasId() ) << "UpdateMsg::hasId() == true" ;
		EXPECT_EQ( update.getId(), 4 ) << "UpdateMsg::getId()" ;

		EXPECT_TRUE( update.hasFilter() ) << "UpdateMsg::hasFilter() == true" ;
		EXPECT_EQ( update.getFilter(), 8 ) << "UpdateMsg::getFilter()" ;

		EXPECT_TRUE( update.hasConflated() ) << "UpdateMsg::hasConflated() == true" ;
		EXPECT_EQ( update.getConflatedCount(), 5 ) << "UpdateMsg::getConflatedCount()" ;
		EXPECT_EQ( update.getConflatedTime(), 20 ) << "UpdateMsg::getConflatedTime()" ;

		EXPECT_TRUE( update.hasExtendedHeader() ) << "UpdateMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( update.getExtendedHeader(), extendedHeader ) << "UpdateMsg::getExtendedHeader()" ;

		EXPECT_EQ( update.getAttrib().getDataType(), DataType::MapEnum ) << "UpdateMsg::getAttrib().getDataType() == DataType::MapEnum" ;
		//get Map (in attrib) from updateMsg
		const Map& mapAttrib = update.getAttrib().getMap();
		//decode Map (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		EXPECT_EQ( update.getPayload().getDataType(), DataType::MapEnum ) << "UpdateMsg::getPayload().getDataType() == DataType::MapEnum" ;
		//get Map (in payload) from respMsg
		const Map& mapPayload = update.getPayload().getMap();
		//decode Map (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}

		EXPECT_TRUE( true ) << "UpdateMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(UpdateMsgTests, testUpdateMsgtoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslUpdateMsg update;

		rsslClearUpdateMsg( &update );

		RsslMsgKey msgKey;

		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>( "TRI.N" );
		nameBuffer.length = 5;

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

		update.msgBase.msgKey = msgKey;
		rsslUpdateMsgApplyHasMsgKey( &update );

		update.msgBase.encDataBody = rsslBuf;
		update.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>( "Status Text" );
		statusText.length = 11;


		UpdateMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&update, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		EXPECT_TRUE( true ) << "UpdateMsg toString Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg toString Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(UpdateMsgTests, testUpdateMsgHybrid)
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
		UpdateMsg updateMsg;

		updateMsg.attrib( fl );
		updateMsg.payload( fl );

		StaticDecoder::setData( &updateMsg, &dictionary );


		EXPECT_EQ( updateMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getattrib()::getDataType()" ;

		EXPECT_EQ( updateMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "UpdateMsg::getPayload()::getDataType()" ;

		EXPECT_TRUE( true ) << "UpdateMsg Hybrid Usage - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Hybrid Usage - exception not expected" ;
	}
}

TEST(UpdateMsgTests, testUpdateMsgClone)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	loadDictionaryFromFile(&dictionary);

	try
	{
		RsslUpdateMsg update;

		rsslClearUpdateMsg(&update);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("TRI.N");
		nameBuffer.length = 5;

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

		update.msgBase.msgKey = msgKey;
		rsslUpdateMsgApplyHasMsgKey(&update);

		update.msgBase.encDataBody = rsslBuf;
		update.msgBase.containerType = RSSL_DT_FIELD_LIST;
		update.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&update);

		RsslMsg updateDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&updateDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		UpdateMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&updateDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		UpdateMsg cloneUpdateMsg(respMsg);

		EXPECT_TRUE(cloneUpdateMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneUpdateMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneUpdateMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneUpdateMsg.toString()) << "Check equal toString()";

		EXPECT_TRUE(true) << "UpdateMsg Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);		

		msgBuf.length = 0;
		free(msgBuf.data);
	}
	catch (const OmmException& ex)
	{
		EXPECT_FALSE(true) << "UpdateMsg Clone - exception not expected. Text: "<< ex.getText();
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(UpdateMsgTests, testUpdateMsgEditClone)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	loadDictionaryFromFile(&dictionary);

	try
	{
		RsslUpdateMsg update;

		rsslClearUpdateMsg(&update);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("TRI.N");
		nameBuffer.length = 5;

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

		update.msgBase.msgKey = msgKey;
		rsslUpdateMsgApplyHasMsgKey(&update);

		update.msgBase.encDataBody = rsslBuf;
		update.msgBase.containerType = RSSL_DT_FIELD_LIST;
		update.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;

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

		RsslMsg updateDecode;

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&update);

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&updateDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		UpdateMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&updateDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		UpdateMsg cloneUpdateMsg(respMsg);

		EXPECT_TRUE(cloneUpdateMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneUpdateMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneUpdateMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneUpdateMsg.toString()) << "Check equal toString()";

		// Edit message
		cloneUpdateMsg.streamId(10);

		StaticDecoder::setData(&cloneUpdateMsg, &dictionary);

		EXPECT_FALSE(cloneUpdateMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(respMsg.toString(), cloneUpdateMsg.toString()) << "Check not equal toString()";
		EXPECT_TRUE(true) << "UpdateMsg Edit Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);		

		msgBuf.length = 0;
		free(msgBuf.data);
	}
	catch (const OmmException& ex)
	{
		EXPECT_FALSE(true) << "UpdateMsg Edit Clone - exception not expected. Text: " << ex.getText();
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(UpdateMsgTests, testUpdateMsgCloneMsgKeyPermissionData)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	char permissionData[] = "permission access to important data";
	const RsslUInt32 permissionDataLen = sizeof(permissionData) / sizeof(char);

	RsslUInt32 seqNum;

	RsslUInt16 flagsTest[] = {
		RSSL_UPMF_NONE,
		RSSL_UPMF_HAS_MSG_KEY,
		RSSL_UPMF_HAS_PERM_DATA,
		RSSL_UPMF_HAS_SEQ_NUM,
		RSSL_UPMF_HAS_CONF_INFO,
		RSSL_UPMF_HAS_MSG_KEY | RSSL_UPMF_HAS_PERM_DATA | RSSL_UPMF_HAS_SEQ_NUM | RSSL_UPMF_HAS_CONF_INFO,
	};
	const size_t nFlags = sizeof(flagsTest) / sizeof(RsslUInt16);
	RsslUInt16 flag;

	char rsslBufferData[1000];
	char msgBufData[2048];

	RsslEncodeIterator encIter;
	RsslDecodeIterator decodeIter;

	loadDictionaryFromFile(&dictionary);

	for (RsslUInt32 i = 0; i < nFlags; ++i)
	{
		flag = flagsTest[i];
		try
		{
			RsslUpdateMsg update;
			rsslClearUpdateMsg(&update);

			update.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
			update.msgBase.streamId = 3;

			update.flags = flag;

			RsslBuffer rsslBuf = RSSL_INIT_BUFFER;

			RsslMsgKey msgKey;
			rsslClearMsgKey(&msgKey);

			if (flag & RSSL_UPMF_HAS_MSG_KEY)
			{

				RsslBuffer nameBuffer;
				nameBuffer.data = const_cast<char*>("TRI.N");
				nameBuffer.length = 5;

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
				update.msgBase.msgKey = msgKey;

				rsslUpdateMsgApplyHasMsgKey(&update);

				update.msgBase.encDataBody = rsslBuf;
				update.msgBase.containerType = RSSL_DT_FIELD_LIST;
			}
			else {
				update.msgBase.encDataBody.data = 0;
				update.msgBase.encDataBody.length = 0;
				update.msgBase.containerType = RSSL_DT_NO_DATA;
			}

			/* Add Permission Info */
			if (flag & RSSL_UPMF_HAS_PERM_DATA)
			{
				update.permData.length = permissionDataLen;
				update.permData.data = permissionData;
			}

			/* Add Item Sequence Number */
			if (flag & RSSL_UPMF_HAS_SEQ_NUM)
			{
				seqNum = i;
				update.seqNum = seqNum;
			}

			/* Add Conflation Info */
			if (flag & RSSL_UPMF_HAS_CONF_INFO)
			{
				update.conflationCount = 42;
				update.conflationTime = 424;
			}

			RsslBuffer msgBuf;
			msgBuf.length = sizeof(msgBufData) / sizeof(char);
			msgBuf.data = msgBufData;

			RsslMsg updateDecode;
			UpdateMsg respMsg;

			prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&update, decodeIter, (RsslMsg*)&updateDecode, respMsg, dictionary);

			// Clone message
			UpdateMsg cloneUpdateMsg(respMsg);

			EXPECT_EQ(cloneUpdateMsg.getDomainType(), respMsg.getDomainType()) << "Compare domainType";
			EXPECT_EQ(cloneUpdateMsg.getDomainType(), RSSL_DMT_MARKET_BY_PRICE) << "Compare domainType: should be equal to " << RSSL_DMT_MARKET_BY_PRICE;

			EXPECT_EQ(cloneUpdateMsg.getStreamId(), respMsg.getStreamId()) << "Compare streamId";
			EXPECT_EQ(cloneUpdateMsg.getStreamId(), 3) << "Compare streamId: should be equal to 3";

			EXPECT_EQ(cloneUpdateMsg.hasMsgKey(), respMsg.hasMsgKey()) << "Compare hasMsgKey";
			EXPECT_EQ(cloneUpdateMsg.hasName(), respMsg.hasName()) << "Compare hasName";
			EXPECT_EQ(cloneUpdateMsg.hasNameType(), respMsg.hasNameType()) << "Compare hasNameType";
			EXPECT_EQ(cloneUpdateMsg.hasServiceId(), respMsg.hasServiceId()) << "Compare hasServiceId";
			EXPECT_EQ(cloneUpdateMsg.hasId(), respMsg.hasId()) << "Compare hasId";
			EXPECT_EQ(cloneUpdateMsg.hasFilter(), respMsg.hasFilter()) << "Compare hasFilter";
			EXPECT_EQ(cloneUpdateMsg.hasExtendedHeader(), respMsg.hasExtendedHeader()) << "Compare hasExtendedHeader";

			EXPECT_EQ(cloneUpdateMsg.hasSeqNum(), respMsg.hasSeqNum()) << "Compare hasSeqNum";
			EXPECT_EQ(cloneUpdateMsg.hasPermissionData(), respMsg.hasPermissionData()) << "Compare hasPermissionData";
			EXPECT_EQ(cloneUpdateMsg.hasConflated(), respMsg.hasConflated()) << "Compare hasConflated";
			EXPECT_EQ(cloneUpdateMsg.hasPublisherId(), respMsg.hasPublisherId()) << "Compare hasPublisherId";
			EXPECT_EQ(cloneUpdateMsg.hasServiceName(), respMsg.hasServiceName()) << "Compare hasServiceName";

			EXPECT_STREQ(cloneUpdateMsg.toString(), respMsg.toString()) << "Check equal toString()";

			EXPECT_EQ(cloneUpdateMsg.hasMsgKey(), (flag & RSSL_UPMF_HAS_MSG_KEY) > 0) << "Compare hasMsgKey: " << (flag & RSSL_UPMF_HAS_MSG_KEY);
			if (cloneUpdateMsg.hasMsgKey() && respMsg.hasMsgKey())
			{
				if (cloneUpdateMsg.hasServiceId())
				{
					EXPECT_EQ(cloneUpdateMsg.getServiceId(), respMsg.getServiceId()) << "Compare serviceId";
				}
				if (cloneUpdateMsg.hasName())
				{
					EXPECT_EQ(cloneUpdateMsg.getName(), respMsg.getName()) << "Compare name";
				}
				if (cloneUpdateMsg.hasNameType())
				{
					EXPECT_EQ(cloneUpdateMsg.getNameType(), respMsg.getNameType()) << "Compare nameType";
				}
				if (cloneUpdateMsg.hasId())
				{
					EXPECT_EQ(cloneUpdateMsg.getId(), respMsg.getId()) << "Compare id";
				}
				if (cloneUpdateMsg.hasFilter())
				{
					EXPECT_EQ(cloneUpdateMsg.getFilter(), respMsg.getFilter()) << "Compare filter";
				}
			}

			EXPECT_EQ(cloneUpdateMsg.hasSeqNum(), (flag & RSSL_UPMF_HAS_SEQ_NUM) > 0) << "Compare hasSeqNum: " << (flag & RSSL_UPMF_HAS_SEQ_NUM);
			if (cloneUpdateMsg.hasSeqNum() && respMsg.hasSeqNum())
			{
				EXPECT_EQ(cloneUpdateMsg.getSeqNum(), respMsg.getSeqNum()) << "Compare SeqNum";
				EXPECT_EQ(seqNum, cloneUpdateMsg.getSeqNum()) << "Compare SeqNum: " << seqNum;
			}

			EXPECT_EQ(cloneUpdateMsg.hasPermissionData(), (flag & RSSL_UPMF_HAS_PERM_DATA) > 0) << "Compare hasPermissionData: " << (flag & RSSL_UPMF_HAS_PERM_DATA);
			if (cloneUpdateMsg.hasPermissionData() && respMsg.hasPermissionData())
			{
				const EmaBuffer& permDataOrig = cloneUpdateMsg.getPermissionData();
				const EmaBuffer& permDataCopy = respMsg.getPermissionData();
				EmaBuffer permData(permissionData, permissionDataLen);

				EXPECT_EQ(permDataOrig.length(), permDataCopy.length()) << "Compare length of EmaBuffer Permission Data";
				EXPECT_EQ(permDataOrig.length(), permissionDataLen) << "Compare length of EmaBuffer Permission Data: " << permissionDataLen;
				EXPECT_EQ(permDataOrig, permDataCopy) << "Compare EmaBuffer Permission Data";
				EXPECT_EQ(permData, permDataCopy) << "Compare EmaBuffer Permission Data: " << permissionData;
			}

			EXPECT_EQ(cloneUpdateMsg.hasConflated(), (flag & RSSL_UPMF_HAS_CONF_INFO) > 0) << "Compare hasConflated: " << (flag & RSSL_UPMF_HAS_CONF_INFO);
			if (cloneUpdateMsg.hasConflated() && respMsg.hasConflated())
			{
				EXPECT_EQ(cloneUpdateMsg.getConflatedCount(), respMsg.getConflatedCount()) << "Compare getConflatedCount";
				EXPECT_EQ(cloneUpdateMsg.getConflatedCount(), 42) << "Compare getConflatedCount: " << 42;
				EXPECT_EQ(cloneUpdateMsg.getConflatedTime(), respMsg.getConflatedTime()) << "Compare getConflatedTime";
				EXPECT_EQ(cloneUpdateMsg.getConflatedTime(), 424) << "Compare getConflatedTime: " << 424;
			}

			EXPECT_TRUE(true) << "UpdateMsg Clone Success";
		}
		catch (const OmmException& ex)
		{
			EXPECT_FALSE(true) << "UpdateMsg Clone - exception not expected. Text: " << ex.getText();
		}
	}

	rsslDeleteDataDictionary(&dictionary);
}
