///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 120__MarketPrice__Subscription__Lambda__FieldPrimitive
// Steps:
// - opens up MarketPrice service
// - subscribes to a MarketPrice item and passes lambda expression
// - outputs a received Bid value for the item

#include "Domain/MarketPrice/Include/Mp.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace thomsonreuters::eoa::foundation;
using namespace thomsonreuters::eoa::domain::marketprice;
using namespace std;

int main( int argc, char* argv[] )
{
	try {
		Mp::ConsumerService	mpService( "DIRECT_FEED" );

		mpService.subscribe( Mp::ReqSpec( "TRI.N" ), []( const Mp::ConsumerItem& subscription ) 
				{ cout << "Symbol: " << subscription.getSymbol() << "  :  Bid : " << subscription.getQuote().getBidAsString() << endl; } );

		this_thread::sleep_for( chrono::seconds( 60 ) );

	} catch ( const OmmException& excp ) {
		cout << excp << endl; 
	}
	return 0;
}

/* Expected abridged output

Symbol: TRI.N  :  Bid : 40.99
Symbol: TRI.N  :  Bid : 41.00
Symbol: TRI.N  :  Bid : 41.01
Symbol: TRI.N  :  Bid : 41.02
Symbol: TRI.N  :  Bid : 41.03
Symbol: TRI.N  :  Bid : 41.04

*/
