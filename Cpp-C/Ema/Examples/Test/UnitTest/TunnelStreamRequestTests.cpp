/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

TEST(TunnelStreamRequestTests, testTsrLoginMsg)
{

	try
	{
		TunnelStreamRequest tsr;

		tsr.loginReqMsg( ReqMsg().domainType( MMT_LOGIN ).attrib( ElementList().addUInt( ENAME_ALLOW_SUSPECT_DATA, 1 ).addAscii( ENAME_APP_ID, "256" ).complete() ) );

		const ReqMsg& loginReqMsg = tsr.getLoginReqMsg();

		EXPECT_EQ( loginReqMsg.getDomainType(), MMT_LOGIN ) << "ReqMsg::getDomainType() == MMT_LOGIN" ;

		EXPECT_EQ( loginReqMsg.getAttrib().getDataType(), DataType::ElementListEnum ) << "ReqMsg::getAttrib()::getDataType() == DataType::ElementListEnum" ;

		const ElementList& loginReqMsgAttrib = loginReqMsg.getAttrib().getElementList();

		EXPECT_TRUE( loginReqMsgAttrib.forth() ) << "contains first entry" ;

		const ElementEntry& eEntry = loginReqMsgAttrib.getEntry();

		EXPECT_EQ( eEntry.getName(), ENAME_ALLOW_SUSPECT_DATA ) << "contains ENAME_ALLOW_SUSPECT_DATA" ;
		EXPECT_EQ( eEntry.getUInt(), 1 ) << "contains 1" ;

		EXPECT_TRUE( loginReqMsgAttrib.forth() ) << "contains second entry" ;

		EXPECT_EQ( eEntry.getName(), ENAME_APP_ID ) << "contains ENAME_APP_ID" ;
		EXPECT_STREQ( eEntry.getAscii(), "256" ) << "contains \"256\"" ;

		EXPECT_FALSE( loginReqMsgAttrib.forth() ) << "at end" ;

		EXPECT_TRUE( true ) << "Login Request Message on TunnelStreamRequest - exception not expectedd" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Login Request Message on TunnelStreamRequest - exception not expectedd" ;
	}
}

TEST(TunnelStreamRequestTests, testTsrCopyConstructor)
{

	try
	{
		TunnelStreamRequest tsr;

		tsr.loginReqMsg( ReqMsg().domainType( MMT_LOGIN ).attrib( ElementList().addUInt( ENAME_ALLOW_SUSPECT_DATA, 1 ).addAscii( ENAME_APP_ID, "256" ).complete() ) );

		TunnelStreamRequest tsrCopy( tsr );

		const ReqMsg& loginReqMsg = tsr.getLoginReqMsg();

		EXPECT_EQ( loginReqMsg.getDomainType(), MMT_LOGIN ) << "ReqMsg::getDomainType() == MMT_LOGIN" ;

		EXPECT_EQ( loginReqMsg.getAttrib().getDataType(), DataType::ElementListEnum ) << "ReqMsg::getAttrib()::getDataType() == DataType::ElementListEnum" ;

		const ElementList& loginReqMsgAttrib = loginReqMsg.getAttrib().getElementList();

		EXPECT_TRUE( loginReqMsgAttrib.forth() ) << "contains first entry" ;

		const ElementEntry& eEntry = loginReqMsgAttrib.getEntry();

		EXPECT_EQ( eEntry.getName(), ENAME_ALLOW_SUSPECT_DATA ) << "contains ENAME_ALLOW_SUSPECT_DATA" ;
		EXPECT_EQ( eEntry.getUInt(), 1 ) << "contains 1" ;

		EXPECT_TRUE( loginReqMsgAttrib.forth() ) << "contains second entry" ;

		EXPECT_EQ( eEntry.getName(), ENAME_APP_ID) << "contains ENAME_APP_ID" ;
		EXPECT_STREQ( eEntry.getAscii(), "256" ) << "contains \"256\"" ;

		EXPECT_FALSE( loginReqMsgAttrib.forth() ) << "at end" ;

		EXPECT_TRUE( true ) << "TunnelStreamRequest copy constructor - exception not expectedd" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "TunnelStreamRequest copy constructor - exception not expectedd" ;
	}
}

