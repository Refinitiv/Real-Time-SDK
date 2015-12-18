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

package com.thomsonreuters.eoa.example.domain;
import com.thomsonreuters.eoa.domain.marketprice.Mp;
import com.thomsonreuters.eoa.domain.marketprice.Mp.*;
import com.thomsonreuters.eoa.foundation.OmmException;

public class Consumer130 {
	public static void main(String[] args) {
		try
		{
			Mp.ConsumerService mpService = Mp.EoaFactory.createConsumerService( "DIRECT_FEED" );
			
			mpService.subscribe( Mp.EoaFactory.createReqSpec( "TRI.N") , new Mp.ConsumerItemClient() {
			
				@Override
				public void onConsumerItemSync(ConsumerItem current, RefreshInfo ri, Object closure) {
					try
					{
						System.out.println( "Symbol: " + current.getSymbol() + "\n" );
						print(current.getQoute());
					}catch ( OmmException excp ){
						System.out.println( excp.getMessage() );
					}
				}
			
				@Override
				public void onConsumerItemUpdate(ConsumerItem current, UpdateInfo ui, Object closure) {
					try
					{
						print(ui.getQoute());
					}catch ( OmmException excp ){
						System.out.println( excp.getMessage() );
					}
				}
			
				@Override
				public void onConsumerItemStatus(ConsumerItem current, StatusInfo si, Object closure) {
					if ( current.isOk() )
						System.out.println( "Subscription is Ok " );
					else
						System.out.println( "Subscription is not Ok " );
				}
				
				void print(Mp.Qoute qoute) throws OmmException
				{
					System.out.println( "Ask: " + qoute.getAsk() + "\n"
							+ "Bid: " + qoute.getBid() + "\n"
							+ "Trade Price: " + qoute.getTradePrice() + "\n" );
				}
			} );
			
			Thread.sleep( 60000 );
			
		}catch ( InterruptedException | OmmException excp ){
			System.out.println( excp.getMessage() );
		}
	}
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
