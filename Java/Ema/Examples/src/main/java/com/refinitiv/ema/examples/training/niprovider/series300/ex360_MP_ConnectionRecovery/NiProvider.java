/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series300.ex360_MP_ConnectionRecovery;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;


class AppClient implements OmmProviderClient
{
	boolean _connectionUp;
	
	boolean isConnectionUp()
	{
		return _connectionUp;
	}
		
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());
		
		if ( refreshMsg.state().streamState() == OmmState.StreamState.OPEN)
		{
			if (refreshMsg.state().dataState() == OmmState.DataState.OK)
				_connectionUp = true;
			else
				_connectionUp = false;
		}
		else
			_connectionUp = false;		
	}
	
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
		{
			System.out.println("Item State: " +statusMsg.state());
			if ( statusMsg.state().streamState() == OmmState.StreamState.OPEN)
			{
				if (statusMsg.state().dataState() == OmmState.DataState.OK)
					_connectionUp = true;
				else
				{
					_connectionUp = false;
				}
			}
			else
				_connectionUp = false;					
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
}


public class NiProvider 
{
	public static void main(String[] args)
	{			
		AppClient appClient = new AppClient();
		boolean sendRefreshMsg = false;
			
		OmmProvider provider = null;
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
							
			provider = EmaFactory.createOmmProvider(config.operationModel(OmmNiProviderConfig.OperationModel.USER_DISPATCH)
					.username("user"), appClient);

            provider.dispatch( 1000000 );
                        
			long itemHandle = 6;
				
			FieldList fieldList = EmaFactory.createFieldList();
				
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
				
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
				
			provider.dispatch( 1000000 );
				
			for( int i = 0; i < 60; i++ )
			{
				if ( appClient.isConnectionUp())
				{
					if ( sendRefreshMsg )
					{
						fieldList.clear();
						fieldList.add( EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(25, 14700 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(30, 10 + i,  OmmReal.MagnitudeType.EXPONENT_0));
						fieldList.add( EmaFactory.createFieldEntry().real(31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));
							
						provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
								.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
								.payload(fieldList).complete(true), itemHandle);
						
						sendRefreshMsg = false;
					}
					else
					{
						fieldList.clear();
						fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
						
						provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload( fieldList ), itemHandle );
					}
				}
				else
				{
					sendRefreshMsg = true;
				}
				provider.dispatch( 1000000 );
			}
		}
		catch (OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}					
}
