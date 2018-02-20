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

//encoding by UPA and decoding by EMA
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
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "UpdateMsg Hybrid Usage - exception not expected" ;
	}
}

