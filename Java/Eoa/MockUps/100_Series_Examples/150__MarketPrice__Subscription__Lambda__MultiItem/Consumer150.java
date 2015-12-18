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

package com.thomsonreuters.eoa.example.domain;

import com.thomsonreuters.eoa.domain.marketprice.Mp;
import com.thomsonreuters.eoa.foundation.OmmException;

public class Consumer150 {

	public static void main(String[] args) {
		try
		{
			Mp.ConsumerService mpService = Mp.EoaFactory.createConsumerService("DIRECT_FEED");
			
			mpService.subscribe( Mp.EoaFactory.createReqSpec("TRI.N"), ( mpSubscription ) -> 
			{ System.out.println( "Symbol: " + mpSubscription.getSymbol() + " : Bid: " + mpSubscription.getQoute().getBidAsString() + "\n" ); } );
	
			mpService.subscribe( Mp.EoaFactory.createReqSpec("MSFT.O"), ( mpSubscription ) -> 
			{ System.out.println( "Symbol: " + mpSubscription.getSymbol() + " : Bid: " + mpSubscription.getQoute().getBidAsString() + "\n" ); } );
			
			Thread.sleep( 60000 );
			
		} catch ( InterruptedException | OmmException excp ){
			System.out.println( excp.getMessage() );
		}
	}
}

/* Abridged Output

Symbol: TRI.N  :  Bid : 40.31
Symbol: MSFT.O  :  Bid : 23.91
Symbol: TRI.N  :  Bid : 40.32
Symbol: MSFT.O  :  Bid : 23.92

*/
