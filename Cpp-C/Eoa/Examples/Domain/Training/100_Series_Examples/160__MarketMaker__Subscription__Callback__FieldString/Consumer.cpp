///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

// 160__MarketMaker__Subscription__Callback__FieldString
// Steps:
// - opens up MarketMaker service
// - subscribes to a MarketMaker item and passes callback client
// - when a network event happens, retrieves the MarketMakerQuotes and prints their individual
//   field values from the received Mm::MarketMakerList

using namespace foundation;
using namespace marketmaker;
using namespace std;

void AppClient::print( const Mm::MarketMakerList& mmList )
{
	for ( const auto& marketmakerquote : mmList )
	{
		cout << "MMID: " << marketmakerquote.getIdAsString() << endl;
		cout << "Action: " << marketmakerquote.getActionAsString() << endl;
		cout << "Quote time: " << marketmakerquote.getQuoteTimeAsString() << endl;
		cout << "Bid: " << marketmakerquote.getBidAsString() << endl;
		cout << "Bid Size: " << marketmakerquote.getBidSizeAsString() << endl;
		cout << "Ask: " << marketmakerquote.getAskAsString() << endl;
		cout << "Ask Size: " << marketmakerquote.getAskSizeAsString() << endl << endl;
	}
}

void AppClient::onConsumerItemSync( const Mm::ConsumerItem& current, const Mm::RefreshInfo& , void* )
{
	cout << "Symbol: " << current.getSymbol() << endl << endl;

	print( current.getMarketMakerList() );
}

void AppClient::onConsumerItemPartial( const Mm::ConsumerItem& , const Mm::RefreshInfo& ri, void* )
{
	print( ri.getMarketMakerList() );
}

void AppClient::onConsumerItemUpdate( const Mm::ConsumerItem& , const Mm::UpdateInfo& ui, void* )
{
	print( ui.getMarketMakerList() );
}

void AppClient::onConsumerItemStatus( const Mm::ConsumerItem& current, const Mm::StatusInfo&, void* )
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

		Mm::ConsumerService mmConsumerService( "DIRECT_FEED" );

		mmConsumerService.subscribe( "BBH.ITC", client );

		this_thread::sleep_for( chrono::seconds( 60 ) );

	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

/* Expected abridged output

Symbol: 

*/
