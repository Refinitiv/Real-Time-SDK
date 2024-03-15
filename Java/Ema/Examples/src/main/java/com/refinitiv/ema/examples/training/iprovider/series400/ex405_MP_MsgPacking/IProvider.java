///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.iprovider.series400.ex405_MP_MsgPacking;

import java.util.ArrayList;

import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PackedMsg;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;
	public long clientHandle = 0;
	OmmProvider provider = null;
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			case EmaRdm.MMT_MARKET_PRICE :
				processMarketPriceRequest(reqMsg, event);
				break;
			default :
				processInvalidItemRequest(reqMsg, event);
				break;
		}
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		clientHandle = event.clientHandle();
		
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle() );
	}
	
	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if( itemHandle != 0 )
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		// Send a packed message including the first refresh and 10 updates
		PackedMsg packedMsg = EmaFactory.createPackedMsg(provider);
		packedMsg.initBuffer(clientHandle);
		
		packedMsg.addMsg(EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
				payload(fieldList).complete(true), event.handle());
		
		for( int j = 0; j < 10; j++ )
		{
			fieldList.clear();
			fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + j, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + j, OmmReal.MagnitudeType.EXPONENT_0));
			
			UpdateMsg msg = EmaFactory.createUpdateMsg().payload( fieldList );
			
			packedMsg.addMsg(msg, event.handle());
		}
		
		event.provider().submit( packedMsg );

		itemHandle = event.handle();
	}
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle() );
	}
}

public class IProvider
{
	public static void main(String[] args)
	{
		ArrayList<ChannelInformation> channelInformationList = new ArrayList<>();
		AppClient appClient = new AppClient();
		try
		{
			FieldList fieldList = EmaFactory.createFieldList();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			appClient.provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
			while(appClient.itemHandle == 0)
			{
				appClient.provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			PackedMsg packedMsg = EmaFactory.createPackedMsg(appClient.provider);
			packedMsg.initBuffer(appClient.clientHandle);
			
			// Once connected, run application for 60 seconds, submitting 60 total packed messages.
			for( int i = 0; i < 60; i++ )
			{
				// Check that our connection is still ongoing.
				appClient.provider.connectedClientChannelInfo(channelInformationList);
				if (channelInformationList.size() == 0)
				{
					System.out.println("Our ongoing connection has been closed, ending application.");
					break;	
				}
				
				// Each message packs 10 individual update messages before submitting the full packed message.
				for( int j = 0; j < 10; j++ )
				{
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + j, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + j, OmmReal.MagnitudeType.EXPONENT_0));
					
					UpdateMsg msg = EmaFactory.createUpdateMsg().payload( fieldList );
					
					packedMsg.addMsg(msg, appClient.itemHandle);
				}
				if (packedMsg.packedMsgCount() > 0)
				{
					appClient.provider.submit(packedMsg);
					packedMsg.initBuffer(appClient.clientHandle);	// Re-initialize buffer for next set of packed messages.
				}
				Thread.sleep(1000);
			}
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (appClient.provider != null) appClient.provider.uninitialize();
		}
	}
}
