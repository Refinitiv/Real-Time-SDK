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

// encoding by ETA and decoding by EMA
TEST(PostMsgTests, testPostMsgDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslPostMsg post;

		rsslClearPostMsg( &post );

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

		post.msgBase.msgKey = msgKey;
		rsslPostMsgApplyHasMsgKey( &post );

		post.msgBase.encDataBody = rsslBuf;
		post.msgBase.containerType = RSSL_DT_FIELD_LIST;

		post.msgBase.domainType = 6;

		post.partNum = 16;
		rsslPostMsgApplyHasPartNum( &post );

		post.postId = 17;
		rsslPostMsgApplyHasPostId( &post );

		post.seqNum = 12;
		rsslPostMsgApplyHasSeqNum( &post );

		rsslPostMsgApplyPostComplete( &post );

		post.postUserInfo.postUserAddr = 01;
		post.postUserInfo.postUserId = 02;

		post.postUserRights = RSSL_PSUR_CREATE | RSSL_PSUR_DELETE | RSSL_PSUR_MODIFY_PERM;
		rsslPostMsgApplyHasPostUserRights( &post );

		PostMsg postMsg;

		StaticDecoder::setRsslData( &postMsg, ( RsslMsg* )&post, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_TRUE( postMsg.hasMsgKey() ) << "PostMsg::hasMsgKey() == true" ;

		EXPECT_TRUE( postMsg.hasName() ) << "PostMsg::hasName() == true" ;

		EXPECT_STREQ( postMsg.getName(), "ABCDEF" ) << "PostMsg::getName()" ;

		EXPECT_TRUE( postMsg.hasNameType() ) << "PostMsg::hasNameType() == true" ;

		EXPECT_EQ( postMsg.getNameType(), 1 ) << "PostMsg::getNameType()" ;

		EXPECT_TRUE( postMsg.hasServiceId() ) << "PostMsg::hasServiceId() == true" ;

		EXPECT_EQ( postMsg.getServiceId(), 2 ) << "PostMsg::getServiceId()" ;

		EXPECT_TRUE( postMsg.hasId() ) << "PostMsg::hasId() == true" ;

		EXPECT_EQ( postMsg.getId(), 4 ) << "PostMsg::getId()" ;

		EXPECT_TRUE( postMsg.hasFilter() ) << "PostMsg::hasFilter() == true" ;

		EXPECT_EQ( postMsg.getFilter(), 8 ) << "PostMsg::getFilter()" ;

		EXPECT_EQ( postMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "PostMsg::getAttribType()" ;

		EXPECT_EQ( postMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "PostMsg::getPayloadType()" ;

		EXPECT_EQ( postMsg.getDomainType(), MMT_MARKET_PRICE ) << "PostMsg::getDomainType()" ;

		EXPECT_TRUE( postMsg.hasPartNum() ) << "PostMsg::hasPartNum()" ;

		EXPECT_EQ( postMsg.getPartNum(), 16 ) << "PostMsg::getPartNum()" ;

		EXPECT_TRUE( postMsg.hasPostId() ) << "PostMsg::hasPostId()" ;

		EXPECT_EQ( postMsg.getPostId(), 17 ) << "PostMsg::getPostId()" ;

		EXPECT_TRUE( postMsg.hasSeqNum() ) << "PostMsg::hasSeqNum()" ;

		EXPECT_EQ( postMsg.getSeqNum(), 12 ) << "PostMsg::getSeqNum()" ;

		EXPECT_TRUE( postMsg.getComplete() ) << "PostMsg::getComplete()" ;

		EXPECT_EQ( postMsg.getPublisherIdUserAddress(), 01 ) << "PostMsg::getPublisherIdUserAddress()" ;

		EXPECT_EQ( postMsg.getPublisherIdUserId(), 02 ) << "PostMsg::getPublisherIdUserId()" ;

		EXPECT_TRUE( postMsg.hasPostUserRights() ) << "PostMsg::hasPostUserRights()" ;

		EXPECT_TRUE( ( postMsg.getPostUserRights() & PostMsg::CreateEnum ) != 0 ) << "PostMsg::getPostUserRights() & PostMsg::CreateEnum" ;

		EXPECT_TRUE( ( postMsg.getPostUserRights() & PostMsg::DeleteEnum ) != 0 ) << "PostMsg::getPostUserRights() & PostMsg::DeleteEnum" ;

		EXPECT_TRUE( ( postMsg.getPostUserRights() & PostMsg::ModifyPermissionEnum ) != 0 ) << "PostMsg::getPostUserRights() & PostMsg::ModifyPermissionEnum" ;

		EXPECT_TRUE( true ) << "PostMsg Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "PostMsg Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(PostMsgTests, testPostMsgFieldListEncodeDecode)
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
		PostMsg post, postEmpty;

		const EmaString postMsgString =
			"PostMsg\n"
			"    streamId=\"1\"\n"
			"    domain=\"MarketPrice Domain\"\n"
			"    publisherIdUserId=\"0\"\n"
			"    publisherIdUserAddress=\"0\"\n"
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
			"PostMsgEnd\n";

		const EmaString postMsgEmptyString =
			"PostMsg\n"
			"    streamId=\"0\"\n"
			"    domain=\"MarketPrice Domain\"\n"
			"    publisherIdUserId=\"0\"\n"
			"    publisherIdUserAddress=\"0\"\n"
			"PostMsgEnd\n";

		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		post.streamId( 1 );

		post.domainType( MMT_MARKET_PRICE );
		post.name( name );
		post.serviceName( serviceName );
		post.nameType( 1 );
		post.id( 4 );
		post.filter( 8 );

		post.extendedHeader( extendedHeader );
		post.attrib( flEnc );
		post.payload( flEnc );
		EXPECT_EQ( post.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n") << "PostMsg.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( post.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "PostMsg.toString() == Dictionary is not loaded.";

		EXPECT_EQ( postEmpty.toString(emaDataDictionary), postMsgEmptyString ) << "PostMsg.toString() == postMsgEmptyString";

		EXPECT_EQ( post.toString( emaDataDictionary ), postMsgString ) << "PostMsg.toString() == postMsgString";

		StaticDecoder::setData(&post, &dictionary);

		PostMsg postClone( post );
		postClone.clear();
		EXPECT_EQ( postClone.toString( emaDataDictionary ), postMsgEmptyString ) << "PostMsg.toString() == postMsgEmptyString";

		EXPECT_EQ( post.toString(), postMsgString ) << "PostMsg.toString() == postMsgString";

		EXPECT_TRUE( post.hasMsgKey() ) << "PostMsg::hasMsgKey() == true" ;

		EXPECT_EQ( post.getStreamId(), 1 ) << "PostMsg::getStreamId()" ;

		EXPECT_EQ( post.getDomainType(), MMT_MARKET_PRICE ) << "PostMsg::getDomainType()" ;

		EXPECT_TRUE( post.hasName() ) << "PostMsg::hasName() == true" ;
		EXPECT_STREQ( post.getName(), "TRI.N" ) << "PostMsg::getName()" ;

		EXPECT_FALSE( post.hasServiceName() ) << "PostMsg::hasServiceName() == false" ;

		EXPECT_TRUE( post.hasNameType() ) << "PostMsg::hasNameType() == true" ;
		EXPECT_EQ( post.getNameType(), 1 ) << "PostMsg::getNameType()" ;

		EXPECT_FALSE( post.hasServiceId() ) << "PostMsg::hasServiceId() == false" ;

		EXPECT_TRUE( post.hasId() ) << "PostMsg::hasId() == true" ;
		EXPECT_EQ( post.getId(), 4 ) << "PostMsg::getId()" ;

		EXPECT_TRUE( post.hasFilter() ) << "PostMsg::hasFilter() == true" ;
		EXPECT_EQ( post.getFilter(), 8 ) << "PostMsg::getFilter()" ;

		EXPECT_TRUE( post.hasExtendedHeader() ) << "PostMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( post.getExtendedHeader(), extendedHeader ) << "PostMsg::getExtendedHeader()" ;

		EXPECT_FALSE( post.hasPartNum() ) << "PostMsg::hasPartNum()" ;

		EXPECT_FALSE( post.hasPostId() ) << "PostMsg::hasPostId()" ;

		EXPECT_FALSE( post.hasSeqNum() ) << "PostMsg::hasSeqNum()" ;

		EXPECT_FALSE( post.getComplete() ) << "PostMsg::getComplete()" ;

		EXPECT_EQ( post.getPublisherIdUserAddress(), 0 ) << "PostMsg::getPublisherIdUserAddress()" ;

		EXPECT_EQ( post.getPublisherIdUserId(), 0 ) << "PostMsg::getPublisherIdUserId()" ;

		EXPECT_FALSE( post.hasPostUserRights() ) << "PostMsg::hasPostUserRights()" ;

		EXPECT_EQ( post.getAttrib().getDataType(), DataType::FieldListEnum ) << "PostMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;

		const FieldList& flAttrib = static_cast<const FieldList&>( post.getAttrib().getData() );

		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( post.getPayload().getDataType(), DataType::FieldListEnum ) << "PostMsg::getPayloadType()" ;

		const FieldList& flPayload = static_cast<const FieldList&>( post.getPayload().getData() );

		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		post.clear();
		EmaBuffer header2( "header2", 7 );
		post.serviceId( 22 );
		post.id( 44 );
		post.filter( 88 );
		post.extendedHeader( header2 );
		post.attrib( flEnc );
		post.payload( flEnc );

		StaticDecoder::setData( &post, &dictionary );


		EXPECT_TRUE( post.hasMsgKey() ) << "PostMsg::hasMsgKey() == true" ;

		EXPECT_EQ( post.getStreamId(), 0 ) << "PostMsg::getStreamId()" ;

		EXPECT_EQ( post.getDomainType(), MMT_MARKET_PRICE ) << "PostMsg::getDomainType()" ;

		EXPECT_FALSE( post.hasName() ) << "PostMsg::hasName() == false" ;
		EXPECT_FALSE( post.hasServiceName() ) << "PostMsg::hasServiceName() == false" ;

		EXPECT_TRUE( post.hasServiceId() ) << "PostMsg::hasServiceId() == true" ;
		EXPECT_EQ( post.getServiceId(), 22 ) << "PostMsg::getServiceId()" ;

		EXPECT_TRUE( post.hasId() ) << "PostMsg::hasId() == true" ;
		EXPECT_EQ( post.getId(), 44 ) << "PostMsg::getId()" ;

		EXPECT_TRUE( post.hasFilter() ) << "PostMsg::hasFilter() == true" ;
		EXPECT_EQ( post.getFilter(), 88 ) << "PostMsg::getFilter()" ;

		EXPECT_TRUE( post.hasExtendedHeader() ) << "PostMsg::hasExtendedHeader() == true" ;
		EXPECT_TRUE( post.getExtendedHeader() == header2 ) << "PostMsg::getExtendedHeader()" ;

		EXPECT_FALSE( post.hasPartNum() ) << "PostMsg::hasPartNum()" ;

		EXPECT_FALSE( post.hasPostId() ) << "PostMsg::hasPostId()" ;

		EXPECT_FALSE( post.hasSeqNum() ) << "PostMsg::hasSeqNum()" ;

		EXPECT_FALSE( post.getComplete() ) << "PostMsg::getComplete()" ;

		EXPECT_EQ( post.getPublisherIdUserAddress(), 0 ) << "PostMsg::getPublisherIdUserAddress()" ;

		EXPECT_EQ( post.getPublisherIdUserId(), 0 ) << "PostMsg::getPublisherIdUserId()" ;

		EXPECT_FALSE( post.hasPostUserRights() ) << "PostMsg::hasPostUserRights()" ;

		EXPECT_EQ( post.getAttrib().getDataType(), DataType::FieldListEnum ) << "PostMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;

		const FieldList& flAttrib2 = static_cast<const FieldList&>( post.getAttrib().getData() );

		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib2 );
		}

		EXPECT_EQ( post.getPayload().getDataType(), DataType::FieldListEnum ) << "PostMsg::getLoad().getDataType() == DataType::FieldListEnum" ;

		const FieldList& flPayload2 = static_cast<const FieldList&>( post.getPayload().getData() );

		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload2 );
		}

		EXPECT_TRUE( true ) << "PostMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "PostMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(PostMsgTests, testPostMsgElementListEncodeDecode)
{

	try
	{
		PostMsg post;

		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaString serviceName( "DIRECT_FEED" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );

		post.streamId( 1 );

		post.domainType( MMT_MARKET_PRICE );
		post.name( name );
		post.serviceName( serviceName );
		post.nameType( 1 );
		post.id( 4 );
		post.filter( 8 );

		post.extendedHeader( extendedHeader );
		post.attrib( elEnc );
		post.payload( elEnc );

		StaticDecoder::setData( &post, 0 );


		EXPECT_TRUE( post.hasMsgKey() ) << "PostMsg::hasMsgKey() == true" ;

		EXPECT_EQ( post.getStreamId(), 1 ) << "PostMsg::getStreamId()" ;

		EXPECT_EQ( post.getDomainType(), MMT_MARKET_PRICE ) << "PostMsg::getDomainType()" ;

		EXPECT_TRUE( post.hasName() ) << "PostMsg::hasName() == true" ;
		EXPECT_STREQ( post.getName(), "TRI.N" ) << "PostMsg::getName()" ;

		EXPECT_FALSE( post.hasServiceName() ) << "PostMsg::hasServiceName() == false" ;

		EXPECT_TRUE( post.hasNameType() ) << "PostMsg::hasNameType() == true" ;
		EXPECT_EQ( post.getNameType(), 1 ) << "PostMsg::getNameType()" ;

		EXPECT_FALSE( post.hasServiceId() ) << "PostMsg::hasServiceId() == false" ;

		EXPECT_TRUE( post.hasId() ) << "PostMsg::hasId() == true" ;
		EXPECT_EQ( post.getId(), 4 ) << "PostMsg::getId()" ;

		EXPECT_TRUE( post.hasFilter() ) << "PostMsg::hasFilter() == true" ;
		EXPECT_EQ( post.getFilter(), 8 ) << "PostMsg::getFilter()" ;

		EXPECT_TRUE( post.hasExtendedHeader() ) << "PostMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( post.getExtendedHeader(), extendedHeader ) << "PostMsg::getExtendedHeader()" ;

		EXPECT_EQ( post.getAttrib().getDataType(), DataType::ElementListEnum ) << "PostMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;

		const ElementList& elAttrib = post.getAttrib().getElementList();

		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		EXPECT_EQ( post.getPayload().getDataType(), DataType::ElementListEnum ) << "PostMsg::getLoad().getDataType() == DataType::ElementListEnum" ;

		const ElementList& elPayload = post.getPayload().getElementList();

		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}

		EXPECT_TRUE( true ) << "PostMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "PostMsg ElementList Encode and Decode - exception not expected" ;
	}
}

TEST(PostMsgTests, testPostMsgtoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslPostMsg post;

		rsslClearPostMsg( &post );

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

		post.msgBase.msgKey = msgKey;
		rsslPostMsgApplyHasMsgKey( &post );

		post.msgBase.encDataBody = rsslBuf;
		post.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

		PostMsg postMsg;

		StaticDecoder::setRsslData( &postMsg, ( RsslMsg* )&post, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );


		EXPECT_TRUE( true ) << "PostMsg toString Decode - exception not expected" ;

		rsslBuf.length = 0;
		free(rsslBuf.data);

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "PostMsg toString Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(PostMsgTests, testPostMsgError)
{

	{
		try
		{
			PostMsg msg;

			ElementList attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "PostMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "PostMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			PostMsg msg;

			RefreshMsg attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "PostMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "PostMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

	{
		try
		{
			PostMsg msg;

			ElementList load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "PostMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "PostMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			PostMsg msg;

			RefreshMsg load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "PostMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "PostMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception expected" ;
		}
	}

}

TEST(PostMsgTests, testPostMsgClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslPostMsg post;

		rsslClearPostMsg(&post);

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

		post.msgBase.msgKey = msgKey;
		rsslPostMsgApplyHasMsgKey(&post);

		post.msgBase.encDataBody = rsslBuf;
		post.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&post);

		RsslMsg postDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&postDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		PostMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&postDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		PostMsg clonePostMsg(respMsg);

		EXPECT_TRUE(clonePostMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(clonePostMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(clonePostMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), clonePostMsg.toString()) << "Check equal toString()";

		EXPECT_TRUE(true) << "PostMsg Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "PostMsg Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(PostMsgTests, testPostMsgEditClone)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";

	try
	{
		RsslPostMsg post;

		rsslClearPostMsg(&post);

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

		post.msgBase.msgKey = msgKey;
		rsslPostMsgApplyHasMsgKey(&post);

		post.msgBase.encDataBody = rsslBuf;
		post.msgBase.containerType = RSSL_DT_FIELD_LIST;

		RsslState rsslState;

		rsslState.code = RSSL_SC_INVALID_ARGUMENT;
		rsslState.dataState = RSSL_DATA_NO_CHANGE;
		rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;

		RsslBuffer statusText;
		statusText.data = const_cast<char*>("Status Text");
		statusText.length = 11;

		rsslState.text = statusText;

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

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&post);

		RsslMsg postDecode;
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
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&postDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << endl;
		}

		PostMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&postDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		PostMsg clonePostMsg(respMsg);

		EXPECT_TRUE(clonePostMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(clonePostMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(clonePostMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), clonePostMsg.toString()) << "Check equal toString()";

		// Edit message
		clonePostMsg.streamId(10);

		StaticDecoder::setData(&clonePostMsg, &dictionary);

		EXPECT_FALSE(clonePostMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(respMsg.toString(), clonePostMsg.toString()) << "Check not equal toString()";

		EXPECT_TRUE(true) << "PostMsg Edit Clone Success";

		rsslBuf.length = 0;
		free(rsslBuf.data);

		msgBuf.length = 0;
		free(msgBuf.data);

	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "PostMsg Edit Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(PostMsgTests, testPostMsgCloneMsgKeyPermissionData)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	char permissionData[] = "permission access to important data";
	const RsslUInt32 permissionDataLen = sizeof(permissionData) / sizeof(char);

	RsslUInt32 seqNum;

	RsslUInt16 flagsTest[] = {
		RSSL_PSMF_NONE,
		RSSL_PSMF_HAS_MSG_KEY,
		RSSL_PSMF_HAS_PERM_DATA,
		RSSL_PSMF_HAS_SEQ_NUM,
		RSSL_PSMF_HAS_PART_NUM,
		RSSL_PSMF_HAS_MSG_KEY | RSSL_PSMF_HAS_PERM_DATA | RSSL_PSMF_HAS_SEQ_NUM | RSSL_PSMF_HAS_PART_NUM,
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
			RsslPostMsg post;
			rsslClearPostMsg(&post);

			post.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
			post.msgBase.streamId = 3;
			post.flags = flag;

			RsslBuffer rsslBuf = RSSL_INIT_BUFFER;

			RsslMsgKey msgKey;
			rsslClearMsgKey(&msgKey);

			if (flag & RSSL_PSMF_HAS_MSG_KEY)
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

				post.msgBase.msgKey = msgKey;
				rsslPostMsgApplyHasMsgKey(&post);

				post.msgBase.encDataBody = rsslBuf;
				post.msgBase.containerType = RSSL_DT_FIELD_LIST;
			}
			else {
				post.msgBase.encDataBody.data = 0;
				post.msgBase.encDataBody.length = 0;
				post.msgBase.containerType = RSSL_DT_NO_DATA;
			}

			/* Add Permission Info */
			if (flag & RSSL_PSMF_HAS_PERM_DATA)
			{
				post.permData.length = permissionDataLen;
				post.permData.data = permissionData;
			}

			/* Add Item Sequence Number */
			if (flag & RSSL_PSMF_HAS_SEQ_NUM)
			{
				seqNum = i;
				post.seqNum = seqNum;
			}

			/* Add partNum Info */
			if (flag & RSSL_PSMF_HAS_PART_NUM)
			{
				post.partNum = seqNum + 74;
			}

			RsslBuffer msgBuf;
			msgBuf.length = sizeof(msgBufData) / sizeof(char);
			msgBuf.data = msgBufData;

			RsslMsg postDecode;
			PostMsg respMsg;

			prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&post, decodeIter, (RsslMsg*)&postDecode, respMsg, dictionary);

			PostMsg clonePostMsg(respMsg);

			EXPECT_EQ(clonePostMsg.getDomainType(), respMsg.getDomainType()) << "Compare domainType";
			EXPECT_EQ(clonePostMsg.getDomainType(), RSSL_DMT_MARKET_BY_PRICE) << "Compare domainType: should be equal to " << RSSL_DMT_MARKET_BY_PRICE;

			EXPECT_EQ(clonePostMsg.getStreamId(), respMsg.getStreamId()) << "Compare streamId";
			EXPECT_EQ(clonePostMsg.getStreamId(), 3) << "Compare streamId: should be equal to 3";

			EXPECT_EQ(clonePostMsg.hasMsgKey(), respMsg.hasMsgKey()) << "Compare hasMsgKey";
			EXPECT_EQ(clonePostMsg.hasName(), respMsg.hasName()) << "Compare hasName";
			EXPECT_EQ(clonePostMsg.hasNameType(), respMsg.hasNameType()) << "Compare hasNameType";
			EXPECT_EQ(clonePostMsg.hasServiceId(), respMsg.hasServiceId()) << "Compare hasServiceId";
			EXPECT_EQ(clonePostMsg.hasId(), respMsg.hasId()) << "Compare hasId";
			EXPECT_EQ(clonePostMsg.hasFilter(), respMsg.hasFilter()) << "Compare hasFilter";
			EXPECT_EQ(clonePostMsg.hasExtendedHeader(), respMsg.hasExtendedHeader()) << "Compare hasExtendedHeader";

			EXPECT_EQ(clonePostMsg.hasSeqNum(), respMsg.hasSeqNum()) << "Compare hasSeqNum";
			EXPECT_EQ(clonePostMsg.hasPostId(), respMsg.hasPostId()) << "Compare hasPostId";
			EXPECT_EQ(clonePostMsg.hasPermissionData(), respMsg.hasPermissionData()) << "Compare hasPermissionData";
			EXPECT_EQ(clonePostMsg.hasPartNum(), respMsg.hasPartNum()) << "Compare hasPartNum";
			EXPECT_EQ(clonePostMsg.hasPostUserRights(), respMsg.hasPostUserRights()) << "Compare hasPostUserRights";
			EXPECT_EQ(clonePostMsg.hasServiceName(), respMsg.hasServiceName()) << "Compare hasServiceName";

			EXPECT_STREQ(respMsg.toString(), clonePostMsg.toString()) << "Check equal toString()";

			EXPECT_EQ(clonePostMsg.hasMsgKey(), (flag & RSSL_PSMF_HAS_MSG_KEY) > 0) << "Compare hasMsgKey: " << (flag & RSSL_PSMF_HAS_MSG_KEY);
			if (clonePostMsg.hasMsgKey() && respMsg.hasMsgKey())
			{
				if (clonePostMsg.hasServiceId())
				{
					EXPECT_EQ(clonePostMsg.getServiceId(), respMsg.getServiceId()) << "Compare serviceId";
				}
				if (clonePostMsg.hasName())
				{
					EXPECT_EQ(clonePostMsg.getName(), respMsg.getName()) << "Compare name";
				}
				if (clonePostMsg.hasNameType())
				{
					EXPECT_EQ(clonePostMsg.getNameType(), respMsg.getNameType()) << "Compare nameType";
				}
				if (clonePostMsg.hasId())
				{
					EXPECT_EQ(clonePostMsg.getId(), respMsg.getId()) << "Compare id";
				}
				if (clonePostMsg.hasFilter())
				{
					EXPECT_EQ(clonePostMsg.getFilter(), respMsg.getFilter()) << "Compare filter";
				}
			}

			EXPECT_EQ(clonePostMsg.hasSeqNum(), (flag & RSSL_PSMF_HAS_SEQ_NUM) > 0) << "Compare hasSeqNum: " << (flag & RSSL_PSMF_HAS_SEQ_NUM);
			if (clonePostMsg.hasSeqNum() && respMsg.hasSeqNum())
			{
				EXPECT_EQ(clonePostMsg.getSeqNum(), respMsg.getSeqNum()) << "Compare SeqNum";
				EXPECT_EQ(seqNum, clonePostMsg.getSeqNum()) << "Compare SeqNum: " << seqNum;
			}

			EXPECT_EQ(clonePostMsg.hasPermissionData(), (flag & RSSL_PSMF_HAS_PERM_DATA) > 0) << "Compare hasPermissionData: " << (flag & RSSL_PSMF_HAS_PERM_DATA);
			if (clonePostMsg.hasPermissionData() && respMsg.hasPermissionData())
			{
				const EmaBuffer& permDataOrig = clonePostMsg.getPermissionData();
				const EmaBuffer& permDataCopy = respMsg.getPermissionData();
				EmaBuffer permData(permissionData, permissionDataLen);

				EXPECT_EQ(permDataOrig.length(), permDataCopy.length()) << "Compare length of EmaBuffer Permission Data";
				EXPECT_EQ(permDataOrig.length(), permissionDataLen) << "Compare length of EmaBuffer Permission Data: " << permissionDataLen;
				EXPECT_EQ(permDataOrig, permDataCopy) << "Compare EmaBuffer Permission Data";
				EXPECT_EQ(permData, permDataCopy) << "Compare EmaBuffer Permission Data: " << permissionData;
			}

			EXPECT_EQ(clonePostMsg.hasPartNum(), (flag & RSSL_PSMF_HAS_PART_NUM) > 0) << "Compare hasPartNum: " << (flag & RSSL_PSMF_HAS_PART_NUM);
			if (clonePostMsg.hasPartNum() && respMsg.hasPartNum())
			{
				EXPECT_EQ(clonePostMsg.getPartNum(), respMsg.getPartNum()) << "Compare getPartNum";
				EXPECT_EQ(clonePostMsg.getPartNum(), seqNum + 74) << "Compare getPartNum: " << (seqNum + 74);
			}

			EXPECT_TRUE(true) << "PostMsg Clone Success";
		}
		catch (const OmmException&)
		{
			EXPECT_FALSE(true) << "PostMsg Clone - exception not expected";
		}
	}

	rsslDeleteDataDictionary(&dictionary);
}
