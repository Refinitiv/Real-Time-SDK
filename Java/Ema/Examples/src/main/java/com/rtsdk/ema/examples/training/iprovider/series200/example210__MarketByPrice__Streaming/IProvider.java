///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.iprovider.series200.example210__MarketByPrice__Streaming;

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
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle =  0;
	public String OrderNr = "100";
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
	{
		switch(reqMsg.domainType())
		{
		case EmaRdm.MMT_LOGIN:
			processLoginRequest(reqMsg,providerEvent);
			break;
		case EmaRdm.MMT_MARKET_BY_PRICE:
			processMarketByPriceRequest(reqMsg,providerEvent);
			break;
		default:
			processInvalidItemRequest(reqMsg,providerEvent);
			break;
		}
	}	
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}	
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle());
	}
	
	void processMarketByPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if ( itemHandle != 0 )
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}
		
		Map map = EmaFactory.createMap();
        FieldList summary = EmaFactory.createFieldList();
        FieldList entryLoad = EmaFactory.createFieldList();
        
        summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
        summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
        summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
        summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
        
        map.summaryData(summary);
        
        entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
        entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
        entryLoad.add(EmaFactory.createFieldEntry().rmtes(3435, ByteBuffer.wrap("Market Maker".getBytes())));
        
        map.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryLoad));
        
        event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_PRICE).serviceName(reqMsg.serviceName()).
        		name(reqMsg.name()).state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
        		solicited(true).payload(map).complete(true), event.handle());
		
		itemHandle = event.handle();
	}
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).
				serviceName(reqMsg.serviceName()).domainType(reqMsg.domainType()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle());
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
			FieldList entryLoad = EmaFactory.createFieldList();
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			Map map = EmaFactory.createMap();

			provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig().
					operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH), appClient );
			
			while( appClient.itemHandle == 0 ) 
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			for(int i = 0; i < 60; i++)
			{
				provider.dispatch(1000);
				
			    entryLoad.clear();
                entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
                entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
                entryLoad.add(EmaFactory.createFieldEntry().rmtes(3435, ByteBuffer.wrap("Market Maker".getBytes())));
            
                map.clear();
                map.add(EmaFactory.createMapEntry().keyAscii(appClient.OrderNr, MapEntry.MapAction.UPDATE, entryLoad));
			
			    provider.submit(updateMsg.clear().domainType(EmaRdm.MMT_MARKET_BY_PRICE).payload(map), appClient.itemHandle );
				    
				Thread.sleep(1000);
			}
		}
		catch ( OmmException | InterruptedException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}
}
