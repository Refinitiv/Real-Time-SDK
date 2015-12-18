///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 110__MarketByOrder__Subscription__Anytime__Refresh
// Steps:
// - opens up MarketByOrder service
// - subscribes to a MarketByOrder item
// - refreshes the subscription every second
// - prints out all orders in the subscription

#include "Domain/MarketByOrder/Include/Mbo.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace thomsonreuters::eoa::foundation;
using namespace thomsonreuters::eoa::domain::marketbyorder;
using namespace std;

int main( int argc, char* argv[] )
{
	try {
		Mbo::ConsumerService mboService( "DIRECT_FEED" );

		Mbo::ConsumerItem mboSubscription = mboService.subscribe( Mbo::ReqSpec( "AAO.V" ) );

		cout << "Symbol: " << mboSubscription.getSymbol() << endl << endl;

		for ( UInt64 i = 0; i < 60; ++i )
		{
			this_thread::sleep_for( chrono::seconds( 1 ) );

			mboService.refresh( mboSubscription );

			for ( const auto& order : mboSubscription.getOrderBook() )
			{
				cout << "Order id:  " << order.getIdAsString() << endl;
				cout << "Action:  " << order.getActionAsString() << endl;
				cout << "Price:  " << order.getPriceAsString() << endl;
				cout << "Side:  " << order.getSideAsString() << endl;
				cout << "Size:  " << order.getSizeAsString() << endl << endl;
			}
		}
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

/* Expected abridged output

Symbol: AAO.V

Order id:  1
Action:  Append
Price:  7.75
Side:  BID
Size:  100

Order id:  2
Action:  Append
Price:  7.75
Side:  BID
Size:  6400

Order id:  3
Action:  Append
Price:  7.75
Side:  BID
Size:  9200

Order id:  4
Action:  Append
Price:  7.75
Side:  BID
Size:  1500

*/
