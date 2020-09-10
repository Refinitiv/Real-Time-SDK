///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.iprovider.series100.example140__MarketByOrder__Streaming;

import java.nio.ByteBuffer;

import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Map;
import com.rtsdk.ema.access.MapEntry;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmIProviderConfig;
import com.rtsdk.ema.access.OmmProvider;
import com.rtsdk.ema.access.OmmProviderClient;
import com.rtsdk.ema.access.OmmProviderEvent;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.OmmReal.MagnitudeType;
import com.rtsdk.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;
	public String OrderNr="100";
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			case EmaRdm.MMT_MARKET_BY_ORDER :
				processMarketByOrderRequest(reqMsg, event);
				break;
			default :
				processInvalidItemRequest(reqMsg, event);
				break;
		}
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle() );
	}
	
	void processMarketByOrderRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if( itemHandle != 0 )
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}
		
		FieldList mapSummaryData = EmaFactory.createFieldList();
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(15,  840));
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(53,  1));
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(3423,  1));
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(1709,  2));
		
		FieldList entryData = EmaFactory.createFieldList();
		entryData.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, MagnitudeType.EXPONENT_NEG_2));
		entryData.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
		entryData.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
		entryData.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
		
		Map map = EmaFactory.createMap();
		map.summaryData(mapSummaryData);
		
		map.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryData));
		
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).
				name(reqMsg.name()).serviceName(reqMsg.serviceName()).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
				payload(map).complete(true),
				event.handle() );

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
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();
			FieldList fieldList = EmaFactory.createFieldList();
			Map map = EmaFactory.createMap();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
			while(appClient.itemHandle == 0) Thread.sleep(1000);

			for( int i = 0; i < 60; i++ )
			{
				fieldList.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76 + i * 0.1, MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
				fieldList.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
				
				map.add(EmaFactory.createMapEntry().keyAscii(appClient.OrderNr, MapEntry.MapAction.ADD, fieldList));
				
				provider.submit( EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).payload( map ), appClient.itemHandle );
				
				map.clear();
				fieldList.clear();
				
				Thread.sleep(1000);
			}

			Thread.sleep(60000);
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}
}
