///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 130__MarketPrice__Subscription__Callback__FieldPrimitive
// Steps:
// - opens up MarketPrice service
// - subscribes to a MarketPrice item and passes callback client
// - when a network event happens, retrieves the Bid, Ask and TradePrice values
//   from the received Mp::Quote

#include "Consumer.h"

using namespace foundation;
using namespace marketprice;
using namespace std;

void AppClient::onConsumerItemSync( const Mp::ConsumerItem& current, const Mp::RefreshInfo& , void* )
{
	try {
		cout << "Symbol: " << current.getSymbol() << endl << endl;
		print( current.getQuote() );
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
}

void AppClient::onConsumerItemUpdate( const Mp::ConsumerItem& current, const Mp::UpdateInfo& ui, void* )
{
	try {
		print( ui.getQuote() );
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
}

void AppClient::print( const Mp::Quote& quote )
{
	cout << "Ask: " << quote.getAsk() << endl;
	cout << "Bid: " << quote.getBid() << endl;
	cout << "Trade Price: " << quote.getTradePrice() << endl << endl;
}

void AppClient::onConsumerItemStatus( const Mp::ConsumerItem& current, const Mp::StatusInfo&, void* )
{
	if ( current.isOk() )
		cout << "Subscription is Ok" << endl;
	else
		cout << "Subscription is not Ok" << endl;
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;

		Mp::ConsumerService	mpConsumerService( "DIRECT_FEED" );

		mpConsumerService.subscribe( Mp::ReqSpec( "TRI.N" ), client );

		this_thread::sleep_for( chrono::seconds( 60 ) );

	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

/* Abridged Output

Symbol: TRI.N

Ask: 40.23
Bid: 40.19
Trade Price: 40.2

Ask: 40.24
Bid: 40.2
Trade Price: 40.21

*/
