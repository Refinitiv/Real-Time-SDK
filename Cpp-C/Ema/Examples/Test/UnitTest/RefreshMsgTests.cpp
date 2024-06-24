/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2019, 2024 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

//encoding by ETA and decoding by EMA
TEST(RefreshMsgTests, testRefreshMsgDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslRefreshMsg refresh;

		rsslClearRefreshMsg( &refresh );

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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey( &refresh );

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

		refresh.state = rsslState;

		RefreshMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( respMsg.hasMsgKey() ) << "RefreshMsg::hasMsgKey() == true" ;

		EXPECT_TRUE( respMsg.hasName() ) << "RefreshMsg::hasName() == true" ;

		EXPECT_STREQ( respMsg.getName(), "ABCDEF" ) << "RefreshMsg::getName()" ;

		EXPECT_TRUE( respMsg.hasNameType() ) << "RefreshMsg::hasNameType() == true" ;

		EXPECT_EQ( respMsg.getNameType(), 1 ) << "RefreshMsg::getNameType()" ;

		EXPECT_TRUE( respMsg.hasServiceId() ) << "RefreshMsg::hasServiceId() == true" ;

		EXPECT_EQ( respMsg.getServiceId(), 2 ) << "RefreshMsg::getServiceId()" ;

		EXPECT_TRUE( respMsg.hasId() ) << "RefreshMsg::hasId() == true" ;

		EXPECT_EQ( respMsg.getId(), 4 ) << "RefreshMsg::getId()" ;

		EXPECT_TRUE( respMsg.hasFilter() ) << "RefreshMsg::hasFilter() == true" ;

		EXPECT_EQ( respMsg.getFilter(), 8 ) << "RefreshMsg::getFilter()" ;

		EXPECT_EQ( respMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getAttribType()" ;

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getPayloadType()" ;

		EXPECT_EQ( respMsg.getState().getStatusCode(), OmmState::InvalidArgumentEnum ) << "RefreshMsg::getState()::getCode()" ;

		EXPECT_EQ( respMsg.getState().getDataState(), OmmState::NoChangeEnum ) << "RefreshMsg::getState()::getDataState()" ;

		EXPECT_EQ( respMsg.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "RefreshMsg::getState()::getStreamState()" ;

		EXPECT_STREQ( respMsg.getState().getStatusText(), "Status Text" ) << "RefreshMsg::getState()::getStatusText()" ;

		EXPECT_TRUE( true ) << "RefreshMsg Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(RefreshMsgTests, testRefreshMsgWithOpaque)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslRefreshMsg refresh;
		rsslClearRefreshMsg( &refresh );

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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey( &refresh );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_OPAQUE;

		RefreshMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "respMsg.getPayload().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( respMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "respMsg.getPayload().getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg Decode with Opaque payload - exception not expected" ;
	}
}

TEST(RefreshMsgTests, testRefreshMsgWithXml)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslRefreshMsg refresh;
		rsslClearRefreshMsg( &refresh );

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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey( &refresh );

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_XML;

		RefreshMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::XmlEnum ) << "respMsg.getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( respMsg.getPayload().getXml().getBuffer(), compareTo ) << "respMsg.getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg Decode with Xml payload - exception not expected" ;
	}
}

TEST(RefreshMsgTests, testRefreshMsgWithAnsiPage)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslRefreshMsg refresh;
		rsslClearRefreshMsg( &refresh );

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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey( &refresh );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		RefreshMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "respMsg.getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( respMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "respMsg.getPayload().getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg Decode with AnsiPage payload - exception not expected" ;
	}
}

//encoding by EMA and decoding by EMA
TEST(RefreshMsgTests, testRefreshMsgEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	const EmaString refreshMsgString = 
		"RefreshMsg\n"
		"    streamId=\"1\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    state=\"Open / Ok / None / 'Status Text'\"\n"
		"    itemGroup=\"32 39\"\n"
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
		"RefreshMsgEnd\n";

	const EmaString refreshMsgEmptyString =
		"RefreshMsg\n"
		"    streamId=\"0\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    state=\"Open / Ok / None / ''\"\n"
		"    itemGroup=\"00 00\"\n"
		"RefreshMsgEnd\n";

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
		RefreshMsg refresh, refreshEmpty;

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		refresh.streamId( 1 );

		refresh.domainType( MMT_MARKET_PRICE );
		refresh.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		refresh.itemGroup( itemGroup );
		refresh.name( name );
//serviceName is only kept on the encoded RsslMsg
		refresh.serviceName( serviceName );
		refresh.nameType( 1 );
		refresh.id( 4 );
		refresh.filter( 8 );

		refresh.extendedHeader( extendedHeader );
		refresh.attrib( flEnc );
		refresh.payload( flEnc );
		EXPECT_EQ( refresh.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "RefreshMsg.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( refresh.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n") << "RefreshMsg.toString() == Dictionary is not loaded.";

		EXPECT_EQ( refreshEmpty.toString( emaDataDictionary ), refreshMsgEmptyString ) << "RefreshMsg.toString() == refreshMsgEmptyString";

		EXPECT_EQ( refresh.toString( emaDataDictionary ), refreshMsgString ) << "RefreshMsg.toString() == refreshMsgString";

		StaticDecoder::setData(&refresh, &dictionary);

		RefreshMsg refreshClone( refresh );
		refreshClone.clear();
		EXPECT_EQ( refreshClone.toString( emaDataDictionary ), refreshMsgEmptyString) << "RefreshMsg.toString() == refreshMsgEmptyString";

		EXPECT_EQ( refresh.toString(), refreshMsgString ) << "RefreshMsg.toString() == refreshMsgString";

		EXPECT_TRUE( refresh.hasMsgKey() ) << "RefreshMsg::hasMsgKey() == true" ;

		EXPECT_EQ( refresh.getStreamId(), 1 ) << "RefreshMsg::getStreamId()" ;

		EXPECT_EQ( refresh.getDomainType(), MMT_MARKET_PRICE ) << "RefreshMsg::getDomainType()" ;

		EXPECT_EQ( refresh.getState().getStreamState(), OmmState::OpenEnum ) << "RefreshMsg::getState().getStreamState()" ;
		EXPECT_EQ( refresh.getState().getDataState(), OmmState::OkEnum ) << "RefreshMsg::getState().getDataState()" ;
		EXPECT_EQ( refresh.getState().getStatusCode(), OmmState::NoneEnum ) << "RefreshMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( refresh.getState().getStatusText(), "Status Text" ) << "RefreshMsg::getState().getStatusText()" ;
		EXPECT_STREQ( refresh.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "RefreshMsg::getState().toString()" ;

		EXPECT_STREQ( refresh.getItemGroup(), itemGroup ) << "RefreshMsg::getItemGroup()" ;

		EXPECT_TRUE( refresh.hasName() ) << "RefreshMsg::hasName() == true" ;
		EXPECT_STREQ( refresh.getName(), "TRI.N" ) << "RefreshMsg::getName()" ;

		EXPECT_FALSE( refresh.hasServiceName() ) << "RefreshMsg::hasServiceName() == false" ;

		EXPECT_TRUE( refresh.hasNameType() ) << "RefreshMsg::hasNameType() == true" ;
		EXPECT_EQ( refresh.getNameType(), 1 ) << "RefreshMsg::getNameType()" ;

		EXPECT_FALSE( refresh.hasServiceId() ) << "RefreshMsg::hasServiceId() == false" ;

		EXPECT_TRUE( refresh.hasId() ) << "RefreshMsg::hasId() == true" ;
		EXPECT_EQ( refresh.getId(), 4 ) << "RefreshMsg::getId()" ;

		EXPECT_TRUE( refresh.hasFilter() ) << "RefreshMsg::hasFilter() == true" ;
		EXPECT_EQ( refresh.getFilter(), 8 ) << "RefreshMsg::getFilter()" ;

		EXPECT_TRUE( refresh.hasExtendedHeader() ) << "RefreshMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( refresh.getExtendedHeader(), extendedHeader ) << "RefreshMsg::getExtendedHeader()" ;

		EXPECT_EQ( refresh.getAttrib().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from respMsg
		const FieldList& flAttrib = static_cast<const FieldList&>( refresh.getAttrib().getData() );
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( refresh.getPayload().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getPayloadType()" ;
		//get FieldList (in payload) from respMsg
		const FieldList& flPayload = static_cast<const FieldList&>( refresh.getPayload().getData() );
		//decode FieldList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}


		refresh.clear();
		EmaBuffer itemGroup2( "30", 2 );
		EmaBuffer header2( "header2", 7 );
		refresh.state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Status Text2" );
		refresh.itemGroup( itemGroup2 );
		refresh.serviceId( 22 );
		refresh.id( 44 );
		refresh.filter( 88 );
		refresh.extendedHeader( header2 );
		refresh.attrib( flEnc );
		refresh.payload( flEnc );

		//2nd time (after refresh.clear())
		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &refresh, &dictionary );


		EXPECT_TRUE( refresh.hasMsgKey() ) << "RefreshMsg::hasMsgKey() == true" ;

		EXPECT_EQ( refresh.getStreamId(), 0 ) << "RefreshMsg::getStreamId()" ;

		EXPECT_EQ( refresh.getDomainType(), MMT_MARKET_PRICE ) << "RefreshMsg::getDomainType()" ;
		EXPECT_EQ( refresh.getState().getStreamState(), OmmState::ClosedEnum ) << "RefreshMsg::getState().getStreamState()" ;
		EXPECT_EQ( refresh.getState().getDataState(), OmmState::SuspectEnum ) << "RefreshMsg::getState().getDataState()" ;
		EXPECT_EQ( refresh.getState().getStatusCode(), OmmState::NoneEnum ) << "RefreshMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( refresh.getState().getStatusText(), "Status Text2" ) << "RefreshMsg::getState().getStatusText()" ;
		EXPECT_STREQ( refresh.getState().toString(), "Closed / Suspect / None / 'Status Text2'" ) << "RefreshMsg::getState().toString()" ;

		EXPECT_EQ( refresh.getItemGroup(), itemGroup2) << "RefreshMsg::getItemGroup()" ;

		EXPECT_FALSE( refresh.hasName() ) << "RefreshMsg::hasName() == false" ;
		EXPECT_FALSE( refresh.hasServiceName() ) << "RefreshMsg::hasServiceName() == false" ;

		EXPECT_TRUE( refresh.hasServiceId() ) << "RefreshMsg::hasServiceId() == true" ;
		EXPECT_EQ( refresh.getServiceId(), 22 ) << "RefreshMsg::getServiceId()" ;

		EXPECT_TRUE( refresh.hasId() ) << "RefreshMsg::hasId() == true" ;
		EXPECT_EQ( refresh.getId(), 44 ) << "RefreshMsg::getId()" ;

		EXPECT_TRUE( refresh.hasFilter() ) << "RefreshMsg::hasFilter() == true" ;
		EXPECT_EQ( refresh.getFilter(), 88 ) << "RefreshMsg::getFilter()" ;

		EXPECT_TRUE( refresh.hasExtendedHeader() ) << "RefreshMsg::hasExtendedHeader() == true" ;
		EXPECT_EQ( refresh.getExtendedHeader(), header2 ) << "RefreshMsg::getExtendedHeader()" ;

		EXPECT_EQ( refresh.getAttrib().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from respMsg
		const FieldList& flAttrib2 = static_cast<const FieldList&>( refresh.getAttrib().getData() );
		//decode FieldList (from attrib2)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib2 );
		}

		EXPECT_EQ( refresh.getPayload().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getLoad().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in payload) from respMsg
		const FieldList& flPayload2 = static_cast<const FieldList&>( refresh.getPayload().getData() );
		//decode FieldList (from payload2)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload2 );
		}


		EXPECT_TRUE( true ) << "RefreshMsg Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(RefreshMsgTests, testRefreshMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg refresh;

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		refresh.streamId( 1 );

		refresh.domainType( MMT_MARKET_PRICE );
		refresh.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		refresh.itemGroup( itemGroup );
		refresh.name( name );
//serviceName is only kept on the encoded RsslMsg
		refresh.serviceName( serviceName );
		refresh.nameType( 1 );
		refresh.id( 4 );
		refresh.filter( 8 );

		refresh.extendedHeader( extendedHeader );
		refresh.attrib( flEnc );
		refresh.payload( flEnc );


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &refresh, &dictionary );


		EXPECT_TRUE( refresh.hasMsgKey() ) << "RefreshMsg::hasMsgKey() == true" ;

		EXPECT_EQ( refresh.getStreamId(), 1 ) << "RefreshMsg::getStreamId()" ;

		EXPECT_EQ( refresh.getDomainType(), MMT_MARKET_PRICE ) << "RefreshMsg::getDomainType()" ;

		EXPECT_EQ( refresh.getState().getStreamState(), OmmState::OpenEnum ) << "RefreshMsg::getState().getStreamState()" ;
		EXPECT_EQ( refresh.getState().getDataState(), OmmState::OkEnum ) << "RefreshMsg::getState().getDataState()" ;
		EXPECT_EQ( refresh.getState().getStatusCode(), OmmState::NoneEnum ) << "RefreshMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( refresh.getState().getStatusText(), "Status Text" ) << "RefreshMsg::getState().getStatusText()" ;
		EXPECT_STREQ( refresh.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "RefreshMsg::getState().toString()" ;

		EXPECT_STREQ( refresh.getItemGroup(), itemGroup ) << "RefreshMsg::getItemGroup()" ;

		EXPECT_TRUE( refresh.hasName() ) << "RefreshMsg::hasName() == true" ;
		EXPECT_STREQ( refresh.getName(), "TRI.N" ) << "RefreshMsg::getName()" ;

		EXPECT_FALSE( refresh.hasServiceName() ) << "RefreshMsg::hasServiceName() == false" ;

		EXPECT_TRUE( refresh.hasNameType() ) << "RefreshMsg::hasNameType() == true" ;
		EXPECT_EQ( refresh.getNameType(), 1 ) << "RefreshMsg::getNameType()" ;

		EXPECT_FALSE( refresh.hasServiceId() ) << "RefreshMsg::hasServiceId() == false" ;

		EXPECT_TRUE( refresh.hasId() ) << "RefreshMsg::hasId() == true" ;
		EXPECT_EQ( refresh.getId(), 4 ) << "RefreshMsg::getId()" ;

		EXPECT_TRUE( refresh.hasFilter() ) << "RefreshMsg::hasFilter() == true" ;
		EXPECT_EQ( refresh.getFilter(), 8 ) << "RefreshMsg::getFilter()" ;

		EXPECT_TRUE( refresh.hasExtendedHeader() ) << "RefreshMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( refresh.getExtendedHeader(), extendedHeader ) << "RefreshMsg::getExtendedHeader()" ;

		EXPECT_EQ( refresh.getAttrib().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from respMsg
		const FieldList& flAttrib = refresh.getAttrib().getFieldList();
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( refresh.getPayload().getDataType(), DataType::FieldListEnum ) << "RefreshMsg::getLoad().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in payload) from respMsg
		const FieldList& flPayload = refresh.getPayload().getFieldList();
		//decode FieldList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}


		EXPECT_TRUE( true ) << "RefreshMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(RefreshMsgTests, testRefreshMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg refresh;

		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		refresh.streamId( 1 );

		refresh.domainType( MMT_MARKET_PRICE );
		refresh.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		refresh.itemGroup( itemGroup );
		refresh.name( name );
//serviceName is only kept on the encoded RsslMsg
		refresh.serviceName( serviceName );
		refresh.nameType( 1 );
		refresh.id( 4 );
		refresh.filter( 8 );

		refresh.extendedHeader( extendedHeader );
		refresh.attrib( elEnc );
		refresh.payload( elEnc );

		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &refresh, &dictionary );


		EXPECT_TRUE( refresh.hasMsgKey() ) << "RefreshMsg::hasMsgKey() == true" ;

		EXPECT_EQ( refresh.getStreamId(), 1 ) << "RefreshMsg::getStreamId()" ;

		EXPECT_EQ( refresh.getDomainType(), MMT_MARKET_PRICE ) << "RefreshMsg::getDomainType()" ;

		EXPECT_EQ( refresh.getState().getStreamState(), OmmState::OpenEnum ) << "RefreshMsg::getState().getStreamState()" ;
		EXPECT_EQ( refresh.getState().getDataState(), OmmState::OkEnum ) << "RefreshMsg::getState().getDataState()" ;
		EXPECT_EQ( refresh.getState().getStatusCode(), OmmState::NoneEnum ) << "RefreshMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( refresh.getState().getStatusText(), "Status Text" ) << "RefreshMsg::getState().getStatusText()" ;
		EXPECT_STREQ( refresh.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "RefreshMsg::getState().toString()" ;

		EXPECT_STREQ( refresh.getItemGroup(), itemGroup ) << "RefreshMsg::getItemGroup()" ;

		EXPECT_TRUE( refresh.hasName() ) << "RefreshMsg::hasName() == true" ;
		EXPECT_STREQ( refresh.getName(), "TRI.N" ) << "RefreshMsg::getName()" ;

		EXPECT_FALSE( refresh.hasServiceName() ) << "RefreshMsg::hasServiceName() == false" ;

		EXPECT_TRUE( refresh.hasNameType() ) << "RefreshMsg::hasNameType() == true" ;
		EXPECT_EQ( refresh.getNameType(), 1 ) << "RefreshMsg::getNameType()" ;

		EXPECT_FALSE( refresh.hasServiceId() ) << "RefreshMsg::hasServiceId() == false" ;

		EXPECT_TRUE( refresh.hasId() ) << "RefreshMsg::hasId() == true" ;
		EXPECT_EQ( refresh.getId(), 4 ) << "RefreshMsg::getId()" ;

		EXPECT_TRUE( refresh.hasFilter() ) << "RefreshMsg::hasFilter() == true" ;
		EXPECT_EQ( refresh.getFilter(), 8 ) << "RefreshMsg::getFilter()" ;

		EXPECT_TRUE( refresh.hasExtendedHeader() ) << "RefreshMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( refresh.getExtendedHeader(), extendedHeader ) << "RefreshMsg::getExtendedHeader()" ;

		EXPECT_EQ( refresh.getAttrib().getDataType(), DataType::ElementListEnum ) << "RefreshMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;
		//get ElementList (in attrib) from refreshMsg
		const ElementList& elAttrib = refresh.getAttrib().getElementList();
		//decode ElementList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		EXPECT_EQ( refresh.getPayload().getDataType(), DataType::ElementListEnum ) << "RefreshMsg::getLoad().getDataType() == DataType::ElementListEnum" ;
		//get ElementList (in payload) from refreshMsg
		const ElementList& elPayload = refresh.getPayload().getElementList();
		//decode ElementList (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}


		EXPECT_TRUE( true ) << "RefreshMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(RefreshMsgTests, testRefreshMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg refresh;

		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		refresh.streamId( 1 );

		refresh.domainType( MMT_MARKET_PRICE );
		refresh.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		refresh.itemGroup( itemGroup );
		refresh.name( name );
//serviceName is only kept on the encoded RsslMsg
		refresh.serviceName( serviceName );
		refresh.nameType( 1 );
		refresh.id( 4 );
		refresh.filter( 8 );

		refresh.extendedHeader( extendedHeader );
		refresh.attrib( mapEnc );
		refresh.payload( mapEnc );

		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &refresh, &dictionary );


		EXPECT_TRUE( refresh.hasMsgKey() ) << "RefreshMsg::hasMsgKey() == true" ;

		EXPECT_EQ( refresh.getStreamId(), 1 ) << "RefreshMsg::getStreamId()" ;

		EXPECT_EQ( refresh.getDomainType(), MMT_MARKET_PRICE ) << "RefreshMsg::getDomainType()" ;

		EXPECT_EQ( refresh.getState().getStreamState(), OmmState::OpenEnum ) << "RefreshMsg::getState().getStreamState()" ;
		EXPECT_EQ( refresh.getState().getDataState(), OmmState::OkEnum ) << "RefreshMsg::getState().getDataState()" ;
		EXPECT_EQ( refresh.getState().getStatusCode(), OmmState::NoneEnum ) << "RefreshMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( refresh.getState().getStatusText(), "Status Text" ) << "RefreshMsg::getState().getStatusText()" ;
		EXPECT_STREQ( refresh.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "RefreshMsg::getState().toString()" ;

		EXPECT_STREQ( refresh.getItemGroup(), itemGroup ) << "RefreshMsg::getItemGroup()" ;

		EXPECT_TRUE( refresh.hasName() ) << "RefreshMsg::hasName() == true" ;
		EXPECT_STREQ( refresh.getName(), "TRI.N" ) << "RefreshMsg::getName()" ;

		EXPECT_FALSE( refresh.hasServiceName() ) << "RefreshMsg::hasServiceName() == false" ;

		EXPECT_TRUE( refresh.hasNameType() ) << "RefreshMsg::hasNameType() == true" ;
		EXPECT_EQ( refresh.getNameType(), 1 ) << "RefreshMsg::getNameType()" ;

		EXPECT_FALSE( refresh.hasServiceId() ) << "RefreshMsg::hasServiceId() == false" ;

		EXPECT_TRUE( refresh.hasId() ) << "RefreshMsg::hasId() == true" ;
		EXPECT_EQ( refresh.getId(), 4 ) << "RefreshMsg::getId()" ;

		EXPECT_TRUE( refresh.hasFilter() ) << "RefreshMsg::hasFilter() == true" ;
		EXPECT_EQ( refresh.getFilter(), 8 ) << "RefreshMsg::getFilter()" ;

		EXPECT_TRUE( refresh.hasExtendedHeader() ) << "RefreshMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( refresh.getExtendedHeader(), extendedHeader ) << "RefreshMsg::getExtendedHeader()" ;

		EXPECT_EQ( refresh.getAttrib().getDataType(), DataType::MapEnum ) << "RefreshMsg::getAttrib().getDataType() == DataType::MapEnum" ;
		//get Map (in attrib) from refreshMsg
		const Map& mapAttrib = refresh.getAttrib().getMap();
		//decode Map (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		EXPECT_EQ( refresh.getPayload().getDataType(), DataType::MapEnum ) << "RefreshMsg::getLoad().getDataType() == DataType::MapEnum" ;
		//get Map (in payload) from respMsg
		const Map& mapPayload = refresh.getPayload().getMap();
		//decode Map (from payload)
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}


		EXPECT_TRUE( true ) << "RefreshMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}



TEST(RefreshMsgTests, testRefreshMsgtoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslRefreshMsg refresh;

		rsslClearRefreshMsg( &refresh );

		RsslMsgKey msgKey;

		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("TRI.N");
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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey( &refresh );

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

		refresh.state = rsslState;

		RefreshMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		EXPECT_TRUE( true ) << "RefreshMsg toString Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "RefreshMsg toString Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(RefreshMsgTests, testRefreshMsgEncodeDecodeQos)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		char groupId[] = "3";
		RsslUInt32 groupIdLen = sizeof(groupId) / sizeof(char);

		RsslRefreshMsg refresh;
		rsslClearRefreshMsg(&refresh);

		refresh.msgBase.streamId = 2;
		refresh.msgBase.domainType = RSSL_DMT_LOGIN;

		refresh.groupId.data = groupId;
		refresh.groupId.length = groupIdLen;

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

		char rsslBufChar[1000];
		RsslBuffer rsslBuf;
		rsslBuf.length = sizeof(rsslBufChar) / sizeof(char);
		rsslBuf.data = rsslBufChar;

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;

		rsslMsgKeyApplyHasAttrib(&msgKey);

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey(&refresh);

		refresh.groupId.data = groupId;
		refresh.groupId.length = groupIdLen;

		refresh.msgBase.encDataBody.data = 0;
		refresh.msgBase.encDataBody.length = 0;
		refresh.msgBase.containerType = RSSL_DT_NO_DATA;

		RsslState rsslState;
		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;
		rsslState.text = statusText;
		refresh.state = rsslState;

		refresh.msgBase.encDataBody.data = 0;
		refresh.msgBase.encDataBody.length = 0;


		RsslEncodeIterator encIter;
		RsslDecodeIterator decodeIter;

		char msgBufChar[2048];
		RsslBuffer msgBuf;
		msgBuf.length = sizeof(msgBufChar) / sizeof(char);
		msgBuf.data = msgBufChar;

		RsslMsg refreshDecode;
		RefreshMsg refreshMsg;

		/////
		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_FALSE(refreshMsg.hasQos()) << "RefreshMsg::hasQos() == false";

		try
		{
			refreshMsg.getQos();
			EXPECT_FALSE(true) << "RefreshMsg::getQos() - exception expected";
		}
		catch (const OmmException&)
		{
			EXPECT_TRUE(true) << "RefreshMsg::getQos() - exception expected";
		}

		// Copy test
		RefreshMsg copyRefreshMsg(refreshMsg);
		EXPECT_EQ(refreshMsg.hasQos(), copyRefreshMsg.hasQos()) << "Compare copy hasQos() when Qos is empty";

		/////
		RsslQos qos;
		qos.timeliness = RSSL_QOS_TIME_DELAYED;
		qos.timeInfo = 5000;
		qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		qos.rateInfo = 6000;
		refresh.qos = qos;
		refresh.flags |= RSSL_RFMF_HAS_QOS;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_TRUE(refreshMsg.hasQos()) << "RefreshMsg::hasQos() == true";

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), 5000) << "RefreshMsg::getQos().getTimeliness()== 5000";
		EXPECT_EQ(refreshMsg.getQos().getRate(), 6000) << "RefreshMsg::getQos().getRate()== 6000 ";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "Timeliness: 5000") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "Rate: 6000") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "Timeliness: 5000/Rate: 6000") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg1(refreshMsg);
		EXPECT_EQ(copyRefreshMsg1.hasQos(), refreshMsg.hasQos()) << "Compare copy1 hasQos()";
		EXPECT_EQ(copyRefreshMsg1.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy1 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg1.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy1 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg1.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy1  RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg1.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy1 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg1.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy1 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_REALTIME;
		qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), OmmQos::RealTimeEnum) << "RefreshMsg::getQos().getTimeliness()== OmmQos::RealTimeEnum";
		EXPECT_EQ(refreshMsg.getQos().getRate(), OmmQos::TickByTickEnum) << "RefreshMsg::getQos().getRate()== OmmQos::TickByTickEnum";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "RealTime") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "TickByTick") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "RealTime/TickByTick") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg2(refreshMsg);
		EXPECT_EQ(copyRefreshMsg2.hasQos(), refreshMsg.hasQos()) << "Compare copy2 hasQos()";
		EXPECT_EQ(copyRefreshMsg2.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy2 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg2.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy2 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg2.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy2  RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg2.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy2 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg2.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy2 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), OmmQos::InexactDelayedEnum) << "RefreshMsg::getQos().getTimeliness()== OmmQos::InexactDelayedEnum";
		EXPECT_EQ(refreshMsg.getQos().getRate(), OmmQos::JustInTimeConflatedEnum) << "RefreshMsg::getQos().getRate()== OmmQos::JustInTimeConflatedEnum";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "InexactDelayed") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "JustInTimeConflated") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "InexactDelayed/JustInTimeConflated") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg3(refreshMsg);
		EXPECT_EQ(copyRefreshMsg3.hasQos(), refreshMsg.hasQos()) << "Compare copy3 hasQos()";
		EXPECT_EQ(copyRefreshMsg3.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy3 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg3.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy3 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg3.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy3  RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg3.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy3 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg3.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy3 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_DELAYED;
		qos.timeInfo = 340;
		qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), 340) << "RefreshMsg::getQos().getTimeliness()== 340";
		EXPECT_EQ(refreshMsg.getQos().getRate(), OmmQos::JustInTimeConflatedEnum) << "RefreshMsg::getQos().getRate()== OmmQos::JustInTimeConflatedEnum";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "Timeliness: 340") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "JustInTimeConflated") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "Timeliness: 340/JustInTimeConflated") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg4(refreshMsg);
		EXPECT_EQ(copyRefreshMsg4.hasQos(), refreshMsg.hasQos()) << "Compare copy4 hasQos()";
		EXPECT_EQ(copyRefreshMsg4.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy4 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg4.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy4 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg4.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy4  RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg4.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy4 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg4.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy4 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		qos.rateInfo = 203;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), OmmQos::InexactDelayedEnum) << "RefreshMsg::getQos().getTimeliness()== OmmQos::InexactDelayedEnum";
		EXPECT_EQ(refreshMsg.getQos().getRate(), 203) << "RefreshMsg::getQos().getRate()== 203";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "InexactDelayed") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "Rate: 203") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "InexactDelayed/Rate: 203") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg5(refreshMsg);
		EXPECT_EQ(copyRefreshMsg5.hasQos(), refreshMsg.hasQos()) << "Compare copy5 hasQos()";
		EXPECT_EQ(copyRefreshMsg5.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy5 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg5.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy5 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg5.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy5  RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg5.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy5 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg5.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy5 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_REALTIME;
		qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), OmmQos::RealTimeEnum) << "RefreshMsg::getQos().getTimeliness()== OmmQos::RealTimeEnum";
		EXPECT_EQ(refreshMsg.getQos().getRate(), OmmQos::JustInTimeConflatedEnum) << "RefreshMsg::getQos().getRate()== OmmQos::JustInTimeConflatedEnum";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "RealTime") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "JustInTimeConflated") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "RealTime/JustInTimeConflated") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg6(refreshMsg);
		EXPECT_EQ(copyRefreshMsg6.hasQos(), refreshMsg.hasQos()) << "Compare copy6 hasQos()";
		EXPECT_EQ(copyRefreshMsg6.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy6 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg6.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy6 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg6.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy6 RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg6.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy6 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg6.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy6 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_REALTIME;
		qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		qos.rateInfo = 450;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), OmmQos::RealTimeEnum) << "RefreshMsg::getQos().getTimeliness()== OmmQos::RealTimeEnum";
		EXPECT_EQ(refreshMsg.getQos().getRate(), 450) << "RefreshMsg::getQos().getRate()== 450";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "RealTime") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "Rate: 450") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "RealTime/Rate: 450") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg7(refreshMsg);
		EXPECT_EQ(copyRefreshMsg7.hasQos(), refreshMsg.hasQos()) << "Compare copy7 hasQos()";
		EXPECT_EQ(copyRefreshMsg7.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy7 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg7.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy7 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg7.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy7 RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg7.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy7 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg7.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy7 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), OmmQos::InexactDelayedEnum) << "RefreshMsg::getQos().getTimeliness()== OmmQos::InexactDelayedEnum";
		EXPECT_EQ(refreshMsg.getQos().getRate(), OmmQos::TickByTickEnum) << "RefreshMsg::getQos().getRate()==OmmQos::TickByTickEnum";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "InexactDelayed") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "TickByTick") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "InexactDelayed/TickByTick") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg8(refreshMsg);
		EXPECT_EQ(copyRefreshMsg8.hasQos(), refreshMsg.hasQos()) << "Compare copy8 hasQos()";
		EXPECT_EQ(copyRefreshMsg8.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy8 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg8.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy8 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg8.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy8 RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg8.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy8 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg8.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy8 RefreshMsg::getQos().toString()";

		/////
		qos.timeliness = RSSL_QOS_TIME_DELAYED;
		qos.timeInfo = 29300;
		qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refresh.qos = qos;

		prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, refreshMsg, dictionary);

		EXPECT_EQ(refreshMsg.getQos().getTimeliness(), 29300) << "RefreshMsg::getQos().getTimeliness()== 29300";
		EXPECT_EQ(refreshMsg.getQos().getRate(), OmmQos::TickByTickEnum) << "RefreshMsg::getQos().getRate()==OmmQos::TickByTickEnum";
		EXPECT_STREQ(refreshMsg.getQos().getTimelinessAsString(), "Timeliness: 29300") << "RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(refreshMsg.getQos().getRateAsString(), "TickByTick") << "RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(refreshMsg.getQos().toString(), "Timeliness: 29300/TickByTick") << "RefreshMsg::getQos().toString()";

		RefreshMsg copyRefreshMsg9(refreshMsg);
		EXPECT_EQ(copyRefreshMsg9.hasQos(), refreshMsg.hasQos()) << "Compare copy9 hasQos()";
		EXPECT_EQ(copyRefreshMsg9.getQos().getTimeliness(), refreshMsg.getQos().getTimeliness()) << "Compare copy9 RefreshMsg::getQos().getTimeliness()";
		EXPECT_EQ(copyRefreshMsg9.getQos().getRate(), refreshMsg.getQos().getRate()) << "Compare copy9 RefreshMsg::getQos().getRate() ";
		EXPECT_STREQ(copyRefreshMsg9.getQos().getTimelinessAsString(), refreshMsg.getQos().getTimelinessAsString()) << "Compare copy9 RefreshMsg::getQos().getTimelinessAsString()";
		EXPECT_STREQ(copyRefreshMsg9.getQos().getRateAsString(), refreshMsg.getQos().getRateAsString()) << "Compare copy9 RefreshMsg::getQos().getRateAsString()";
		EXPECT_STREQ(copyRefreshMsg9.getQos().toString(), refreshMsg.getQos().toString()) << "Compare copy9 RefreshMsg::getQos().toString()";

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "RefreshMsg Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(RefreshMsgTests, testRefreshMsgClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslRefreshMsg refresh;

		rsslClearRefreshMsg(&refresh);

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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey(&refresh);

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

		refresh.state = rsslState;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&refresh);

		RsslMsg refreshDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&refreshDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		RefreshMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&refreshDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		RefreshMsg cloneRefreshMsg(respMsg);

		EXPECT_TRUE(cloneRefreshMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneRefreshMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneRefreshMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneRefreshMsg.toString()) << "Check equal toString()";

		EXPECT_TRUE(true) << "RefreshMsg Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "RefreshMsg Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(RefreshMsgTests, testRefreshMsgEditClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslRefreshMsg refresh;

		rsslClearRefreshMsg(&refresh);

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

		refresh.msgBase.msgKey = msgKey;
		rsslRefreshMsgApplyHasMsgKey(&refresh);

		refresh.msgBase.encDataBody = rsslBuf;
		refresh.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

		refresh.state = rsslState;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&refresh);

		RsslMsg refreshDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&refreshDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		RefreshMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&refreshDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		RefreshMsg cloneRefreshMsg(respMsg);

		EXPECT_TRUE(cloneRefreshMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneRefreshMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneRefreshMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneRefreshMsg.toString()) << "Check equal toString()";

		// Edit message
		cloneRefreshMsg.streamId(10);

		StaticDecoder::setData(&cloneRefreshMsg, &dictionary);

		EXPECT_FALSE(cloneRefreshMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(respMsg.toString(), cloneRefreshMsg.toString()) << "Check not equal toString()";
		EXPECT_TRUE(true) << "RefreshMsg Edit Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "RefreshMsg Edit Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(RefreshMsgTests, testRefreshMsgCloneMsgKeyPermissionData)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	char groupId[] = "3";
	const RsslUInt32 groupIdLen = sizeof(groupId) / sizeof(char);

	char stateText[] = "Source Unavailable";
	const RsslUInt32 stateTextLen = sizeof(stateText) / sizeof(char);

	char permissionData[] = "permission access to important data";
	const RsslUInt32 permissionDataLen = sizeof(permissionData) / sizeof(char);
	
	RsslUInt32 seqNum;

	RsslUInt16 flagsTest[] = {
		RSSL_RFMF_NONE,
		RSSL_RFMF_HAS_MSG_KEY,
		RSSL_RFMF_HAS_PERM_DATA,
		RSSL_RFMF_HAS_SEQ_NUM,
		RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_HAS_PERM_DATA | RSSL_RFMF_HAS_SEQ_NUM,
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
			RsslRefreshMsg refresh;
			rsslClearRefreshMsg(&refresh);

			refresh.msgBase.streamId = 2;
			refresh.msgBase.domainType = RSSL_DMT_LOGIN;
			refresh.flags = flag;

			refresh.state.streamState = RSSL_STREAM_OPEN;
			refresh.state.dataState = RSSL_DATA_SUSPECT;
			refresh.state.code = RSSL_SC_NO_RESOURCES;
			refresh.state.text.length = stateTextLen;
			refresh.state.text.data = stateText;

			refresh.groupId.data = groupId;
			refresh.groupId.length = groupIdLen;

			RsslMsgKey msgKey;
			rsslClearMsgKey(&msgKey);

			RsslBuffer rsslBuf = RSSL_INIT_BUFFER;

			if (flag & RSSL_RFMF_HAS_MSG_KEY)
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

				refresh.msgBase.msgKey = msgKey;
				rsslRefreshMsgApplyHasMsgKey(&refresh);

				refresh.msgBase.encDataBody = rsslBuf;
				refresh.msgBase.containerType = RSSL_DT_FIELD_LIST;
			}
			else {
				refresh.msgBase.encDataBody.data = 0;
				refresh.msgBase.encDataBody.length = 0;
				refresh.msgBase.containerType = RSSL_DT_NO_DATA;
			}

			/* Add Permission Info */
			if (flag & RSSL_RFMF_HAS_PERM_DATA)
			{
				refresh.permData.length = permissionDataLen;
				refresh.permData.data = permissionData;
			}

			/* Add Item Sequence Number */
			if (flag & RSSL_RFMF_HAS_SEQ_NUM)
			{
				seqNum = i;
				refresh.seqNum = seqNum;
			}

			RsslBuffer msgBuf;
			msgBuf.length = sizeof(msgBufData) / sizeof(char);
			msgBuf.data = msgBufData;

			RsslMsg refreshDecode;
			RefreshMsg respMsg;

			/////
			prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&refresh, decodeIter, (RsslMsg*)&refreshDecode, respMsg, dictionary);

			// Clone message
			RefreshMsg cloneRefreshMsg(respMsg);

			EXPECT_EQ(cloneRefreshMsg.getDomainType(), respMsg.getDomainType()) << "Compare domainType";
			EXPECT_EQ(cloneRefreshMsg.getDomainType(), RSSL_DMT_LOGIN) << "Compare domainType: should be equal to " << RSSL_DMT_LOGIN;

			EXPECT_EQ(cloneRefreshMsg.getStreamId(), respMsg.getStreamId()) << "Compare streamId";
			EXPECT_EQ(cloneRefreshMsg.getStreamId(), 2) << "Compare streamId: should be equal to 2";

			EXPECT_EQ(cloneRefreshMsg.hasMsgKey(), respMsg.hasMsgKey()) << "Compare hasMsgKey";
			EXPECT_EQ(cloneRefreshMsg.hasName(), respMsg.hasName()) << "Compare hasName";
			EXPECT_EQ(cloneRefreshMsg.hasNameType(), respMsg.hasNameType()) << "Compare hasNameType";
			EXPECT_EQ(cloneRefreshMsg.hasServiceId(), respMsg.hasServiceId()) << "Compare hasServiceId";
			EXPECT_EQ(cloneRefreshMsg.hasId(), respMsg.hasId()) << "Compare hasId";
			EXPECT_EQ(cloneRefreshMsg.hasFilter(), respMsg.hasFilter()) << "Compare hasFilter";
			EXPECT_EQ(cloneRefreshMsg.hasExtendedHeader(), respMsg.hasExtendedHeader()) << "Compare hasExtendedHeader";

			EXPECT_EQ(cloneRefreshMsg.hasQos(), respMsg.hasQos()) << "Compare hasQos";
			EXPECT_EQ(cloneRefreshMsg.hasSeqNum(), respMsg.hasSeqNum()) << "Compare hasSeqNum";
			EXPECT_EQ(cloneRefreshMsg.hasPartNum(), respMsg.hasPartNum()) << "Compare hasPartNum";
			EXPECT_EQ(cloneRefreshMsg.hasPermissionData(), respMsg.hasPermissionData()) << "Compare hasPermissionData";
			EXPECT_EQ(cloneRefreshMsg.hasPublisherId(), respMsg.hasPublisherId()) << "Compare hasPublisherId";
			EXPECT_EQ(cloneRefreshMsg.hasServiceName(), respMsg.hasServiceName()) << "Compare hasServiceName";

			EXPECT_STREQ(respMsg.toString(), cloneRefreshMsg.toString()) << "Check equal toString()";

			EXPECT_EQ(cloneRefreshMsg.hasMsgKey(), (flag & RSSL_RFMF_HAS_MSG_KEY)>0) << "Compare hasMsgKey: " << (flag & RSSL_RFMF_HAS_MSG_KEY);
			if (cloneRefreshMsg.hasMsgKey() && respMsg.hasMsgKey())
			{
				if (cloneRefreshMsg.hasServiceId())
				{
					EXPECT_EQ(cloneRefreshMsg.getServiceId(), respMsg.getServiceId()) << "Compare serviceId";
				}
				if (cloneRefreshMsg.hasName())
				{
					EXPECT_EQ(cloneRefreshMsg.getName(), respMsg.getName()) << "Compare name";
				}
				if (cloneRefreshMsg.hasNameType())
				{
					EXPECT_EQ(cloneRefreshMsg.getNameType(), respMsg.getNameType()) << "Compare nameType";
				}
				if (cloneRefreshMsg.hasId())
				{
					EXPECT_EQ(cloneRefreshMsg.getId(), respMsg.getId()) << "Compare id";
				}
				if (cloneRefreshMsg.hasFilter())
				{
					EXPECT_EQ(cloneRefreshMsg.getFilter(), respMsg.getFilter()) << "Compare filter";
				}
			}

			EXPECT_EQ(cloneRefreshMsg.hasSeqNum(), (flag & RSSL_RFMF_HAS_SEQ_NUM)>0) << "Compare hasSeqNum: " << (flag & RSSL_RFMF_HAS_SEQ_NUM);
			if (cloneRefreshMsg.hasSeqNum())
			{
				EXPECT_EQ(cloneRefreshMsg.getSeqNum(), respMsg.getSeqNum()) << "Compare SeqNum";
				EXPECT_EQ(seqNum, cloneRefreshMsg.getSeqNum()) << "Compare SeqNum: " << seqNum;
			}

			EXPECT_EQ(cloneRefreshMsg.hasPermissionData(), (flag & RSSL_RFMF_HAS_PERM_DATA)>0) << "Compare hasPermissionData: " << (flag & RSSL_RFMF_HAS_PERM_DATA);
			if (cloneRefreshMsg.hasPermissionData())
			{
				const EmaBuffer& permDataOrig = cloneRefreshMsg.getPermissionData();
				const EmaBuffer& permDataCopy = respMsg.getPermissionData();
				EmaBuffer permData(permissionData, permissionDataLen);

				EXPECT_EQ(permDataOrig.length(), permDataCopy.length()) << "Compare length of EmaBuffer Permission Data";
				EXPECT_EQ(permDataOrig.length(), permissionDataLen) << "Compare length of EmaBuffer Permission Data: " << permissionDataLen;
				EXPECT_EQ(permDataOrig, permDataCopy) << "Compare EmaBuffer Permission Data";
				EXPECT_EQ(permData, permDataCopy) << "Compare EmaBuffer Permission Data: " << permissionData;
			}

			EXPECT_TRUE(true) << "RefreshMsg Clone Success";

		}
		catch (const OmmException&)
		{
			EXPECT_FALSE(true) << "RefreshMsg Clone - exception not expected";
		}
	}

	rsslDeleteDataDictionary(&dictionary);
}
