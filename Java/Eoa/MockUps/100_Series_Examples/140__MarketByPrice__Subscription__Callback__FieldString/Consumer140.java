///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 140__MarketByPrice__Subscription__Callback__FieldString
// Steps:
// - opens up MarketByPrice service
// - subscribes to a MarketByPrice item and passes callback client
// - when a network event happens, retrieves the PricePoint and prints their individual
//   field values from the received Mbp::OrderBook

package com.thomsonreuters.eoa.example.domain;

import com.thomsonreuters.eoa.domain.marketbyprice.Mbp;
import com.thomsonreuters.eoa.domain.marketbyprice.Mbp.*;
import com.thomsonreuters.eoa.foundation.OmmException;

public class Consumer140 {
	public static void main(String[] args) {
		try
		{
			Mbp.ConsumerService mbpConsumerService = Mbp.EoaFactory.createConsumerService( "DIRECT_FEED" );
			
			mbpConsumerService.subscribe(Mbp.EoaFactory.createReqSpec( "BBH.ITC" ), new Mbp.MarketByPriceItemClient() {
			
				@Override
				public void onConsumerItemSync(ConsumerItem current,
						RefreshInfo ri, Object closure) {
					
					System.out.println( "Symbol: " + current.getSymbol() + "\n" );
					
					print( current.getOrderBook() );
				}
				
				public void onConsumerItemPartial(ConsumerItem current,
						RefreshInfo ri, Object closure) {
					
					print( ri.getOrderBook() );
				}
				
				@Override
				public void onConsumerItemUpdate(ConsumerItem current,
						UpdateInfo ui, Object closure) {
					
					print( ui.getOrderBook() );
				}
			
				@Override
				public void onConsumerItemStatus(ConsumerItem current,
						StatusInfo si, Object closure) {
					if ( current.isOk() )
						System.out.println( "Subscription is Ok " );
					else
						System.out.println( "Subscription is not Ok " );
				}
				
				void print( Mbp.OrderBook orderBook)
				{
					for( Mbp.PricePoint pricePoint : orderBook )
						System.out.println( "Action: " + pricePoint.getActionAsString() + "\n"
								+ "Price: " + pricePoint.getPriceAsString() + "\n"
								+ "Size: " + pricePoint.getSizeAsString() + "\n"
								+ "Side: " + pricePoint.getSideAsString() + "\n" );
				}
			});
			
			Thread.sleep( 60000 );
			
		} catch ( InterruptedException | OmmException excp ){
			System.out.println( excp.getMessage() );
		}
	}
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
