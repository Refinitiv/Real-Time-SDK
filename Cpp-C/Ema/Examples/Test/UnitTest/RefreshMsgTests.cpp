/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright Thomson Reuters 2018-2019. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

//encoding by UPA and decoding by EMA
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
		EXPECT_EQ( refresh.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "RefreshMsg.toString() == Decoding of just encoded object in the same application is not supported";


		//1st time
		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &refresh, &dictionary );
		EXPECT_NE( refresh.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "RefreshMsg.toString() != Decoding of just encoded object in the same application is not supported";


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

	RsslEncodeIterator rsslEncodeIter;
	rsslClearEncodeIterator( &rsslEncodeIter );
	RsslRefreshMsg refresh;
	char buffer[1024];
	RsslBuffer rsslBuffer;
	rsslBuffer.data = buffer;
	rsslBuffer.length = 1024;

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

	msgKey.attribContainerType = RSSL_DT_NO_DATA;
	rsslMsgKeyApplyHasAttrib( &msgKey );

	refresh.msgBase.msgKey = msgKey;
	rsslRefreshMsgApplyHasMsgKey( &refresh );

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

	RefreshMsg refreshMsg;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_FALSE( refreshMsg.hasQos() ) << "RefreshMsg::hasQos() == false" ;

	try
	{
		refreshMsg.getQos();
		EXPECT_FALSE( true ) << "RefreshMsg::getQos() - exception expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "RefreshMsg::getQos() - exception expected" ;
	}

	RsslQos qos;
	qos.timeliness = RSSL_QOS_TIME_DELAYED;
	qos.timeInfo = 5000;
	qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qos.rateInfo = 6000;
	refresh.qos = qos;
	refresh.flags |= RSSL_RFMF_HAS_QOS;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), 5000 ) << "RefreshMsg::getQos().getTimeliness()== 5000" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), 6000 ) << "RefreshMsg::getQos().getRate()== 6000 " ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "Timeliness: 5000" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "Rate: 6000" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "Timeliness: 5000/Rate: 6000" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_REALTIME;
	qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "RefreshMsg::getQos().getTimeliness()== OmmQos::RealTimeEnum" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), OmmQos::TickByTickEnum ) << "RefreshMsg::getQos().getRate()== OmmQos::TickByTickEnum" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "RealTime" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "TickByTick" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "RealTime/TickByTick" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "RefreshMsg::getQos().getTimeliness()== OmmQos::InexactDelayedEnum" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "RefreshMsg::getQos().getRate()== OmmQos::JustInTimeConflatedEnum" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "InexactDelayed" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "JustInTimeConflated" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "InexactDelayed/JustInTimeConflated" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_DELAYED;
	qos.timeInfo = 340;
	qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), 340 ) << "RefreshMsg::getQos().getTimeliness()== 340" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "RefreshMsg::getQos().getRate()== OmmQos::JustInTimeConflatedEnum" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "Timeliness: 340" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "JustInTimeConflated" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "Timeliness: 340/JustInTimeConflated" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qos.rateInfo = 203;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "RefreshMsg::getQos().getTimeliness()== OmmQos::InexactDelayedEnum" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), 203 ) << "RefreshMsg::getQos().getRate()== 203" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "InexactDelayed" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "Rate: 203" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "InexactDelayed/Rate: 203" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_REALTIME;
	qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "RefreshMsg::getQos().getTimeliness()== OmmQos::RealTimeEnum" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "RefreshMsg::getQos().getRate()== OmmQos::JustInTimeConflatedEnum" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "RealTime" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "JustInTimeConflated" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "RealTime/JustInTimeConflated" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_REALTIME;
	qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qos.rateInfo = 450;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "RefreshMsg::getQos().getTimeliness()== OmmQos::RealTimeEnum" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), 450 ) << "RefreshMsg::getQos().getRate()== 450" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "RealTime" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "Rate: 450" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "RealTime/Rate: 450" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "RefreshMsg::getQos().getTimeliness()== OmmQos::InexactDelayedEnum" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), OmmQos::TickByTickEnum ) << "RefreshMsg::getQos().getRate()==OmmQos::TickByTickEnum" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "InexactDelayed" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "TickByTick" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "InexactDelayed/TickByTick" ) << "RefreshMsg::getQos().toString()" ;

	qos.timeliness = RSSL_QOS_TIME_DELAYED;
	qos.timeInfo = 29300;
	qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refresh.qos = qos;

	StaticDecoder::setRsslData( &refreshMsg, ( RsslMsg* )&refresh, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MAJOR_VERSION, 0 );

	EXPECT_EQ( refreshMsg.getQos().getTimeliness(), 29300 ) << "RefreshMsg::getQos().getTimeliness()== 29300" ;
	EXPECT_EQ( refreshMsg.getQos().getRate(), OmmQos::TickByTickEnum ) << "RefreshMsg::getQos().getRate()==OmmQos::TickByTickEnum" ;
	EXPECT_STREQ( refreshMsg.getQos().getTimelinessAsString(), "Timeliness: 29300" ) << "RefreshMsg::getQos().getTimelinessAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().getRateAsString(), "TickByTick" ) << "RefreshMsg::getQos().getRateAsString()" ;
	EXPECT_STREQ( refreshMsg.getQos().toString(), "Timeliness: 29300/TickByTick" ) << "RefreshMsg::getQos().toString()" ;
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
