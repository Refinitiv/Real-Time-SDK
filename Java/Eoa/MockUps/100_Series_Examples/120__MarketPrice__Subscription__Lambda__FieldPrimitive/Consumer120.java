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

package com.thomsonreuters.eoa.example.domain;

import com.thomsonreuters.eoa.domain.marketprice.Mp;
import com.thomsonreuters.eoa.foundation.OmmException;

public class Consumer120 {
	public static void main(String[] args) {
		try
		{
			Mp.ConsumerService mpService = Mp.EoaFactory.createConsumerService("DIRECT_FEED");
			
			mpService.subscribe( Mp.EoaFactory.createReqSpec("TRI.N"), ( mpSubscription ) -> 
			{ System.out.println( "Symbol: " + mpSubscription.getSymbol() + " : Bid: " + mpSubscription.getQoute().getBidAsString() + "\n" ); } );
			
			Thread.sleep( 60000 );
			
		} catch ( InterruptedException | OmmException excp ){
			System.out.println( excp.getMessage() );
		}
	}
}

/* Expected abridged output

Symbol: TRI.N  :  Bid : 40.99
Symbol: TRI.N  :  Bid : 41.00
Symbol: TRI.N  :  Bid : 41.01
Symbol: TRI.N  :  Bid : 41.02
Symbol: TRI.N  :  Bid : 41.03
Symbol: TRI.N  :  Bid : 41.04

*/
