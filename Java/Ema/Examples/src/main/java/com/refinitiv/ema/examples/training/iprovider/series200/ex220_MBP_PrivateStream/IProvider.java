/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series200.ex220_MBP_PrivateStream;

import java.util.HashMap;
import java.util.LinkedList;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry.MapAction;
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
	HashMap<Long, Boolean> itemHandles = new HashMap<Long, Boolean>();
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			case EmaRdm.MMT_MARKET_BY_PRICE :
				processMarketByPriceRequest(reqMsg, event);
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
	
	void processMarketByPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		itemHandles.put(event.handle(), reqMsg.privateStream());
	
		Map map = EmaFactory.createMap();
		
		FieldList summaryData = EmaFactory.createFieldList();
		summaryData.add(EmaFactory.createFieldEntry().real( 22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		summaryData.add(EmaFactory.createFieldEntry().real( 25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		summaryData.add(EmaFactory.createFieldEntry().real( 30, 9, OmmReal.MagnitudeType.EXPONENT_0));
		summaryData.add(EmaFactory.createFieldEntry().real( 31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		map.summaryData(summaryData);
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().real( 22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real( 25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real( 30, 9, OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add(EmaFactory.createFieldEntry().real( 31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		map.add(EmaFactory.createMapEntry().keyAscii("Key", MapAction.ADD, fieldList));

       event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_PRICE).name( reqMsg.name() ).serviceName( reqMsg.serviceName() ).solicited( true ).
              state( OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed" ).
              privateStream(reqMsg.privateStream()).payload( map ).complete(true), event.handle() );
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
			Map map = EmaFactory.createMap();
			FieldList summaryData = EmaFactory.createFieldList();
			FieldList fieldList = EmaFactory.createFieldList();
			LinkedList<Long> removeHandles = new LinkedList<Long>();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
			while( appClient.itemHandles.size() == 0 ) Thread.sleep(1000);
				
			for( int i = 0; i < 60; i++ )
			{
				map.clear();
				summaryData.clear();
				summaryData.add(EmaFactory.createFieldEntry().real( 22, 3990 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				summaryData.add(EmaFactory.createFieldEntry().real( 25, 3994 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				summaryData.add(EmaFactory.createFieldEntry().real( 30, 9 + i, OmmReal.MagnitudeType.EXPONENT_0));
				summaryData.add(EmaFactory.createFieldEntry().real( 31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				map.summaryData(summaryData);
				
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real( 22, 3990 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real( 25, 3994 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real( 30, 9 + i, OmmReal.MagnitudeType.EXPONENT_0));
				fieldList.add(EmaFactory.createFieldEntry().real( 31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				map.add(EmaFactory.createMapEntry().keyAscii("Key", MapAction.ADD, fieldList));

				removeHandles.clear();
				
				for (HashMap.Entry<Long, Boolean> handle : appClient.itemHandles.entrySet())
				{
					try
					{
						provider.submit( EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_PRICE).
								payload( map), handle.getKey() );
					}
					catch (OmmException excp)
					{
						System.out.println(excp.getMessage());
						removeHandles.add(handle.getKey());
						continue;
					}
				}
				while (removeHandles.size() > 0)
				{
					appClient.itemHandles.remove(removeHandles.get(0));
					removeHandles.remove(0);
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
			if (provider != null) provider.uninitialize();
		}
	}
}
