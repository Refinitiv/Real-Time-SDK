/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series100.ex140_MBO_Streaming;

import java.io.BufferedReader;
import java.io.FileReader;
import java.nio.ByteBuffer;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Vector;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.EmaUtility;
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
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.OmmReal.MagnitudeType;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;
//  APIQA	
//	public String OrderNr="100";
	public String OrderNr="_CACHE_LIST.1093.8" ;
	
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
		
// APIQA 
		
	        LinkedList<String> itemNames = new LinkedList<String>();
	        BufferedReader inputStream = null;
	        String key;

	        try {
	        	inputStream =   new BufferedReader(new FileReader("xmlKeyList"));
	        	
	            while ((key = inputStream.readLine()) != null) {
	            	itemNames.add(key.trim());
	            }
	        } catch (Exception e) {
	        	System.out.println("File not found");
	        	System.out.println(e.getStackTrace());
	        	
	        } finally {
	            if (inputStream != null) 
	            	try {
	            		inputStream.close();
	            	} catch (Exception e1) {
	            		
	            	}
	        }
	        
		
// END APIQA 
		
		FieldList mapSummaryData = EmaFactory.createFieldList();
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(15,  840));
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(53,  1));
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(3423,  1));
		mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(1709,  2));
		
		FieldList entryData = EmaFactory.createFieldList();
    // APIQA 
	//	entryData.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, MagnitudeType.EXPONENT_NEG_2));
	//	entryData.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
	//	entryData.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
	//	entryData.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
		
		Map map = EmaFactory.createMap();
		map.summaryData(mapSummaryData);
		
    // APIQA 
	//	ByteBuffer buf = ByteBuffer.wrap(OrderNr.getBytes());
	//	map.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryData));
		int count = 0;
		Iterator<String> it = itemNames.iterator();
		while (it.hasNext()) {
			key = it.next();
			ByteBuffer buf = ByteBuffer.wrap(key.getBytes());
			map.add(EmaFactory.createMapEntry().keyBuffer(buf, MapEntry.MapAction.ADD, entryData));
			count++;
			if (count % 20 == 0) {
				event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).
						name(reqMsg.name()).serviceName(reqMsg.serviceName()).solicited(true).
						state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Not Completed").
						payload(map).complete(false),
						event.handle() );
				map.clear();
				map.summaryData(mapSummaryData);
				
			}
		}
		
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
        // APIQA 
				
		//		provider.submit( EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).payload( map ), appClient.itemHandle );
				
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
