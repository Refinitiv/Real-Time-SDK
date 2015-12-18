///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

// 140__MarketByPrice__Subscription__Callback__FieldString
// Steps:
// - opens up MarketByPrice service
// - subscribes to a MarketByPrice item and passes callback client
// - when a network event happens, retrieves the PricePoint and prints their individual
//   field values from the received Mbp::OrderBook

using namespace foundation;
using namespace marketbyprice;
using namespace std;

void AppClient::print( const Mbp::OrderBook& orderBook )
{
	for ( const auto& pricePoint : orderBook )
	{
		cout << "Action: " << pricePoint.getActionAsString() << endl;
		cout << "Price: " << pricePoint.getPriceAsString() << endl;
		cout << "Size: " << pricePoint.getSizeAsString() << endl;
		cout << "Side: " << pricePoint.getSideAsString() << endl << endl;
	}
}

void AppClient::onConsumerItemSync( const Mbp::ConsumerItem& current, const Mbp::RefreshInfo& , void* )
{
	cout << "Symbol: " << current.getSymbol() << endl << endl;

	print( current.getOrderBook() );
}

void AppClient::onConsumerItemPartial( const Mbp::ConsumerItem& , const Mbp::RefreshInfo& ri, void* )
{
	print( ri.getOrderBook() );
}

void AppClient::onConsumerItemUpdate( const Mbp::ConsumerItem& , const Mbp::UpdateInfo& ui, void* )
{
	print( ui.getOrderBook() );
}

void AppClient::onConsumerItemStatus( const Mbp::ConsumerItem& current, const Mbp::StatusInfo&, void* )
{
	if ( current.isOk() )
		cout << "Subscription is Ok" << endl;
	else
		cout << "Subbscription is not Ok" << endl;
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;

		Mbp::ConsumerService mbpConsumerService( "DIRECT_FEED" );

		mbpConsumerService.subscribe( Mbp::ReqSpec( "BBH.ITC" ), client );

		this_thread::sleep_for( chrono::seconds( 60 ) );

	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

/* Expected abridged outpot 

Symbol: BBH.ITC

Action: Append
Price: 77.00
Size: 8000
Side: BID

Action: Append
Price: 76.99
Size: 2500
Side: BID

Action: Append
Price: 77.54
Size: 1700
Side: ASK

*/
