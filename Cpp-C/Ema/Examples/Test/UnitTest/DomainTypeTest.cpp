/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Rdm/Impl/RdmUtilities.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;

TEST(DomainTypeTest, testDomainType)
{
	try
	{
		EXPECT_STREQ( rdmDomainToString( MMT_DICTIONARY ), "Dictionary Domain" ) << "DomainType( Blank )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_DIRECTORY ), "Directory Domain" ) << "DomainType( Directory )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_LOGIN ), "Login Domain" ) << "DomainType( Login )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_MARKET_BY_ORDER ), "MarketByOrder Domain" ) << "DomainType( MarketByOrder )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_MARKET_BY_PRICE ), "MarketByPrice Domain" ) << "DomainType( MarketByPrice )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_MARKET_PRICE ), "MarketPrice Domain" ) << "DomainType( MarketPrice )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_MARKET_MAKER ), "MarketMaker Domain" ) << "DomainType( MarketMaker )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_SYMBOL_LIST ), "SymbolList Domain" ) << "DomainType( SymbolList )" ;

		EXPECT_STREQ( rdmDomainToString( MMT_YIELD_CURVE ), "YieldCurve Domain" ) << "DomainType( YieldCurve )" ;

		EXPECT_STREQ( rdmDomainToString( 2000 ), "Unknown RDM Domain. Value='2000'" ) << "DomainType( 2000 )" ;

		EXPECT_TRUE( true ) << "DomainType - exception not expected" ;

	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "DomainType - exception not expected" ;
		cout << excp << endl;
	}
}
