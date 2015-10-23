///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.series100.example120__MarketPrice__FieldListWalk;

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
		if ( refreshMsg.hasName() )
			System.out.println( "Item Name: " + refreshMsg.name() );
		
		if ( refreshMsg.hasServiceName() )
			System.out.println( "Service Name: " + refreshMsg.serviceName() );
		
		System.out.println( "Item State: " + refreshMsg.state() );
		
		if ( DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType() )
			decode( refreshMsg.payload().fieldList() );
		
		System.out.println("\n");
	}
	
	public void onUpdateMsg( UpdateMsg updateMsg, OmmConsumerEvent event ) 
	{
		if ( updateMsg.hasName() )
			System.out.println( "Item Name: " + updateMsg.name() );
		
		if ( updateMsg.hasServiceName() )
			System.out.println( "Service Name: " + updateMsg.serviceName() );
		
		if ( DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType() )
			decode( updateMsg.payload().fieldList() );
		
		System.out.println("\n");
	}

	public void onStatusMsg( StatusMsg statusMsg, OmmConsumerEvent event ) 
	{
		if ( statusMsg.hasName() )
			System.out.println( "Item Name: " + statusMsg.name() );
		
		if ( statusMsg.hasServiceName() )
			System.out.println( "Service Name: " + statusMsg.serviceName() );
		
		if ( statusMsg.hasState() )
			System.out.println( "Service State: " + statusMsg.state() );
		
		System.out.println("\n");
	}

	
	public void onGenericMsg( GenericMsg genericMsg, OmmConsumerEvent consumerEvent ){}
	public void onAckMsg( AckMsg ackMsg, OmmConsumerEvent consumerEvent ){}
	public void onAllMsg( Msg msg, OmmConsumerEvent consumerEvent ){}

	void decode( FieldList fl )
	{
		while ( fl.forth() )
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
			
			consumer.registerClient( reqMsg.serviceName( "DIRECT_FEED" ).name( "IBM.N" ), appClient, 0 );
			
			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch ( InterruptedException | OmmException excp )
		{
			System.out.println( excp.getMessage() );
		}
	}
}


