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

// encoding by UPA and decoding by EMA
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

	try
	{
		PostMsg post;

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

		StaticDecoder::setData( &post, &dictionary );


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

