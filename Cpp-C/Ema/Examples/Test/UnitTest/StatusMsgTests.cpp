/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

TEST(StatusMsgTests, testStatusMsgInStatusMsg)
{

	try
	{
		StatusMsg sMsg;

		sMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" )
		.payload(
		  StatusMsg().state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "HPStatus Text" ) );

		StaticDecoder::setData( &sMsg, 0 );


		EXPECT_TRUE( true ) << "StatusMsg in StatusMsg - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg in StatusMsg - exception not expected" ;
	}
}

//encoding by UPA and decoding by EMA
TEST(StatusMsgTests, testStatusMsgDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslStatusMsg status;

		rsslClearStatusMsg( &status );

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

		status.msgBase.msgKey = msgKey;
		rsslStatusMsgApplyHasMsgKey( &status );

		status.msgBase.encDataBody = rsslBuf;
		status.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState state;

		state.code = RSSL_SC_NONE;
		state.dataState = RSSL_DATA_OK;
		state.streamState = RSSL_STREAM_OPEN;
		state.text.data = 0;
		state.text.length = 0;

		status.state = state;
		status.flags |= RSSL_STMF_HAS_STATE;

		StatusMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&status, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( respMsg.hasMsgKey() ) << "StatusMsg::hasMsgKey() == true" ;

		EXPECT_TRUE( respMsg.hasName() ) << "StatusMsg::hasName() == true" ;

		EXPECT_STREQ( respMsg.getName(), "ABCDEF" ) << "StatusMsg::getName()" ;

		EXPECT_TRUE( respMsg.hasNameType() ) << "StatusMsg::hasNameType() == true" ;

		EXPECT_EQ( respMsg.getNameType(), 1 ) << "StatusMsg::getNameType()" ;

		EXPECT_TRUE( respMsg.hasServiceId() ) << "StatusMsg::hasServiceId() == true" ;

		EXPECT_EQ( respMsg.getServiceId(), 2 ) << "StatusMsg::getServiceId()" ;

		EXPECT_TRUE( respMsg.hasId() ) << "StatusMsg::hasId() == true" ;

		EXPECT_EQ( respMsg.getId(), 4 ) << "StatusMsg::getId()" ;

		EXPECT_TRUE( respMsg.hasFilter() ) << "StatusMsg::hasFilter() == true" ;

		EXPECT_EQ( respMsg.getFilter(), 8 ) << "StatusMsg::getFilter()" ;

		EXPECT_EQ( respMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "StatusMsg::getAttribType()" ;

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "StatusMsg::getPayloadType()" ;

		EXPECT_EQ( respMsg.getState().getStatusCode(), OmmState::NoneEnum ) << "StatusMsg::getState()::getCode()" ;

		EXPECT_EQ( respMsg.getState().getDataState(), OmmState::OkEnum ) << "StatusMsg::getState()::getDataState()" ;

		EXPECT_EQ( respMsg.getState().getStreamState(), OmmState::OpenEnum ) << "StatusMsg::getState()::getStreamState()" ;

		EXPECT_TRUE( true ) << "StatusMsg Decode - exception not expected" ;
		
		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(StatusMsgTests, testStatusMsgNoState)
{

	try
	{
		RsslStatusMsg status;

		rsslClearStatusMsg( &status );

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

		status.msgBase.msgKey = msgKey;
		rsslStatusMsgApplyHasMsgKey( &status );

		status.msgBase.encDataBody.data = 0;
		status.msgBase.encDataBody.length  = 0;
		status.msgBase.containerType = RSSL_DT_NO_DATA;

		StatusMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&status, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_TRUE( respMsg.hasMsgKey() ) << "StatusMsg::hasMsgKey() == true" ;

		EXPECT_TRUE( respMsg.hasName() ) << "StatusMsg::hasName() == true" ;

		EXPECT_STREQ( respMsg.getName(), "ABCDEF" ) << "StatusMsg::getName()" ;

		EXPECT_TRUE( respMsg.hasNameType() ) << "StatusMsg::hasNameType() == true" ;

		EXPECT_EQ( respMsg.getNameType(), 1 ) << "StatusMsg::getNameType()" ;

		EXPECT_TRUE( respMsg.hasServiceId() ) << "StatusMsg::hasServiceId() == true" ;

		EXPECT_EQ( respMsg.getServiceId(), 2 ) << "StatusMsg::getServiceId()" ;

		EXPECT_TRUE( respMsg.hasId() ) << "StatusMsg::hasId() == true" ;

		EXPECT_EQ( respMsg.getId(), 4 ) << "StatusMsg::getId()" ;

		EXPECT_TRUE( respMsg.hasFilter() ) << "StatusMsg::hasFilter() == true" ;

		EXPECT_EQ( respMsg.getFilter(), 8 ) << "StatusMsg::getFilter()" ;

		EXPECT_EQ( respMsg.getAttrib().getDataType(), DataType::NoDataEnum ) << "StatusMsg::getAttribType()" ;

		EXPECT_EQ( respMsg.getPayload().getDataType(), DataType::NoDataEnum ) << "StatusMsg::getPayloadType()" ;

		EXPECT_FALSE( respMsg.hasState() ) << "StatusMsg::hasState() == false" ;

		EXPECT_TRUE( true ) << "StatusMsg NoState - exception not expected" ;

		try
		{
			respMsg.getState();
			EXPECT_FALSE( true ) << "StatusMsg NoState - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "StatusMsg NoState - exception expected" ;
		}

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg NoState - exception not expected" ;
	}
}

TEST(StatusMsgTests, testStatusMsgContainsOpaqueDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslStatusMsg rsslStatusMsg;
		rsslClearStatusMsg( &rsslStatusMsg );

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

		rsslStatusMsg.msgBase.msgKey = msgKey;
		rsslStatusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32> ( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		rsslStatusMsg.msgBase.encDataBody = rsslBuf;
		rsslStatusMsg.msgBase.containerType = RSSL_DT_OPAQUE;

		StatusMsg statusMsg;

		StaticDecoder::setRsslData( &statusMsg, ( RsslMsg* )&rsslStatusMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( statusMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "StatusMsg::getPayload().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( statusMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "StatusMsg::getPayload().getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Decode with Opaque payload - exception not expected" ;
	}
}

TEST(StatusMsgTests, testStatusMsgContainsXmlDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslStatusMsg rsslStatusMsg;
		rsslClearStatusMsg( &rsslStatusMsg );

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

		rsslStatusMsg.msgBase.msgKey = msgKey;
		rsslStatusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32> ( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		rsslStatusMsg.msgBase.encDataBody = rsslBuf;
		rsslStatusMsg.msgBase.containerType = RSSL_DT_XML;

		StatusMsg statusMsg;

		StaticDecoder::setRsslData( &statusMsg, ( RsslMsg* )&rsslStatusMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( statusMsg.getPayload().getDataType(), DataType::XmlEnum ) << "StatusMsg::getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( statusMsg.getPayload().getXml().getBuffer(), compareTo ) << "StatusMsg::getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg Decode with Xml payload - exception not expected" ;
	}
}

TEST(StatusMsgTests, testStatusMsgContainsAnsiPageDecodeAll)
{

	RsslDataDictionary dictionary;

	try
	{
		RsslStatusMsg rsslStatusMsg;
		rsslClearStatusMsg( &rsslStatusMsg );

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

		rsslStatusMsg.msgBase.msgKey = msgKey;
		rsslStatusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32> ( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		rsslStatusMsg.msgBase.encDataBody = rsslBuf;
		rsslStatusMsg.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		StatusMsg statusMsg;

		StaticDecoder::setRsslData( &statusMsg, ( RsslMsg* )&rsslStatusMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( statusMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "StatusMsg::getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( statusMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "StatusMsg::getPayload().getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg Decode with AnsiPage payload - exception not expected" ;
	}
}

//encoding by EMA and decoding by EMA
TEST(StatusMsgTests, testStatusMsgEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		StatusMsg status;

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		status.streamId( 3 );

		status.domainType( MMT_MARKET_PRICE );
		status.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		status.itemGroup( itemGroup );  //itemGroup is ok for status
		status.name( name );
//serviceName is only kept on the encoded RsslMsg
		status.serviceName( serviceName );
		status.nameType( 1 );
		status.id( 4 );
		status.filter( 8 );

		status.extendedHeader( extendedHeader );
		status.attrib( flEnc );
		status.payload( flEnc );  //there is no payload for status
		EXPECT_EQ( status.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "StatusMsg.toString() == Decoding of just encoded object in the same application is not supported";


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &status, &dictionary );
		EXPECT_NE( status.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "StatusMsg.toString() != Decoding of just encoded object in the same application is not supported";


		EXPECT_TRUE( status.hasMsgKey() ) << "StatusMsg::hasMsgKey() == true" ;

		EXPECT_EQ( status.getStreamId(), 3 ) << "StatusMsg::getStreamId()" ;

		EXPECT_EQ( status.getDomainType(), MMT_MARKET_PRICE ) << "StatusMsg::getDomainType()" ;

		EXPECT_EQ( status.getState().getStreamState(), OmmState::OpenEnum ) << "StatusMsg::getState().getStreamState()" ;
		EXPECT_EQ( status.getState().getDataState(), OmmState::OkEnum ) << "StatusMsg::getState().getDataState()" ;
		EXPECT_EQ( status.getState().getStatusCode(), OmmState::NoneEnum ) << "StatusMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( status.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "StatusMsg::getState().toString()" ;

		EXPECT_TRUE( status.hasItemGroup() ) << "StatusMsg::hasItemGroup() == true" ;

		EXPECT_TRUE( status.hasName() ) << "StatusMsg::hasName() == true" ;
		EXPECT_STREQ( status.getName(), "TRI.N" ) << "StatusMsg::getName()" ;

		EXPECT_FALSE( status.hasServiceName() ) << "StatusMsg::hasServiceName() == false" ;

		EXPECT_TRUE( status.hasNameType() ) << "StatusMsg::hasNameType() == true" ;
		EXPECT_EQ( status.getNameType(), 1 ) << "StatusMsg::getNameType()" ;

		EXPECT_FALSE( status.hasServiceId() ) << "StatusMsg::hasServiceId() == false" ;

		EXPECT_TRUE( status.hasId() ) << "StatusMsg::hasId() == true" ;
		EXPECT_EQ( status.getId(), 4 ) << "StatusMsg::getId()" ;

		EXPECT_TRUE( status.hasFilter() ) << "StatusMsg::hasFilter() == true" ;
		EXPECT_EQ( status.getFilter(), 8 ) << "StatusMsg::getFilter()" ;

		EXPECT_TRUE( status.hasExtendedHeader() ) << "StatusMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( status.getExtendedHeader(), extendedHeader ) << "StatusMsg::getExtendedHeader()" ;

		EXPECT_EQ( status.getAttrib().getDataType(), DataType::FieldListEnum ) << "StatusMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from statusMsg
		const FieldList& flAttrib = static_cast<const FieldList&>( status.getAttrib().getData() );
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_TRUE( true ) << "StatusMsg Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(StatusMsgTests, testStatusMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		StatusMsg status;

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		status.streamId( 3 );

		status.domainType( MMT_MARKET_PRICE );
		status.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		status.itemGroup( itemGroup );
		status.name( name );
		status.serviceName( serviceName );
		status.nameType( 1 );
		status.id( 4 );
		status.filter( 8 );

		status.extendedHeader( extendedHeader );
		status.attrib( flEnc );
		status.payload( flEnc );  //there is no payload for status


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &status, &dictionary );


		EXPECT_TRUE( status.hasMsgKey() ) << "StatusMsg::hasMsgKey() == true" ;

		EXPECT_EQ( status.getStreamId(), 3 ) << "StatusMsg::getStreamId()" ;

		EXPECT_EQ( status.getDomainType(), MMT_MARKET_PRICE ) << "StatusMsg::getDomainType()" ;

		EXPECT_EQ( status.getState().getStreamState(), OmmState::OpenEnum ) << "StatusMsg::getState().getStreamState()" ;
		EXPECT_EQ( status.getState().getDataState(), OmmState::OkEnum ) << "StatusMsg::getState().getDataState()" ;
		EXPECT_EQ( status.getState().getStatusCode(), OmmState::NoneEnum ) << "StatusMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( status.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "StatusMsg::getState().toString()" ;

		EXPECT_TRUE( status.hasItemGroup() ) << "StatusMsg::hasItemGroup() == true" ;

		EXPECT_TRUE( status.hasName() ) << "StatusMsg::hasName() == true" ;
		EXPECT_STREQ( status.getName(), "TRI.N" ) << "StatusMsg::getName()" ;

		EXPECT_FALSE( status.hasServiceName() ) << "StatusMsg::hasServiceName() == false" ;

		EXPECT_TRUE( status.hasNameType() ) << "StatusMsg::hasNameType() == true" ;
		EXPECT_EQ( status.getNameType(), 1 ) << "StatusMsg::getNameType()" ;

		EXPECT_FALSE( status.hasServiceId() ) << "StatusMsg::hasServiceId() == false" ;

		EXPECT_TRUE( status.hasId() ) << "StatusMsg::hasId() == true" ;
		EXPECT_EQ( status.getId(), 4 ) << "StatusMsg::getId()" ;

		EXPECT_TRUE( status.hasFilter() ) << "StatusMsg::hasFilter() == true" ;
		EXPECT_EQ( status.getFilter(), 8 ) << "StatusMsg::getFilter()" ;

		EXPECT_TRUE( status.hasExtendedHeader() ) << "StatusMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( status.getExtendedHeader(), extendedHeader ) << "StatusMsg::getExtendedHeader()" ;

		EXPECT_EQ( status.getAttrib().getDataType(), DataType::FieldListEnum ) << "StatusMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;
		//get FieldList (in attrib) from statusMsg
		const FieldList& flAttrib = status.getAttrib().getFieldList();
		//decode FieldList (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_TRUE( true ) << "StatusMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(StatusMsgTests, testStatusMsgElementListEncodeDecode)
{

	try
	{
		StatusMsg status;

		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		status.streamId( 3 );

		status.domainType( MMT_MARKET_PRICE );
		status.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		status.itemGroup( itemGroup );
		status.name( name );
		status.serviceName( serviceName );
		status.nameType( 1 );
		status.id( 4 );
		status.filter( 8 );

		status.extendedHeader( extendedHeader );
		status.attrib( elEnc );
		status.payload( elEnc );

		StaticDecoder::setData( &status, 0 );


		EXPECT_TRUE( status.hasMsgKey() ) << "StatusMsg::hasMsgKey() == true" ;

		EXPECT_EQ( status.getStreamId(), 3 ) << "StatusMsg::getStreamId()" ;

		EXPECT_EQ( status.getDomainType(), MMT_MARKET_PRICE ) << "StatusMsg::getDomainType()" ;

		EXPECT_EQ( status.getState().getStreamState(), OmmState::OpenEnum ) << "StatusMsg::getState().getStreamState()" ;
		EXPECT_EQ( status.getState().getDataState(), OmmState::OkEnum ) << "StatusMsg::getState().getDataState()" ;
		EXPECT_EQ( status.getState().getStatusCode(), OmmState::NoneEnum ) << "StatusMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( status.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "StatusMsg::getState().toString()" ;

		EXPECT_TRUE( status.hasItemGroup() ) << "StatusMsg::hasItemGroup() == true" ;

		EXPECT_TRUE( status.hasName() ) << "StatusMsg::hasName() == true" ;
		EXPECT_STREQ( status.getName(), "TRI.N" ) << "StatusMsg::getName()" ;

		EXPECT_FALSE( status.hasServiceName() ) << "StatusMsg::hasServiceName() == false" ;

		EXPECT_TRUE( status.hasNameType() ) << "StatusMsg::hasNameType() == true" ;
		EXPECT_EQ( status.getNameType(), 1 ) << "StatusMsg::getNameType()" ;

		EXPECT_FALSE( status.hasServiceId() ) << "StatusMsg::hasServiceId() == false" ;

		EXPECT_TRUE( status.hasId() ) << "StatusMsg::hasId() == true" ;
		EXPECT_EQ( status.getId(), 4 ) << "StatusMsg::getId()" ;

		EXPECT_TRUE( status.hasFilter() ) << "StatusMsg::hasFilter() == true" ;
		EXPECT_EQ( status.getFilter(), 8 ) << "StatusMsg::getFilter()" ;

		EXPECT_TRUE( status.hasExtendedHeader() ) << "StatusMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( status.getExtendedHeader(), extendedHeader ) << "StatusMsg::getExtendedHeader()" ;

		EXPECT_EQ( status.getAttrib().getDataType(), DataType::ElementListEnum ) << "StatusMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;

		const ElementList& flAttrib = status.getAttrib().getElementList();


		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( flAttrib );
		}


		EXPECT_TRUE( true ) << "StatusMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg ElementList Encode and Decode - exception not expected" ;
	}
}

TEST(StatusMsgTests, testStatusMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		StatusMsg status;

		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		status.streamId( 3 );

		status.domainType( MMT_MARKET_PRICE );
		status.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		status.itemGroup( itemGroup );
		status.name( name );
//serviceName is only kept on the encoded RsslMsg
		status.serviceName( serviceName );
		status.nameType( 1 );
		status.id( 4 );
		status.filter( 8 );

		status.extendedHeader( extendedHeader );
		status.attrib( mapEnc );
		status.payload( mapEnc );  //there is no payload for status


		//Now do EMA decoding of RespMsg
		StaticDecoder::setData( &status, &dictionary );


		EXPECT_TRUE( status.hasMsgKey() ) << "StatusMsg::hasMsgKey() == true" ;

		EXPECT_EQ( status.getStreamId(), 3 ) << "StatusMsg::getStreamId()" ;

		EXPECT_EQ( status.getDomainType(), MMT_MARKET_PRICE ) << "StatusMsg::getDomainType()" ;

		EXPECT_EQ( status.getState().getStreamState(), OmmState::OpenEnum ) << "StatusMsg::getState().getStreamState()" ;
		EXPECT_EQ( status.getState().getDataState(), OmmState::OkEnum ) << "StatusMsg::getState().getDataState()" ;
		EXPECT_EQ( status.getState().getStatusCode(), OmmState::NoneEnum ) << "StatusMsg::getState().getStatusCode()" ;
		EXPECT_STREQ( status.getState().toString(), "Open / Ok / None / 'Status Text'" ) << "StatusMsg::getState().toString()" ;

		EXPECT_TRUE( status.hasItemGroup() ) << "StatusMsg::hasItemGroup() == true" ;

		EXPECT_TRUE( status.hasName() ) << "StatusMsg::hasName() == true" ;
		EXPECT_STREQ( status.getName(), "TRI.N" ) << "StatusMsg::getName()" ;

		EXPECT_FALSE( status.hasServiceName() ) << "StatusMsg::hasServiceName() == false" ;

		EXPECT_TRUE( status.hasNameType() ) << "StatusMsg::hasNameType() == true" ;
		EXPECT_EQ( status.getNameType(), 1 ) << "StatusMsg::getNameType()" ;

		EXPECT_FALSE( status.hasServiceId() ) << "StatusMsg::hasServiceId() == false" ;

		EXPECT_TRUE( status.hasId() ) << "StatusMsg::hasId() == true" ;
		EXPECT_EQ( status.getId(), 4 ) << "StatusMsg::getId()" ;

		EXPECT_TRUE( status.hasFilter() ) << "StatusMsg::hasFilter() == true" ;
		EXPECT_EQ( status.getFilter(), 8 ) << "StatusMsg::getFilter()" ;

		EXPECT_TRUE( status.hasExtendedHeader() ) << "StatusMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( status.getExtendedHeader(), extendedHeader ) << "StatusMsg::getExtendedHeader()" ;

		EXPECT_EQ( status.getAttrib().getDataType(), DataType::MapEnum ) << "StatusMsg::getAttrib().getDataType() == DataType::MapEnum" ;
		//get Map (in attrib) from statusMsg
		const Map& mapAttrib = status.getAttrib().getMap();
		//decode Map (from attrib)
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		EXPECT_TRUE( true ) << "StatusMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(StatusMsgTests, testStatusMsgtoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslStatusMsg status;

		rsslClearStatusMsg( &status );

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

		status.msgBase.msgKey = msgKey;
		rsslStatusMsgApplyHasMsgKey( &status );

		status.msgBase.encDataBody = rsslBuf;
		status.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>( "Status Text" );
		statusText.length = 11;


		StatusMsg respMsg;

		StaticDecoder::setRsslData( &respMsg, ( RsslMsg* )&status, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		EXPECT_TRUE( true ) << "StatusMsg toString Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "StatusMsg toString Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(StatusMsgTests, testStatusMsgError)
{

	{
		try
		{
			StatusMsg msg;

			ElementList attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "StatusMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "StatusMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			StatusMsg msg;

			RefreshMsg attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "StatusMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "StatusMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

	{
		try
		{
			StatusMsg msg;

			ElementList load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "StatusMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "StatusMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			StatusMsg msg;

			RefreshMsg load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "StatusMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "StatusMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

}

TEST(StatusMsgTests, testStatusMsgClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslStatusMsg status;

		rsslClearStatusMsg(&status);

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

		status.msgBase.msgKey = msgKey;
		rsslStatusMsgApplyHasMsgKey(&status);

		status.msgBase.encDataBody = rsslBuf;
		status.msgBase.containerType = RSSL_DT_FIELD_LIST;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&status);

		RsslMsg statusDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&statusDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		StatusMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&statusDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		StatusMsg cloneStatusMsg(respMsg);

		EXPECT_TRUE(cloneStatusMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneStatusMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneStatusMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneStatusMsg.toString()) << "Check equal toString()";

		EXPECT_TRUE(true) << "StatusMsg Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "StatusMsg Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(StatusMsgTests, testStatusMsgEditClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslStatusMsg status;

		rsslClearStatusMsg(&status);

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

		status.msgBase.msgKey = msgKey;
		rsslStatusMsgApplyHasMsgKey(&status);

		status.msgBase.encDataBody = rsslBuf;
		status.msgBase.containerType = RSSL_DT_FIELD_LIST;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&status);

		RsslMsg statusDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&statusDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		StatusMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&statusDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		StatusMsg cloneStatusMsg(respMsg);

		EXPECT_TRUE(cloneStatusMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneStatusMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneStatusMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneStatusMsg.toString()) << "Check equal toString()";

		// Edit message
		cloneStatusMsg.streamId(10);

		StaticDecoder::setData(&cloneStatusMsg, &dictionary);

		EXPECT_FALSE(cloneStatusMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(respMsg.toString(), cloneStatusMsg.toString()) << "Check not equal toString()";
		EXPECT_TRUE(true) << "StatusMsg Edit Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "StatusMsg Edit Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}
