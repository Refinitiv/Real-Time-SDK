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

package com.thomsonreuters.eoa.example.domain;

import com.thomsonreuters.eoa.domain.marketbyorder.Mbo;
import com.thomsonreuters.eoa.foundation.OmmException;

public class Consumer110 {
	public static void main(String[] args) {
		try
		{
			Mbo.ConsumerService mboservice = Mbo.EoaFactory.createConsumerService( "DIRECT_FEED" );
			
			Mbo.ConsumerItem mboSubscription = mboservice.subscribe( Mbo.EoaFactory.createReqSpec( "AAV.O") );
			
			System.out.println( "Symbol: " + mboSubscription.getSymbol() + "\n" );
		
			for( int i = 0 ; i < 60; ++i ) {
				
				Thread.sleep( 1000 );
				
				mboservice.refresh( mboSubscription );
			
				for( Mbo.Order order : mboSubscription.getOrderBook() )
					System.out.println( "Order id: " + order.getIdAsString() + "\n" 
										+ "Action: " + order.getActionAsString() + "\n"
										+ "Price: " + order.getPriceAsString() + "\n"
										+ "Side: " + order.getPriceAsString() + "\n"
										+ "Size: " + order.getSizeAsString() + "\n" );
			}
		} catch ( InterruptedException | OmmException excp ){
			System.out.println( excp.getMessage() );
		}
	}
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
