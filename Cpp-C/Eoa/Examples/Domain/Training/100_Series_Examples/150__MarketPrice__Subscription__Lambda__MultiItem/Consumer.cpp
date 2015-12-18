///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 150__MarketPrice__Subscription__Lambda__MultiItem
// Steps:
// - opens up MarketPrice service
// - subscribes to two MarketPrice items and passes labda expressions
// - outputs received Bid values for the items

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

		mpService.subscribe( Mp::ReqSpec( "MSFT.O" ), []( const Mp::ConsumerItem& subscription )
				{ cout << "Symbol: " << subscription.getSymbol() << "  :  Bid : " << subscription.getQuote().getBidAsString() << endl; } );

		this_thread::sleep_for( chrono::seconds( 60 ) );

	} catch ( const OmmException& excp ) {
		cout << excp << endl; 
	}
	return 0;
}

// new abridged output

/* Abridged Output

Symbol: TRI.N  :  Bid : 40.31
Symbol: MSFT.O  :  Bid : 23.91
Symbol: TRI.N  :  Bid : 40.32
Symbol: MSFT.O  :  Bid : 23.92

*/
