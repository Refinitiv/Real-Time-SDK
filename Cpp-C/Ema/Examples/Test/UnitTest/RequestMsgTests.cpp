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

	try
	{
		ReqMsg reqMsg;

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
		EXPECT_EQ( reqMsg.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ReqMsg.toString() == Decoding of just encoded object in the same application is not supported";


		//Now do EMA decoding of ReqMsg
		StaticDecoder::setData( &reqMsg, &dictionary );
		EXPECT_NE( reqMsg.toString(), "\nDecoding of just encoded object in the same application is not supported\n") << "ReqMsg.toString() != Decoding of just encoded object in the same application is not supported";


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

TEST(RequestMsgTests, testReqMsgWithServiceIdAndName)
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

		reqMsg.serviceId( 10 ).serviceName( EmaString( "DIRECT_FEED" ) );

		EXPECT_FALSE( true ) << "Setting ServiceName while id is set. Exception expected." ;
	}
	catch ( const OmmException& )
	{
		EXPECT_TRUE( true ) << "Setting ServiceName while id is set. Exception expected." ;
	}
}

TEST(RequestMsgTests, testRequestMsgEncodeDecodeQos)
{

	ReqMsg reqMsg;

	reqMsg.qos( 5000, 6000 );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), 5000 ) << "ReqMsg::getQosTimeliness()== 5000" ;
	EXPECT_EQ( reqMsg.getQosRate(), 6000 ) << "ReqMsg::getQosRate()== 6000 " ;

	reqMsg.clear();
	reqMsg.qos( 65539, 78895 ); // Out of range
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum ) << "ReqMsg::getQosTimeliness() == ReqMsg::BestDelayedTimelinessEnum " ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum ) << "ReqMsg::getQosRate() == ReqMsg::JustInTimeConflatedEnum" ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, ReqMsg::BestConflatedRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestConflatedRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestConflatedRateEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::BestConflatedRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::BestConflatedRateEnum ) << "ReqMsg::getQosRate()== ReqMsg::BestConflatedRateEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::TickByTickEnum ) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, ReqMsg::JustInTimeConflatedEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum ) << "ReqMsg::getQosRate()== ReqMsg::JustInTimeConflatedEnum " ;

	reqMsg.clear();
	reqMsg.qos( 23, ReqMsg::JustInTimeConflatedEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), 23 ) << "ReqMsg::getQosTimeliness()== 23" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum ) << "ReqMsg::getQosRate()== ReqMsg::JustInTimeConflatedEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, 5623 );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum ) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum" ;
	EXPECT_EQ( reqMsg.getQosRate(), 5623 ) << "ReqMsg::getQosRate()== 5623 " ;

	reqMsg.clear();
	reqMsg.qos( 9999, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );

	EXPECT_EQ( reqMsg.getQosTimeliness(), 9999 ) << "ReqMsg::getQosTimeliness()== 9999" ;
	EXPECT_EQ( reqMsg.getQosRate(), ReqMsg::TickByTickEnum ) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum " ;

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::TickByTickEnum) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum";

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::TickByTickEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::TickByTickEnum) << "ReqMsg::getQosRate()== ReqMsg::TickByTickEnum";
	

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestTimelinessEnum, 4455 );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), 4455) << "ReqMsg::getQosRate()== 4455";

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, 5555 );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), 5555) << "ReqMsg::getQosRate()== 5555";

	reqMsg.clear();
	reqMsg.qos( ReqMsg::RealTimeEnum, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::RealTimeEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::RealTimeEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::BestRateEnum) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum";
		
	reqMsg.clear();
	reqMsg.qos( 1235, ReqMsg::BestRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), 1235) << "ReqMsg::getQosTimeliness()== 1235";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::BestRateEnum) << "ReqMsg::getQosRate()== ReqMsg::BestRateEnum";
	

	reqMsg.clear();
	reqMsg.qos( 5678, ReqMsg::BestConflatedRateEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), 5678) << "ReqMsg::getQosTimeliness()== 5678";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::BestConflatedRateEnum) << "ReqMsg::getQosRate()== ReqMsg::BestConflatedRateEnum";

	reqMsg.clear();
	reqMsg.qos( ReqMsg::BestDelayedTimelinessEnum, ReqMsg::JustInTimeConflatedEnum );
	StaticDecoder::setData( &reqMsg, 0 );
	EXPECT_EQ(reqMsg.getQosTimeliness(), ReqMsg::BestDelayedTimelinessEnum) << "ReqMsg::getQosTimeliness()== ReqMsg::BestDelayedTimelinessEnum";
	EXPECT_EQ(reqMsg.getQosRate(), ReqMsg::JustInTimeConflatedEnum) << "ReqMsg::getQosRate()== ReqMsg::JustInTimeConflatedEnum";
}

