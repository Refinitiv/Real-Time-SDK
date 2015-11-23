///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.series100.example121__MarketPrice__FieldListSearchByName;

import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg( RefreshMsg refreshMsg, OmmConsumerEvent event )
	{
		System.out.println( "Item Name: " + ( refreshMsg.hasName() ? refreshMsg.name() : "<not set>" ) );
		System.out.println( "Service Name: " + ( refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>" ) );
		
		System.out.println( "Item State: " + refreshMsg.state() );
		
		System.out.println( "Item Handle: " + event.handle() );
		System.out.println( "Item Closure: " + event.closure().hashCode() );
		
		if ( DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType() )
			decode( refreshMsg.payload().fieldList() );
		
		System.out.println();
	}
	
	public void onUpdateMsg( UpdateMsg updateMsg, OmmConsumerEvent event ) 
	{
		System.out.println( "Item Name: " + ( updateMsg.hasName() ? updateMsg.name() : "<not set>" ) );
		System.out.println( "Service Name: " + ( updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>" ) );
		
		System.out.println( "Item Handle: " + event.handle() );
		System.out.println( "Item Closure: " + event.closure() );
		
		if ( DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType() )
			decode( updateMsg.payload().fieldList() );
		
		System.out.println();
	}

	public void onStatusMsg( StatusMsg statusMsg, OmmConsumerEvent event ) 
	{
		System.out.println( "Item Name: " + ( statusMsg.hasName() ? statusMsg.name() : "<not set>" ) );
		System.out.println( "Service Name: " + ( statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>" ) );

		if ( statusMsg.hasState() )
			System.out.println( "Item State: " +statusMsg.state() );
		
		System.out.println( "Item Handle: " + event.handle() );
		System.out.println( "Item Closure: " + event.closure() );
		
		System.out.println();
	}
	
	public void onGenericMsg( GenericMsg genericMsg, OmmConsumerEvent consumerEvent ){}
	public void onAckMsg( AckMsg ackMsg, OmmConsumerEvent consumerEvent ){}
	public void onAllMsg( Msg msg, OmmConsumerEvent consumerEvent ){}
	
	void decode( FieldList fl )
	{
		while ( fl.forth( "BID" ) )	// look for a fid with matching name
			System.out.println( "Fid: " + fl.entry().fieldId() + " Name: " + fl.entry().name() + " value: " + fl.entry().load() );
	}
}

public class Consumer 
{
	public static void main( String[] args )
	{
		try
		{
			AppClient appClient = new AppClient();
			
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			
			OmmConsumer consumer  = EmaFactory.createOmmConsumer( config.host( "localhost:14002"  ).username( "user" ) );
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient( reqMsg.serviceName( "DIRECT_FEED" ).name( "IBM.N" ), appClient, 0 , 1 );
			
			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch ( InterruptedException | OmmException excp )
		{
			System.out.println( excp.getMessage() );
		}
	}
}


