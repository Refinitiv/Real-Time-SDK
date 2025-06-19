/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series100.ex150_MBO_Snapshot;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
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
			case EmaRdm.MMT_DIRECTORY :
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
        entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
        
        String OrderNr="100";
        map.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryLoad));
		
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).
				name(reqMsg.name()).serviceName(reqMsg.serviceName()).solicited(true).
				state(OmmState.StreamState.NON_STREAMING, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
				payload(map).complete(true),
				event.handle() );
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

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
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
